#include <Arduino.h>
#include <DHT.h>

/*
 * ==============================================================================
 * EJERCICIO 08: Sensor Digital DHT22 (Temperatura y Humedad Ambiental)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.3. Práctica 1 con ESP32.pdf (Páginas 37 a 40)
 * Enlace Wokwi: https://wokwi.com/projects/442885860030479361
 *
 * Consigna:
 * "Se posee un sensor DHT22, se desea medir cada 1 minuto los valores de
 *  temperatura y humedad e imprimirlos por el monitor serial. Implementar una
 *  solución usando el ESP32 para esta aplicación."
 *
 * Características del Sensor DHT22 (AM2302):
 * - Es un sensor digital calibrado de fábrica.
 * - Conexión:
 *     Pin 1: VCC  -> Conectado a 3.3V del ESP32.
 *     Pin 2: DATA -> Conectado a un pin digital (GPIO 13 en este ejemplo).
 *     Pin 3: NC   -> No conectado.
 *     Pin 4: GND  -> Conectado a GND del ESP32.
 * - Protocolo: Envía una trama serie de 40 bits cronometrados. La librería <DHT.h>
 *   se encarga de decodificarla y verificar el checksum.
 * - Función isnan(): Comprueba si la lectura dio "Not a Number" (error de comunicación).
 * ==============================================================================
 */

// 1. Pines y tipo de sensor
const int dhtPin = 13; // Pin de datos (GPIO 13)

// Instanciamos el objeto DHT especificando el pin y el modelo (DHT22)
DHT dht(dhtPin, DHT22);

// Intervalo de medición solicitado: 1 minuto (60.000 ms)
// (Para pruebas rápidas en Wokwi o laboratorio se puede ajustar a 5.000 ms)
const unsigned long INTERVALO_LECTURA_MS = 60000;
unsigned long previousMillisDHT = 0;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 08: SENSOR DIGITAL DHT22 (TEMP/HUMEDAD)");
    Serial.println("==================================================");

    // Inicializamos la comunicación con el sensor DHT22
    dht.begin();

    Serial.println("-> Sensor inicializado. Tomando primera medicion...");
    Serial.println("--------------------------------------------------");
}

void loop() {
    unsigned long currentMillis = millis();

    // Verificamos si pasó el intervalo de 1 minuto sin usar delay() bloqueante
    // (o en el primer ciclo al arrancar)
    if (currentMillis - previousMillisDHT >= INTERVALO_LECTURA_MS || previousMillisDHT == 0) {
        previousMillisDHT = currentMillis;

        // Lectura de temperatura en grados Celsius (°C)
        float temperatura = dht.readTemperature();

        // Lectura de humedad relativa en porcentaje (%)
        float humedad = dht.readHumidity();

        // Validación de datos: Comprobamos si alguna lectura falló (retorna NaN)
        if (isnan(humedad) || isnan(temperatura)) {
            Serial.println(F("[ERROR] ¡Falla en la lectura del sensor DHT22! Reintentando en el proximo ciclo..."));
            return;
        }

        // Si se midió correctamente, imprimimos los valores formateados
        Serial.println("----------------------------------------");
        Serial.printf("[%lu s] Lectura Exitosa DHT22:\n", currentMillis / 1000);
        Serial.println("  -> Temperatura: " + String(temperatura, 2) + " °C");
        Serial.println("  -> Humedad:     " + String(humedad, 1) + " %");
        Serial.println("----------------------------------------");
    }
}

