#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_LDR_PATH "/sys/kernel/smartlamp/ldr"
#define DEFAULT_INTERVAL_MS 1000
#define MIN_PERCENT 10

static volatile sig_atomic_t running = 1;

// Função para capturar sinais de interrupção (como o Ctrl+C)
static void handle_signal(int signal)
{
    (void)signal;
    running = 0;
}

// Função utilitária que limita o valor entre um mínimo e um máximo
static int __attribute__((unused)) clamp(int value, int min, int max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

// Função para ler um número inteiro de um ficheiro
static int read_int_file(const char *path, int *value)
{
    // PASSO 1: Abre o ficheiro no modo de leitura ("r")
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return -ENOENT; // Retorna erro se não encontrar o ficheiro
    }

    // PASSO 2: Lê um número inteiro ("%d") e guarda no endereço de 'value'
    if (fscanf(file, "%d", value) != 1) {
        fclose(file);
        return -EIO; // Retorna erro de Entrada/Saída se falhar a leitura
    }

    // PASSO 3: Fecha o ficheiro
    fclose(file);
    return 0; // Retorna 0 indicando sucesso
}

// Função que converte o valor do LDR para o percentual de brilho
static int ldr_to_percent(int ldr)
{
    // PASSO 1: Limita o valor lido entre 0 e 100
    int percent = clamp(ldr, 0, 100);

    // PASSO 2: Aplica a regra do brilho mínimo para evitar ecrã apagado
    if (percent < MIN_PERCENT) {
        percent = MIN_PERCENT;
    }

    return percent;
}

// Função para pausar a execução em milissegundos
static void sleep_ms(int milliseconds)
{
    struct timespec request;

    request.tv_sec = milliseconds / 1000;
    request.tv_nsec = (long)(milliseconds % 1000) * 1000000L;

    while (running && nanosleep(&request, &request) == -1 && errno == EINTR) {
    }
}

// Função Principal
int main(void)
{
    // Prepara o programa para ser fechado de forma segura com Ctrl+C
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Loop infinito do daemon (corre enquanto running for 1)
    while (running) {
        int ldr;
        int percent;

        // Tenta ler o ficheiro usando a nossa função criada
        if (read_int_file(DEFAULT_LDR_PATH, &ldr) == 0) {
            // Se ler com sucesso, calcula o brilho e imprime
            percent = ldr_to_percent(ldr);
            printf("ldr=%d brightness_percent=%d\n", ldr, percent);
            fflush(stdout); // Força a impressão imediata no terminal
        } else {
            fprintf(stderr, "failed to read %s\n", DEFAULT_LDR_PATH);
        }

        // Aguarda o intervalo padrão (1000ms = 1 segundo) antes de ler novamente
        sleep_ms(DEFAULT_INTERVAL_MS);
    }

    return EXIT_SUCCESS;
}