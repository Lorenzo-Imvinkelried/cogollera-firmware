#include <Arduino.h>
#include <DHT.h>

/*
 * ==============================================================================
 * EJERCICIO 04B: Controlador PID Modular Orientado a Objetos (POO)
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.4. Práctica 2 con ESP32.pdf (Página 37)
 * Enlace Wokwi Oficial: https://wokwi.com/projects/442981504996359169
 *
 * Consigna:
 * "Modularizar el algoritmo de control PID implementándolo dentro de una clase C++
 *  (Programación Orientada a Objetos). La clase debe encapsular el estado interno
 *  (error acumulado, error previo, marcas temporales, ganancias y límites) y proveer
 *  una interfaz limpia e intuitiva mediante métodos:
 *  - setGains(kp, ki, kd)
 *  - setOutputLimits(min, max)
 *  - setSetpoint(sp)
 *  - compute(input)
 *  - reset()
 *  Esto permite instanciar múltiples controladores PID independientes en el mismo
 *  sistema (ej: uno para temperatura, otro para velocidad de un motor, etc.)."
 *
 * Ventajas de la Arquitectura POO en Sistemas Embebidos:
 * -----------------------------------------------------
 * 1. Encapsulamiento: No existen variables globales dispersas para el lazo de control.
 * 2. Reutilización de Código: La misma clase sirve para cualquier variable física.
 * 3. Mantenibilidad: Modificaciones al filtro derivativo o al anti-windup quedan
 *    centralizadas en la clase sin alterar la lógica de la aplicación principal.
 * ==============================================================================
 */

// ==============================================================================
// CLASE MODULAR: ControladorPID
// ==============================================================================
class ControladorPID {
private:
    double _kp;
    double _ki;
    double _kd;

    double _setpoint;
    double _outMin;
    double _outMax;
    double _iMax;

    double _lastError;
    double _cumError;
    unsigned long _lastTime;

    // Métricas para depuración y telemetría
    double _pTerm;
    double _iTerm;
    double _dTerm;

    void actualizarAntiWindup() {
        if (_ki > 0.0) {
            _iMax = _outMax / (_ki * 2.0);
        } else {
            _iMax = 0.0;
        }
    }

public:
    /**
     * Constructor del Controlador PID.
     * @param kp Ganancia Proporcional.
     * @param ki Ganancia Integral.
     * @param kd Ganancia Derivativa.
     * @param outMin Límite inferior de salida física (ej: 0 para PWM).
     * @param outMax Límite superior de salida física (ej: 255 para PWM de 8 bits).
     */
    ControladorPID(double kp, double ki, double kd, double outMin = 0.0, double outMax = 255.0)
        : _kp(kp), _ki(ki), _kd(kd), _setpoint(0.0), _outMin(outMin), _outMax(outMax),
          _lastError(0.0), _cumError(0.0), _lastTime(0), _pTerm(0.0), _iTerm(0.0), _dTerm(0.0) {
        actualizarAntiWindup();
    }

    /**
     * Configura el punto de consigna (Setpoint deseado).
     */
    void setSetpoint(double sp) {
        _setpoint = sp;
    }

    /**
     * Modifica las ganancias del controlador en tiempo de ejecución.
     */
    void setGains(double kp, double ki, double kd) {
        _kp = kp;
        _ki = ki;
        _kd = kd;
        actualizarAntiWindup();
    }

    /**
     * Define los límites físicos de saturación de la salida.
     */
    void setOutputLimits(double minLimit, double maxLimit) {
        _outMin = minLimit;
        _outMax = maxLimit;
        actualizarAntiWindup();
    }

    /**
     * Reinicia el estado interno acumulado del controlador (Anti-Windup y Derivativo).
     */
    void reset() {
        _lastError = 0.0;
        _cumError  = 0.0;
        _lastTime  = millis();
    }

    /**
     * Calcula la acción de control correspondiente a la medición actual.
     * @param input Valor medido de la variable de proceso (PV).
     * @return Señal de control acotada u(k) para el actuador.
     */
    double compute(double input) {
        unsigned long now = millis();
        if (_lastTime == 0) {
            _lastTime = now;
            return _outMin;
        }

        double dt = (double)(now - _lastTime) / 1000.0;
        if (dt <= 0.0) dt = 0.001; // Protección contra división por cero

        // 1. Cálculo del error
        double error = _setpoint - input;

        // 2. Término Proporcional
        _pTerm = _kp * error;

        // 3. Término Integral con Anti-Windup
        _cumError += error * dt;
        _cumError = constrain(_cumError, -_iMax, _iMax);
        _iTerm = _ki * _cumError;

        // 4. Término Derivativo
        double rateError = (error - _lastError) / dt;
        _dTerm = _kd * rateError;

        // 5. Señal de control combinada y saturación
        double output = _pTerm + _iTerm + _dTerm;
        output = constrain(output, _outMin, _outMax);

        // 6. Actualización de estado
        _lastError = error;
        _lastTime = now;

        return output;
    }

    // Getters para monitoreo
    double getSetpoint() const { return _setpoint; }
    double getPTerm()    const { return _pTerm; }
    double getITerm()    const { return _iTerm; }
    double getDTerm()    const { return _dTerm; }
};

// ==============================================================================
// PROGRAMA PRINCIPAL DE APLICACIÓN
// ==============================================================================

const int DHT_PIN = 13;
const int PWM_PIN = 23;

const int PWM_CHANNEL    = 0;
const int PWM_FREQ       = 5000;
const int PWM_RESOLUTION = 8;

DHT dht(DHT_PIN, DHT22);

// Instancia de nuestro controlador PID orientado a objetos
ControladorPID pidTemp(0.1, 0.01, 0.001, 0.0, 255.0);

unsigned long previousMillis = 0;
const unsigned long PERIODO_MUESTREO_MS = 2000;

void configurarPWM() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);
#else
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN, PWM_CHANNEL);
#endif
}

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
    Serial.println(" EJERCICIO 04B: CONTROLADOR PID MODULAR (CLASE POO)");
    Serial.println("==================================================");

    dht.begin();
    configurarPWM();
    escribirPWM(0);

    // Fijamos el objetivo en 24 °C
    pidTemp.setSetpoint(24.0);

    Serial.println("[SETUP] Clase ControladorPID instanciada y configurada.");
    Serial.printf("[SETUP] Setpoint establecido en: %.1f °C\n", pidTemp.getSetpoint());
    Serial.println("--------------------------------------------------");
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= PERIODO_MUESTREO_MS) {
        previousMillis = currentMillis;

        float temperatura = dht.readTemperature();
        if (isnan(temperatura)) {
            Serial.println("[ERROR] Lectura de DHT22 fallida.");
            return;
        }

        // Ejecutamos el método compute() de la clase encapsulada
        double controlPWM = pidTemp.compute(temperatura);

        // Aplicamos la acción de control al hardware
        escribirPWM((int)controlPWM);

        // Mostramos diagnóstico
        Serial.printf("[PID-POO] Temp: %.2f °C | SP: %.2f °C | PWM: %3d/255 | P: %+.2f | I: %+.2f | D: %+.2f\n",
                      temperatura, pidTemp.getSetpoint(), (int)controlPWM,
                      pidTemp.getPTerm(), pidTemp.getITerm(), pidTemp.getDTerm());
    }
}

