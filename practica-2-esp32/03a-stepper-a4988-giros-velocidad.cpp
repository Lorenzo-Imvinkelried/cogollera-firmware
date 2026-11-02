#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 03A: Motor Paso a Paso con Driver A4988 (Giros y Velocidad)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.4. Práctica 2 con ESP32.pdf (Páginas 21 a 31)
 * Enlace Wokwi Oficial: https://wokwi.com/projects/442976722030999553
 *
 * Consigna Base de la Cátedra:
 * "Conectar un motor paso a paso bipolar (NEMA 17) controlado mediante un driver
 *  A4988 a los pines del ESP32:
 *  - DIR  -> GPIO 14 (Control de sentido de giro)
 *  - STEP -> GPIO 12 (Generación de tren de pulsos de paso)
 *  Comandar el motor para que:
 *  1. Gire 1 vuelta completa en sentido horario a velocidad estándar
 *     (retardo de 3000 µs entre pulsos).
 *  2. Realice una pausa de 1 segundo (1000 ms).
 *  3. Gire 2 vueltas completas en sentido antihorario al DOBLE de velocidad
 *     (retardo de 1500 µs entre pulsos).
 *  4. Realice una pausa de 1 segundo y repita indefinidamente."
 *
 * Principio del Driver A4988 y Motor Paso a Paso:
 * -----------------------------------------------
 * 1. Geometría NEMA 17:
 *    - Ángulo por paso (Full Step): 1.8°
 *    - Pasos por revolución completa: 360° / 1.8° = 200 pasos.
 *
 * 2. Control Simplificado por Driver A4988:
 *    El driver se encarga de conmutar secuencialmente la corriente en las 4 bobinas
 *    del motor (puentes H internos). El ESP32 sólo necesita gobernar dos líneas:
 *    - DIR: Nivel lógico que define la polaridad y secuencia de avance.
 *           HIGH = Sentido Horario (CW).
 *           LOW  = Sentido Antihorario (CCW).
 *    - STEP: Cada transición LOW -> HIGH conmuta el rotor exactamente 1 paso (1.8°).
 *
 * 3. Control de Velocidad:
 *    La velocidad angular es directamente proporcional a la frecuencia de pulsos
 *    f = 1 / T. Al reducir el tiempo entre pulsos a la mitad (de 3000 µs a 1500 µs),
 *    la frecuencia se duplica y el motor gira al doble de velocidad (RPM).
 * ==============================================================================
 */

// --- 1. Definición de Pines ---
const int DIR_PIN  = 14; // Pin de dirección conectado a DIR del A4988
const int STEP_PIN = 12; // Pin de pulsos conectado a STEP del A4988

// --- 2. Parámetros Cinemáticos ---
const int PASOS_POR_VUELTA = 200; // 360° / 1.8° = 200 pasos

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 03A: STEPPER A4988 (GIROS Y VELOCIDAD) ");
    Serial.println("==================================================");

    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);

    Serial.printf("[SETUP] Driver A4988 -> DIR: GPIO %d | STEP: GPIO %d\n", DIR_PIN, STEP_PIN);
    Serial.println("--------------------------------------------------");
}

void loop() {
    // ============================================================
    // FASE 1: 1 Vuelta en sentido Horario (CW) - Velocidad Estándar
    // ============================================================
    Serial.println("[MOVIMIENTO] 1 Vuelta Horaria (200 pasos, delay = 3000 µs)...");
    digitalWrite(DIR_PIN, HIGH); // Sentido horario

    for (int i = 0; i < PASOS_POR_VUELTA; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(3000);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(3000);
    }

    Serial.println("[PAUSA] Esperando 1000 ms...");
    delay(1000);

    // ============================================================
    // FASE 2: 2 Vueltas en sentido Antihorario (CCW) - Doble Velocidad
    // ============================================================
    Serial.println("[MOVIMIENTO] 2 Vueltas Antihorarias (400 pasos, delay = 1500 µs - Doble Velocidad)...");
    digitalWrite(DIR_PIN, LOW); // Sentido antihorario

    for (int i = 0; i < (2 * PASOS_POR_VUELTA); i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(1500); // La mitad del tiempo -> Doble frecuencia de paso
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(1500);
    }

    Serial.println("[PAUSA] Esperando 1000 ms...");
    delay(1000);
}

