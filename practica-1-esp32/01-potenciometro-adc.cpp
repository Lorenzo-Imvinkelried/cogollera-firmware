#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 01: Lectura de Entradas Analógicas con Potenciómetro
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.3. Práctica 1 con ESP32.pdf (Páginas 4 a 6)
 * Enlace Wokwi: https://wokwi.com/projects/410104863739757569
 *
 * Consigna:
 * "Leer e indicar por el monitor serial el valor de un potenciómetro (que indica
 *  su posición) usando una entrada analógica."
 *
 * Teoría del circuito:
 * - Un potenciómetro es una resistencia variable de tres terminales que actúa
 *   como divisor de tensión regulable.
 * - Terminal 1: Conectado a GND (0V).
 * - Terminal 3: Conectado a VCC (3.3V del ESP32).
 * - Terminal central (Cursor / Señal): Entrega un voltaje analógico continuo entre 0V y 3.3V
 *   según la rotación del eje.
 *
 * ADC del ESP32:
 * - Resolución: 12 bits (rango: 0 a 4095).
 * - Nota del profesor: En entradas analógicas NO es obligatorio usar pinMode(),
 *   ya que analogRead() configura automáticamente el pin como entrada analógica.
 * ==============================================================================
 */

// 1. Definición del pin analógico
// (Se usa GPIO 32, perteneciente al conversor ADC1 del ESP32, inmune al encendido del Wi-Fi)
const int pinPotenciometro = 32;

void setup() {
    // Inicializamos la consola serie a 115200 baudios
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println("  EJERCICIO 01: LECTURA DE POTENCIOMETRO (ADC)    ");
    Serial.println("==================================================");
    Serial.println("-> Rango nativo: 0 (0V) a 4095 (3.3V)");
    Serial.println("--------------------------------------------------");
}

void loop() {
    // 1. Lectura del valor analógico del ADC (0 a 4095)
    int valor_pot = analogRead(pinPotenciometro);

    // 2. Cálculo del porcentaje (0 a 100%) usando map()
    int porcentaje = map(valor_pot, 0, 4095, 0, 100);

    // 3. Cálculo del voltaje real estimado en Volts (0.00V a 3.30V)
    float voltaje = (valor_pot * 3.3f) / 4095.0f;

    // 4. Salida por el monitor serial
    Serial.print("Valor crudo ADC (12 bits): ");
    Serial.print(valor_pot);
    Serial.print(" | Voltaje: ");
    Serial.print(voltaje, 2);
    Serial.print(" V | Posicion: ");
    Serial.print(porcentaje);
    Serial.println("%");

    // Pausa de 1 segundo entre lecturas
    delay(1000);
}

