#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 04: Parpadeo de LED Cíclico SIN DELAY (Uso de 'millis()')
 * ==============================================================================
 * Cátedra: Sistemas Embebidos / IoT con ESP32
 * Fuente: video.pdf (Página 7)
 * Enlace Wokwi oficial visto en clase: https://wokwi.com/projects/442519340056753153
 *
 * Consigna:
 * "Encender y apagar un LED por 2 segundos en cada estado. El LED debe permanecer
 *  parpadeando cíclicamente, pero sin el uso de delay."
 *
 * Explicación Teórica de 'millis()':
 * - 'millis()' devuelve el número de milisegundos transcurridos desde que el ESP32
 *   inició su ejecución (es un contador de tiempo tipo cronómetro en hardware).
 * - En lugar de congelar la CPU con delay(), guardamos una variable de referencia
 *   'previousMillis' y en cada vuelta del loop preguntamos:
 *      ¿Pasaron ya los 2000 ms desde la última vez?
 *      (currentMillis - previousMillis >= interval)
 * - Si pasaron, conmutamos el estado del LED y actualizamos previousMillis.
 * - Si NO pasaron, el procesador sigue ejecutando el resto de las líneas del loop
 *   a millones de instrucciones por segundo sin perder tiempo.
 * ==============================================================================
 */

// 1. Definición de pines y constantes
const int PIN_LED = 23;            // Pin del LED (con resistencia 220Ω a GND)
const unsigned long INTERVALO_MS = 2000; // 2 segundos (2000 ms)

// 2. Variables para control de tiempo y estado
int estadoLed = LOW;
unsigned long previousMillis = 0; // Almacena el último instante en que cambió el LED

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, estadoLed);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 04: BLINK 2 SEGUNDOS CON 'millis()'    ");
    Serial.println("==================================================");
    Serial.println("-> Cero delays: el microcontrolador nunca se congela.");
}

void loop() {
    // Tomamos la marca de tiempo actual del reloj del microcontrolador
    unsigned long currentMillis = millis();

    // Verificamos si ya transcurrió el intervalo requerido
    if (currentMillis - previousMillis >= INTERVALO_MS) {
        // Guardamos el nuevo punto de referencia en el tiempo
        previousMillis = currentMillis;

        // Alternamos el estado del LED (si estaba LOW pasa a HIGH, y viceversa)
        if (estadoLed == LOW) {
            estadoLed = HIGH;
            Serial.printf("[%lu ms] LED -> [ENCENDIDO]\n", currentMillis);
        } else {
            estadoLed = LOW;
            Serial.printf("[%lu ms] LED -> [APAGADO]\n", currentMillis);
        }

        // Aplicamos el cambio al pin físico
        digitalWrite(PIN_LED, estadoLed);
    }

    // Aquí abajo el procesador puede seguir haciendo otras tareas simultáneas
    // (leer pulsadores, comunicarse por Wi-Fi, etc.) sin ser bloqueado.
}

