#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/*
 * ==============================================================================
 * TRABAJO PRÁCTICO: CLASIFICADOR AUTOMÁTICO DE HUEVOS (3 CARRILES)
 * CON PANTALLA LCD 16x2 (I2C) Y CONTROL DE VELOCIDAD POR POTENCIÓMETRO
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Vega - Perdomo)
 * Carrera: Ingeniería en Sistemas de Información (ISI)
 *
 * MÁQUINA DE CLASIFICACIÓN FÍSICA (Fila Superior):
 *   [1. Motor Stepper A4988] ===> [2. Ultrasonido HC-SR04] ===> [3. Servomotor Desviador]
 *       (Avanza el Huevo)             (Mide el Calibre)          (0° Chico | 90° Medio | 180° Grande)
 *
 * PANEL DE OPERADOR Y HMI (Fila Inferior):
 *   [Botón Emergencia] + [Potenciómetro Stepper] <===> [ESP32] <===> [Pantalla LCD 16x2 I2C]
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

const int PIN_POT_VEL      = 35; // Entrada analógica (Potenciómetro conectado al motor stepper)
const int PIN_BTN_ESTOP    = 26; // Interrupción externa (Parada de emergencia)

const int PIN_I2C_SDA      = 21; // Bus I2C Datos para Pantalla LCD
const int PIN_I2C_SCL      = 22; // Bus I2C Reloj para Pantalla LCD

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
// 4. INSTANCIAS DE DISPOSITIVOS Y VARIABLES GLOBALES
// ==============================================================================
Servo servoDesviador;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Dirección I2C 0x27, 16 columnas x 2 filas

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
int velocidadCintaPorc  = 50;

// ==============================================================================
// 5. RUTINA DE INTERRUPCIÓN (ISR) - PARADA DE EMERGENCIA
// ==============================================================================
void IRAM_ATTR isrParadaEmergencia() {
    unsigned long ahora = millis();
    if (ahora > 1000 && (ahora - tiempoUltimoISR > 250)) {
        emergenciaActiva = !emergenciaActiva; // Conmuta estado de emergencia
        tiempoUltimoISR = ahora;
    }
}

// ==============================================================================
// 6. FUNCIONES DE ACTUALIZACIÓN DE PANTALLA LCD
// ==============================================================================

/**
 * Muestra en la pantalla LCD los contadores de los 3 tipos de huevos y la velocidad del stepper.
 */
void actualizarPantallaLCD(const char* estadoTexto = nullptr) {
    char linea0[17];
    char linea1[17];

    if (emergenciaActiva) {
        lcd.setCursor(0, 0);
        lcd.print("!EMERGENCIA E-STOP");
        lcd.setCursor(0, 1);
        lcd.print("CINTA DETENIDA  ");
        return;
    }

    if (estadoTexto != nullptr) {
        // Muestra mensaje temporal de estado
        snprintf(linea0, sizeof(linea0), "%-16s", estadoTexto);
    } else {
        // Línea 0: Total de huevos y velocidad actual del motor stepper fijada por el potenciómetro
        snprintf(linea0, sizeof(linea0), "TOT:%-4lu V:%3d%%", totalHuevos, velocidadCintaPorc);
    }

    // Línea 1: Contadores individuales de los 3 tipos de huevos (Chico, Mediano, Grande)
    snprintf(linea1, sizeof(linea1), "C:%-3lu M:%-3lu G:%-3lu", huevosChicos, huevosMedianos, huevosGrandes);

    lcd.setCursor(0, 0);
    lcd.print(linea0);
    lcd.setCursor(0, 1);
    lcd.print(linea1);
}

// ==============================================================================
// 7. FUNCIONES AUXILIARES DE HARDWARE Y GENERADOR RANDOM
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
 * Obtiene el calibre del huevo con variabilidad dinámica continua.
 */
float obtenerCalibreHuevo() {
    float distSensor = medirDistanciaHCSR04();
    float calibreSensor = DISTANCIA_BASE_CINTA_CM - distSensor;

    // Si el usuario movió manualmente el sensor en Wokwi, respetamos su medición
    if (distSensor < 6.0 || distSensor > 8.0) {
        return calibreSensor;
    }

    // De lo contrario, genera calibres variados al azar para que el servo se mueva dinámicamente
    int categoriaRandom = random(1, 4);
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
 * Lee el potenciómetro conectado al motor stepper y calcula el delay y porcentaje.
 */
int obtenerRetardoPasoUs() {
    int adcPot = analogRead(PIN_POT_VEL);
    // Calcula porcentaje de velocidad para mostrar en la pantalla LCD (20% a 100%)
    velocidadCintaPorc = map(adcPot, 0, 4095, 20, 100);
    // Mapea a tiempo de microsegundos: Pot al mínimo -> 3000 us (lento) | Pot al máximo -> 1000 us (rápido)
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
        delay(10);
    }
    anguloActualServo = anguloDestino;
    servoDesviador.write(anguloActualServo);
}

