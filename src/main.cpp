#include <Arduino.h>

int contador = 0;

void setup() {
    Serial.begin(115200);
    delay(1000); // Espera 1 segundo para estabilizar la conexión serie
    Serial.println("--- ESP32 INICIADO CON EXITO ---");
}

void loop() {
    Serial.print("ESP32 funcionando... Contador: ");
    Serial.println(contador++);
    delay(1000);
}
