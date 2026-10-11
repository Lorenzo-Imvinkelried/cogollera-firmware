#include <Arduino.h>

/*
 * EJEMPLO 02: Lectura del Sensor de Temperatura Interno del Silicio
 * -----------------------------------------------------------------
 * El ESP32 cuenta con un sensor de temperatura integrado en su chip.
 * Mide la temperatura interna de la CPU para diagnóstico térmico.
 *
 * Función utilizada:
 * - temperatureRead(): devuelve la temperatura interna en grados Celsius (°C).
 */

const int LED_INDICADOR = 2;

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_INDICADOR, OUTPUT);

    Serial.println("==========================================");
    Serial.println("   MONITOR DE TEMPERATURA INTERNA ESP32   ");
    Serial.println("==========================================");
}

void loop() {
    // Lectura del sensor de temperatura interno (en °C)
    float tempC = temperatureRead();
    float tempF = (tempC * 9.0 / 5.0) + 32.0;

    // Destello del LED azul al tomar cada medición
    digitalWrite(LED_INDICADOR, HIGH);
    delay(50);
    digitalWrite(LED_INDICADOR, LOW);

    // Salida por monitor serie
    Serial.print("Temperatura interna del chip: ");
    Serial.print(tempC, 2);
    Serial.print(" °C  |  ");
    Serial.print(tempF, 2);
    Serial.println(" °F");

    delay(950); // Total: 1 segundo por ciclo
}
