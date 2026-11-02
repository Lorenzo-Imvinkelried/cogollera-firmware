# Enlaces Oficiales de Simulación en Wokwi
## Cátedra: UTN Santa Fe - Tecnologías para la Automatización (Unidad 5)
### Guía: U5.4. Práctica 2 con ESP32 (Actuadores y Control PID)

La siguiente tabla recopila todos los enlaces oficiales de simulación en Wokwi incluidos en las diapositivas de la cátedra por el Prof. Mariano Miguel Perdomo.

---

### 🌐 Tabla de Enlaces y Hardware

| Ejercicio / Tema | Diapositivas | Enlace Wokwi Oficial | Pines de Conexión en la Simulación |
| :--- | :--- | :--- | :--- |
| **1. Relé + Sensor DHT22** (Control On/Off Calefacción) | Diap. 4 - 10 | [Abrir en Wokwi](https://wokwi.com/projects/442971277123992577) | `DHT22 DATA`: GPIO 13<br>`RELAY IN`: GPIO 23<br>`NO`: Calefactor<br>`NC`: Standby |
| **2. Servomotor** (0°-90°-180°, Barrido y Potenciómetro) | Diap. 11 - 20 | [Abrir en Wokwi](https://wokwi.com/projects/442973167174987777) | `SERVO PWM`: GPIO 33<br>`POTENCIÓMETRO`: GPIO 35 (ADC)<br>`VCC`: 5V / `GND`: GND |
| **3. Stepper NEMA 17 + A4988** (Giros, Pulsadores y Pasos) | Diap. 21 - 31 | [Abrir en Wokwi](https://wokwi.com/projects/442976722030999553) | `A4988 DIR`: GPIO 14<br>`A4988 STEP`: GPIO 12<br>`BTN 1`: GPIO 26<br>`BTN 2`: GPIO 27 |
| **4. Controlador PID Discreto** (Temperatura con DHT22 y PWM) | Diap. 32 - 39 | [Abrir en Wokwi](https://wokwi.com/projects/442981504996359169) | `DHT22 DATA`: GPIO 13<br>`PWM OUT`: GPIO 23 (8 bits, 5000 Hz)<br>`CALEFACTOR`: Transistor / Carga |

---

### 🛠️ Guía Rápida para Probar las Consignas en Wokwi

#### 1. Simulación de Relé y Calefactor (`01-rele-control-calefaccion.cpp`)
1. Inicia la simulación haciendo clic en el botón verde **Play** en Wokwi.
2. Haz clic sobre el sensor **DHT22** para abrir el panel de control interactivo.
3. Arrastra el control deslizante de temperatura:
   - Si la temperatura es $\le 10^\circ\text{C}$, el relé conmutará encendiendo la lámpara/calefactor en el contacto **NO**.
   - Si la temperatura supera los $10^\circ\text{C}$, el relé se desconectará volviendo al contacto **NC** (luz de reposo).

#### 2. Simulación de Servomotor (`02a`, `02b` y `02c`)
- **Posicionamiento y Barrido (`02a` y `02b`):** Observa la aguja del servomotor desplazarse suavemente sin saltos abruptos.
- **Control con Potenciómetro (`02c`):** Agrega un componente `potentiometer` en Wokwi conectado a `GPIO 35`, `3.3V` y `GND`. Al girar la perilla, el servomotor seguirá proporcionalmente el ángulo de $0^\circ$ a $180^\circ$.

#### 3. Simulación de Stepper y Driver A4988 (`03a`, `03b` y `03c`)
- **Giro y Velocidad (`03a`):** El motor dará 1 vuelta en sentido horario, se detendrá 1 segundo y luego dará 2 vueltas rápidas en reversa.
- **2 Pulsadores (`03b` y `03c`):** Agrega 2 pulsadores `pushbutton` a los pines `GPIO 26` y `GPIO 27` con el otro extremo a `GND` (aprovechando `INPUT_PULLUP`).
  - En `03b`: Pulsador 1 da 1 vuelta horaria; Pulsador 2 da 1 vuelta antihoraria.
  - En `03c`: Pulsador 1 da 1 solo paso de $1.8^\circ$; Pulsador 2 alterna el sentido de avance.

#### 4. Simulación del Controlador PID (`04a` y `04b`)
1. Inicia la simulación y abre el **Serial Monitor** (115200 baudios).
2. Haz clic sobre el DHT22 y varía la temperatura alejándola de la consigna ($24.0^\circ\text{C}$).
3. Observa cómo el algoritmo PID ajusta dinámicamente el ciclo de trabajo PWM ($0$ a $255$) combinando las acciones proporcional, integral (con anti-windup) y derivativa.

