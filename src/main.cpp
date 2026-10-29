#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

/*
 * ==============================================================================
 * EJEMPLO 09: Doble Barrera IR + Envío Inmediato a Supabase en Tiempo Real
 * ==============================================================================
 * Placa: ESP32-S V1.1 (38 pines)
 * 
 * ¿Cómo funciona la arquitectura Dual-Core (FreeRTOS)?
 * - NÚCLEO 1 (Loop principal): Muestrea los 2 sensores IR a miles de veces por segundo.
 *   CERO DELAYS. Reacciona en microsegundos y activa el LED testigo.
 * 
 * - NÚCLEO 0 (Tarea en segundo plano): Se conecta a Wi-Fi y envía la medición
 *   a Supabase por HTTPS (TLS seguro). El apretón criptográfico HTTPS nunca
 *   congela la barrera óptica: la trampa NUNCA queda ciega.
 *
 * Datos enviados a Supabase (tabla 'mediciones'):
 * - contador: Número acumulado de cogolleras capturadas
 * - tipo_evento: 'captura_cogollera' o 'latido_online'
 * - temperatura: Temperatura interna del procesador (°C)
 * - sensor1: Lectura analógica del haz superior
 * - sensor2: Lectura analógica del haz inferior
 */

// =============================================================================
// 1. CONFIGURACIÓN WI-FI Y SUPABASE
// =============================================================================
const char* ssid = "Horaim_Wiltel";
const char* password = "lonapaju4";

const char* SUPABASE_URL = "https://pqydjwpjclfnwfrzpsvb.supabase.co/rest/v1/mediciones";
const char* SUPABASE_KEY = "sb_publishable_jGjzJtPJeXKfqIwatNmRIA_oqSGTYRZ";

// Intervalo de latido periódico para telemetría (cada 60 segundos si no hay capturas)
const unsigned long INTERVALO_LATIDO_MS = 60000;

// =============================================================================
// 2. HARDWARE Y PINES
// =============================================================================
const int PIN_IR_1        = 34; // Haz 1 (Sensor superior - ADC1)
const int PIN_IR_2        = 35; // Haz 2 (Sensor inferior - ADC1)
const int PIN_LED_EXTERNO = 26; // LED Testigo en protoboard
const int PIN_LED_AZUL    = 2;  // LED azul integrado en la placa

// Duración del pulso de aviso visual del LED (ms)
const unsigned long TIEMPO_LED_VISUAL = 350;

// Variables de calibración dinámica de la barrera
int umbral1 = 3400;
int umbral2 = 3400;
int reposo1 = 0;
int reposo2 = 0;

// Contadores
unsigned long contadorCogolleras = 0;
unsigned long contadorDescartes  = 0;

bool cogolleraEnCurso = false;
bool descarteEnCurso  = false;
unsigned long tiempoApagarLED = 0;

// =============================================================================
// 3. ESTRUCTURA Y COLA ASÍNCRONA PARA SUPABASE
// =============================================================================
struct EventoCaptura {
    unsigned long conteo;
    float temperatura;
    int s1;
    int s2;
    char tipoEvento[25]; // "captura_cogollera" o "latido_online"
};

QueueHandle_t colaEnvios;

// Función que ejecuta el envío HTTPS a Supabase
void enviarHttpASupabase(const EventoCaptura& ev) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[SUPABASE] Wi-Fi desconectado. Reintentando conexion...");
        WiFi.reconnect();
        return;
    }

    WiFiClientSecure client;
    client.setInsecure(); // No validar certificados SSL para agilizar envío

    HTTPClient https;
    if (!https.begin(client, SUPABASE_URL)) {
        Serial.println("[SUPABASE] Error al inicializar cliente HTTPS.");
        return;
    }

    https.addHeader("Content-Type", "application/json");
    https.addHeader("apikey", SUPABASE_KEY);
    https.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
    https.addHeader("Prefer", "return=minimal");

    // Construcción del JSON
    String payload = "{";
    payload += "\"contador\":" + String(ev.conteo) + ",";
    payload += "\"tipo_evento\":\"" + String(ev.tipoEvento) + "\",";
    payload += "\"temperatura\":" + String(ev.temperatura, 2) + ",";
    payload += "\"sensor1\":" + String(ev.s1) + ",";
    payload += "\"sensor2\":" + String(ev.s2);
    payload += "}";

    Serial.printf("\n[SUPABASE -> CLOUD] Transmitiendo evento '%s' (#%lu)... ", ev.tipoEvento, ev.conteo);
    int codigoHttp = https.POST(payload);

    if (codigoHttp >= 200 && codigoHttp < 300) {
        Serial.printf("EXITO (HTTP %d)\n", codigoHttp);
    } else {
        Serial.printf("ERROR (HTTP %d): %s\n", codigoHttp, https.errorToString(codigoHttp).c_str());
    }

    https.end();
}

