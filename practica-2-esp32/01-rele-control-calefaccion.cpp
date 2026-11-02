#include <Arduino.h>
#include <DHT.h>

/*
 * ==============================================================================
 * EJERCICIO 01: Control de Carga de Potencia mediante Relé y Sensor DHT22
 * ==============================================================================
 * Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
 * Fuente: U5.4. Práctica 2 con ESP32.pdf (Páginas 4 a 10)
 * Enlace Wokwi Oficial: https://wokwi.com/projects/442971277123992577
 *
 * Consigna:
 * "Se posee un sensor DHT22 y un módulo de relé conectado al ESP32.
 *  Implementar un control termostático On/Off para un calefactor:
 *  - Si la temperatura medida es MENOR o IGUAL a 10 °C: Activar el relé
 *    (cierra el contacto Normalmente Abierto NO y enciende el calefactor).
 *  - Si la temperatura medida es MAYOR a 10 °C: Desactivar el relé
 *    (vuelve al contacto Normalmente Cerrado NC y apaga el calefactor,
 *     encendiendo una luz indicadora de reposo/standby)."
 *
 * Fundamentos del Relé Electromecánico:
 * -------------------------------------
 * 1. Aislamiento Galvánico:
 *    El ESP32 opera a 3.3V y sus GPIOs sólo suministran ~12 mA. No puede alimentar
 *    cargas de 220 VAC ni motores de alto amperaje. La bobina del relé y el
 *    optoacoplador aíslan eléctricamente la lógica digital de la etapa de potencia.
 *
 * 2. Bornes de Salida del Relé:
 *    - COM (Común): Borne donde ingresa la línea viva/fase de la red de potencia.
 *    - NO (Normally Open): Abierto en reposo. Se conecta cuando el pin del ESP32
 *      activa la bobina del relé. Conecta el calefactor.
 *    - NC (Normally Closed): Conectado en reposo. Se desconecta al activar el relé.
 *      Conecta el indicador de reposo (luz verde/piloto).
 *
 * 3. Lógica de Activación:
 *    En este circuito didáctico de cátedra, el relé se comanda en lógica activa
 *    en ALTO (HIGH = Bobina activada). Nota: En muchos módulos comerciales con
 *    optoacoplador la lógica suele ser activa en BAJO (LOW).
 * ==============================================================================
 */

// --- 1. Definición de Pines ---
const int DHT_PIN   = 13; // Pin de datos del sensor digital DHT22
const int RELAY_PIN = 23; // Pin digital conectado al IN del módulo relé

// --- 2. Parámetros del Sistema de Calefacción ---
const float TEMP_UMBRAL = 10.0; // Umbral de conmutación en grados Celsius (°C)
const unsigned long INTERVALO_LECTURA_MS = 2000; // DHT22 requiere mín. 2 seg entre lecturas

// Instanciamos el objeto DHT (tipo DHT22)
DHT dht(DHT_PIN, DHT22);

// Variable para temporización no bloqueante
unsigned long ultimaLectura = 0;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("==================================================");
    Serial.println(" EJERCICIO 01: CONTROL DE CALEFACCIÓN CON RELÉ   ");
    Serial.println("==================================================");

    // Configuramos el pin del relé como salida digital
    pinMode(RELAY_PIN, OUTPUT);

    // Estado inicial seguro: Relé apagado (calefactor desconectado)
    digitalWrite(RELAY_PIN, LOW);

    // Inicializamos el sensor DHT22
    dht.begin();

    Serial.println("[SETUP] Sensor DHT22 iniciado en GPIO 13.");
    Serial.println("[SETUP] Relé configurado en GPIO 23 (Salida).");
    Serial.printf("[SETUP] Umbral de activación: <= %.1f °C\n", TEMP_UMBRAL);
    Serial.println("--------------------------------------------------");
}

void loop() {
    unsigned long tiempoActual = millis();

    // Verificamos si transcurrió el intervalo de muestreo (2 segundos)
    if (tiempoActual - ultimaLectura >= INTERVALO_LECTURA_MS) {
        ultimaLectura = tiempoActual;

        // Leemos temperatura en grados Celsius (°C)
        float temperatura = dht.readTemperature();
        float humedad = dht.readHumidity();

        // Validamos lectura correcta (descarte de NaN)
        if (isnan(temperatura) || isnan(humedad)) {
            Serial.println("[ERROR] Falló la lectura del sensor DHT22. Verifique conexiones.");
            return;
        }

        Serial.printf("[SENSOR] Temp: %.2f °C | Humedad: %.1f %% | ", temperatura, humedad);

        // Algoritmo de control termostático On/Off
        if (temperatura <= TEMP_UMBRAL) {
            // Activar relé (NO cerrado -> CALEFACTOR ENCENDIDO)
            digitalWrite(RELAY_PIN, HIGH);
            Serial.printf("ESTADO: [CALENTANDO] -> Relé activado (HIGH)\n");
        } else {
            // Desactivar relé (NC cerrado -> CALEFACTOR APAGADO / STANDBY ENCENDIDO)
            digitalWrite(RELAY_PIN, LOW);
            Serial.printf("ESTADO: [STANDBY]    -> Relé desactivado (LOW)\n");
        }
    }
}

