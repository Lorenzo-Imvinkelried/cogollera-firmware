#include <Arduino.h>

/*
 * EJEMPLO 01: Parpadeo del LED integrado (Blink)
 * -----------------------------------------------
 * Placa: ESP32 DevKit V1 (30 o 38 pines)
 * Pin del LED azul on-board: GPIO 2
 *
 * Funcionamiento:
 * Enciende el LED azul durante 1 segundo, lo apaga 1 segundo
 * y emite el estado actual por el puerto serie (115200 baudios).
 */

const int LED_AZUL = 2;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Configuramos el pin 2 como salida digital
    pinMode(LED_AZUL, OUTPUT);

    Serial.println("--- PARPADEO DE LED INTEGRADO INICIADO ---");
}

void loop() {
    // Encendemos el LED azul
    digitalWrite(LED_AZUL, HIGH);
    Serial.println("-> LED Azul: [ENCENDIDO]");
    delay(1000);

    // Apagamos el LED azul
    digitalWrite(LED_AZUL, LOW);
    Serial.println("-> LED Azul: [APAGADO]");
    delay(1000);
}
