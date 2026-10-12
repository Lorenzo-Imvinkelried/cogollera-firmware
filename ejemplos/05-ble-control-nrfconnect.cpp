#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <NimBLEDevice.h>

// =============================================================================
// EJEMPLO 05: SUPABASE CLOUD + CONTROL LOCAL VÍA NIMBLE BLUETOOTH
// =============================================================================
// Utiliza NimBLE-Arduino:
//  - Consume apenas ~20 KB de RAM (versus ~110 KB del BLE clásico).
//  - Deja libre suficiente memoria RAM para el apretón criptográfico HTTPS (TLS).
//  - Resuelve la convivencia entre Wi-Fi/HTTPS (Supabase) y Bluetooth sin caídas de memoria.
// =============================================================================

// Credenciales Wi-Fi (2.4 GHz)
const char* ssid = "Horaim_Wiltel";
const char* password = "lonapaju4";

// Credenciales Supabase
const char* SUPABASE_URL = "https://pqydjwpjclfnwfrzpsvb.supabase.co/rest/v1/mediciones";
const char* SUPABASE_KEY = "sb_publishable_jGjzJtPJeXKfqIwatNmRIA_oqSGTYRZ";

unsigned long intervaloEnvioSupabaseMs = 10000; // 10 segundos por defecto

const int PIN_LED = 2;
bool estadoLed = false;
unsigned long intervaloParpadeoMs = 0; // 0 = sin parpadeo, > 0 = parpadeo cada X ms
unsigned long ultimoParpadeo = 0;
bool estadoLedBlink = false;

unsigned long ultimaTransmisionSupabase = 0;
int contadorEnvios = 0;
int ultimoCodigoHttp = 0;

#define DISPOSITIVO_BLE_NOMBRE "ESP32_COGOLLERA_BLE"

#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_LED_UUID          "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_BLINK_UUID        "c3d82a10-2b47-4933-912b-3e5e4fa01928"
#define CHAR_INTERVALO_UUID    "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"
#define CHAR_CMD_UUID          "d9d75094-1188-466d-96e0-2580dfbf5535"
#define CHAR_TELEMETRIA_UUID   "a5b9e728-66a7-4c07-b37d-2495d469279a"

NimBLEServer* pServidor = nullptr;
NimBLECharacteristic* pCharLed = nullptr;
NimBLECharacteristic* pCharBlink = nullptr;
NimBLECharacteristic* pCharIntervalo = nullptr;
NimBLECharacteristic* pCharComando = nullptr;
NimBLECharacteristic* pCharTelemetria = nullptr;

bool clienteConectado = false;
bool clienteAnteriorConectado = false;

void destelloEnvio() {
    if (intervaloParpadeoMs > 0) return;
    for (int i = 0; i < 2; i++) {
        digitalWrite(PIN_LED, HIGH);
        delay(60);
        digitalWrite(PIN_LED, LOW);
        delay(60);
    }
}

void asignarNombreDescriptor(NimBLECharacteristic* pChar, const char* nombre) {
    NimBLEDescriptor* pDesc = pChar->createDescriptor("2901", NIMBLE_PROPERTY::READ);
    if (pDesc != nullptr) {
        pDesc->setValue(nombre);
    }
}

String generarResumenTelemetria() {
    float temp = temperatureRead();
    String wifiStatus = (WiFi.status() == WL_CONNECTED) ? "WiFi:OK" : "WiFi:OFF";
    return "Ciclo #" + String(contadorEnvios) +
           " | Temp: " + String(temp, 1) + " C" +
           " | " + wifiStatus +
           " | HTTP: " + String(ultimoCodigoHttp) +
           " | Int: " + String(intervaloEnvioSupabaseMs / 1000) + "s";
}

