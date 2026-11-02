#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 06: Interrupciones Externas - Semáforo con Parada de Emergencia
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.3. Práctica 1 con ESP32.pdf (Páginas 28 a 32)
 * Enlace Wokwi: https://wokwi.com/projects/410179671454768129
 *
 * Consigna:
 * "Implementar un pulsador de emergencia que permita encender todas las lámparas
 *  (considerarlas como LEDs) de un semáforo al mismo tiempo. Además de la
 *  interrupción, debe estar implementado el semáforo."
 *
 * Justificación de la Interrupción:
 * El semáforo normal cambia cada 2 segundos usando delay(2000) (un ciclo completo
 * tarda 6 segundos). Si no tuviéramos interrupciones, el programa ignoraría por
 * completo la pulsación de emergencia durante los delays.
 * Gracias a attachInterrupt(), en el instante exacto en que se presiona el botón,
 * la CPU suspende el delay de inmediato, ejecuta la ISR 'encenderTodas()' y prende
 * los 3 colores juntos sin esperar.
 *
 * Modificadores Clave:
 * - IRAM_ATTR: Obliga a compilar la función en la memoria RAM interna en lugar de
 *   la memoria Flash, permitiendo una respuesta inmediata en nanosegundos.
 * ==============================================================================
 */

// 1. Asignación de pines del semáforo y del pulsador
const int pinRojo      = 32; // LED Rojo
const int pinAmarillo  = 33; // LED Amarillo
const int pinVerde     = 25; // LED Verde
const int pinPulsador  = 26; // Pulsador de emergencia (con INPUT_PULLUP)

// 2. Rutina de Servicio de Interrupción (ISR)
// Se almacena en la RAM del ESP32 con IRAM_ATTR para máxima velocidad
void IRAM_ATTR encenderTodas() {
    digitalWrite(pinRojo, HIGH);
    digitalWrite(pinAmarillo, HIGH);
    digitalWrite(pinVerde, HIGH);
}

void setup() {
    Serial.begin(115200);
    delay(500);

    // Configuración de los pines de salida del semáforo
    pinMode(pinRojo, OUTPUT);
    pinMode(pinAmarillo, OUTPUT);
    pinMode(pinVerde, OUTPUT);

    // Entrada digital con resistencia Pull-Up interna
    pinMode(pinPulsador, INPUT_PULLUP);

    // Asociación de la interrupción:
    // - digitalPinToInterrupt(pin): Asegura compatibilidad entre familias de microcontroladores.
    // - encenderTodas: Función callback a ejecutar.
    // - FALLING: Se dispara cuando se presiona el botón (transición de HIGH a LOW con INPUT_PULLUP).
    //   (La diapositiva original menciona RISING que se dispara al soltar el botón; ambas son válidas).
    attachInterrupt(digitalPinToInterrupt(pinPulsador), encenderTodas, FALLING);

    Serial.println("======================================================================");
    Serial.println(" EJERCICIO 06: SEMAFORO CON INTERRUPCION DE EMERGENCIA (IRAM_ATTR)    ");
    Serial.println("======================================================================");
    Serial.println("-> Semáforo cíclico: Verde (2s) -> Amarillo (2s) -> Rojo (2s)");
    Serial.println("-> Presiona el botón de emergencia en CUALQUIER momento para activar alerta.");
    Serial.println("----------------------------------------------------------------------");
}

void loop() {
    // 1. Fase Verde
    Serial.println("[SEMAFORO] -> Luz VERDE (Paso libre)");
    digitalWrite(pinVerde, HIGH);
    digitalWrite(pinAmarillo, LOW);
    digitalWrite(pinRojo, LOW);
    delay(2000);

    // 2. Fase Amarillo
    Serial.println("[SEMAFORO] -> Luz AMARILLA (Precaucion)");
    digitalWrite(pinAmarillo, HIGH);
    digitalWrite(pinVerde, LOW);
    digitalWrite(pinRojo, LOW);
    delay(2000);

    // 3. Fase Rojo
    Serial.println("[SEMAFORO] -> Luz ROJA (Detencion)");
    digitalWrite(pinRojo, HIGH);
    digitalWrite(pinAmarillo, LOW);
    digitalWrite(pinVerde, LOW);
    delay(2000);
}

