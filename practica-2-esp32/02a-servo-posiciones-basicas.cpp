#include <Arduino.h>
#include <ESP32Servo.h>

/*
 * ==============================================================================
 * EJERCICIO 02A: Posicionamiento Básico de Servomotor (0°, 90°, 180°)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.4. Práctica 2 con ESP32.pdf (Páginas 11 a 20)
 * Enlace Wokwi Oficial: https://wokwi.com/projects/442973167174987777
 *
 * Consigna:
 * "Conectar un servomotor estándar al ESP32 (pin de control en GPIO 33).
 *  Comandar el servo para que se desplace secuencialmente a las posiciones
 *  clave de su rango de operación:
 *  - 0° (mínimo)
 *  - 90° (centro / punto medio)
 *  - 180° (máximo)
 *  Permaneciendo 1 segundo en cada posición antes de cambiar a la siguiente."
 *
 * Fundamentos del Servomotor:
 * ----------------------------
 * 1. Señal de Control PWM a 50 Hz:
 *    A diferencia de un motor DC que gira continuamente con una tensión continua,
 *    el servomotor recibe un tren de pulsos con frecuencia fija de 50 Hz (T = 20 ms).
 *    El ancho del pulso en nivel alto (Duty Cycle en microsegundos) codifica el ángulo:
 *    - ~1000 µs (1.0 ms) -> 0°
 *    - ~1500 µs (1.5 ms) -> 90°
 *    - ~2000 µs - 2400 µs -> 180°
 *
 * 2. Biblioteca ESP32Servo:
 *    En el microcontrolador ESP32 no se usa la librería tradicional de Arduino
 *    (diseñada para los timers del ATmega328P de 8 bits). Se emplea la biblioteca
 *    <ESP32Servo.h>, la cual asigna canales del módulo LEDC / temporizadores de
 *    hardware de 16 bits del ESP32 para una señal limpia y precisa.
 *
 * 3. Alimentación en el mundo real:
 *    Nunca alimentar la línea roja de potencia del servo directamente desde el pin
 *    de 3.3V o 5V del ESP32 si este se alimenta por USB, porque las corrientes de
 *    pico del motor (500mA - 1A al arrancar) causan caídas de tensión (Brownouts)
 *    que reinician el microcontrolador. Usar fuente de 5V externa y unir masas (GND).
 * ==============================================================================
 */

// Instanciamos el objeto servo
Servo myservo;

// Definición de pines y parámetros de temporización
const int SERVO_PIN = 33; // Pin de señal PWM conectado al cable naranja/amarillo del servo

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 02A: PRUEBA DE POSICIONES DE SERVOMOTOR");
    Serial.println("==================================================");

    // Permite la asignación automática de timers por hardware en ESP32
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    // Frecuencia estándar para servomotores analógicos
    myservo.setPeriodHertz(50);

    // Vinculamos el servo al pin asignando el rango de pulsos típico (500 a 2400 µs)
    myservo.attach(SERVO_PIN, 500, 2400);

    Serial.printf("[SETUP] Servomotor vinculado al GPIO %d (50 Hz, 500us - 2400us)\n", SERVO_PIN);
}

void loop() {
    // 1. Posición 0 Grados
    Serial.println("[MOVIMIENTO] Posicionando en 0°...");
    myservo.write(0);
    delay(1000); // Espera mecánica para que el rotor alcance la posición

    // 2. Posición 90 Grados (Punto central)
    Serial.println("[MOVIMIENTO] Posicionando en 90° (Centro)...");
    myservo.write(90);
    delay(1000);

    // 3. Posición 180 Grados (Límite superior)
    Serial.println("[MOVIMIENTO] Posicionando en 180° (Máximo)...");
    myservo.write(180);
    delay(1000);
}

