#include <Arduino.h>
#include <ESP32Servo.h>

/*
 * ==============================================================================
 * EJERCICIO 02B: Barrido Suave de Servomotor (Consigna 2)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.4. Práctica 2 con ESP32.pdf (Página 19)
 * Enlace Wokwi Oficial: https://wokwi.com/projects/442973167174987777
 *
 * Consigna 2 de la Cátedra:
 * "Realizar un movimiento de 0 a 180 grados pero que sea fluido, es decir
 *  que avance de a 1 grado cada 20 ms tanto para ir de 0 a 180 como para
 *  volver de 180 a 0."
 *
 * Fundamento Físico y Mecánico:
 * -----------------------------
 * Si enviamos instantáneamente la orden `myservo.write(180)` estando el servo
 * en 0°, el motor intentará acelerar a máxima velocidad física. Esto provoca:
 * 1. Picos de corriente abruptos (estrés eléctrico y riesgo de reset del ESP32).
 * 2. Inercia y sobrepaso mecánico (vibraciones y desgaste en los engranajes plásticos).
 * 3. Movimiento brusco inadecuado para brazos robóticos, cámaras pan-tilt o válvulas.
 *
 * Al incrementar el ángulo gradualmente (1° cada 20 ms, sincronizado con la frecuencia
 * de refresco PWM de 50 Hz del servo donde cada período dura exactamente 20 ms),
 * logramos una trayectoria suave y controlada sin sobrecorrientes.
 * ==============================================================================
 */

Servo myservo;

// Definición de pines y parámetros
const int SERVO_PIN = 33;
const int RETARDO_PASO_MS = 20; // 20 milisegundos entre cada grado

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 02B: BARRIDO SUAVE DE 0° A 180° Y VUELTA");
    Serial.println("==================================================");

    // Asignación de temporizadores hardware
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    myservo.setPeriodHertz(50);
    myservo.attach(SERVO_PIN, 500, 2400);

    // Posición inicial de reposo
    myservo.write(0);
    delay(1000);
    Serial.println("[SETUP] Servomotor calibrado en 0°. Iniciando barrido continuo...");
}

void loop() {
    // --- 1. Ida: Barrido fluido de 0° a 180° ---
    Serial.println("[BARRIDO] Avanzando de 0° a 180°...");
    for (int angulo = 0; angulo <= 180; angulo++) {
        myservo.write(angulo);
        delay(RETARDO_PASO_MS); // 20 ms por grado -> 180 * 20 ms = 3.6 segundos totales
    }

    delay(500); // Pequeña pausa en el extremo superior para amortiguar inercia

    // --- 2. Vuelta: Barrido fluido de 180° a 0° ---
    Serial.println("[BARRIDO] Regresando de 180° a 0°...");
    for (int angulo = 180; angulo >= 0; angulo--) {
        myservo.write(angulo);
        delay(RETARDO_PASO_MS); // 20 ms por grado
    }

    delay(500); // Pequeña pausa en el extremo inferior
}

