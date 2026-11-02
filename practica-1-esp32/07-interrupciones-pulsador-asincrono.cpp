#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 07: Interrupciones Asíncronas con Bandera 'volatile'
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.3. Práctica 1 con ESP32.pdf (Páginas 33 a 36)
 * Enlace Wokwi: https://wokwi.com/projects/442816839406691329
 *
 * Consigna:
 * "En una aplicación, puede ser difícil que se detecte cuándo se presiona un pulsador
 *  porque es muy probable que no se esté mirando a la entrada del pulsador en el
 *  momento en que este se pulsa.
 *  Las tareas asociadas al pulsador no son urgentes, sin embargo es importante
 *  detectar que fue pulsado.
 *  Se desea guardar mediante una interrupción la información de que el pulsador fue presionado."
 *
 * Explicación Teórica de 'volatile':
 * - Cuando una variable es modificada dentro de una interrupción (ISR) y leída
 *   en el bucle principal loop(), el compilador debe saber que su valor puede
 *   cambiar en cualquier instante fuera del flujo secuencial de código.
 * - Sin la palabra clave 'volatile', el compilador podría optimizar el código
 *   guardando la variable en un registro interno de la CPU, ignorando los cambios
 *   hechos por la interrupción.
 * - Con 'volatile', se garantiza que el microcontrolador siempre lea y escriba
 *   el valor directamente en la memoria RAM.
 * ==============================================================================
 */

// 1. Asignación de pines
const int pinPulsador = 32; // Pulsador conectado a GPIO 32 y GND (INPUT_PULLUP)

// 2. Bandera volátil para comunicar la ISR con el bucle principal loop()
volatile bool estadoPulsador = false;
volatile unsigned long conteoPulsacionesISR = 0;

// 3. Rutina de Servicio de Interrupción (ISR)
void IRAM_ATTR isr() {
    estadoPulsador = true;
    conteoPulsacionesISR++;
}

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(pinPulsador, INPUT_PULLUP);

    // Asociamos la interrupción al pin
    // Con INPUT_PULLUP, RISING detecta cuando se suelta el pulsador (de LOW a HIGH)
    attachInterrupt(digitalPinToInterrupt(pinPulsador), isr, RISING);

    Serial.println("======================================================================");
    Serial.println(" EJERCICIO 07: CAPTURA ASINCRONA CON 'volatile' E INTERRUPCIONES      ");
    Serial.println("======================================================================");
    Serial.println("-> El bucle principal tiene demoras largas de 10 segundos (tareas pesadas).");
    Serial.println("-> Presiona el pulsador en cualquier momento: el evento NUNCA se perdera.");
    Serial.println("----------------------------------------------------------------------");
}

void loop() {
    Serial.println("\n[LOOP] Ejecutando tarea pesada 1 (duracion: 5 segundos)...");
    delay(5000); // Tarea pesada bloqueante simulada

    // Verificamos si durante la tarea 1 alguien presionó el botón
    if (estadoPulsador) {
        Serial.println("  >>> ¡ATENCION! Se detecto que se presiono el pulsador.");
        Serial.printf("      Total de pulsos registrados por hardware: %lu\n", conteoPulsacionesISR);

        // Bajamos la bandera (acknowledge)
        estadoPulsador = false;
    } else {
        Serial.println("  (Sin pulsaciones durante la tarea 1)");
    }

    Serial.println("[LOOP] Ejecutando tarea pesada 2 (duracion: 5 segundos)...");
    delay(5000); // Tarea pesada bloqueante simulada

    // Volvemos a verificar si durante la tarea 2 alguien presionó el botón
    if (estadoPulsador) {
        Serial.println("  >>> ¡ATENCION! Se detecto que se presiono el pulsador.");
        Serial.printf("      Total de pulsos registrados por hardware: %lu\n", conteoPulsacionesISR);
        estadoPulsador = false;
    } else {
        Serial.println("  (Sin pulsaciones durante la tarea 2)");
    }
}

