#include <Arduino.h>
#include <ESP32Servo.h>

/*
 * ==============================================================================
 * TRABAJO PRÁCTICO: CLASIFICADOR AUTOMÁTICO DE HUEVOS (3 CARRILES)
 * CON GENERADOR ALEATORIO DE LOTES DE PRODUCCIÓN (VARIEDAD DINÁMICA)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Prof. Perdomo)
 * Carrera: Ingeniería en Sistemas de Información (ISI)
 *
 * MÁQUINA DE CLASIFICACIÓN FÍSICA (Fila Superior):
 *   [1. Motor Stepper A4988] ===> [2. Ultrasonido HC-SR04] ===> [3. Servomotor Desviador]
 *       (Avanza el Huevo)             (Mide el Calibre)          (0° Chico | 90° Medio | 180° Grande)
 *
 * PANEL DE OPERADOR (Fila Inferior):
 *   [Botón Emergencia] + [Potenciómetro Velocidad] <=========> [ESP32 DevKit]
 * ==============================================================================
 */

// ==============================================================================
// 1. ASIGNACIÓN DE PINES
// ==============================================================================
const int PIN_STEPPER_DIR  = 14; // Dirección de avance de cinta
const int PIN_STEPPER_STEP = 12; // Tren de pulsos para avanzar pasos

const int PIN_US_TRIG      = 5;  // Disparo del pulso ultrasónico
const int PIN_US_ECHO      = 18; // Recepción del eco ultrasónico

const int PIN_SERVO_PWM    = 33; // Señal PWM de control angular a 50 Hz

const int PIN_POT_VEL      = 35; // Entrada analógica (Potenciómetro de velocidad)
const int PIN_BTN_ESTOP    = 26; // Interrupción externa (Parada de emergencia)

// ==============================================================================
// 2. PARÁMETROS MECÁNICOS Y DE CALIBRE
// ==============================================================================
const float DISTANCIA_BASE_CINTA_CM = 12.0; // Distancia del sensor al fondo vacío
const int PASOS_POR_POSICION        = 200;  // 1 vuelta = 200 pasos (1 casillero)

// Ángulos de la compuerta desviadora del Servomotor
const int ANGULO_CARRIL_CHICO   = 0;   // 0°   -> Carril 1: Huevos Chicos  (< 4.0 cm)
const int ANGULO_CARRIL_MEDIANO = 90;  // 90°  -> Carril 2: Huevos Medianos(4.0 a 5.5 cm)
const int ANGULO_CARRIL_GRANDE  = 180; // 180° -> Carril 3: Huevos Grandes (> 5.5 cm)

// ==============================================================================
// 3. MÁQUINA DE ESTADOS FINITOS (FSM)
// ==============================================================================
enum EstadoSistema {
    ESTADO_AVANZAR_CINTA,
    ESTADO_MEDIR_CALIBRE,
    ESTADO_POSICIONAR_DESVIADOR,
    ESTADO_DESCARGA_CARRIL,
    ESTADO_PARADA_EMERGENCIA
};

EstadoSistema estadoActual = ESTADO_AVANZAR_CINTA;
EstadoSistema estadoPrevio = ESTADO_AVANZAR_CINTA;

// ==============================================================================
// 4. VARIABLES GLOBALES Y TELEMETRÍA
// ==============================================================================
Servo servoDesviador;

// Contadores de Producción
unsigned long totalHuevos    = 0;
unsigned long huevosChicos   = 0;
unsigned long huevosMedianos = 0;
unsigned long huevosGrandes  = 0;

// Variables de Control Asíncrono
volatile bool emergenciaActiva = false;
volatile unsigned long tiempoUltimoISR = 0;

int anguloObjetivoServo = 90;
int anguloActualServo   = 0;
float ultimoCalibreMedido = 0.0;

