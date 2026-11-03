# UTN Santa Fe - Ingeniería en Sistemas de Información
## Tecnologías para la Automatización - Cátedra Prof. Jorge Vega / Mariano Perdomo
### Trabajo Práctico: Clasificador Automático de Huevos en 3 Carriles
#### Con Pantalla LCD 16x2 (I2C) y Regulación de Velocidad de Stepper por Potenciómetro

Este proyecto implementa una estación clasificadora de huevos en 3 carriles (Chico, Mediano y Grande) basada en el SoC ESP32, con pantalla industrial LCD 16x2 I2C para visualización de métricas en tiempo real y consola de control HMI.

---

## 🏗️ 1. Arquitectura del Sistema

```
  [Alimentador de Huevos]
            │
            ▼
 [1. Motor Stepper A4988] <── Regulado en velocidad por Potenciómetro (ADC)
            │ (Avance indexado)
            ▼
 [2. Estación de Medición de Calibre]
            └── Sensor Ultrasónico HC-SR04 (Mide altura: Calibre = 12 cm - Distancia)
            │
            ▼
 [3. Compuerta Desviadora con Servomotor]
            ├── 0°   ──────► Carril 1: HUEVOS CHICOS  (S - Calibre < 4.0 cm)
            ├── 90°  ──────► Carril 2: HUEVOS MEDIANOS(M - Calibre 4.0 a 5.5 cm)
            └── 180° ──────► Carril 3: HUEVOS GRANDES (L - Calibre > 5.5 cm)
            │
 [Visualización e Interacción HMI]
            ├── Pantalla LCD 16x2 I2C: Muestra Total, Velocidad y Contadores S / M / L
            └── Pulsador de Parada de Emergencia (E-Stop): Interrupción externa por hardware (ISR)
```

---

## 📺 2. Visualización en la Pantalla LCD 16x2 (I2C)

La pantalla LCD se comunica por el bus **I2C** (`GPIO 21 SDA` y `GPIO 22 SCL`) y muestra en tiempo real:

```text
+----------------+
|TOT:24   V: 85% |  <-- Línea 0: Total procesado y Velocidad del Stepper (%)
|C:5   M:12  G:7 |  <-- Línea 1: Contadores de Huevos Chicos, Medianos y Grandes
+----------------+
```

* **Al medir un huevo nuevo:** La línea 0 indica transitoriamente la categoría detectada (ej: `HUEVO MEDIANO(M)`).
* **Al activar la Parada de Emergencia:** La pantalla cambia inmediatamente a `!EMERGENCIA E-STOP / CINTA DETENIDA`.

---

## 🔌 3. Distribución de Pines (Pinout ESP32)

| Componente | Pin ESP32 | Protocolo / Tipo | Función en el Sistema |
| :--- | :--- | :--- | :--- |
| **A4988 DIR** | `GPIO 14` | Salida Digital | Dirección de giro de la cinta transportadora |
| **A4988 STEP** | `GPIO 12` | Salida Digital | Pulsos de avance del motor paso a paso |
| **HC-SR04 TRIG** | `GPIO 5` | Salida Digital | Disparo de ráfaga ultrasónica (pulso 10 µs) |
| **HC-SR04 ECHO** | `GPIO 18` | Entrada Digital | Medición del tiempo de vuelo con `pulseIn()` |
| **Servomotor PWM** | `GPIO 33` | Salida PWM (50 Hz) | Posicionamiento de la compuerta a 0°, 90° o 180° |
| **Potenciómetro Stepper** | `GPIO 35` | Entrada Analógica | Regula la velocidad del motor stepper (20% a 100%) |
| **Parada de Emergencia** | `GPIO 26` | Entrada con Interrupción | ISR inmediata en flanco de bajada (`INPUT_PULLUP`) |
| **Pantalla LCD SDA** | `GPIO 21` | Bus I2C Datos | Comunicación bidireccional I2C |
| **Pantalla LCD SCL** | `GPIO 22` | Bus I2C Reloj | Línea de reloj del bus I2C |

---

## 🚀 4. Cómo Correrlo en Wokwi

1. En tu proyecto de Wokwi, andá a la pestaña **`diagram.json`** y pegá el contenido de [`diagram.json`](./diagram.json).
2. Andá a la pestaña **`sketch.ino`** y pegá el contenido de [`tp_pedromo.cpp`](./tp_pedromo.cpp).
3. En la pestaña **`libraries.txt`**, asegurate de que figuren:
   ```plaintext
   ESP32Servo
   LiquidCrystal_I2C
   ```
4. Dale al botón verde **Play (Start Simulation)** y abrí el **Serial Monitor** (115200 baudios).

---

## 🧪 5. Cómo Probar la Interacción en Wokwi

1. **Mirar la Pantalla LCD:** Verás cómo los contadores de huevos **C (Chicos)**, **M (Medianos)** y **G (Grandes)** se incrementan automáticamente en cada ciclo.
2. **Variar la Velocidad del Stepper:** Girá la perilla del **Potenciómetro**:
   * El porcentaje `V: XX%` en la pantalla LCD cambia al instante.
   * El motor paso a paso gira más rápido o más lento en consecuencia.
3. **Parada de Emergencia:** Apretá el botón rojo:
   * El motor se congela en el microsegundo exacto.
   * La pantalla LCD muestra `!EMERGENCIA E-STOP`. Presionalo de nuevo para reanudar.
