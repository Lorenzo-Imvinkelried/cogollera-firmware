#include <Arduino.h>

/*
 * ==============================================================================
 * EJEMPLO 08: Doble Barrera IR - Detector de Bicho Grande / Cogollera
 * ==============================================================================
 * Placa: ESP32-S V1.1 (38 pines) - Vista Dorsal / Panza Abajo
 * 
 * Lógica de clasificación de tamaño:
 * - Haz 1 (GPIO 34) y Haz 2 (GPIO 35) están separados por 8-12 mm.
 * - Si cae un bicho pequeño o polvo (< 5mm): Corta solo 1 haz -> Se DESCARTA.
 * - Si cae una COGOLLERA (15-30mm): Corta AMBOS haces simultáneamente -> +1 CONTADOR.
 * 
 * Distribución de pines:
 * - Receptor IR 1: GPIO 34 (Pin 15 Der)
 * - Receptor IR 2: GPIO 35 (Pin 14 Der)
 * - LED Testigo:   GPIO 26 (Pin 10 Der) + LED azul integrado (GPIO 2)
 * - Alimentación:  3.3V (Pin 19 Der) y GND (Pin 06 Der)
 */

// Asignación de pines
const int PIN_IR_1        = 34; // Sensor IR 1 (Haz superior)
const int PIN_IR_2        = 35; // Sensor IR 2 (Haz inferior)
const int PIN_LED_EXTERNO = 26; // LED de aviso en protoboard
const int PIN_LED_AZUL    = 2;  // LED integrado en la placa

// Duración del aviso visual del LED al confirmar una captura (ms)
const unsigned long TIEMPO_LED_VISUAL = 350;

// Umbrales calculados por auto-calibración individual
int umbral1 = 3400;
int umbral2 = 3400;
int reposo1 = 0;
int reposo2 = 0;

// Contadores y estados
unsigned long contadorCogolleras = 0;
unsigned long contadorDescartes  = 0;

bool cogolleraEnCurso = false;
bool descarteEnCurso  = false;

unsigned long tiempoApagarLED = 0;
unsigned long tiempoUltimoReporte = 0;

void calibrarAmbosSensores() {
    Serial.println("\n[CALIBRANDO] Evaluando ambos haces en reposo (manten despejada la trampa)...");

    long suma1 = 0;
    long suma2 = 0;
    const int MUESTRAS = 40;

    for (int i = 0; i < MUESTRAS; i++) {
        suma1 += analogRead(PIN_IR_1);
        suma2 += analogRead(PIN_IR_2);

        // Parpadeo de diagnóstico en ambos LEDs durante la calibración
        bool blink = (i % 4 < 2);
        digitalWrite(PIN_LED_EXTERNO, blink ? HIGH : LOW);
        digitalWrite(PIN_LED_AZUL, blink ? HIGH : LOW);
        delay(25);
    }

    digitalWrite(PIN_LED_EXTERNO, LOW);
    digitalWrite(PIN_LED_AZUL, LOW);

    reposo1 = suma1 / MUESTRAS;
    reposo2 = suma2 / MUESTRAS;

    // Cálculo del punto medio dinámico para cada receptor de forma independiente
    umbral1 = reposo1 + ((4095 - reposo1) * 0.50);
    umbral2 = reposo2 + ((4095 - reposo2) * 0.50);

    Serial.printf("-> Sensor 1 (GPIO 34): Reposo = %4d | Umbral = %4d\n", reposo1, umbral1);
    Serial.printf("-> Sensor 2 (GPIO 35): Reposo = %4d | Umbral = %4d\n", reposo2, umbral2);
    Serial.println("-> Calibracion EXITOSA: Sistema vigilando caidas de cogollera.");
    Serial.println("----------------------------------------------------------------------");
}

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(PIN_LED_EXTERNO, OUTPUT);
    pinMode(PIN_LED_AZUL, OUTPUT);

    analogReadResolution(12);

    Serial.println();
    Serial.println("======================================================================");
    Serial.println("   ESP32: TRAMPA INTELIGENTE COGOLLERA (DOBLE BARRERA INFRARROJA)    ");
    Serial.println("======================================================================");

    calibrarAmbosSensores();
}

void loop() {
    int v1 = analogRead(PIN_IR_1);
    int v2 = analogRead(PIN_IR_2);
    unsigned long ahora = millis();

    bool corte1 = (v1 >= umbral1);
    bool corte2 = (v2 >= umbral2);

    // CASO 1: AMBOS HACES CORTADOS SIMULTÁNEAMENTE -> BICHO GRANDE (COGOLLERA)
    if (corte1 && corte2) {
        if (!cogolleraEnCurso) {
            cogolleraEnCurso = true;
            contadorCogolleras++;

            // Encendemos los LEDs de alarma
            digitalWrite(PIN_LED_EXTERNO, HIGH);
            digitalWrite(PIN_LED_AZUL, HIGH);
            tiempoApagarLED = ahora + TIEMPO_LED_VISUAL;

            Serial.println();
            Serial.println("******************************************************************");
            Serial.printf(" >>> [COGOLLERA CAPTURADA #%lu] ¡BICHO GRANDE DETECTADO! <<<\n", contadorCogolleras);
            Serial.printf("     Sensores cortados: Haz 1 (%d >= %d) | Haz 2 (%d >= %d)\n", v1, umbral1, v2, umbral2);
            Serial.println("******************************************************************");
        }
    }
    // CASO 2: SOLO UNO DE LOS HACES SE CORTA -> BICHO CHICO / POLVILLO (DESCARTADO)
    else if (corte1 || corte2) {
        if (!cogolleraEnCurso && !descarteEnCurso) {
            descarteEnCurso = true;
            contadorDescartes++;
            Serial.printf("[Descarte #%lu] Objeto chico ignorado (Haz 1: %d | Haz 2: %d)\n", 
                          contadorDescartes, v1, v2);
        }
    }
    // CASO 3: AMBOS HACES LIBRES -> SE RESTABLECEN LOS DISPARADORES
    else {
        cogolleraEnCurso = false;
        descarteEnCurso  = false;
    }

    // Apagado del LED tras cumplirse el pulso de aviso visual
    if (!cogolleraEnCurso && ahora >= tiempoApagarLED) {
        digitalWrite(PIN_LED_EXTERNO, LOW);
        digitalWrite(PIN_LED_AZUL, LOW);
    }

    // Reporte de vigilancia periódica en reposo cada 2 segundos
    if (ahora - tiempoUltimoReporte >= 2000) {
        tiempoUltimoReporte = ahora;
        if (!corte1 && !corte2) {
            Serial.printf("[Vigilancia Reposo] Haces libres (S1: %4d, S2: %4d) | Cogolleras: %lu | Descartes: %lu\n", 
                          v1, v2, contadorCogolleras, contadorDescartes);
        }
    }
}
