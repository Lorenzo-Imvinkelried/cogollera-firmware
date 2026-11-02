#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 09: Sensor Ultrasónico HC-SR04 (Medición de Distancia y Alarma)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.3. Práctica 1 con ESP32.pdf (Páginas 41 a 47)
 * Enlace Wokwi: https://wokwi.com/projects/442888513380344833
 *
 * Consigna:
 * "Se posee un sensor ultrasónico HC-SR04 en un vehículo. Se desea medir continuamente
 *  la distancia y dar un aviso cuando se tenga un objeto a 50 cm o menos de distancia.
 *  Implementar una solución para esta tarea usando ESP32. Para esto, crear una función
 *  específica encargada de medir la distancia, la cual se llama desde el loop."
 *
 * Principio Físico y Matemático:
 * 1. Disparo (Trigger):
 *    Se envía un pulso en nivel HIGH durante 10 microsegundos por el pin Trigger (GPIO 25).
 *    El transductor ultrasónico emite 8 pulsos a 40 kHz.
 * 2. Recepción de Eco (Echo):
 *    El pin Echo (GPIO 33) se pone en HIGH por el tiempo exacto que demora la onda
 *    sonora en rebotar en el obstáculo y volver al sensor.
 * 3. Cálculo de Distancia:
 *    - Velocidad del sonido en el aire: v = 343 m/s = 0.0343 cm/µs.
 *    - Como el sonido hace un trayecto de ida y vuelta:
 *         Distancia = (Velocidad * Tiempo) / 2 = (0.0343 * Duracion_µs) / 2
 *
 * Conexión del circuito (Wokwi / Protoboard):
 * - VCC: Conectado a 5V del ESP32 (¡El sensor HC-SR04 requiere 5V para sus transductores!).
 * - Trigger: Conectado al GPIO 25 (Salida digital).
 * - Echo: Conectado al GPIO 33 (Entrada digital).
 * - GND: Conectado a GND del ESP32.
 * ==============================================================================
 */

// 1. Asignación de pines
const int trigPin = 25; // Trigger (Disparador ultrasónico)
const int echoPin = 33; // Echo (Receptor de eco)

// Constante de proximidad solicitada en la consigna
const float DISTANCIA_ALERTA_CM = 50.0;

// 2. Función modular específica encargada de medir la distancia en cm
float leerDistancia_cm() {
    // Nos aseguramos de que el Trigger esté en bajo
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Generamos el pulso de disparo de 10 microsegundos
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Medimos el tiempo en microsegundos que el pin Echo permanece en HIGH
    // (Timeout de 30.000 µs equivale a ~5 metros de alcance máximo)
    unsigned long duracion_us = pulseIn(echoPin, HIGH, 30000);

    // Si no hubo eco (fuera de rango o tiempo agotado)
    if (duracion_us == 0) {
        return -1.0;
    }

    // Calculamos y retornamos la distancia en centímetros
    return (duracion_us * 0.0343f) / 2.0f;
}

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    digitalWrite(trigPin, LOW);

    Serial.println("======================================================================");
    Serial.println(" EJERCICIO 09: SENSOR ULTRASONICO HC-SR04 (RADAR DE VEHICULO)         ");
    Serial.println("======================================================================");
    Serial.printf("-> Umbral de colision: Menor o igual a %.1f cm\n", DISTANCIA_ALERTA_CM);
    Serial.println("----------------------------------------------------------------------");
}

void loop() {
    float distancia = leerDistancia_cm();

    if (distancia < 0) {
        Serial.println("[RADAR] Fuera de rango o sin eco.");
    } else {
        Serial.printf("[RADAR] Distancia actual: %.2f cm\n", distancia);

        // Verificamos si hay un obstáculo a 50 cm o menos
        if (distancia <= DISTANCIA_ALERTA_CM) {
            Serial.println("  >>> ¡ALERTA! Hay un objeto cerca de colisionar <<<");
            Serial.printf("      Vehiculo a %.2f cm del obstaculo.\n", distancia);
            Serial.println("  --------------------------------------------------");
        }
    }

    // Intervalo de muestreo para facilitar el testeo por consola
    // (En un vehículo real puede reducirse a 100 ms)
    delay(1000);
}

