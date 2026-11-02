#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 03C: Control de Paso Único e Inversión de Giro (Consigna 3)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.4. Práctica 2 con ESP32.pdf (Página 31)
 * Enlace Wokwi Oficial: https://wokwi.com/projects/442976722030999553
 *
 * Consigna 3 de la Cátedra:
 * "Implementar el control del motor paso a paso mediante 2 pulsadores:
 *  - Pulsador 1 (GPIO 26): Cada vez que se presiona, el motor debe dar
 *    exactamente 1 paso (1.8°).
 *  - Pulsador 2 (GPIO 27): Cada vez que se presiona, debe cambiar el sentido
 *    de giro actual (conmutar entre sentido horario y antihorario)."
 *
 * Fundamentos de la Máquina de Estados / Control Incremental:
 * -----------------------------------------------------------
 * 1. Control de Paso Individual (Jogging):
 *    Al enviar un único pulso en `STEP_PIN` (HIGH -> retardo -> LOW), el driver A4988
 *    conmuta la secuencia magnética de las bobinas en 1 posición discreta.
 *    En un motor NEMA 17 típico de 200 pasos, esto corresponde exactamente a:
 *      360° / 200 = 1.8° de desplazamiento angular.
 *
 * 2. Conmutación de Sentido (Toggle):
 *    Una variable booleana `sentidoHorario` almacena el estado actual. Al presionar
 *    el pulsador 2, se invierte (`sentidoHorario = !sentidoHorario`) y se aplica
 *    el nuevo nivel lógico al pin `DIR_PIN`.
 * ==============================================================================
 */

// --- 1. Definición de Pines ---
const int DIR_PIN  = 14;
const int STEP_PIN = 12;

const int BTN_PASO_PIN = 26; // Pulsador 1: Avanzar 1 paso (1.8°)
const int BTN_DIR_PIN  = 27; // Pulsador 2: Invertir sentido de giro

// --- 2. Variables de Estado ---
bool sentidoHorario = true; // true = Horario (HIGH), false = Antihorario (LOW)
long contadorPasos  = 0;

int estadoAnteriorPaso = HIGH;
int estadoAnteriorDir  = HIGH;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 03C: CONTROL DE 1 PASO Y SENTIDO DE DIR");
    Serial.println("==================================================");

    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);

    pinMode(BTN_PASO_PIN, INPUT_PULLUP);
    pinMode(BTN_DIR_PIN, INPUT_PULLUP);

    // Estado inicial de dirección
    digitalWrite(DIR_PIN, sentidoHorario ? HIGH : LOW);

    Serial.println("[SETUP] Pines configurados:");
    Serial.printf("  - DIR: GPIO %d | STEP: GPIO %d\n", DIR_PIN, STEP_PIN);
    Serial.printf("  - Pulsador 1 (Avanzar 1 Paso): GPIO %d\n", BTN_PASO_PIN);
    Serial.printf("  - Pulsador 2 (Cambiar Dirección): GPIO %d\n", BTN_DIR_PIN);
    Serial.println("[ESTADO INICIAL] Sentido: HORARIO (HIGH)");
    Serial.println("--------------------------------------------------");
}

void loop() {
    int estadoActualPaso = digitalRead(BTN_PASO_PIN);
    int estadoActualDir  = digitalRead(BTN_DIR_PIN);

    // --- 1. Detección de Pulsación para AVANZAR 1 PASO ---
    if (estadoAnteriorPaso == HIGH && estadoActualPaso == LOW) {
        // Generamos un pulso cuadrado limpio en el pin STEP
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(2000);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(2000);

        if (sentidoHorario) {
            contadorPasos++;
        } else {
            contadorPasos--;
        }

        float anguloAcumulado = contadorPasos * 1.8;
        Serial.printf("[PASO] Avanzado 1.8° | Sentido: %s | Paso #%ld (Ángulo neto: %.1f°)\n",
                      sentidoHorario ? "HORARIO" : "ANTIHORARIO", contadorPasos, anguloAcumulado);

        delay(50); // Antirrebote
    }

    // --- 2. Detección de Pulsación para INVERTIR SENTIDO ---
    if (estadoAnteriorDir == HIGH && estadoActualDir == LOW) {
        // Conmutamos la variable de sentido
        sentidoHorario = !sentidoHorario;

        // Actualizamos el nivel físico en el pin DIR
        digitalWrite(DIR_PIN, sentidoHorario ? HIGH : LOW);

        Serial.printf("[DIRECCIÓN CAMBIADA] Nuevo sentido: %s (%s)\n",
                      sentidoHorario ? "HORARIO" : "ANTIHORARIO",
                      sentidoHorario ? "HIGH" : "LOW");

        delay(50); // Antirrebote
    }

    estadoAnteriorPaso = estadoActualPaso;
    estadoAnteriorDir  = estadoActualDir;
}

