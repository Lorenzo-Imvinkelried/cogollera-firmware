#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// =============================================================================
// DATOS DE TU RED WI-FI (2.4 GHz)
// =============================================================================
const char* ssid = "Horaim_Wiltel";
const char* password = "lonapaju4";

// Servidor web en el puerto estándar 80
WebServer servidor(80);

// Pin del LED azul integrado
const int LED_AZUL = 2;
bool estadoLed = false; // Estado actual del LED (falso = apagado, verdadero = encendido)

// Destello rápido en el LED para indicar que se recargó la página,
// respetando el estado en el que ya estaba el LED
void destelloRecarga() {
    // Si estaba apagado, destella encendiéndose un instante
    // Si estaba prendido, hace un guiño apagándose un instante
    digitalWrite(LED_AZUL, estadoLed ? LOW : HIGH);
    delay(100);
    digitalWrite(LED_AZUL, estadoLed ? HIGH : LOW);
}

// Función que genera y responde la página web principal
void manejarPaginaPrincipal() {
    String ipCliente = servidor.client().remoteIP().toString();
    Serial.println(">>> [PAGINA RECARGADA] Visita desde IP: " + ipCliente);

    // Destello de actividad al recargar
    destelloRecarga();

    float tempC = temperatureRead();

    String html = "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>ESP32 - Panel de Control</title>";
    html += "<style>";
    html += "body { font-family: 'Segoe UI', sans-serif; background: #0f172a; color: #f8fafc; text-align: center; padding: 25px; }";
    html += ".card { background: #1e293b; border-radius: 18px; padding: 25px; max-width: 400px; margin: 0 auto; box-shadow: 0 10px 25px rgba(0,0,0,0.5); border: 1px solid #334155; }";
    html += "h1 { color: #38bdf8; margin-bottom: 5px; font-size: 24px; }";
    html += ".temp-box { background: #334155; border-radius: 12px; padding: 15px; margin: 20px 0; }";
    html += ".temp-val { font-size: 36px; font-weight: bold; color: #4ade80; }";
    html += ".estado-box { font-size: 18px; margin-bottom: 15px; }";
    html += ".btn { display: inline-block; padding: 16px 32px; border-radius: 12px; text-decoration: none; font-weight: bold; font-size: 18px; transition: 0.2s; box-shadow: 0 4px 12px rgba(0,0,0,0.3); }";
    html += ".btn-on { background: #2563eb; color: white; }";
    html += ".btn-off { background: #dc2626; color: white; }";
    html += ".btn:hover { opacity: 0.9; }";
    html += ".info { margin-top: 25px; font-size: 13px; color: #94a3b8; }";
    html += "</style></head><body>";

    html += "<div class='card'>";
    html += "<h1>ESP32 Cogollera</h1>";
    html += "<p style='color: #94a3b8; font-size: 14px;'>Red: " + String(ssid) + "</p>";

    html += "<div class='temp-box'>";
    html += "<div style='color: #cbd5e1; font-size: 14px;'>Temperatura del Chip</div>";
    html += "<div class='temp-val'>" + String(tempC, 1) + " °C</div>";
    html += "</div>";

    html += "<div class='estado-box'>";
    html += "Estado del LED: <b style='color: " + String(estadoLed ? "#60a5fa" : "#94a3b8") + ";'>" + String(estadoLed ? "ENCENDIDO" : "APAGADO") + "</b>";
    html += "</div>";

    // Botón de encender o apagar el LED
    if (estadoLed) {
        html += "<a class='btn btn-off' href='/led/toggle'>APAGAR LED</a>";
    } else {
        html += "<a class='btn btn-on' href='/led/toggle'>ENCENDER LED</a>";
    }

    html += "<div class='info'>";
    html += "<div>IP Local: <b>" + WiFi.localIP().toString() + "</b></div>";
    html += "<div>Tu IP: " + ipCliente + "</div>";
    html += "</div>";

    html += "</div></body></html>";

    servidor.send(200, "text/html", html);
}

// Función que conmuta el LED cuando tocas el botón de la página
void alternarLed() {
    estadoLed = !estadoLed;
    digitalWrite(LED_AZUL, estadoLed ? HIGH : LOW);
    Serial.println(estadoLed ? "-> Web: LED ENCENDIDO" : "-> Web: LED APAGADO");
    
    // Redirige automáticamente de vuelta a la página principal
    servidor.sendHeader("Location", "/");
    servidor.send(303);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_AZUL, OUTPUT);
    digitalWrite(LED_AZUL, LOW);

    Serial.println("\n==========================================");
    Serial.println("         CONEXION WI-FI DEL ESP32         ");
    Serial.println("==========================================");

    Serial.print("Conectando a la red: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 30) {
        delay(500);
        Serial.print(".");
        intentos++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n\n------------------------------------------");
        Serial.println(">>> CONEXION EXITOSA A WI-FI <<<");
        Serial.print("Direccion IP asignada: http://");
        Serial.println(WiFi.localIP());
        Serial.print("Intensidad de senal (RSSI): ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        Serial.println("------------------------------------------");

        // Rutas del servidor web
        servidor.on("/", manejarPaginaPrincipal);
        servidor.on("/led/toggle", alternarLed);
        servidor.on("/favicon.ico", []() { servidor.send(204); }); // Evita error en consola por el icono
        servidor.begin();
        Serial.println("Servidor Web HTTP iniciado en el puerto 80.\n");
    } else {
        Serial.println("\n\n[ERROR] No se pudo conectar a la red Wi-Fi.");
    }
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        servidor.handleClient();
    }
}
