#include <Arduino.h>
#include <ESP32Servo.h>

/*
 * ==============================================================================
 * EJERCICIO 02C: Control Angular Proporcional con Potenciómetro (Consigna 3)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.4. Práctica 2 con ESP32.pdf (Página 20)
 * Enlace Wokwi Oficial: https://wokwi.com/projects/442973167174987777
 *
 * Consigna 3 de la Cátedra:
 * "Comandar la posición angular del servomotor usando un potenciómetro.
 *  - Cuando el potenciómetro esté al mínimo (0V / 0), el servo debe estar en 0°.
 *  - Cuando el potenciómetro esté al máximo (3.3V / 4095), el servo debe estar en 180°."
 *
 * Fundamentos de la Conversión y Mapeo:
 * -------------------------------------
 * 1. ADC del ESP32:
 *    El convertidor analógico a digital del ESP32 tiene una resolución por defecto
 *    de 12 bits, lo que produce un rango de lecturas discretas de:
 *      0 a (2^12 - 1) = 0 a 4095.
 *      0 V    -> 0
 *      3.3 V  -> 4095
 *
 * 2. Función map():
 *    Realiza una transformación lineal entre dos intervalos numéricos:
 *      angulo = map(valorADC, 0, 4095, 0, 180);
 *    Formula subyacente:
 *      angulo = (valorADC - 0) * (180 - 0) / (4095 - 0)
 *
 * 3. Filtrado y Estabilidad (Anti-Jitter):
 *    Los convertidores ADC sufren de ruido eléctrico de alta frecuencia. Si enviamos
 *    la señal cruda directamente al servo, el motor vibrará continuamente (jitter)
 *    consumiendo energía y dañando los engranajes. En este código se implementa una
 *    pequeña zona muerta (deadband) o filtro pasa-bajos que sólo actualiza la posición
 *    si el cambio angular es significativo (>= 1°).
 * ==============================================================================
 */

Servo myservo;

// Definición de pines
const int SERVO_PIN = 33; // Pin de control PWM del servo
const int POT_PIN   = 35; // Pin de entrada analógica ADC1_CH7 (GPIO 35)

// Variables de estado
int ultimoAngulo = -1;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 02C: CONTROL PROPORCIONAL CON POTENCIÓMETRO");
    Serial.println("==================================================");

    // Asignación de temporizadores hardware
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    myservo.setPeriodHertz(50);
    myservo.attach(SERVO_PIN, 500, 2400);

    // Pin del potenciómetro en modo entrada
    pinMode(POT_PIN, INPUT);

    Serial.printf("[SETUP] Servo en GPIO %d | Potenciómetro en GPIO %d (ADC)\n", SERVO_PIN, POT_PIN);
    Serial.println("Gire el potenciómetro para variar el ángulo del servomotor.");
    Serial.println("--------------------------------------------------");
}

void loop() {
    // 1. Leemos el valor crudo del ADC (0 a 4095)
    int valorADC = analogRead(POT_PIN);

    // 2. Mapeamos la lectura analógica al rango de 0 a 180 grados
    int nuevoAngulo = map(valorADC, 0, 4095, 0, 180);

    // 3. Calculamos la tensión real equivalente en Volts (para visualización)
    float tensionVolts = (valorADC / 4095.0) * 3.3;

    // 4. Filtro anti-vibración (actualiza el servo sólo si cambia al menos 1 grado)
    if (abs(nuevoAngulo - ultimoAngulo) >= 1) {
        myservo.write(nuevoAngulo);
        ultimoAngulo = nuevoAngulo;

        Serial.printf("[CONTROL] ADC: %4d | Tensión: %.2f V | Ángulo: %3d°\n",
                      valorADC, tensionVolts, nuevoAngulo);
    }

    // Pequeño retardo para no saturar el canal serie ni sobrecargar el ADC
    delay(25);
}

