#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 03: TAREA DE CÁTEDRA - Barra Indicadora de Nivel con 3 LEDs
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.3. Práctica 1 con ESP32.pdf (Página 14 - "Consigna de entradas analógicas")
 *
 * Consigna Oficial:
 * "Implemente en Wokwi el circuito y código necesario para que, según la posición
 *  de un potenciómetro se tenga el siguiente comportamiento de tres leds usando el ESP32:
 *  - Si la entrada es mayor o igual a 1365, encender un led verde, si es menor, apagarlo.
 *  - Si la entrada es mayor o igual a 2730, encender un led amarillo, si es menor, apagarlo.
 *  - Si la entrada es igual a 4095, encender un led rojo, si es menor, apagarlo."
 *
 * Explicación conceptual:
 * - 4095 / 3 = 1365 (Representa el primer tercio de la escala / Nivel Bajo).
 * - 1365 * 2 = 2730 (Representa el segundo tercio de la escala / Nivel Medio).
 * - 4095 = Escala completa (Valor máximo del ADC / Nivel Crítico).
 * Funciona como una barra progresiva tipo vúmetro o termómetro de criticidad industrial:
 *   * [ 0 - 1364]   : Todos apagados.
 *   * [1365 - 2729] : LED Verde encendido.
 *   * [2730 - 4094] : LED Verde + LED Amarillo encendidos.
 *   * [4095]        : LED Verde + LED Amarillo + LED Rojo (Crítico al máximo).
 *
 * Conexión del circuito (Wokwi / Protoboard):
 * - Potenciómetro: Terminal central a GPIO 32, extremos a 3.3V y GND.
 * - LED Verde:    GPIO 25 -> Resistencia 220Ω -> Ánodo (+), Cátodo (-) a GND.
 * - LED Amarillo: GPIO 26 -> Resistencia 220Ω -> Ánodo (+), Cátodo (-) a GND.
 * - LED Rojo:     GPIO 27 -> Resistencia 220Ω -> Ánodo (+), Cátodo (-) a GND.
 * ==============================================================================
 */

// 1. Asignación de pines
const int pinPotenciometro = 32; // Entrada analógica ADC1 (GPIO 32)
const int pinLedVerde      = 25; // Salida digital LED Verde
const int pinLedAmarillo   = 26; // Salida digital LED Amarillo
const int pinLedRojo       = 27; // Salida digital LED Rojo

// 2. Umbrales numéricos fijados por la consigna de cátedra
const int UMBRAL_VERDE    = 1365;
const int UMBRAL_AMARILLO = 2730;
const int UMBRAL_ROJO     = 4095;

void setup() {
    Serial.begin(115200);
    delay(500);

    // Configuramos los pines de los LEDs como salidas digitales
    pinMode(pinLedVerde, OUTPUT);
    pinMode(pinLedAmarillo, OUTPUT);
    pinMode(pinLedRojo, OUTPUT);

    // Nos aseguramos de iniciar con todos los LEDs apagados
    digitalWrite(pinLedVerde, LOW);
    digitalWrite(pinLedAmarillo, LOW);
    digitalWrite(pinLedRojo, LOW);

    Serial.println("======================================================================");
    Serial.println("  EJERCICIO 03: TAREA DE CATEDRA - BARRA DE CRITICIDAD CON 3 LEDS     ");
    Serial.println("======================================================================");
    Serial.println("Condiciones:");
    Serial.println(" 1. Lectura >= 1365 -> LED Verde encendido (sino apagar)");
    Serial.println(" 2. Lectura >= 2730 -> LED Amarillo encendido (sino apagar)");
    Serial.println(" 3. Lectura == 4095 -> LED Rojo encendido (sino apagar)");
    Serial.println("----------------------------------------------------------------------");
}

void loop() {
    // Lectura del valor analógico del potenciómetro (0 a 4095)
    int valorPot = analogRead(pinPotenciometro);

    // Condición 1: LED Verde
    if (valorPot >= UMBRAL_VERDE) {
        digitalWrite(pinLedVerde, HIGH);
    } else {
        digitalWrite(pinLedVerde, LOW);
    }

    // Condición 2: LED Amarillo
    if (valorPot >= UMBRAL_AMARILLO) {
        digitalWrite(pinLedAmarillo, HIGH);
    } else {
        digitalWrite(pinLedAmarillo, LOW);
    }

    // Condición 3: LED Rojo (solo en el tope exacto 4095)
    // Nota técnica: en potenciómetros reales de protoboard con tolerancia,
    // a veces llega a 4094 o 4095; aquí evaluamos exactamente == 4095 como pide la cátedra.
    if (valorPot >= UMBRAL_ROJO) {
        digitalWrite(pinLedRojo, HIGH);
    } else {
        digitalWrite(pinLedRojo, LOW);
    }

    // Reporte por consola
    Serial.printf("Potenciometro: %4d / 4095 | LEDs: [Verde: %s] [Amarillo: %s] [Rojo: %s]\n",
                  valorPot,
                  (valorPot >= UMBRAL_VERDE) ? "ON " : "OFF",
                  (valorPot >= UMBRAL_AMARILLO) ? "ON " : "OFF",
                  (valorPot >= UMBRAL_ROJO) ? "ON " : "OFF");

    delay(250); // Muestreo fluido 4 veces por segundo
}

