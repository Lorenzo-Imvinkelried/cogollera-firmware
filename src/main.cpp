#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// =============================================================================
// CONFIGURACIÓN DE RED WI-FI (2.4 GHz)
// =============================================================================
const char* ssid = "Horaim_Wiltel";
const char* password = "lonapaju4";

// =============================================================================
// CONFIGURACIÓN DE SUPABASE
// =============================================================================
const char* SUPABASE_URL = "https://pqydjwpjclfnwfrzpsvb.supabase.co/rest/v1/mediciones";
const char* SUPABASE_KEY = "sb_publishable_jGjzJtPJeXKfqIwatNmRIA_oqSGTYRZ";

// Intervalo de transmisión (10 segundos = 10000 ms)
const unsigned long INTERVALO_ENVIO_MS = 10000;

// =============================================================================
// HARDWARE Y VARIABLES GLOBALES
// =============================================================================
const int LED_AZUL = 2; // Pin del LED azul integrado en el ESP32

unsigned long ultimaTransmision = 0;
int contadorEnvios = 0;

// Destello en el LED al enviar datos con éxito
void destelloEnvio() {
    digitalWrite(LED_AZUL, HIGH);
    delay(70);
    digitalWrite(LED_AZUL, LOW);
    delay(70);
    digitalWrite(LED_AZUL, HIGH);
    delay(70);
    digitalWrite(LED_AZUL, LOW);
}

// =============================================================================
// ENVÍO DE DATOS A SUPABASE VÍA HTTPS POST
// =============================================================================
bool enviarMedicionASupabase(float temp, int cont) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[SUPABASE] Wi-Fi desconectado. Cancelando envio.");
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure(); // No verificar certificados SSL en el ESP32

    HTTPClient https;
    if (!https.begin(client, SUPABASE_URL)) {
        Serial.println("[SUPABASE] Error: No se pudo iniciar el cliente HTTPS.");
        return false;
    }

    https.addHeader("Content-Type", "application/json");
    https.addHeader("apikey", SUPABASE_KEY);
    https.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
    https.addHeader("Prefer", "return=minimal");

    // Construcción del JSON: {"temperatura": 37.25, "contador": 1}
    String payload = "{\"temperatura\":" + String(temp, 2) + ",\"contador\":" + String(cont) + "}";

    Serial.printf("[SUPABASE] Transmitiendo ciclo #%d (Temp: %.2f °C)... ", cont, temp);
    int codigoHttp = https.POST(payload);

    bool exito = (codigoHttp >= 200 && codigoHttp < 300);
    if (exito) {
        Serial.printf("EXITO (HTTP %d)\n", codigoHttp);
        destelloEnvio();
    } else if (codigoHttp > 0) {
        Serial.printf("RESPUESTA HTTP %d\n", codigoHttp);
        String cuerpo = https.getString();
        if (cuerpo.length() > 0) {
            Serial.println("  Detalle de respuesta: " + cuerpo);
        }
    } else {
        Serial.printf("FALLO DE CONEXION: %s\n", https.errorToString(codigoHttp).c_str());
    }

    https.end();
    return exito;
}

// =============================================================================
// SETUP Y LOOP
// =============================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_AZUL, OUTPUT);
    digitalWrite(LED_AZUL, LOW);

    Serial.println("\n==========================================");
    Serial.println("   COGOLLERA IOT - TELEMETRIA SUPABASE    ");
    Serial.println("==========================================");
    Serial.printf("Conectando a Wi-Fi: %s\n", ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 30) {
        delay(500);
        Serial.print(".");
        intentos++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n>>> CONEXION WI-FI EXITOSA <<<");
        Serial.print("IP Asignada: ");
        Serial.println(WiFi.localIP());
        Serial.printf("Senal RSSI: %d dBm\n", WiFi.RSSI());
        Serial.printf("Transmision configurada cada %lu segundos hacia Supabase.\n\n", INTERVALO_ENVIO_MS / 1000);

        // Envío inicial al encender
        contadorEnvios++;
        float temp = temperatureRead();
        enviarMedicionASupabase(temp, contadorEnvios);
        ultimaTransmision = millis();
    } else {
        Serial.println("\n[ERROR] No se pudo conectar a la red Wi-Fi. Revisa credenciales.");
    }
}

void loop() {
    unsigned long ahora = millis();

    // Envío periódico cada INTERVALO_ENVIO_MS
    if (ahora - ultimaTransmision >= INTERVALO_ENVIO_MS) {
        ultimaTransmision = ahora;

        if (WiFi.status() == WL_CONNECTED) {
            contadorEnvios++;
            float temp = temperatureRead();
            enviarMedicionASupabase(temp, contadorEnvios);
        } else {
            Serial.println("[WIFI] Conexion perdida. Reintentando reconectar...");
            WiFi.reconnect();
        }
    }
}