// ==============================================================================
// 5. RUTINA DE INTERRUPCIÓN (ISR) - PARADA DE EMERGENCIA
// ==============================================================================
void IRAM_ATTR isrParadaEmergencia() {
    unsigned long ahora = millis();
    if (ahora > 1000 && (ahora - tiempoUltimoISR > 250)) {
        emergenciaActiva = !emergenciaActiva;
        tiempoUltimoISR = ahora;
    }
}

// ==============================================================================
// 6. FUNCIONES AUXILIARES DE HARDWARE Y GENERADOR RANDOM
// ==============================================================================

/**
 * Mide distancia ultrasónica real con el HC-SR04 en centímetros.
 */
float medirDistanciaHCSR04() {
    digitalWrite(PIN_US_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_US_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_US_TRIG, LOW);

    long duracionUs = pulseIn(PIN_US_ECHO, HIGH, 25000); // Timeout 25ms
    if (duracionUs == 0) return DISTANCIA_BASE_CINTA_CM;

    return (duracionUs * 0.0343) / 2.0;
}

/**
 * GENERADOR ALEATORIO DE LLEGADA DE HUEVOS (Variabilidad Dinámica)
 * ----------------------------------------------------------------
 * Si el usuario no movió manualmente el sensor HC-SR04 en Wokwi, esta función
 * simula la llegada aleatoria de huevos de distintos tamaños:
 *   - 33% probabilidad: Huevo Chico   (Calibre entre 2.6 y 3.8 cm) -> Servo a 0°
 *   - 34% probabilidad: Huevo Mediano (Calibre entre 4.2 y 5.3 cm) -> Servo a 90°
 *   - 33% probabilidad: Huevo Grande  (Calibre entre 5.7 y 6.9 cm) -> Servo a 180°
 */
float obtenerCalibreHuevo() {
    // 1. Leemos el sensor físico HC-SR04
    float distSensor = medirDistanciaHCSR04();
    float calibreSensor = DISTANCIA_BASE_CINTA_CM - distSensor;

    // Si el usuario movió manualmente el slider del sensor en Wokwi (lejos del default ~7 cm)
    // respetamos su entrada manual:
    if (distSensor < 6.0 || distSensor > 8.0) {
        return calibreSensor;
    }

    // 2. Si el sensor está en su valor por defecto, generamos variedad aleatoria continua:
    int categoriaRandom = random(1, 4); // Elige 1, 2 o 3 al azar
    float calibreRandom = 0.0;

    if (categoriaRandom == 1) {
        calibreRandom = random(26, 39) / 10.0; // 2.6 a 3.8 cm (Chico)
    } else if (categoriaRandom == 2) {
        calibreRandom = random(42, 53) / 10.0; // 4.2 a 5.2 cm (Mediano)
    } else {
        calibreRandom = random(57, 69) / 10.0; // 5.7 a 6.8 cm (Grande)
    }

    return calibreRandom;
}

/**
 * Obtiene el tiempo entre pulsos según la posición del potenciómetro.
 */
int obtenerRetardoPasoUs() {
    int adcPot = analogRead(PIN_POT_VEL);
    // Mapea: Pot al mínimo -> 3000 us (lento) | Pot al máximo -> 1000 us (rápido)
    return map(adcPot, 0, 4095, 3000, 1000);
}

/**
 * Mueve el servomotor con suavidad hacia el ángulo objetivo.
 */
void moverServoSuave(int anguloDestino) {
    if (anguloDestino == anguloActualServo) return;

    int paso = (anguloDestino > anguloActualServo) ? 3 : -3;
    while (abs(anguloActualServo - anguloDestino) >= 3) {
        if (emergenciaActiva) break;
        anguloActualServo += paso;
        servoDesviador.write(anguloActualServo);
        delay(10); // Velocidad ágil de giro del servo
    }
    anguloActualServo = anguloDestino;
    servoDesviador.write(anguloActualServo);
}

/**
 * Imprime el resumen de producción en el Serial Monitor.
 */
