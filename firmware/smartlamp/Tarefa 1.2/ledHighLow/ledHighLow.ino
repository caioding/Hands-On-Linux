#define LED_PIN 2

int i = 0;

void setup(){
    pinMode(LED_PIN, OUTPUT);
}

void loop(){
    if(i < 5){
        i++;
        digitalWrite(LED_PIN, LOW);
        delay(2000);
        digitalWrite(LED_PIN, HIGH);
        delay(2000);

    }
    digitalWrite(LED_PIN, LOW);

}
