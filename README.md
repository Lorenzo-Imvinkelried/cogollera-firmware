# Proyecto ESP32 - Cogollera

Guía explicativa en español sobre la estructura del proyecto, las carpetas y el origen de todas las herramientas y dependencias instaladas.

---

## 1. Aclaración sobre los README en inglés existentes
Al inicializar un proyecto con **PlatformIO**, se generan automáticamente tres archivos `README` en inglés como guía técnica predeterminada:
- `include/README`: Explica cómo organizar archivos de cabecera (`.h`).
- `lib/README`: Explica cómo crear librerías propias o módulos privados.
- `test/README`: Explica cómo ejecutar pruebas unitarias.

Este documento (`README.md`) resume y reemplaza la necesidad de consultar esos archivos en inglés.

---

## 2. Estructura de Carpetas del Proyecto

| Carpeta / Archivo | ¿Para qué sirve? |
| :--- | :--- |
| **`src/`** | Carpeta principal de código fuente. Aquí se encuentra el archivo de entrada [`src/main.cpp`](file:///d:/Usuario/Desktop/cogollera-firmware/src/main.cpp). |
| **`circuitos/`** | Diagramas esquemáticos y guías paso a paso de conexión en protoboard (`.txt`). |
| **`ejemplos/`** | Ejemplos independientes y modulares de prueba (LEDs, Wi-Fi, BLE, Supabase, Infrarrojos). |
| **`include/`** | Para archivos de cabecera C/C++ (`.h`). Sirve para compartir declaraciones de funciones, estructuras o constantes entre varios archivos sin duplicar código. |
| **`lib/`** | Para librerías locales y módulos propios del proyecto. Si creas un módulo específico (por ejemplo, control de sensores o reles), se coloca aquí en su propia subcarpeta. |
| **`test/`** | Para tests unitarios automatizados que verifican el funcionamiento del código en el microcontrolador. |
| **`platformio.ini`** | Archivo de configuración central del proyecto. Especifica la placa (`esp32dev`), la plataforma (`espressif32`) y el framework (`arduino`). |
| **`.pio/`** | Carpeta generada automáticamente durante la compilación. Guarda los archivos binarios intermedios (`.o`), librerías compiladas y el firmware final (`firmware.bin`). Está excluida en [`.gitignore`](file:///d:/Usuario/Desktop/proyecto-cogollera/.gitignore). |
| **`GEMINI.md`** y **`.agents/`** | Reglas del proyecto para el asistente de IA (convención de commits en imperativo y en español simple). |

---

## 3. ¿Qué se instaló y de dónde proviene?

Todo el ecosistema fue descargado y administrado automáticamente por **PlatformIO** desde su registro oficial seguro ([PlatformIO Registry](https://registry.platformio.org)) y los repositorios oficiales del fabricante del chip ([Espressif Systems](https://github.com/espressif)).

Los paquetes se descargan y almacenan a nivel de usuario en tu disco local en:  
`C:\Users\Usuario\.platformio\`

### Detalle de los componentes instalados:

1. **Plataforma Espressif 32 (`espressif32`)**
   - **Origen:** [PlatformIO Registry / Espressif](https://registry.platformio.org/platforms/platformio/espressif32)
   - **Ubicación local:** `C:\Users\Usuario\.platformio\platforms\espressif32\`
   - **Qué hace:** Provee la integración con el hardware de la familia ESP32 (soporte de placas, memorias, esquemas de partición).

2. **Framework Arduino para ESP32 (`framework-arduinoespressif32`)**
   - **Origen:** Repositorio oficial de Espressif ([arduino-esp32](https://github.com/espressif/arduino-esp32)).
   - **Ubicación local:** `C:\Users\Usuario\.platformio\packages\framework-arduinoespressif32\`
   - **Qué hace:** Es la capa de software que permite programar el ESP32 usando la sintaxis y funciones de Arduino (`setup()`, `loop()`, `digitalWrite()`, `Serial`, `WiFi`, etc.).

3. **Compilador Xtensa (`toolchain-xtensa-esp32`)**
   - **Origen:** Compilador GCC adaptado por Espressif para la arquitectura Xtensa.
   - **Ubicación local:** `C:\Users\Usuario\.platformio\packages\toolchain-xtensa-esp32\`
   - **Qué hace:** Convierte el código C/C++ escrito por el usuario en código de máquina (binario) ejecutable por el procesador del ESP32.

4. **Herramienta de Flasheo (`tool-esptoolpy`)**
   - **Origen:** Herramienta oficial de Espressif en Python ([esptool](https://github.com/espressif/esptool)).
   - **Ubicación local:** `C:\Users\Usuario\.platformio\packages\tool-esptoolpy\`
   - **Qué hace:** Comunica la computadora con el ESP32 a través del puerto serie USB para subir el firmware compilado y gestionar la memoria Flash del chip.

5. **Motor de Construcción (`tool-scons`)**
   - **Origen:** Sistema de construcción SCons para PlatformIO.
   - **Ubicación local:** `C:\Users\Usuario\.platformio\packages\tool-scons\`
   - **Qué hace:** Coordina las tareas de compilación, orden de dependencias y generación de los archivos binarios.

---

## 4. Comandos Básicos de Uso

- **Compilar el proyecto:**
  ```powershell
  pio run
  ```
- **Compilar y subir al ESP32 (con el cable USB conectado):**
  ```powershell
  pio run --target upload
  ```
- **Abrir el monitor serie (para ver mensajes de `Serial.println`):**
  ```powershell
  pio device monitor
  ```

---

## 5. Ejemplos Modulares (`ejemplos/`)

El proyecto cuenta con ejemplos de prueba independientes y listos para usar:
0. `000-test-pines-salidas.cpp`: Test de diagnóstico general (árbol de navidad) para medir con multímetro o LED cada salida digital segura de la placa.
1. `01-led-interno-onoff.cpp`: Encendido y apagado básico del LED integrado (GPIO 2).
2. `02-temperatura-interna.cpp`: Lectura de la temperatura interna del chip con `temperatureRead()`.
3. `03-conexion-wifi-servidor-web.cpp`: Conexión Wi-Fi y servidor web local HTTP para control.
4. `04-envio-supabase-telemetria.cpp`: Conexión Wi-Fi y subida periódica de telemetría a Supabase por HTTPS.
5. `05-ble-control-nrfconnect.cpp`: Integración completa de telemetría hacia Supabase (Wi-Fi + HTTPS) con control local en tiempo real por Bluetooth Low Energy usando la app **nRF Connect**.
6. `06-led-externo-onoff.cpp`: Control de parpadeo de LED externo en protoboard (GPIO 26) con resistencia de protección y diagnóstico sincronizado con el LED interno.
7. `07-detector-infrarrojo.cpp`: Detector de barrera/obstáculos infrarrojo (LED IR + Fototransistor) usando entrada analógica (GPIO 34) con umbral calibrable y alerta visual por LED.
8. `08-doble-barrera-cogollera.cpp`: Doble barrera infrarroja (GPIO 34 y GPIO 35) con clasificador de tamaño para detectar capturas de insectos grandes (cogolleras) y descartar bichos pequeños o polvillo.

### 💡 Optimización de Memoria RAM con NimBLE
El ESP32 utiliza por defecto la librería Bluetooth clásica de Espressif (*Bluedroid*), la cual consume **más de 100 KB de memoria RAM**. Al combinar Bluetooth con peticiones seguras HTTPS a la nube (Supabase), la memoria se agotaba y provocaba el error `(-16) BIGNUM - Memory allocation failed`.

Para solucionarlo, en `05-ble-control-nrfconnect.cpp` y en el firmware principal se implementó **`NimBLE-Arduino`** (`#include <NimBLEDevice.h>`), una arquitectura Bluetooth ultraligera:
- **Consumo de RAM:** Apenas ~20 KB (ahorra más de 85 KB de RAM).
- **Compatibilidad:** Permite que el cifrado TLS/HTTPS conviva con conexiones activas de Bluetooth sin saturar el microcontrolador.
- **Partición:** Se configuró `board_build.partitions = huge_app.csv` en `platformio.ini` para disponer de 3.1 MB de almacenamiento Flash.
