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
    // TASK 3.2: abra path para leitura e leia um numero inteiro.
    // Retorne 0 em caso de sucesso ou um codigo negativo em caso de erro.
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return -EIO;
    }
    
    if (fscanf(file, "%d", value) != 1) {
        fclose(file);
        return -EIO;
    }
    
    fclose(file);
    return 0;
}

// Função que converte o valor do LDR para o percentual de brilho
static int ldr_to_percent(int ldr)
{
    // TASK 3.2: limite o LDR para 0-100 e aplique um brilho minimo.
    //(void)ldr;

    if (MIN_PERCENT <= ldr && ldr <= 100) {
        return ldr;
    } else if (ldr <= MIN_PERCENT) {
        return MIN_PERCENT;
    }
    return MIN_PERCENT;
}

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