void imprimirReporteProduccion() {
    Serial.println("---------------------------------------------------------");
    Serial.printf("  [PRODUCCIÓN TOTAL]: %lu huevos clasificados\n", totalHuevos);
    Serial.printf("  -> Carril 1 (CHICOS  < 4.0 cm): %lu (%.1f%%)\n",
                  huevosChicos, totalHuevos ? (huevosChicos * 100.0 / totalHuevos) : 0);
    Serial.printf("  -> Carril 2 (MEDIOS 4.0-5.5cm): %lu (%.1f%%)\n",
                  huevosMedianos, totalHuevos ? (huevosMedianos * 100.0 / totalHuevos) : 0);
    Serial.printf("  -> Carril 3 (GRANDES > 5.5 cm): %lu (%.1f%%)\n",
                  huevosGrandes, totalHuevos ? (huevosGrandes * 100.0 / totalHuevos) : 0);
    Serial.println("---------------------------------------------------------\n");
}

// ==============================================================================
// 7. SETUP
// ==============================================================================
void setup() {
    Serial.begin(115200);
    delay(400);

    // Inicializamos la semilla del generador aleatorio con ruido térmico del ADC
    randomSeed(analogRead(36) + millis());

    Serial.println("\n============================================================");
    Serial.println("  UTN SANTA FE - INGENIERÍA EN SISTEMAS DE INFORMACIÓN      ");
    Serial.println("  TECNOLOGÍAS PARA LA AUTOMATIZACIÓN - CÁTEDRA PERDOMO       ");
    Serial.println("  TP: CLASIFICADOR AUTOMÁTICO DE HUEVOS (3 CARRILES)        ");
    Serial.println("  [MODO DINÁMICO]: GENERADOR ALEATORIO DE LOTES ACTIVADO    ");
    Serial.println("============================================================\n");

    // Stepper A4988
    pinMode(PIN_STEPPER_DIR, OUTPUT);
    pinMode(PIN_STEPPER_STEP, OUTPUT);
    digitalWrite(PIN_STEPPER_DIR, HIGH); // Sentido de avance hacia el clasificador

    // Sensor Ultrasónico HC-SR04
    pinMode(PIN_US_TRIG, OUTPUT);
    pinMode(PIN_US_ECHO, INPUT);

    // Parada de Emergencia (Interrupción Externa)
    pinMode(PIN_BTN_ESTOP, INPUT_PULLUP);
    delay(50);
    emergenciaActiva = false;
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_ESTOP), isrParadaEmergencia, FALLING);

    // Servomotor Desviador
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    servoDesviador.setPeriodHertz(50);
    servoDesviador.attach(PIN_SERVO_PWM, 500, 2400);

    // Arranca en 0° (Carril Chico) para que el primer movimiento sea visible
    servoDesviador.write(0);
    anguloActualServo = 0;

    Serial.println("[OK] Sistema iniciado y listo para clasificar.");
    Serial.println("[INFO] Los huevos llegarán con calibres variados al azar.");
    Serial.println("============================================================\n");
}

