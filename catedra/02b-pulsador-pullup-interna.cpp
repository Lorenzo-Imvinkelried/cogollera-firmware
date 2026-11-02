#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 02-B: Pulsador con Resistencia PULL-UP INTERNA (INPUT_PULLUP)
 * ==============================================================================
 * Cátedra: Sistemas Embebidos / IoT con ESP32
 * Fuente: video.pdf (Página 5)
 *
 * Consigna:
 * "Detectar cuando se presiona un pulsador e indicar por consola cuando esto ocurra:
 *  b) usando resistencia pull up interna."
 *
 * Teoría de Pull-Up Interna:
 * - El procesador del ESP32 cuenta con resistencias internas conectadas a 3.3V
 *   dentro del propio silicio del chip (~45 kΩ).
 * - ¡NO REQUIERE NINGUNA RESISTENCIA EXTERNA EN LA PROTOBOARD!
 * - Conexión física súper simple:
 *     * Un pin del pulsador va al GPIO 32.
 *     * El otro pin del pulsador va directo a GND.
 * - Modo en código: pinMode(PIN_PULSADOR, INPUT_PULLUP);
 * - Comportamiento (Lógica Inversa):
 *     * En REPOSO: HIGH (3.3V)
 *     * Al PULSAR: LOW (0V)
 * ==============================================================================
 */

// 1. Definición de pines
const int PIN_PULSADOR = 32; // Pin de entrada digital (GPIO 32)
const int TIEMPO_ANTIRREBOTE_MS = 200;

int ultimoEstadoPulsador = HIGH;
unsigned long contadorPulsaciones = 0;

void setup() {
    Serial.begin(115200);
    delay(500);

    // Activamos la resistencia PULL-UP interna del microcontrolador
    pinMode(PIN_PULSADOR, INPUT_PULLUP);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 02-B: PULSADOR PULL-UP INTERNA         ");
    Serial.println("==================================================");
    Serial.println("-> Sin resistencias externas: Pulsador entre GPIO 32 y GND");
    Serial.println("-> En reposo: HIGH (3.3V)");
    Serial.println("-> Al presionar: LOW (0V)");
    Serial.println("-> Presiona el pulsador para probar...");
    Serial.println("--------------------------------------------------");
}

void loop() {
    int estadoActual = digitalRead(PIN_PULSADOR);

    // Detección de flanco de bajada (HIGH -> LOW)
    if (estadoActual == LOW && ultimoEstadoPulsador == HIGH) {
        contadorPulsaciones++;
        Serial.printf("[EVENTO #%lu] ¡Pulsador PRESIONADO! (Pull-Up Interna = LOW)\n", contadorPulsaciones);

        // Retardo de 200ms para evitar falsos rebotes mecánicos
        delay(TIEMPO_ANTIRREBOTE_MS);
    }

    ultimoEstadoPulsador = estadoActual;
}

