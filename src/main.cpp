#include <Arduino.h>

/*
 * ==============================================================================
 * EJEMPLO 07: Detector de Barrera IR de Alta Velocidad con LED Testigo Externo
 * ==============================================================================
 * Placa: ESP32-S V1.1 (38 pines) - Vista Dorsal / Panza Abajo
 * 
 * Distribución de conexiones según 'circuito_detector_infrarrojo.txt':
 * 1. Emisor IR (LED transparente):
 *    - Ánodo (+) a Resistencia 120Ω (o 220Ω) conectada a 3V3 (Carril rojo +).
 *    - Cátodo (-) a GND (Carril azul -).
 * 
 * 2. Receptor IR (Fototransistor lente negra):
 *    - Colector a Resistencia 47kΩ a 3V3, y señal conectada a GPIO 34 (Pin 15 Der).
 *    - Emisor a GND (Carril azul -).
 * 
 * 3. LED Testigo Externo (Aviso de detección):
 *    - Conectado a GPIO 26 / P26 (Pin 10 Der) con Resistencia de 220Ω a Ánodo (+).
 *    - Cátodo (-) a GND (Carril azul -).
 * 
 * Características del firmware:
 * - Auto-calibración en el arranque: Mide el nivel de reposo y calcula el umbral óptimo.
 * - Test inicial de LEDs: Hace parpadear el LED externo y el interno al encender.
 * - Detección ultrarrápida sin delays: Muestreo continuo a miles de lecturas/seg.
 * - Extensor de pulso: Mantiene encendido el LED externo 250 ms por cada corte.
 */

// Asignación de pines
const int PIN_SENSOR_IR   = 34; // Pin P34 (Entrada analógica ADC1 - Pin 15 Der)
const int PIN_LED_EXTERNO = 26; // Pin P26 (LED externo en protoboard - Pin 10 Der)
const int PIN_LED_AZUL    = 2;  // LED azul integrado en la placa

// Duración del pulso de luz del LED externo al detectar (milisegundos)
const unsigned long TIEMPO_LED_VISUAL = 250;

// Variables dinámicas de calibración
int valorReposo = 0;
int umbralDeteccion = 3400; // Se calcula automáticamente al calibrar

// Variables de control de estado y tiempo
bool hazBloqueado = false;
unsigned long tiempoApagarLED = 0;
unsigned long contadorDetecciones = 0;
unsigned long tiempoUltimoReporte = 0;

void calibrarSensor() {
    Serial.println("\n[CALIBRANDO] Evaluando luz ambiente con el haz libre...");
    Serial.println("-> Asegurate de no tapar los sensores durante 1 segundo.");

    // Parpadeo sincronizado de ambos LEDs (externo e interno) para verificar funcionamiento
    long suma = 0;
    const int MUESTRAS = 40;
    for (int i = 0; i < MUESTRAS; i++) {
        suma += analogRead(PIN_SENSOR_IR);
        
        bool estadoBlink = (i % 4 < 2);
        digitalWrite(PIN_LED_EXTERNO, estadoBlink ? HIGH : LOW);
        digitalWrite(PIN_LED_AZUL, estadoBlink ? HIGH : LOW);
        
        delay(25);
    }
    digitalWrite(PIN_LED_EXTERNO, LOW);
    digitalWrite(PIN_LED_AZUL, LOW);

    valorReposo = suma / MUESTRAS;

    // El umbral se calcula automáticamente en el punto medio exacto
    // entre el valor de reposo y el tope de corte (4095)
    int margenDisponible = 4095 - valorReposo;
    umbralDeteccion = valorReposo + (margenDisponible * 0.50);

    Serial.printf("-> Nivel medido en reposo (Libre): %d\n", valorReposo);
    Serial.printf("-> Umbral dinamico asignado:     %d\n", umbralDeteccion);

    if (valorReposo > 3700) {
        Serial.println("[AVISO] El sensor recibe poca luz. Alinea los LEDs frente a frente.");
    } else {
        Serial.println("-> Calibracion EXITOSA: Sistema activo y vigilando.");
    }
    Serial.println("--------------------------------------------------");
}

void setup() {
    Serial.begin(115200);
    delay(500);

    // Configuramos ambos LEDs como salidas digitales
    pinMode(PIN_LED_EXTERNO, OUTPUT);
    pinMode(PIN_LED_AZUL, OUTPUT);

    // Resolución de 12 bits para lectura analógica (0 - 4095)
    analogReadResolution(12);

    Serial.println();
    Serial.println("==================================================");
    Serial.println("  ESP32: BARRERA IR CON LED TESTIGO EN GPIO 26    ");
    Serial.println("==================================================");
    Serial.println("-> Sensor IR:  GPIO 34 (P34)");
    Serial.println("-> LED Alerta: GPIO 26 (P26) y LED Azul (GPIO 2)");

    // Calibramos automáticamente al iniciar
    calibrarSensor();
}

void loop() {
    // Lectura de alta velocidad (toma solo unos microsegundos)
    int valorIR = analogRead(PIN_SENSOR_IR);
    unsigned long ahora = millis();

    // Verificamos si el haz fue interrumpido
    bool corteDetectado = (valorIR >= umbralDeteccion);

    // DISPARO DE DETECCIÓN: Transición de libre a cortado
    if (corteDetectado && !hazBloqueado) {
        hazBloqueado = true;
        contadorDetecciones++;

        // Encendemos tanto el LED testigo externo como el interno
        digitalWrite(PIN_LED_EXTERNO, HIGH);
        digitalWrite(PIN_LED_AZUL, HIGH);

        // Mantenemos encendido el LED por el tiempo visual mínimo (250ms)
        tiempoApagarLED = ahora + TIEMPO_LED_VISUAL;

        Serial.printf("[DETECCION #%lu] ¡¡ CORTE DETECTADO !! -> LED ENCENDIDO (IR: %d)\n", 
                      contadorDetecciones, valorIR);
    } 
    // El objeto terminó de pasar y el haz vuelve a estar libre
    else if (!corteDetectado && hazBloqueado) {
        hazBloqueado = false;
    }

    // Apagado automático del LED testigo una vez cumplido el pulso visual
    if (!hazBloqueado && ahora >= tiempoApagarLED) {
        digitalWrite(PIN_LED_EXTERNO, LOW);
        digitalWrite(PIN_LED_AZUL, LOW);
    }

    // Reporte periódico cada 1.5 segundos cuando no hay actividad
    if (ahora - tiempoUltimoReporte >= 1500) {
        tiempoUltimoReporte = ahora;
        if (!hazBloqueado) {
            Serial.printf("[Vigilancia] IR: %4d (Umbral: %4d) | Detecciones: %lu | LED: [APAGADO]\n", 
                          valorIR, umbralDeteccion, contadorDetecciones);
        }
    }
}
