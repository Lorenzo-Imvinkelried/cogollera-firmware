#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 04: Salidas PWM - Incremento y Decremento Suave (Fade de LED)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.3. Práctica 1 con ESP32.pdf (Páginas 18 a 20)
 * Enlace Wokwi: https://wokwi.com/projects/410109256783209473
 *
 * Consigna:
 * "Incrementar y decrementar suavemente la intensidad lumínica de un diodo LED."
 *
 * Teoría de PWM en ESP32:
 * - El ESP32 cuenta con el periférico LEDC (LED Control) de 16 canales independientes.
 * - Frecuencia: 5000 Hz (ideal para iluminación humana sin parpadeo perceptible).
 * - Resolución: 8 bits (2^8 = 256 niveles de duty cycle: de 0 = 0% a 255 = 100%).
 *
 * Conexión del circuito (Wokwi / Protoboard):
 * - Ánodo (+) del LED: Conectado a GPIO 32.
 * - Cátodo (-) del LED: Conectado a GND a través de una resistencia de 220Ω.
 * ==============================================================================
 */

// 1. Pines y parámetros de configuración PWM
const int pinLed    = 32;   // GPIO donde está conectado el LED
const int frecuencia = 5000; // Frecuencia en Hz (5 kHz estándar)
const int resolucion = 8;    // Resolución en bits (0 a 255)

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 04: VARIACION DE BRILLO CON PWM (FADE) ");
    Serial.println("==================================================");

    // Inicializamos el pin con PWM usando la API oficial simplificada del ESP32
    // En las versiones modernas de arduino-esp32 (v3.x / Wokwi):
    #if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
        ledcAttach(pinLed, frecuencia, resolucion);
    #else
        // Compatibilidad con arduino-esp32 v2.x (usado en algunas versiones de PlatformIO)
        ledcSetup(0, frecuencia, resolucion);
        ledcAttachPin(pinLed, 0);
    #endif

    Serial.println("-> Modulando ancho de pulso de 0% a 100% de forma suave.");
}

void loop() {
    Serial.println("\n[PWM] -> Aumentando brillo (0 a 255)...");

    // 1. Incremento de intensidad lumínica del LED
    for (int dutyCycle = 0; dutyCycle <= 255; ++dutyCycle) {
        #if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
            ledcWrite(pinLed, dutyCycle);
        #else
            ledcWrite(0, dutyCycle);
        #endif
        delay(15); // Transición suave (255 * 15ms ≈ 3.8 segundos)
    }

    Serial.println("[PWM] Brillo MAXIMO alcanzado. Esperando 2 segundos...");
    delay(2000);

    Serial.println("[PWM] -> Disminuyendo brillo (255 a 0)...");

    // 2. Decremento de intensidad lumínica del LED
    for (int dutyCycle = 255; dutyCycle >= 0; --dutyCycle) {
        #if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
            ledcWrite(pinLed, dutyCycle);
        #else
            ledcWrite(0, dutyCycle);
        #endif
        delay(15);
    }

    Serial.println("[PWM] Brillo MINIMO (Apagado). Esperando 2 segundos...");
    delay(2000);
}