// Tarea independiente de FreeRTOS que corre en el Core 0
void tareaSegundoPlanoSupabase(void* pvParameters) {
    Serial.println("[CORE 0] Tarea de transmision en la nube iniciada.");
    EventoCaptura evento;

    for (;;) {
        // Espera a que haya un evento en la cola para transmitir
        if (xQueueReceive(colaEnvios, &evento, portMAX_DELAY) == pdTRUE) {
            enviarHttpASupabase(evento);
        }
    }
}

// =============================================================================
// 4. CALIBRACIÓN DE LA BARRERA
// =============================================================================
void calibrarAmbosSensores() {
    Serial.println("\n[CALIBRANDO] Evaluando ambos haces en reposo...");
    long suma1 = 0;
    long suma2 = 0;
    const int MUESTRAS = 40;

    for (int i = 0; i < MUESTRAS; i++) {
        suma1 += analogRead(PIN_IR_1);
        suma2 += analogRead(PIN_IR_2);

        bool blink = (i % 4 < 2);
        digitalWrite(PIN_LED_EXTERNO, blink ? HIGH : LOW);
        digitalWrite(PIN_LED_AZUL, blink ? HIGH : LOW);
        delay(25);
    }

    digitalWrite(PIN_LED_EXTERNO, LOW);
    digitalWrite(PIN_LED_AZUL, LOW);

    reposo1 = suma1 / MUESTRAS;
    reposo2 = suma2 / MUESTRAS;

    umbral1 = reposo1 + ((4095 - reposo1) * 0.50);
    umbral2 = reposo2 + ((4095 - reposo2) * 0.50);

    Serial.printf("-> Sensor 1 (P34): Reposo = %4d | Umbral = %4d\n", reposo1, umbral1);
    Serial.printf("-> Sensor 2 (P35): Reposo = %4d | Umbral = %4d\n", reposo2, umbral2);
    Serial.println("-> Calibracion EXITOSA: Sistema activo y conectado.");
    Serial.println("----------------------------------------------------------------------");
}

// =============================================================================
// 5. SETUP
// =============================================================================
void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(PIN_LED_EXTERNO, OUTPUT);
    pinMode(PIN_LED_AZUL, OUTPUT);
    analogReadResolution(12);

    Serial.println();
    Serial.println("======================================================================");
    Serial.println("   ESP32: TRAMPA COGOLLERA CON TELEMETRIA SUPABASE EN TIEMPO REAL     ");
    Serial.println("======================================================================");

    // 1. Crear cola de eventos asíncronos (hasta 10 capturas en cola)
    colaEnvios = xQueueCreate(10, sizeof(EventoCaptura));

    // 2. Iniciar conexión Wi-Fi
    Serial.printf("[WIFI] Conectando a red: %s\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
        delay(400);
        Serial.print(".");
        intentos++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WIFI] Conectado exitosamente. IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\n[WIFI] No se pudo conectar de inmediato. Reintentara en segundo plano.");
    }

    // 3. Lanzar tarea de transmisión en el Núcleo 0 (Core 0)
    xTaskCreatePinnedToCore(
        tareaSegundoPlanoSupabase,
        "TareaSupabase",
        6144,
        NULL,
        1,
        NULL,
        0 // Core 0
    );

    // 4. Calibrar sensores ópticos
    calibrarAmbosSensores();

    // 5. Enviar latido inicial de arranque a Supabase
    EventoCaptura latidoInicial;
    latidoInicial.conteo = 0;
    latidoInicial.temperatura = temperatureRead();
    latidoInicial.s1 = reposo1;
    latidoInicial.s2 = reposo2;
    strcpy(latidoInicial.tipoEvento, "latido_online");
    xQueueSend(colaEnvios, &latidoInicial, 0);
}

