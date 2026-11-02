#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 02-C: Pulsador con PULL-DOWN (Externa e Interna)
 * ==============================================================================
 * Cátedra: Sistemas Embebidos / IoT con ESP32
 * Fuente: video.pdf (Páginas 4 y 5)
 *
 * Teoría de Pull-Down:
 * - A diferencia del pull-up, aquí la lógica es DIRECTA:
 *     * En REPOSO: La entrada se mantiene en LOW (0V - Tierra).
 *     * Al PULSAR: Se conecta a 3.3V y la entrada pasa a HIGH (1).
 *
 * Dos enfoques posibles:
 * 1. Pull-Down Externa:
 *    - Resistencia de 10 kΩ entre el GPIO y GND.
 *    - Pulsador entre el GPIO y 3.3V.
 *    - Modo en código: pinMode(PIN_PULSADOR, INPUT);
 *
 * 2. Pull-Down Interna (soporte nativo en ESP32):
 *    - No requiere resistencia física.
 *    - Pulsador entre el GPIO y 3.3V.
 *    - Modo en código: pinMode(PIN_PULSADOR, INPUT_PULLDOWN);
 * ==============================================================================
 */

// 1. Definición de pines
const int PIN_PULSADOR = 32; // Pin de entrada digital (GPIO 32)
const int TIEMPO_ANTIRREBOTE_MS = 200;

int ultimoEstadoPulsador = LOW;
unsigned long contadorPulsaciones = 0;

void setup() {
    Serial.begin(115200);
    delay(500);

    // Activamos la resistencia PULL-DOWN interna del chip ESP32
    // (Si usas resistencia de 10k externa a GND, puedes poner simplemente INPUT)
    pinMode(PIN_PULSADOR, INPUT_PULLDOWN);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 02-C: PULSADOR PULL-DOWN (LOGICA DIRECTA)");
    Serial.println("==================================================");
    Serial.println("-> En reposo: LOW (0V)");
    Serial.println("-> Al presionar: HIGH (3.3V)");
    Serial.println("-> Presiona el pulsador para probar...");
    Serial.println("--------------------------------------------------");
}

void loop() {
    int estadoActual = digitalRead(PIN_PULSADOR);

    // Detección de flanco de subida (LOW -> HIGH)
    if (estadoActual == HIGH && ultimoEstadoPulsador == LOW) {
        contadorPulsaciones++;
        Serial.printf("[EVENTO #%lu] ¡Pulsador PRESIONADO! (Pull-Down = HIGH)\n", contadorPulsaciones);

        // Retardo para absorber el rebote mecánico
        delay(TIEMPO_ANTIRREBOTE_MS);
    }

    ultimoEstadoPulsador = estadoActual;
}

