#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 02-A: Pulsador con Resistencia PULL-UP EXTERNA
 * ==============================================================================
 * Cátedra: Sistemas Embebidos / IoT con ESP32
 * Fuente: video.pdf (Páginas 4 y 5)
 *
 * Consigna:
 * "Detectar cuando se presiona un pulsador e indicar por consola cuando esto ocurra:
 *  a) usando resistencia pull up externa."
 *
 * Teoría de Pull-Up Externa:
 * - Se conecta una resistencia de 10 kΩ entre el pin digital y 3.3V (VCC).
 * - El pulsador se conecta entre el pin digital y GND.
 * - Comportamiento:
 *     * En REPOSO (sin pulsar): La resistencia "tira hacia arriba" la entrada a 3.3V -> Lee HIGH (1).
 *     * Al PULSAR: El pulsador deriva la corriente directo a tierra -> Lee LOW (0).
 * - ¡LÓGICA INVERSA!: Presionado = LOW (0).
 *
 * Filtro antirrebote (Debounce):
 * - Se introduce un retardo de 200 ms como se explica en la clase para evitar
 *   que las vibraciones de las láminas metálicas provoquen lecturas repetidas falsas.
 * ==============================================================================
 */

// 1. Definición de pines
const int PIN_PULSADOR = 32; // Pin de entrada digital (GPIO 32)
const int TIEMPO_ANTIRREBOTE_MS = 200; // Retardo según explicación de clase

// Variable para recordar el estado anterior y detectar el flanco de bajada (cuando pasa de reposo a presionado)
int ultimoEstadoPulsador = HIGH;
unsigned long contadorPulsaciones = 0;

void setup() {
    Serial.begin(115200);
    delay(500);

    // Como la resistencia pull-up está conectada físicamente afuera en la protoboard,
    // configuramos el pin simplemente como INPUT estándar.
    pinMode(PIN_PULSADOR, INPUT);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 02-A: PULSADOR PULL-UP EXTERNA (10k)  ");
    Serial.println("==================================================");
    Serial.println("-> En reposo: Entrada en HIGH (3.3V)");
    Serial.println("-> Al presionar: Entrada en LOW (0V - Tierra)");
    Serial.println("-> Presiona el pulsador para probar...");
    Serial.println("--------------------------------------------------");
}

void loop() {
    // Leemos el pin de entrada digital
    int estadoActual = digitalRead(PIN_PULSADOR);

    // Detectamos el flanco: Estaba en HIGH (reposo) y pasó a LOW (presionado)
    if (estadoActual == LOW && ultimoEstadoPulsador == HIGH) {
        contadorPulsaciones++;
        Serial.printf("[EVENTO #%lu] ¡Pulsador PRESIONADO! (Pull-Up Externa = LOW)\n", contadorPulsaciones);

        // Retardo para eliminar el efecto rebote del resorte mecánico
        delay(TIEMPO_ANTIRREBOTE_MS);
    }

    ultimoEstadoPulsador = estadoActual;
}

