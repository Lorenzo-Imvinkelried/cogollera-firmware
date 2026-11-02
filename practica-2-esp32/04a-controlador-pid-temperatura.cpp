#include <Arduino.h>
#include <DHT.h>

/*
 * ==============================================================================
 * EJERCICIO 04A: Controlador PID Discreto de Temperatura desde Cero
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.4. Práctica 2 con ESP32.pdf (Páginas 32 a 39)
 * Enlace Wokwi Oficial: https://wokwi.com/projects/442981504996359169
 *
 * Consigna:
 * "Implementar un lazo de control cerrado de temperatura utilizando un sensor
 *  DHT22 como variable de proceso (PV - Process Variable) y una salida PWM
 *  para modular la potencia de un calefactor (MV - Manipulated Variable).
 *  El algoritmo de control debe ser un PID implementado matemáticamente desde
 *  cero en tiempo discreto, incorporando protección contra saturación del
 *  acumulador integral (Anti-Windup)."
 *
 * Fundamentos Matemáticos del Algoritmo PID Discreto:
 * ----------------------------------------------------
 * 1. Ecuación Continua Ideal:
 *      u(t) = Kp * e(t) + Ki * ∫ e(τ) dτ + Kd * (de(t) / dt)
 *    Donde el error es e(t) = Setpoint - Entrada(t).
 *
 * 2. Discretización en el ESP32 (Periodo de Muestreo Δt = elapsedTime):
 *    - Acción Proporcional (P):
 *        P = Kp * error
 *        Reacciona proporcionalmente a la diferencia actual. A mayor error,
 *        mayor potencia instantánea.
 *
 *    - Acción Integral (I) - Aproximación Trapezoidal / Rectangular:
 *        cumError += error * Δt
 *        I = Ki * cumError
 *        Acumula la historia de los desvíos pasados para eliminar por completo
 *        el error en régimen permanente (Offset) que el modo P no puede corregir.
 *
 *    - Anti-Windup (Limitación del Acumulador Integral):
 *        Si el calefactor ya está al 100% (255) y la temperatura tarda en subir,
 *        la integral continuaría creciendo a valores astronómicos. Al alcanzar
 *        el setpoint, tardaría minutos en descargarse, provocando un overshoot
 *        inadmisible.
 *        Por eso se acota: cumError = constrain(cumError, -iMax, iMax),
 *        donde iMax = outMax / (Ki * 2).
 *
 *    - Acción Derivativa (D) - Diferencia Finita hacia Atrás de Euler:
 *        rateError = (error - lastError) / Δt
 *        D = Kd * rateError
 *        Mide la velocidad de aproximación al objetivo y actúa como un "freno",
 *        amortiguando las oscilaciones y el sobrepaso (overshoot).
 *
 *    - Salida Acotada (Saturación Física):
 *        output = constrain(P + I + D, 0, 255) -> Aplicado al ciclo de trabajo PWM.
 * ==============================================================================
 */

// --- 1. Asignación de Pines y Periféricos ---
const int DHT_PIN = 13; // Sensor de temperatura DHT22
const int PWM_PIN = 23; // Salida modulada hacia el actuador calefactor

// Configuración del módulo PWM (LEDC del ESP32)
const int PWM_CHANNEL    = 0;
const int PWM_FREQ       = 5000; // Frecuencia de 5 kHz (óptima para evitar zumbido audible)
const int PWM_RESOLUTION = 8;    // 8 bits de resolución: Valores de 0 a 255

// Instancia del sensor DHT22
DHT dht(DHT_PIN, DHT22);

// --- 2. Parámetros del Controlador PID ---
double setPoint = 24.0; // Temperatura objetivo deseada en °C

// Constantes de sintonización (Gains) sugeridas por la cátedra
double kP = 0.1;
double kI = 0.01;
double kD = 0.001;

// Límites de la salida física (PWM de 8 bits: 0 a 255)
const double outMin = 0.0;
const double outMax = 255.0;

// Límite Anti-Windup para el error acumulado
double iMax = outMax / (kI * 2.0);

// Variables de estado del lazo de control
double error = 0.0;
double lastError = 0.0;
double cumError = 0.0;
double rateError = 0.0;
double output = 0.0;

unsigned long previousTime = 0;
const unsigned long INTERVALO_PID_MS = 2000; // El DHT22 requiere muestreo cada 2 seg

/**
 * Función compatible para configurar el PWM LEDC según la versión del Core de ESP32.
 */
void configurarPWM() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);
#else
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN, PWM_CHANNEL);
#endif
}

/**
 * Función compatible para escribir el ciclo de trabajo PWM.
 */
void escribirPWM(int duty) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcWrite(PWM_PIN, duty);
#else
    ledcWrite(PWM_CHANNEL, duty);
#endif
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 04A: CONTROLADOR PID DISCRETO DESDE CERO");
    Serial.println("==================================================");

    // Inicialización de hardware
    dht.begin();
    configurarPWM();
    escribirPWM(0); // Inicia con el calefactor apagado

    previousTime = millis();

    Serial.println("[SETUP] Sensor DHT22 iniciado en GPIO 13.");
    Serial.println("[SETUP] Salida PWM configurada en GPIO 23 (8 bits, 5 kHz).");
    Serial.printf("[SETUP] Setpoint objetivo: %.2f °C\n", setPoint);
    Serial.printf("[SETUP] Ganancias PID -> Kp: %.3f | Ki: %.4f | Kd: %.4f\n", kP, kI, kD);
    Serial.printf("[SETUP] Límite Anti-Windup (iMax): %.2f\n", iMax);
    Serial.println("--------------------------------------------------");
}

void loop() {
    unsigned long currentTime = millis();
    unsigned long deltaMs = currentTime - previousTime;

    // Ejecutamos el lazo de control cada período de muestreo (2 segundos)
    if (deltaMs >= INTERVALO_PID_MS) {
        // 1. Cálculo del intervalo de tiempo exacto transcurrido Δt en segundos
        double elapsedTime = (double)deltaMs / 1000.0;

        // 2. Lectura de la Variable de Proceso (PV - Process Variable)
        float input = dht.readTemperature();

        if (isnan(input)) {
            Serial.println("[ERROR] Lectura inválida del DHT22. Manteniendo estado anterior.");
            return;
        }

        // 3. Cálculo del Error instantáneo
        error = setPoint - input;

        // 4. Modo Integral con protección Anti-Windup
        cumError += error * elapsedTime;
        cumError = constrain(cumError, -iMax, iMax); // Evita saturación profunda

        // 5. Modo Derivativo (velocidad de cambio del error)
        rateError = (error - lastError) / elapsedTime;

        // 6. Ecuación combinada del PID
        double pTerm = kP * error;
        double iTerm = kI * cumError;
        double dTerm = kD * rateError;

        output = pTerm + iTerm + dTerm;

        // 7. Saturación física de la señal de control (0 a 255)
        output = constrain(output, outMin, outMax);

        // 8. Aplicación de la señal de control al actuador PWM
        escribirPWM((int)output);

        // 9. Telemetría y diagnóstico en tiempo real
        Serial.printf("[PID] Temp: %.2f °C | SP: %.2f °C | Err: %+.2f | ", input, setPoint, error);
        Serial.printf("P: %+.2f | I: %+.2f | D: %+.2f | ", pTerm, iTerm, dTerm);
        Serial.printf("PWM: %3d/255 (%.1f %%)\n", (int)output, (output / 255.0) * 100.0);

        // 10. Actualización de variables de estado para el siguiente ciclo
        lastError = error;
        previousTime = currentTime;
    }
}

