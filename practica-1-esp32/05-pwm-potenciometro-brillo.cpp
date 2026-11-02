#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 05: Control de Brillo de LED con Potenciómetro (ADC + PWM)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.3. Práctica 1 con ESP32.pdf (Páginas 21 a 23)
 * Enlace Wokwi: https://wokwi.com/projects/442800733118882817
 *
 * Consigna:
 * "Modificar la intensidad lumínica de un diodo LED mediante un potenciómetro."
 *
 * Integración Analógica-Digital:
 * - El potenciómetro entrega un valor analógico de 0 a 4095 (12 bits) en el pin GPIO 35.
 * - El PWM opera con una resolución de 8 bits (valores de 0 a 255) en el pin GPIO 32.
 * - Usamos la función map(potValor, 0, 4095, 0, 255) para adaptar la escala del ADC
 *   directamente al ciclo de trabajo (duty cycle) del LED.
 * ==============================================================================
 */

// 1. Asignación de pines
const int ledPin = 32; // Salida PWM hacia el LED
const int potPin = 35; // Entrada analógica ADC1 conectada al potenciómetro

// 2. Parámetros de PWM
const int frecuencia = 5000; // 5 kHz
const int resolucion = 8;    // 8 bits (0 a 255)

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 05: CONTROL DE BRILLO CON POTENCIOMETRO");
    Serial.println("==================================================");

    // Inicializamos el pin PWM
    #if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
        ledcAttach(ledPin, frecuencia, resolucion);
    #else
        ledcSetup(0, frecuencia, resolucion);
        ledcAttachPin(ledPin, 0);
    #endif

    Serial.println("-> Gira el potenciometro para variar la intensidad del LED.");
    Serial.println("--------------------------------------------------");
}

void loop() {
    // 1. Leemos el valor del potenciómetro (0 a 4095)
    int potValor = analogRead(potPin);

    // 2. Mapeamos de la escala del ADC (12 bits) a la del PWM (8 bits)
    // Nota: Si configuráramos el PWM con resolución de 12 bits, no haría falta mapear.
    int dutyCycle = map(potValor, 0, 4095, 0, 255);

    // 3. Escribimos el Duty Cycle en el pin del LED
    #if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
        ledcWrite(ledPin, dutyCycle);
    #else
        ledcWrite(0, dutyCycle);
    #endif

    // 4. Imprimimos el valor actual en consola
    float porcentajeBrillo = (dutyCycle * 100.0f) / 255.0f;
    Serial.printf("Potenciometro: %4d / 4095 -> Duty cycle: %3d / 255 (Brillo: %.1f%%)\n",
                  potValor, dutyCycle, porcentajeBrillo);

    delay(200); // Actualización fluida y suave
}