bool enviarMedicionASupabase(float temp, int cont) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[SUPABASE] Wi-Fi desconectado. Reintentando...");
        WiFi.reconnect();
        return false;
    }

    Serial.printf("[MEMORIA] Heap libre antes de HTTPS: %u bytes (Min: %u bytes)\n", ESP.getFreeHeap(), ESP.getMinFreeHeap());

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient https;
    https.setTimeout(10000);
    if (!https.begin(client, SUPABASE_URL)) {
        Serial.println("[SUPABASE] Error: No se pudo iniciar el cliente HTTPS.");
        return false;
    }

    https.addHeader("Content-Type", "application/json");
    https.addHeader("apikey", SUPABASE_KEY);
    https.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
    https.addHeader("Prefer", "return=minimal");

    String payload = "{\"temperatura\":" + String(temp, 2) + ",\"contador\":" + String(cont) + "}";

    Serial.printf("[SUPABASE] Enviando medicion #%d (%.2f °C)... ", cont, temp);
    int codigoHttp = https.POST(payload);
    ultimoCodigoHttp = codigoHttp;

    bool exito = (codigoHttp >= 200 && codigoHttp < 300);
    if (exito) {
        Serial.printf("EXITO (HTTP %d)\n", codigoHttp);
        destelloEnvio();
    } else if (codigoHttp > 0) {
        Serial.printf("RESPUESTA HTTP %d\n", codigoHttp);
    } else {
        Serial.printf("FALLO: %s\n", https.errorToString(codigoHttp).c_str());
    }

    https.end();
    client.stop();

    if (clienteConectado && pCharTelemetria != nullptr) {
        String datos = generarResumenTelemetria();
        pCharTelemetria->setValue(datos.c_str());
        pCharTelemetria->notify();
    }

    return exito;
}

class ServidorCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) override {
        clienteConectado = true;
        Serial.println("\n[BLE] >>> Celular conectado (nRF Connect) <<<");
    }

    void onDisconnect(NimBLEServer* pServer) override {
        clienteConectado = false;
        Serial.println("[BLE] >>> Celular desconectado <<<");
    }
};

class LedCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string val = pCharacteristic->getValue();
        String valor = String(val.c_str());
        valor.trim();

        intervaloParpadeoMs = 0;
        pCharBlink->setValue("0");

        if (valor == "1" || valor.equalsIgnoreCase("ON")) {
            estadoLed = true;
            digitalWrite(PIN_LED, HIGH);
            pCharacteristic->setValue("1");
            Serial.println("[BLE] LED encendido fijo.");
        } else if (valor == "0" || valor.equalsIgnoreCase("OFF")) {
            estadoLed = false;
            digitalWrite(PIN_LED, LOW);
            pCharacteristic->setValue("0");
            Serial.println("[BLE] LED apagado.");
        }
    }
};

class BlinkCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string val = pCharacteristic->getValue();
        String valor = String(val.c_str());
        valor.trim();
        valor.toLowerCase();
        valor.replace("ms", "");
        valor.trim();

        long ms = valor.toInt();
        if (ms == 0 && val.length() >= 2) {
            const uint8_t* data = (const uint8_t*)val.data();
            ms = data[0] | (data[1] << 8);
        }

        if (ms > 0) {
            intervaloParpadeoMs = ms;
            ultimoParpadeo = millis();
            Serial.printf("[BLE] Parpadeo LED fijado en: cada %lu ms\n", intervaloParpadeoMs);
            pCharacteristic->setValue(String(intervaloParpadeoMs).c_str());
        } else {
            intervaloParpadeoMs = 0;
            digitalWrite(PIN_LED, LOW);
            estadoLed = false;
            pCharLed->setValue("0");
            pCharacteristic->setValue("0");
            Serial.println("[BLE] Parpadeo desactivado.");
        }
    }
};

class IntervaloCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string val = pCharacteristic->getValue();
        String valor = String(val.c_str());
        valor.trim();
        valor.toLowerCase();
        valor.replace("ms", "");
        valor.trim();

        long nuevoIntervalo = valor.toInt();
        if (nuevoIntervalo == 0 && val.length() >= 2) {
            const uint8_t* data = (const uint8_t*)val.data();
            nuevoIntervalo = data[0] | (data[1] << 8);
        }

        if (nuevoIntervalo >= 1000 && nuevoIntervalo <= 3600000) {
            intervaloEnvioSupabaseMs = nuevoIntervalo;
            Serial.printf("[BLE] >>> Nuevo intervalo hacia Supabase: %lu ms (%.1f seg) <<<\n",
                          intervaloEnvioSupabaseMs, (float)intervaloEnvioSupabaseMs / 1000.0);
            pCharacteristic->setValue(String(intervaloEnvioSupabaseMs).c_str());
        } else {
            Serial.printf("[BLE] Error en intervalo: '%s'. Minimo 1000 ms.\n", valor.c_str());
        }
    }
};

class ComandoCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string val = pCharacteristic->getValue();
        String cmd = String(val.c_str());
        cmd.trim();

        Serial.printf("[BLE COMANDO] Recibido: '%s'\n", cmd.c_str());
        String respuesta = "";

        if (cmd.equalsIgnoreCase("ENVIAR") || cmd.equalsIgnoreCase("POST")) {
            contadorEnvios++;
            float temp = temperatureRead();
            bool ok = enviarMedicionASupabase(temp, contadorEnvios);
            respuesta = ok ? ("OK: Medida #" + String(contadorEnvios) + " enviada a Supabase") : "ERR: Fallo conexion Supabase";
        } else if (cmd.equalsIgnoreCase("LED ON")) {
            intervaloParpadeoMs = 0;
            pCharBlink->setValue("0");
            estadoLed = true;
            digitalWrite(PIN_LED, HIGH);
            pCharLed->setValue("1");
            respuesta = "OK: LED encendido";
        } else if (cmd.equalsIgnoreCase("LED OFF")) {
            intervaloParpadeoMs = 0;
            pCharBlink->setValue("0");
            estadoLed = false;
            digitalWrite(PIN_LED, LOW);
            pCharLed->setValue("0");
            respuesta = "OK: LED apagado";
        } else if (cmd.startsWith("BLINK ")) {
            long ms = cmd.substring(6).toInt();
            if (ms > 0) {
                intervaloParpadeoMs = ms;
                pCharBlink->setValue(String(intervaloParpadeoMs).c_str());
                respuesta = "OK: Parpadeo cada " + String(intervaloParpadeoMs) + " ms";
            } else {
                intervaloParpadeoMs = 0;
                digitalWrite(PIN_LED, LOW);
                pCharBlink->setValue("0");
                respuesta = "OK: Parpadeo apagado";
            }
        } else if (cmd.startsWith("SET_INTERVAL ")) {
            long ms = cmd.substring(13).toInt();
            if (ms >= 1000) {
                intervaloEnvioSupabaseMs = ms;
                pCharIntervalo->setValue(String(intervaloEnvioSupabaseMs).c_str());
                respuesta = "OK: Intervalo Supabase = " + String(intervaloEnvioSupabaseMs) + " ms";
            } else {
                respuesta = "ERR: Minimo 1000 ms";
            }
        } else if (cmd.equalsIgnoreCase("TEMP")) {
            float temp = temperatureRead();
            respuesta = "Temp interna: " + String(temp, 2) + " C";
        } else if (cmd.equalsIgnoreCase("STATUS") || cmd.equalsIgnoreCase("INFO")) {
            respuesta = generarResumenTelemetria();
        } else {
            respuesta = "ERR: Comandos validos: 'ENVIAR', 'BLINK 200', 'LED ON', 'LED OFF', 'TEMP', 'INFO'";
        }

        Serial.println("  -> Respuesta BLE: " + respuesta);
        pCharacteristic->setValue(respuesta.c_str());
    }
};

class TelemetriaCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic) override {
        String datos = generarResumenTelemetria();
        pCharacteristic->setValue(datos.c_str());
        Serial.println("[BLE] Telemetria leida manualmente desde nRF Connect.");
    }
};

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    Serial.println("\n=======================================================");
    Serial.println("   COGOLLERA IOT: SUPABASE + NIMBLE BLUETOOTH (LOW RAM)");
    Serial.println("=======================================================");

    Serial.printf("[BLE] Iniciando servidor NimBLE: %s...\n", DISPOSITIVO_BLE_NOMBRE);
    NimBLEDevice::init(DISPOSITIVO_BLE_NOMBRE);
    pServidor = NimBLEDevice::createServer();
    pServidor->setCallbacks(new ServidorCallbacks());

    NimBLEService* pServicio = pServidor->createService(SERVICE_UUID);

    pCharLed = pServicio->createCharacteristic(CHAR_LED_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    pCharLed->setCallbacks(new LedCallbacks());
    pCharLed->setValue("0");
    asignarNombreDescriptor(pCharLed, "Control Fijo LED (1=ON / 0=OFF)");

    pCharBlink = pServicio->createCharacteristic(CHAR_BLINK_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    pCharBlink->setCallbacks(new BlinkCallbacks());
    pCharBlink->setValue("0");
    asignarNombreDescriptor(pCharBlink, "Velocidad Parpadeo LED en ms (0=Apagado)");

    pCharIntervalo = pServicio->createCharacteristic(CHAR_INTERVALO_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    pCharIntervalo->setCallbacks(new IntervaloCallbacks());
    pCharIntervalo->setValue(String(intervaloEnvioSupabaseMs).c_str());
    asignarNombreDescriptor(pCharIntervalo, "Intervalo de Envio a Supabase en ms");

    pCharComando = pServicio->createCharacteristic(CHAR_CMD_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    pCharComando->setCallbacks(new ComandoCallbacks());
    pCharComando->setValue("Listo. Envia 'ENVIAR', 'BLINK 200' o 'INFO'");
    asignarNombreDescriptor(pCharComando, "Consola de Comandos de Texto");

    pCharTelemetria = pServicio->createCharacteristic(CHAR_TELEMETRIA_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pCharTelemetria->setCallbacks(new TelemetriaCallbacks());
    pCharTelemetria->setValue(generarResumenTelemetria().c_str());
    asignarNombreDescriptor(pCharTelemetria, "Telemetria Sensor en Tiempo Real");

    pServicio->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
    Serial.println("[BLE] Bluetooth NimBLE activo y listo.");

    Serial.printf("[WIFI] Conectando a red: %s\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 25) {
        delay(400);
        Serial.print(".");
        intentos++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WIFI] >>> Conectado con exito! <<<");
        Serial.print("[WIFI] IP Asignada: ");
        Serial.println(WiFi.localIP());

        contadorEnvios++;
        float temp = temperatureRead();
        enviarMedicionASupabase(temp, contadorEnvios);
        ultimaTransmisionSupabase = millis();
    } else {
        Serial.println("\n[WIFI] No se pudo conectar de inmediato. Reintentara en segundo plano.");
    }

    Serial.println("=======================================================\n");
}

void loop() {
    unsigned long ahora = millis();

    if (!clienteConectado && clienteAnteriorConectado) {
        delay(500);
        pServidor->startAdvertising();
        Serial.println("[BLE] Publicidad reiniciada.");
        clienteAnteriorConectado = clienteConectado;
    }
    if (clienteConectado && !clienteAnteriorConectado) {
        clienteAnteriorConectado = clienteConectado;
    }

    if (intervaloParpadeoMs > 0) {
        if (ahora - ultimoParpadeo >= intervaloParpadeoMs) {
            ultimoParpadeo = ahora;
            estadoLedBlink = !estadoLedBlink;
            digitalWrite(PIN_LED, estadoLedBlink ? HIGH : LOW);
        }
    }

    if (ahora - ultimaTransmisionSupabase >= intervaloEnvioSupabaseMs) {
        ultimaTransmisionSupabase = ahora;

        if (WiFi.status() == WL_CONNECTED) {
            contadorEnvios++;
            float temp = temperatureRead();
            enviarMedicionASupabase(temp, contadorEnvios);
        } else {
            Serial.println("[WIFI] Desconectado. Reintentando...");
            WiFi.reconnect();
        }
    }

    delay(10);
}