// ==============================================================================
// 8. SETUP
// ==============================================================================
void setup() {
    Serial.begin(115200);
    delay(400);

    // Semilla aleatoria a partir del ADC flotante
    randomSeed(analogRead(36) + millis());

    Serial.println("\n============================================================");
    Serial.println("  UTN SANTA FE - INGENIERÍA EN SISTEMAS DE INFORMACIÓN      ");
    Serial.println("  TECNOLOGÍAS PARA LA AUTOMATIZACIÓN - VEGA / PERDOMO       ");
    Serial.println("  TP: CLASIFICADOR DE HUEVOS CON PANTALLA LCD 16x2 (I2C)    ");
    Serial.println("============================================================\n");

    // Inicialización del Bus I2C y la Pantalla LCD
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CLASIFICADOR UTN");
    lcd.setCursor(0, 1);
    lcd.print("INICIANDO SIST..");
    delay(1200);

    // Pines del Stepper A4988
    pinMode(PIN_STEPPER_DIR, OUTPUT);
    pinMode(PIN_STEPPER_STEP, OUTPUT);
    digitalWrite(PIN_STEPPER_DIR, HIGH); // Sentido horario de avance

    // Pines del Sensor Ultrasónico HC-SR04
    pinMode(PIN_US_TRIG, OUTPUT);
    pinMode(PIN_US_ECHO, INPUT);

    // Pulsador de Parada de Emergencia (Interrupción Externa)
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
    servoDesviador.write(0); // Posición inicial en 0°
    anguloActualServo = 0;

    // Actualización inicial de la pantalla LCD
    actualizarPantallaLCD();

    Serial.println("[OK] Sistema y Pantalla LCD 16x2 iniciados.");
    Serial.println("============================================================\n");
}

// ==============================================================================
// 9. LOOP PRINCIPAL (MÁQUINA DE ESTADOS FINITOS)
// ==============================================================================
void loop() {
    // -------------------------------------------------------------------------
    // GESTIÓN DE EMERGENCIA ASÍNCRONA (ISR)
    // -------------------------------------------------------------------------
    if (emergenciaActiva) {
        if (estadoActual != ESTADO_PARADA_EMERGENCIA) {
            estadoPrevio = estadoActual;
            estadoActual = ESTADO_PARADA_EMERGENCIA;
            actualizarPantallaLCD();
            Serial.println("\n>>> [ALERTA] ¡PARADA DE EMERGENCIA ACTIVADA!");
            Serial.println(">>> Cinta detenida. Presione el botón rojo para reanudar.\n");
        }
        delay(100);
        return;
    } else if (estadoActual == ESTADO_PARADA_EMERGENCIA) {
        Serial.println(">>> [SISTEMA] Emergencia liberada. Reanudando...\n");
        estadoActual = estadoPrevio;
        actualizarPantallaLCD();
    }

    // -------------------------------------------------------------------------
    // FLUJO DE CLASIFICACIÓN
    // -------------------------------------------------------------------------
    switch (estadoActual) {

        // EVENTO 1: AVANCE DE CINTA CON CONTROL DE VELOCIDAD POR POTENCIÓMETRO
        case ESTADO_AVANZAR_CINTA: {
            int retardoUs = obtenerRetardoPasoUs();
            actualizarPantallaLCD(); // Muestra velocidad actual y contadores

            Serial.printf("[PASO 1: CINTA] Avanzando motor stepper 200 pasos (Vel: %d%%, delay: %d us)...\n",
                          velocidadCintaPorc, retardoUs);

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

        // EVENTO 2: MEDICIÓN DE CALIBRE Y CATEGORIZACIÓN
        case ESTADO_MEDIR_CALIBRE: {
            ultimoCalibreMedido = obtenerCalibreHuevo();

            Serial.printf("[SENSOR HC-SR04] ¡Huevo detectado! -> Calibre: %.1f cm\n", ultimoCalibreMedido);

            // Toma de decisión del carril y actualización de contadores
            if (ultimoCalibreMedido < 4.0) {
                anguloObjetivoServo = ANGULO_CARRIL_CHICO;
                huevosChicos++;
                actualizarPantallaLCD("HUEVO CHICO (S) ");
                Serial.printf("[DECISIÓN] -> HUEVO CHICO (S, %.1f cm)   ===> Carril 1 [Servo a %d°]\n",
                              ultimoCalibreMedido, anguloObjetivoServo);
            } else if (ultimoCalibreMedido <= 5.5) {
                anguloObjetivoServo = ANGULO_CARRIL_MEDIANO;
                huevosMedianos++;
                actualizarPantallaLCD("HUEVO MEDIANO(M)");
                Serial.printf("[DECISIÓN] -> HUEVO MEDIANO (M, %.1f cm) ===> Carril 2 [Servo a %d°]\n",
                              ultimoCalibreMedido, anguloObjetivoServo);
            } else {
                anguloObjetivoServo = ANGULO_CARRIL_GRANDE;
                huevosGrandes++;
                actualizarPantallaLCD("HUEVO GRANDE (L)");
                Serial.printf("[DECISIÓN] -> HUEVO GRANDE (L, %.1f cm)  ===> Carril 3 [Servo a %d°]\n",
                              ultimoCalibreMedido, anguloObjetivoServo);
            }

            totalHuevos++;
            estadoActual = ESTADO_POSICIONAR_DESVIADOR;
            break;
        }

        // EVENTO 3: APERTURA DE COMPUERTA / SERVO
        case ESTADO_POSICIONAR_DESVIADOR:
            Serial.printf("[PASO 2: DESVIADOR] Moviendo servomotor a %d°...\n", anguloObjetivoServo);
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
            actualizarPantallaLCD(); // Vuelve a mostrar pantalla de totales

            delay(600); // Pausa para visualizar posición
            estadoActual = ESTADO_AVANZAR_CINTA;
            break;

        case ESTADO_PARADA_EMERGENCIA:
            break;
    }
}
