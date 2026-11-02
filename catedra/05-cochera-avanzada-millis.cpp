#include <Arduino.h>

/*
 * ==============================================================================
 * EJERCICIO 05: Propuesta de Cátedra - Cochera SIN DELAY (Máquina de Estados)
 * ==============================================================================
 * Cátedra: Sistemas Embebidos / IoT con ESP32
 * Fuente: video.pdf (Página 8 - "Propuesta para seguir practicando")
 *
 * Consigna oficial de la cátedra:
 * "Rehacer el resto de los ejemplos prácticos hechos previamente, reemplazando
 *  el uso del delay con un enfoque similar al visto en el último ejemplo
 *  del parpadeo del led."
 *
 * ¿Por qué este ejercicio es el más importante de la clase?
 * En el Ejercicio 03 (con delay), el parpadeo de 3 veces bloqueaba la placa durante
 * 6 segundos.
 * Aquí resolvemos la consigna mediante una MÁQUINA DE ESTADOS NO BLOQUEANTE:
 * 1. El pulsador se lee en cada ciclo sin demoras.
 * 2. Al pulsarlo, se inicia la secuencia de 3 parpadeos (1s ON, 1s OFF).
 * 3. Se usa 'millis()' para alternar los estados del LED cada 1000 ms.
 * 4. El procesador queda 100% libre para seguir realizando otras operaciones.
 * ==============================================================================
 */

// 1. Pines y constantes
const int PIN_PULSADOR  = 32;       // Pulsador con INPUT_PULLUP
const int PIN_LUMINARIA = 23;       // LED de advertencia de cochera

const unsigned long TIEMPO_PASO_MS = 1000; // 1 segundo encendido / 1 segundo apagado
const int TOTAL_CAMBIOS = 6;               // 3 ciclos = 3 encendidos + 3 apagados = 6 transiciones

// 2. Variables de control de tiempo y máquina de estados
bool secuenciaActiva = false;
int cambiosRealizados = 0;
int estadoLuminaria = LOW;
unsigned long previousMillisLuminaria = 0;

// Variables para antirrebote del pulsador sin delay
int ultimoEstadoPulsador = HIGH;
unsigned long tiempoUltimoDebounce = 0;
const unsigned long DEBOUNCE_DELAY_MS = 50;

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(PIN_PULSADOR, INPUT_PULLUP);
    pinMode(PIN_LUMINARIA, OUTPUT);
    digitalWrite(PIN_LUMINARIA, LOW);

    Serial.println("======================================================================");
    Serial.println(" EJERCICIO 05: COCHERA NO BLOQUEANTE CON 'millis()' (DESAFIO CATEDRA) ");
    Serial.println("======================================================================");
    Serial.println("-> Presiona el pulsador en cualquier momento.");
    Serial.println("-> Cero delays: el microcontrolador responde instantaneamente.");
    Serial.println("----------------------------------------------------------------------");
}

void loop() {
    unsigned long currentMillis = millis();

    // -------------------------------------------------------------------------
    // ETAPA 1: Lectura del pulsador con antirrebote no bloqueante
    // -------------------------------------------------------------------------
    int lecturaPulsador = digitalRead(PIN_PULSADOR);

    // Detección de flanco de bajada (de HIGH a LOW al presionar con INPUT_PULLUP)
    if (lecturaPulsador == LOW && ultimoEstadoPulsador == HIGH) {
        if (currentMillis - tiempoUltimoDebounce >= DEBOUNCE_DELAY_MS) {
            tiempoUltimoDebounce = currentMillis;

            if (!secuenciaActiva) {
                // Iniciamos la secuencia de advertencia de la cochera
                secuenciaActiva = true;
                cambiosRealizados = 0;
                estadoLuminaria = HIGH; // Primer parpadeo: Encendido inmediato
                digitalWrite(PIN_LUMINARIA, estadoLuminaria);
                previousMillisLuminaria = currentMillis;

                Serial.println("\n[PULSO DETECTADO] Vehiculo saliendo -> Iniciando 3 parpadeos...");
                Serial.printf("  [%lu ms] Parpadeo 1/3: [LUMINARIA ENCENDIDA]\n", currentMillis);
            } else {
                Serial.println("  (Aviso: La secuencia ya se encuentra en curso...)");
            }
        }
    }
    ultimoEstadoPulsador = lecturaPulsador;

    // -------------------------------------------------------------------------
    // ETAPA 2: Control de tiempo no bloqueante para el parpadeo de la luminaria
    // -------------------------------------------------------------------------
    if (secuenciaActiva) {
        if (currentMillis - previousMillisLuminaria >= TIEMPO_PASO_MS) {
            previousMillisLuminaria = currentMillis;
            cambiosRealizados++;

            if (cambiosRealizados < TOTAL_CAMBIOS) {
                // Alternamos el estado de la luminaria (HIGH <-> LOW)
                estadoLuminaria = (estadoLuminaria == HIGH) ? LOW : HIGH;
                digitalWrite(PIN_LUMINARIA, estadoLuminaria);

                int parpadeoActual = (cambiosRealizados / 2) + 1;
                const char* textoEstado = (estadoLuminaria == HIGH) ? "ENCENDIDA" : "APAGADA";
                Serial.printf("  [%lu ms] Parpadeo %d/3: [LUMINARIA %s]\n", 
                              currentMillis, parpadeoActual, textoEstado);
            } else {
                // Secuencia completada: apagamos el LED y restablecemos la máquina de estados
                secuenciaActiva = false;
                estadoLuminaria = LOW;
                digitalWrite(PIN_LUMINARIA, LOW);
                Serial.printf("  [%lu ms] [FIN SECUENCIA] Luminaria en reposo. Sistema listo.\n\n", currentMillis);
            }
        }
    }

    // -------------------------------------------------------------------------
    // ETAPA 3: Otras tareas concurrentes
    // -------------------------------------------------------------------------
    // Aquí el microcontrolador puede realizar cualquier otra tarea en paralelo
    // (medir sensores, enviar a Supabase, etc.) sin sufrir pausas.
}