// =============================================================================
// 6. LOOP PRINCIPAL (CORRE EN EL CORE 1 A MÁXIMA VELOCIDAD)
// =============================================================================
unsigned long ultimoLatidoMs = 0;
unsigned long tiempoUltimoReporte = 0;

void loop() {
    int v1 = analogRead(PIN_IR_1);
    int v2 = analogRead(PIN_IR_2);
    unsigned long ahora = millis();

    bool corte1 = (v1 >= umbral1);
    bool corte2 = (v2 >= umbral2);

    // CASO 1: AMBOS HACES CORTADOS -> ¡COGOLLERA CAPTURADA!
    if (corte1 && corte2) {
        if (!cogolleraEnCurso) {
            cogolleraEnCurso = true;
            contadorCogolleras++;

            // Encendemos LEDs testigos
            digitalWrite(PIN_LED_EXTERNO, HIGH);
            digitalWrite(PIN_LED_AZUL, HIGH);
            tiempoApagarLED = ahora + TIEMPO_LED_VISUAL;

            Serial.println();
            Serial.println("******************************************************************");
            Serial.printf(" >>> [COGOLLERA CAPTURADA #%lu] ¡BICHO GRANDE DETECTADO! <<<\n", contadorCogolleras);
            Serial.printf("     Sensores cortados: Haz 1 (%d >= %d) | Haz 2 (%d >= %d)\n", v1, umbral1, v2, umbral2);
            Serial.println("******************************************************************");

            // Encolamos el evento para que el Core 0 lo transmita a Supabase sin congelar la barrera
            EventoCaptura captura;
            captura.conteo = contadorCogolleras;
            captura.temperatura = temperatureRead();
            captura.s1 = v1;
            captura.s2 = v2;
            strcpy(captura.tipoEvento, "captura_cogollera");
            xQueueSend(colaEnvios, &captura, 0);
        }
    }
    // CASO 2: SOLO UN HAZ CORTADO -> BICHO CHICO / POLVILLO (DESCARTADO)
    else if (corte1 || corte2) {
        if (!cogolleraEnCurso && !descarteEnCurso) {
            descarteEnCurso = true;
            contadorDescartes++;
            Serial.printf("[Descarte #%lu] Objeto chico ignorado (Haz 1: %d | Haz 2: %d)\n", 
                          contadorDescartes, v1, v2);
        }
    }
    // CASO 3: HACES LIBRES
    else {
        cogolleraEnCurso = false;
        descarteEnCurso  = false;
    }

    // Apagado del LED tras cumplirse el pulso visual
    if (!cogolleraEnCurso && ahora >= tiempoApagarLED) {
        digitalWrite(PIN_LED_EXTERNO, LOW);
        digitalWrite(PIN_LED_AZUL, LOW);
    }

    // Latido periódico de telemetría (cada 60 segundos)
    if (ahora - ultimoLatidoMs >= INTERVALO_LATIDO_MS) {
        ultimoLatidoMs = ahora;
        if (!corte1 && !corte2) {
            EventoCaptura latido;
            latido.conteo = contadorCogolleras;
            latido.temperatura = temperatureRead();
            latido.s1 = v1;
            latido.s2 = v2;
            strcpy(latido.tipoEvento, "latido_online");
            xQueueSend(colaEnvios, &latido, 0);
        }
    }

    // Monitor serie en reposo cada 3 segundos
    if (ahora - tiempoUltimoReporte >= 3000) {
        tiempoUltimoReporte = ahora;
        if (!corte1 && !corte2) {
            Serial.printf("[Vigilando] Haces: (S1: %4d, S2: %4d) | Cogolleras: %lu | Descartes: %lu\n", 
                          v1, v2, contadorCogolleras, contadorDescartes);
        }
    }
}
