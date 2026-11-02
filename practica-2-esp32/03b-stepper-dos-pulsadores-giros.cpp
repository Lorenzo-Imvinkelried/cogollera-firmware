#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 03B: Control de Stepper con 2 Pulsadores (Consigna 2)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.4. Práctica 2 con ESP32.pdf (Página 30)
 * Enlace Wokwi Oficial: https://wokwi.com/projects/442976722030999553
 *
 * Consigna 2 de la Cátedra:
 * "Implementar el control del motor paso a paso mediante 2 pulsadores:
 *  - Pulsador 1 (GPIO 26): Al presionarlo, el motor debe dar 1 vuelta completa
 *    (200 pasos) en sentido horario.
 *  - Pulsador 2 (GPIO 27): Al presionarlo, el motor debe dar 1 vuelta completa
 *    (200 pasos) en sentido antihorario."
 *
 * Consideraciones de Implementación Industrial / Embebidos:
 * --------------------------------------------------------
 * 1. Configuración INPUT_PULLUP:
 *    Se utilizan las resistencias internas de pull-up del ESP32. El pulsador se
 *    conecta entre el GPIO y GND:
 *    - En reposo (suelto)    : Nivel HIGH (3.3V).
 *    - Al presionar (activo) : Nivel LOW (0V - GND).
 *
 * 2. Detección de Flanco de Bajada (Falling Edge) y Antirrebote:
 *    Para evitar que mantener presionado el pulsador dispare giros indefinidos
 *    o que el ruido mecánico (rebotes) genere múltiples disparos, se detecta
 *    la transición de HIGH a LOW con un retardo antirrebote (debounce).
 * ==============================================================================
 */

// --- 1. Definición de Pines ---
const int DIR_PIN  = 14;
const int STEP_PIN = 12;

const int BTN_CW_PIN  = 26; // Pulsador 1: Giro Horario (Clockwise)
const int BTN_CCW_PIN = 27; // Pulsador 2: Giro Antihorario (Counter-Clockwise)

// --- 2. Parámetros Cinemáticos ---
const int PASOS_POR_VUELTA = 200; // 1 vuelta completa = 200 pasos
const int RETARDO_PASO_US  = 2000; // Microsegundos entre pulsos

// --- 3. Variables para Antirrebote y Detección de Estado ---
int estadoAnteriorCW  = HIGH;
int estadoAnteriorCCW = HIGH;

/**
 * Función auxiliar para hacer girar el motor N pasos en una dirección dada.
 * @param pasos Número de pasos a avanzar.
 * @param sentido HIGH para Horario, LOW para Antihorario.
 */
void girarMotor(int pasos, int sentido) {
    digitalWrite(DIR_PIN, sentido);
    delayMicroseconds(100); // Tiempo de establecimiento (Setup Time del A4988)

    for (int i = 0; i < pasos; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(RETARDO_PASO_US);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(RETARDO_PASO_US);
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 03B: STEPPER CON 2 PULSADORES (1 VUELTA)");
    Serial.println("==================================================");

    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);

    // Pines de pulsadores con resistencia interna PULL-UP
    pinMode(BTN_CW_PIN, INPUT_PULLUP);
    pinMode(BTN_CCW_PIN, INPUT_PULLUP);

    Serial.println("[SETUP] Pines configurados:");
    Serial.printf("  - DIR: GPIO %d | STEP: GPIO %d\n", DIR_PIN, STEP_PIN);
    Serial.printf("  - Pulsador 1 (Horario): GPIO %d\n", BTN_CW_PIN);
    Serial.printf("  - Pulsador 2 (Antihorario): GPIO %d\n", BTN_CCW_PIN);
    Serial.println("Presione un pulsador para ejecutar 1 vuelta...");
    Serial.println("--------------------------------------------------");
}

void loop() {
    int estadoActualCW  = digitalRead(BTN_CW_PIN);
    int estadoActualCCW = digitalRead(BTN_CCW_PIN);

    // --- Detección de Flanco de Bajada en Pulsador 1 (Horario) ---
    if (estadoAnteriorCW == HIGH && estadoActualCW == LOW) {
        Serial.println("[COMANDO] Pulsador 1 presionado -> Ejecutando 1 vuelta HORARIA (200 pasos)...");
        girarMotor(PASOS_POR_VUELTA, HIGH);
        Serial.println("[LISTO] Giro horario completado.");
        delay(50); // Antirrebote mecánico
    }

    // --- Detección de Flanco de Bajada en Pulsador 2 (Antihorario) ---
    if (estadoAnteriorCCW == HIGH && estadoActualCCW == LOW) {
        Serial.println("[COMANDO] Pulsador 2 presionado -> Ejecutando 1 vuelta ANTIHORARIA (200 pasos)...");
        girarMotor(PASOS_POR_VUELTA, LOW);
        Serial.println("[LISTO] Giro antihorario completado.");
        delay(50); // Antirrebote mecánico
    }

    estadoAnteriorCW  = estadoActualCW;
    estadoAnteriorCCW = estadoActualCCW;
}

