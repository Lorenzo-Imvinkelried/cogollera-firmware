#include <Arduino.h>

/*
 * ==============================================================================
 * EJEMPLO 000: Consola Interactiva de Test de Pines ESP32 (Control por Terminal)
 * ==============================================================================
 * Placa: ESP32-S V1.1 (38 pines) - Vista Dorsal / Panza Abajo
 * 
 * Uso:
 * Abre el monitor serie (pio device monitor) y escribe el pin que quieras encender:
 * - Escribe el número del GPIO (ej: 26, 13, 2, 33 o p26, p13...) y presiona ENTER.
 * - Se enciende UN SOLO PIN a la vez a 3.3V de forma FIJA para medir seguro.
 * - Escribe 'off' o '0' para apagarlo.
 * - Escribe 'auto' para ver el efecto secuencial rápido (1 pin a la vez, 120ms).
 * - Escribe 'ayuda' o 'lista' para ver todos los pines disponibles.
 */

struct PinInfo {
    int gpio;
    const char* ubicacion;
};

// Lista de pines seguros para salida digital
const PinInfo PINES_VALIDOS[] = {
    // --- LADO DERECHO ---
    {13, "Lado DERECHO - Pin 05 (P13)"},
    {12, "Lado DERECHO - Pin 07 (P12)"},
    {14, "Lado DERECHO - Pin 08 (P14)"},
    {27, "Lado DERECHO - Pin 09 (P27)"},
    {26, "Lado DERECHO - Pin 10 (P26)"},
    {25, "Lado DERECHO - Pin 11 (P25)"},
    {33, "Lado DERECHO - Pin 12 (P33)"},
    {32, "Lado DERECHO - Pin 13 (P32)"},

    // --- LADO IZQUIERDO ---
    {15, "Lado IZQUIERDO - Pin 04 (P15)"},
    {2,  "Lado IZQUIERDO - Pin 05 (P2 - LED Azul)"},
    {4,  "Lado IZQUIERDO - Pin 07 (P4)"},
    {16, "Lado IZQUIERDO - Pin 08 (P16)"},
    {17, "Lado IZQUIERDO - Pin 09 (P17)"},
    {5,  "Lado IZQUIERDO - Pin 10 (P5)"},
    {18, "Lado IZQUIERDO - Pin 11 (P18)"},
    {19, "Lado IZQUIERDO - Pin 12 (P19)"},
    {21, "Lado IZQUIERDO - Pin 14 (P21)"},
    {22, "Lado IZQUIERDO - Pin 17 (P22)"},
    {23, "Lado IZQUIERDO - Pin 18 (P23)"}
};

const int TOTAL_PINES = sizeof(PINES_VALIDOS) / sizeof(PINES_VALIDOS[0]);
int pinActivoActual = -1;

void apagarTodosLosPines() {
    for (int i = 0; i < TOTAL_PINES; i++) {
        digitalWrite(PINES_VALIDOS[i].gpio, LOW);
    }
    pinActivoActual = -1;
}

void mostrarMenuAyuda() {
    Serial.println();
    Serial.println("======================================================================");
    Serial.println("      CONSOLA INTERACTIVA DE TEST DE SALIDAS (ESP32-S 38 PINES)       ");
    Serial.println("======================================================================");
    Serial.println("Comandos disponibles (1 solo pin activo a la vez por seguridad):");
    Serial.println(" -> Escribe el numero del pin (ej: 26, 13, 2, 33) para darle 3.3V fijo.");
    Serial.println(" -> 'off' o '0'   : Apaga todos los pines (0V).");
    Serial.println(" -> 'auto'        : Modo secuencial rapido (1 por 1, 120ms cada uno).");
    Serial.println(" -> 'lista'       : Muestra la lista de pines con su posicion fisica.");
    Serial.println("----------------------------------------------------------------------");
    Serial.print("Ingresa tu opcion: ");
}

void mostrarListaPines() {
    Serial.println();
    Serial.println("--- PINES DISPONIBLES PARA SALIDA DIGITAL ---");
    for (int i = 0; i < TOTAL_PINES; i++) {
        Serial.printf(" [GPIO %2d] -> %s\n", PINES_VALIDOS[i].gpio, PINES_VALIDOS[i].ubicacion);
    }
    Serial.println("---------------------------------------------");
}

