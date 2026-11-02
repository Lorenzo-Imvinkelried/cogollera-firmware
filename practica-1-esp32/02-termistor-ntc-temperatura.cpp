#include <Arduino.h>
#include <math.h>

/*
 * ==============================================================================
 * EJERCICIO 02: Medición de Temperatura con Termistor NTC y Ecuación de Steinhart-Hart
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.3. Práctica 1 con ESP32.pdf (Páginas 7 a 13)
 * Enlace Wokwi: https://wokwi.com/projects/442528601802077185
 *
 * Consigna:
 * "Leer e indicar por el monitor serial el valor de temperatura medido por un
 *  termistor NTC (Negative Temperature Coefficient)."
 *
 * Explicación de las Ecuaciones Físicas:
 * 1. Divisor de tensión:
 *    El sensor forma un divisor de tensión con una resistencia auxiliar Raux = 10kΩ.
 *    El voltaje medido es:
 *       Vm = Vcc * (R_NTC / (Raux + R_NTC))
 *    Como el ADC del ESP32 entrega un valor digital entre 0 y 4095 proporcional a Vcc:
 *       Vm / Vcc = lectura / 4095.0
 *    Despejando la resistencia del NTC:
 *       R_NTC = Raux / ( (4095.0 / lectura) - 1 )
 *
 * 2. Ecuación del parámetro Beta (Aproximación de Steinhart-Hart):
 *    Relaciona la resistencia del NTC con la temperatura absoluta T (en Kelvin):
 *       R_NTC = R_0 * exp( beta * ( (1/T) - (1/T_0) ) )
 *    Despejando la temperatura T_kelvin:
 *       T_kelvin = 1.0 / ( (log(R_NTC / R_0) / beta) + (1.0 / T_0) )
 *    Donde log() es el logaritmo natural (ln).
 *
 * 3. Conversión a Celsius:
 *       T_celsius = T_kelvin - 273.15
 * ==============================================================================
 */

// 1. Pines y constantes del sensor NTC según hoja de datos y Wokwi
const int ntcPin = 33;            // Pin ADC1 (GPIO 33)
const float T_0  = 298.15;        // Temperatura de referencia: 25 °C en Kelvin (273.15 + 25)
const float R_0  = 10000.0;       // Resistencia nominal a 25 °C: 10 kΩ
const float beta = 3950.0;        // Coeficiente Beta del termistor
const float Raux = 10000.0;       // Resistencia auxiliar fija del divisor: 10 kΩ
const float Vcc  = 3.3;           // Voltaje de alimentación del ESP32

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 02: SENSOR DE TEMPERATURA NTC (ADC)    ");
    Serial.println("==================================================");
    Serial.println("-> Modelo matematico: Steinhart-Hart / Parametro Beta");
    Serial.println("--------------------------------------------------");
}

void loop() {
    // 1. Tomamos la lectura analógica del ADC (0 a 4095)
    int lectura = analogRead(ntcPin);

    // Protección para evitar división por cero en caso de lectura nula o desconexión
    if (lectura <= 0) lectura = 1;
    if (lectura >= 4095) lectura = 4094;

    // 2. Calculamos la resistencia actual del NTC (en Ohms)
    float R_NTC = Raux / ((4095.0f / (float)lectura) - 1.0f);

    // 3. Calculamos la temperatura en grados Kelvin usando la ecuación de Beta
    float T_kelvin = 1.0f / ((log(R_NTC / R_0) / beta) + (1.0f / T_0));

    // 4. Convertimos a grados Celsius (°C)
    float T_celsius = T_kelvin - 273.15f;

    // 5. Imprimimos resultados por el monitor serie
    Serial.print("Lectura ADC: ");
    Serial.print(lectura);
    Serial.print(" | R_NTC: ");
    Serial.print(R_NTC, 1);
    Serial.print(" ohms | Temperatura: ");
    Serial.print(T_celsius, 2);
    Serial.println(" °C");

    // Intervalo de medición de 5 segundos como solicita la cátedra
    delay(5000);
}