// ==============================================================================
// 8. LOOP PRINCIPAL (MÁQUINA DE ESTADOS FINITOS)
// ==============================================================================
void loop() {
    // -------------------------------------------------------------------------
    // GESTIÓN DE EMERGENCIA ASÍNCRONA (ISR)
    // -------------------------------------------------------------------------
    if (emergenciaActiva) {
        if (estadoActual != ESTADO_PARADA_EMERGENCIA) {
            estadoPrevio = estadoActual;
            estadoActual = ESTADO_PARADA_EMERGENCIA;
            Serial.println("\n>>> [ALERTA] ¡PARADA DE EMERGENCIA ACTIVADA POR OPERADOR!");
            Serial.println(">>> Cinta detenida. Presione nuevamente el botón rojo para reanudar.\n");
        }
        delay(100);
        return;
    } else if (estadoActual == ESTADO_PARADA_EMERGENCIA) {
        Serial.println(">>> [SISTEMA] Emergencia liberada. Reanudando cinta transportadora...\n");
        estadoActual = estadoPrevio;
    }

    // -------------------------------------------------------------------------
    // FLUJO DE CLASIFICACIÓN
    // -------------------------------------------------------------------------
    switch (estadoActual) {

        // EVENTO 1: AVANCE DE CINTA
        case ESTADO_AVANZAR_CINTA: {
            int retardoUs = obtenerRetardoPasoUs();
            Serial.println("[PASO 1: CINTA] Avanzando 200 pasos hacia el sensor...");

            for (int i = 0; i < PASOS_POR_POSICION; i++) {
                if (emergenciaActiva) return;
                digitalWrite(PIN_STEPPER_STEP, HIGH);
                delayMicroseconds(retardoUs);
                digitalWrite(PIN_STEPPER_STEP, LOW);
                delayMicroseconds(retardoUs);
            }

            delay(200); // Pausa mecánica
            estadoActual = ESTADO_MEDIR_CALIBRE;
            break;
        }

        // EVENTO 2: MEDICIÓN DE CALIBRE (CON RANDOM AUTOMÁTICO)
        case ESTADO_MEDIR_CALIBRE: {
            // Obtenemos el calibre (aleatorio para variedad dinámica, o manual si se mueve el sensor)
            ultimoCalibreMedido = obtenerCalibreHuevo();

            Serial.printf("[SENSOR HC-SR04] ¡Llegó un huevo! -> CALIBRE (ALTURA): %.1f cm\n", ultimoCalibreMedido);

            // Toma de decisión del carril según el calibre medido
            if (ultimoCalibreMedido < 4.0) {
                anguloObjetivoServo = ANGULO_CARRIL_CHICO;
                huevosChicos++;
                Serial.printf("[DECISIÓN] -> HUEVO CHICO (S, %.1f cm)   ===> Carril 1 [Servo a %d°]\n",
                              ultimoCalibreMedido, anguloObjetivoServo);
            } else if (ultimoCalibreMedido <= 5.5) {
                anguloObjetivoServo = ANGULO_CARRIL_MEDIANO;
                huevosMedianos++;
                Serial.printf("[DECISIÓN] -> HUEVO MEDIANO (M, %.1f cm) ===> Carril 2 [Servo a %d°]\n",
                              ultimoCalibreMedido, anguloObjetivoServo);
            } else {
                anguloObjetivoServo = ANGULO_CARRIL_GRANDE;
                huevosGrandes++;
                Serial.printf("[DECISIÓN] -> HUEVO GRANDE (L, %.1f cm)  ===> Carril 3 [Servo a %d°]\n",
                              ultimoCalibreMedido, anguloObjetivoServo);
            }

            totalHuevos++;
            estadoActual = ESTADO_POSICIONAR_DESVIADOR;
            break;
        }

        // EVENTO 3: APERTURA DE COMPUERTA / SERVO (MOVIMIENTO VISIBLE)
        case ESTADO_POSICIONAR_DESVIADOR:
            Serial.printf("[PASO 2: DESVIADOR] Moviendo servomotor de %d° a %d°...\n",
                          anguloActualServo, anguloObjetivoServo);
            moverServoSuave(anguloObjetivoServo);
            estadoActual = ESTADO_DESCARGA_CARRIL;
            break;

        // EVENTO 4: DESCARGA HACIA EL CARRIL
        case ESTADO_DESCARGA_CARRIL:
            Serial.println("[PASO 3: DESCARGA] Empujando huevo hacia la rampa del carril...");
            for (int i = 0; i < 80; i++) {
                if (emergenciaActiva) return;
                digitalWrite(PIN_STEPPER_STEP, HIGH);
                delayMicroseconds(1500);
                digitalWrite(PIN_STEPPER_STEP, LOW);
                delayMicroseconds(1500);
            }

            Serial.println("[LISTO] Huevo encarrilado con éxito.");
            imprimirReporteProduccion();

            delay(600); // Pausa visual para ver el servo en su posición
            estadoActual = ESTADO_AVANZAR_CINTA;
            break;

        case ESTADO_PARADA_EMERGENCIA:
            break;
    }
}