void modoAutoRapido() {
    Serial.println("\n[AUTO] Ejecutando barrido secuencial rapido (1 pin a la vez)...");
    apagarTodosLosPines();
    for (int ronda = 0; ronda < 2; ronda++) {
        for (int i = 0; i < TOTAL_PINES; i++) {
            digitalWrite(PINES_VALIDOS[i].gpio, HIGH);
            delay(120);
            digitalWrite(PINES_VALIDOS[i].gpio, LOW);
        }
    }
    Serial.println("[AUTO] Finalizado. Volviendo a control manual.");
    Serial.print("\nIngresa tu opcion: ");
}

void procesarComando(String cmd) {
    cmd.trim();
    if (cmd.length() == 0) return;

    cmd.toLowerCase();

    // Comando apagar
    if (cmd == "off" || cmd == "0" || cmd == "apagar") {
        apagarTodosLosPines();
        Serial.println("\n[TODO APAGADO] Todos los pines estan en 0V.");
        Serial.print("Ingresa otro pin o comando: ");
        return;
    }

    // Comando ayuda
    if (cmd == "ayuda" || cmd == "help" || cmd == "?") {
        mostrarMenuAyuda();
        return;
    }

    // Comando lista
    if (cmd == "lista" || cmd == "list") {
        mostrarListaPines();
        Serial.print("Ingresa el pin a encender: ");
        return;
    }

    // Comando automático (1 por 1)
    if (cmd == "auto") {
        modoAutoRapido();
        return;
    }

    // Si pusieron 'p26' en vez de '26', quitamos la 'p'
    if (cmd.startsWith("p")) {
        cmd = cmd.substring(1);
    }

    int pinSolicitado = cmd.toInt();

    // Comprobamos si es un pin de solo entrada
    if (pinSolicitado == 34 || pinSolicitado == 35 || pinSolicitado == 36 || pinSolicitado == 39) {
        Serial.printf("\n[ERROR] GPIO %d es un pin de SOLO ENTRADA (Input-Only).", pinSolicitado);
        Serial.println(" No tiene circuiteria para entregar 3.3V.");
        Serial.print("Ingresa otro pin valido: ");
        return;
    }

    // Comprobamos si es un pin de memoria Flash SPI
    if (pinSolicitado >= 6 && pinSolicitado <= 11) {
        Serial.printf("\n[PELIGRO] GPIO %d pertenece a la memoria Flash SPI interna.", pinSolicitado);
        Serial.println(" No se debe tocar para no tildar la placa.");
        Serial.print("Ingresa otro pin valido: ");
        return;
    }

    // Buscamos si es un pin de salida válido
    int indiceEncontrado = -1;
    for (int i = 0; i < TOTAL_PINES; i++) {
        if (PINES_VALIDOS[i].gpio == pinSolicitado) {
            indiceEncontrado = i;
            break;
        }
    }

    if (indiceEncontrado != -1) {
        apagarTodosLosPines(); // SEGURIDAD: Apagamos el anterior para que SOLO HAYA UNO ACTIVO
        pinActivoActual = pinSolicitado;
        digitalWrite(pinActivoActual, HIGH);

        Serial.println();
        Serial.println("**********************************************************************");
        Serial.printf(" [ACTIVO -> 3.3V FIJO] GPIO %d ENCENDIDO (Unico pin energizado)\n", pinActivoActual);
        Serial.printf(" Ubicacion fisica: %s\n", PINES_VALIDOS[indiceEncontrado].ubicacion);
        Serial.println(" El pin quedara en 3.3V de forma indefinida hasta que elijas otro.");
        Serial.println("**********************************************************************");
        Serial.print("Mide tranquilo con el multimetro. Para apagar escribe 'off' o ingresa otro pin: ");
    } else {
        Serial.printf("\n[DESCONOCIDO] '%s' no es un pin valido.", cmd.c_str());
        Serial.println(" Escribe 'lista' para ver los pines disponibles.");
        Serial.print("Ingresa tu opcion: ");
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Inicializamos todos los pines como salidas en nivel bajo
    for (int i = 0; i < TOTAL_PINES; i++) {
        pinMode(PINES_VALIDOS[i].gpio, OUTPUT);
        digitalWrite(PINES_VALIDOS[i].gpio, LOW);
    }

    mostrarMenuAyuda();
}

void loop() {
    // Lectura de comandos enviados por el monitor serie
    if (Serial.available() > 0) {
        String entrada = Serial.readStringUntil('\n');
        procesarComando(entrada);
    }
}
