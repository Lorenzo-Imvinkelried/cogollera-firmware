# UTN Santa Fe - Ingeniería en Sistemas de Información
## Tecnologías para la Automatización - Cátedra Prof. Mariano Miguel Perdomo
### Trabajo Práctico: Clasificador Automático de Huevos en 3 Carriles (Modelo Simplificado)

Versión limpia y focalizada del clasificador dimensional de huevos en 3 carriles (Chico, Mediano y Grande), diseñada para una defensa clara, robusta y sin componentes superfluos.

---

## 🏗️ 1. Arquitectura de la Máquina

```
  [Alimentador de Huevos]
            │
            ▼
 [1. Cinta con Motor Stepper A4988]  <── Velocidad regulable con Potenciómetro (ADC)
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
 [Seguridad por Hardware]
            └── Pulsador de Parada de Emergencia (E-Stop) con Interrupción Externa (ISR)
```

---

## 🧩 2. Componentes Utilizados (Prácticas 1 y 2)

| Componente | Rol en el Clasificador | Práctica |
| :--- | :--- | :--- |
| **Motor Paso a Paso (NEMA 17) + Driver A4988** | **Cinta Transportadora:** Avanza la cinta en pasos discretos (200 pasos por casillero). | **Práctica 2** |
| **Sensor Ultrasónico HC-SR04** | **Medidor de Calibre:** Mide la altura del huevo para determinar su tamaño. | **Práctica 1** |
| **Servomotor (`ESP32Servo`)** | **Compuerta Desviadora:** Se posiciona en 0°, 90° o 180° para encarrilar el huevo. | **Práctica 2** |
| **Potenciómetro (ADC)** | **Control de Velocidad:** Regula los microsegundos entre pasos del motor. | **Práctica 1** |
| **Pulsador con Interrupción (ISR)** | **Parada de Emergencia:** Detiene la máquina inmediatamente en caso de atasco. | **Práctica 1** |
| **Máquina de Estados Finitos (FSM)** | **Lógica de Sistemas:** Ciclo sin retardos bloqueantes. | **Sistemas** |

---

## 🔌 3. Distribución de Pines (Pinout ESP32)

| Componente | Pin ESP32 | Función |
| :--- | :--- | :--- |
| **A4988 DIR** | `GPIO 14` | Sentido de giro del motor |
| **A4988 STEP** | `GPIO 12` | Tren de pulsos de avance de cinta |
| **HC-SR04 TRIG** | `GPIO 5` | Disparo del sensor de distancia |
| **HC-SR04 ECHO** | `GPIO 18` | Medición del tiempo de vuelo |
| **Servomotor PWM** | `GPIO 33` | Señal de control angular a 50 Hz |
| **Potenciómetro Velocidad** | `GPIO 35` | Entrada analógica ADC1_CH7 |
| **Parada de Emergencia** | `GPIO 26` | Interrupción de hardware (`INPUT_PULLUP`) |

---

## 🚀 4. Cómo Correrlo en Wokwi

1. En tu proyecto de Wokwi, andá a la pestaña **`diagram.json`** y pegá el contenido de [`diagram.json`](./diagram.json).
2. Andá a la pestaña **`sketch.ino`** y pegá el contenido de [`tp_pedromo.cpp`](./tp_pedromo.cpp).
3. En la pestaña **`libraries.txt`**, asegurate de que figure únicamente:
   ```plaintext
   ESP32Servo
   ```
4. Dale al botón verde **Play** y abrí el **Serial Monitor** (115200 baudios).

---

## 🧪 5. Cómo Probarlo en la Simulación

1. **Variar el tamaño del huevo:** Hacé clic en el sensor **HC-SR04** y mové el deslizador `distance`:
   * **`9 cm`** $\rightarrow$ Calibre $= 12 - 9 = 3\text{ cm}$ (Huevo Chico) $\rightarrow$ El servo va a **$0^\circ$** (Carril 1).
   * **`7 cm`** $\rightarrow$ Calibre $= 12 - 7 = 5\text{ cm}$ (Huevo Mediano) $\rightarrow$ El servo va a **$90^\circ$** (Carril 2).
   * **`5 cm`** $\rightarrow$ Calibre $= 12 - 5 = 7\text{ cm}$ (Huevo Grande) $\rightarrow$ El servo va a **$180^\circ$** (Carril 3).
2. **Acelerar o frenar la cinta:** Girá la perilla del **Potenciómetro**.
3. **Parada de Emergencia:** Presioná el **Botón Rojo** y verás que el motor se congela al instante. Presionalo de nuevo para reanudar.
