#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 03: Luminaria de Cochera con 'delay' (Entradas + Salidas)
 * ==============================================================================
 * Cátedra: Sistemas Embebidos / IoT con ESP32
 * Fuente: video.pdf (Página 6)
 *
 * Consigna:
 * "Implemente en Wokwi el circuito y código necesario para que, cuando se presiona
 *  un pulsador, un LED parpadee 3 veces estando 1 segundo encendido y un segundo
 *  apagado. Simulando una luminaria que parpadea, por ejemplo, al presionar un
 *  pulsador si se está sacando un vehículo de una cochera."
 *
 * Conexión del circuito (Wokwi / Protoboard):
 * - LED (Luminaria): GPIO 23 -> Resistencia 220Ω -> Ánodo (+), Cátodo (-) a GND.
 * - Pulsador: GPIO 32 a un terminal, el otro terminal a GND (usando INPUT_PULLUP).
 *
 * Limitación pedagógica explicada en clase:
 * Este programa utiliza 'delay(1000)'. Durante los 6 segundos que dura el parpadeo
 * (3 veces x 2 segundos por ciclo), el microcontrolador queda totalmente CONGELADO.
 * Si alguien vuelve a apretar el pulsador o se quiere leer otro sensor, la placa
 * no lo detectará. Esto motiva el uso de 'millis()' en los ejercicios siguientes.
 * ==============================================================================
 */

// 1. Definición de pines y constantes
const int PIN_PULSADOR = 32;       // Pulsador de cochera
const int PIN_LUMINARIA = 23;      // LED luminaria de advertencia
const int REPETICIONES = 3;        // 3 parpadeos
const int TIEMPO_PASO_MS = 1000;   // 1 segundo por estado

int ultimoEstadoPulsador = HIGH;

void setup() {
    Serial.begin(115200);
    delay(500);

    // Configuramos los pines
    pinMode(PIN_PULSADOR, INPUT_PULLUP);
    pinMode(PIN_LUMINARIA, OUTPUT);

    // Nos aseguramos de que arranque apagada
    digitalWrite(PIN_LUMINARIA, LOW);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 03: LUMINARIA COCHERA (CON DELAY)      ");
    Serial.println("==================================================");
    Serial.println("-> Presiona el pulsador para activar la salida del vehiculo.");
    Serial.println("--------------------------------------------------");
}

void loop() {
    int estadoPulsador = digitalRead(PIN_PULSADOR);

    // Detección de pulsación (con INPUT_PULLUP, presionar = LOW)
    if (estadoPulsador == LOW && ultimoEstadoPulsador == HIGH) {
        Serial.println("\n[ALERTA COCHERA] Vehiculo saliendo: Iniciando secuencia de 3 parpadeos...");

        // Ejecutamos los 3 ciclos de parpadeo solicitados
        for (int i = 1; i <= REPETICIONES; i++) {
            // Encendido durante 1 segundo
            digitalWrite(PIN_LUMINARIA, HIGH);
            Serial.printf("  -> Parpadeo %d/%d: [ENCENDIDO 1s]\n", i, REPETICIONES);
            delay(TIEMPO_PASO_MS);

            // Apagado durante 1 segundo
            digitalWrite(PIN_LUMINARIA, LOW);
            Serial.printf("  -> Parpadeo %d/%d: [APAGADO   1s]\n", i, REPETICIONES);
            delay(TIEMPO_PASO_MS);
        }

        Serial.println("[FIN SECUENCIA] Luminaria en reposo. Esperando proxima salida.\n");
    }

    ultimoEstadoPulsador = estadoPulsador;
}

