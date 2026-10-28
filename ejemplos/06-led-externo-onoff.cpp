#include <Arduino.h>

/*
 * ==============================================================================
 * EJEMPLO 06: Control de LED Externo en Protoboard (Blink)
 * ==============================================================================
 * Placa: ESP32-S V1.1 (38 pines)
 * 
 * Conexión física:
 * - GPIO 26 (Pin 10 Lado Derecho): Conectado a Resistencia (220Ω-1kΩ) -> Ánodo (+) LED
 * - GND (Pin 06 Lado Derecho o Pin 13/19 Lado Izquierdo): Conectado al Cátodo (-) LED
 * 
 * Diagnóstico visual:
 * - Parpadean tanto el LED Externo (GPIO 26) como el LED Azul Interno (GPIO 2).
 *   Si el LED azul parpadea pero el externo no, revisa la polaridad de las patas
 *   del LED o que la resistencia esté bien insertada.
 */

// Definición de pines
const int PIN_LED_EXTERNO = 26; // GPIO 26 (Pin 10 derecho según diagrama)
const int PIN_LED_AZUL    = 2;  // LED integrado en la placa ESP32

void setup() {
    // Inicialización del puerto serie para monitoreo a 115200 baudios
    Serial.begin(115200);
    delay(1000);

    // Configuración de los pines como salidas digitales
    pinMode(PIN_LED_EXTERNO, OUTPUT);
    pinMode(PIN_LED_AZUL, OUTPUT);

    Serial.println();
    Serial.println("==================================================");
    Serial.println("   ESP32: CONTROL DE LED EXTERNO EN PROTOBOARD   ");
    Serial.println("==================================================");
    Serial.println("-> Pin LED Externo: GPIO 26");
    Serial.println("-> Pin LED Integrado: GPIO 2");
    Serial.println("-> Estado: Sistema iniciado correctamente.");
    Serial.println("--------------------------------------------------");
}

void loop() {
    // 1. Encendemos ambos LEDs
    digitalWrite(PIN_LED_EXTERNO, HIGH);
    digitalWrite(PIN_LED_AZUL, HIGH);
    Serial.println("[ESTADO] LEDs: -> ENCENDIDOS (HIGH)");
    delay(1000); // Mantenemos encendido durante 1 segundo

    // 2. Apagamos ambos LEDs
    digitalWrite(PIN_LED_EXTERNO, LOW);
    digitalWrite(PIN_LED_AZUL, LOW);
    Serial.println("[ESTADO] LEDs: -> APAGADOS (LOW)");
    delay(1000); // Mantenemos apagado durante 1 segundo
}

