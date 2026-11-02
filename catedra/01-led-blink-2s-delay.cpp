#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 01: Encendido y Apagado de LED (Blink clásico con delay)
 * ==============================================================================
 * Cátedra: Sistemas Embebidos / IoT con ESP32
 * Fuente: video.pdf (Página 4)
 *
 * Consigna:
 * "Encender y apagar un LED por 2 segundos por cada estado."
 *
 * Conexión del circuito (Wokwi / Protoboard):
 * - GPIO 23: Conectado a Resistencia 220Ω -> Ánodo (+) del LED
 * - Cátodo (-) del LED: Conectado a GND
 * (O bien usar el LED azul integrado en GPIO 2)
 *
 * Wokwi:
 * Puedes probarlo en https://wokwi.com/projects/new/esp32
 * ==============================================================================
 */

// 1. Definición de pines y constantes globales
const int PIN_LED = 23;            // Pin del LED (o GPIO 2 para el integrado)
const int TIEMPO_ESTADO_MS = 2000; // 2 segundos en milisegundos

void setup() {
    // Inicializamos comunicación serie para monitoreo en consola a 115200 baudios
    Serial.begin(115200);
    delay(500);

    // Configuramos el pin como salida digital (OUTPUT)
    pinMode(PIN_LED, OUTPUT);

    Serial.println("==================================================");
    Serial.println("  EJERCICIO 01: BLINK 2 SEGUNDOS CON DELAY       ");
    Serial.println("==================================================");
}

void loop() {
    // 1. Encendemos el LED (HIGH = 3.3V)
    digitalWrite(PIN_LED, HIGH);
    Serial.println("LED -> [ENCENDIDO] (Esperando 2 segundos)");
    delay(TIEMPO_ESTADO_MS); // Bloquea la ejecución durante 2000 ms

    // 2. Apagamos el LED (LOW = 0V)
    digitalWrite(PIN_LED, LOW);
    Serial.println("LED -> [APAGADO]   (Esperando 2 segundos)");
    delay(TIEMPO_ESTADO_MS); // Bloquea la ejecución durante 2000 ms
}

