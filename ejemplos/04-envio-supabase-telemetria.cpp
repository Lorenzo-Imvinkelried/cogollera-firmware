#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// =============================================================================
// EJEMPLO 04: ENVÍO PERIÓDICO DE MEDICIONES A SUPABASE
// =============================================================================

// Credenciales Wi-Fi (2.4 GHz)
const char* ssid = "Horaim_Wiltel";
const char* password = "lonapaju4";

// Credenciales de Supabase
const char* SUPABASE_URL = "https://pqydjwpjclfnwfrzpsvb.supabase.co/rest/v1/mediciones";
const char* SUPABASE_KEY = "sb_publishable_jGjzJtPJeXKfqIwatNmRIA_oqSGTYRZ";

// Intervalo de transmisión (10 segundos)
const unsigned long INTERVALO_ENVIO_MS = 10000;

const int LED_AZUL = 2;
unsigned long ultimaTransmision = 0;
int contador = 0;

void parpadearLed(int veces, int duracionMs) {
    for (int i = 0; i < veces; i++) {
        digitalWrite(LED_AZUL, HIGH);
        delay(duracionMs);
        digitalWrite(LED_AZUL, LOW);
        delay(duracionMs);
    }
}

void enviarMedicionASupabase(float temp, int cont) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[SUPABASE] Wi-Fi desconectado.");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure(); // No validar certificados SSL

    HTTPClient https;
    if (!https.begin(client, SUPABASE_URL)) {
        Serial.println("[SUPABASE] Error al iniciar conexion HTTPS.");
        return;
    }

    https.addHeader("Content-Type", "application/json");
    https.addHeader("apikey", SUPABASE_KEY);
    https.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
    https.addHeader("Prefer", "return=minimal");

    String payload = "{\"temperatura\":" + String(temp, 2) + ",\"contador\":" + String(cont) + "}";

    Serial.printf("[SUPABASE] Enviando ciclo #%d (%.2f °C)... ", cont, temp);
    int codigoHttp = https.POST(payload);

    if (codigoHttp >= 200 && codigoHttp < 300) {
        Serial.printf("EXITO (HTTP %d)\n", codigoHttp);
        parpadearLed(2, 60);
    } else {
        Serial.printf("ERROR (HTTP %d): %s\n", codigoHttp, https.errorToString(codigoHttp).c_str());
    }

    https.end();
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_AZUL, OUTPUT);
    digitalWrite(LED_AZUL, LOW);

    Serial.println("\n--- Iniciando conexion Wi-Fi ---");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConectado a Wi-Fi. IP: " + WiFi.localIP().toString());
}

void loop() {
    unsigned long ahora = millis();
    if (ahora - ultimaTransmision >= INTERVALO_ENVIO_MS) {
        ultimaTransmision = ahora;

        if (WiFi.status() == WL_CONNECTED) {
            contador++;
            float temp = temperatureRead();
            enviarMedicionASupabase(temp, contador);
        } else {
            Serial.println("Wi-Fi desconectado, reconectando...");
            WiFi.reconnect();
        }
    }
}
