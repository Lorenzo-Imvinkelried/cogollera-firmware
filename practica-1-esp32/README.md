# UTN Santa Fe - Ingeniería en Sistemas de Información
## Tecnologías para la Automatización - Unidad 5: Sistemas Embebidos (Parte 3)
### Guía Práctica 1 con ESP32 - Resolución Completa y Explicaciones

Carpeta dedicada a la resolución, explicación teórica paso a paso y código fuente de cada uno de los ejercicios y consignas de la guía oficial de la cátedra (`U5.3. Práctica 1 con ESP32.pdf` por Mariano Miguel Perdomo).

---

## 📖 1. Marco Teórico y Conceptos Fundamentales

### A. Entradas Analógicas y el ADC del ESP32
* **ADC (Conversor Analógico a Digital):** Traduce una señal continua de voltaje (0 a 3.3V) a un valor numérico digital discreto.
* **Resolución estándar (12 bits):**
  * Rango de valores: $2^{12} = 4096$ niveles posibles (de `0` a `4095`).
  * Escalón de voltaje: $\frac{3.3\text{ V}}{4095} \approx 0.8\text{ mV}$ por unidad.
* ⚠️ **Regla de Oro de Hardware (ADC1 vs ADC2):**
  * El ESP32 tiene dos conversores internos: **ADC1** y **ADC2**.
  * Al encender el módulo Wi-Fi, el **ADC2 queda inhabilitado** porque la radio Wi-Fi comparte ese circuito.
  * Por eso, para sensores analógicos se recomienda siempre usar pines del **ADC1**: GPIO 32, 33, 34, 35, 36 (VP), 39 (VN).

---

### B. Sensor de Temperatura NTC (Termistor y Ecuaciones)
Un termistor **NTC** (*Negative Temperature Coefficient*) es una resistencia cuyo valor disminuye a medida que aumenta la temperatura.
1. **Divisor de Tensión:**
   El módulo incluye una resistencia auxiliar fija $R_{aux} = 10\text{ k}\Omega$ conectada en serie con el termistor a $V_{cc} = 3.3\text{ V}$.
   $$V_m = V_{cc} \cdot \frac{R_{NTC}}{R_{aux} + R_{NTC}}$$
   Despejando la resistencia del termistor a partir de la lectura del ADC ($0$ a $4095$):
   $$R_{NTC} = \frac{R_{aux}}{\frac{V_{cc}}{V_m} - 1} = \frac{R_{aux}}{\left(\frac{4095.0}{\text{lectura}}\right) - 1}$$
2. **Ecuación del Parámetro $\beta$ (Aproximación de Steinhart-Hart):**
   $$R_{NTC} = R_0 \cdot e^{\beta \left(\frac{1}{T} - \frac{1}{T_0}\right)}$$
   Despejando la temperatura absoluta $T$ en Kelvin:
   $$T = \left( \frac{\ln(R_{NTC} / R_0)}{\beta} + \frac{1}{T_0} \right)^{-1}$$
   * Constantes del sensor: $T_0 = 298.15\text{ K}$ ($25^\circ\text{C}$), $R_0 = 10\text{ k}\Omega$, $\beta = 3950$.
   * Conversión final a Celsius: $T_{^\circ\text{C}} = T_{\text{Kelvin}} - 273.15$.

---

### C. Salidas PWM (Pulse Width Modulation)
El ESP32 no tiene una salida analógica pura en todos los pines (solo 2 DACs), por lo que genera un valor analógico aparente mediante modulación por ancho de pulso:
* **Frecuencia (Hz):** Pulsos por segundo (estándar: $5000\text{ Hz}$).
* **Resolución (bits):** Cantidad de niveles discretos del ciclo de trabajo (típicamente 8 bits = 0 a 255).
* **Duty Cycle (Ciclo de Trabajo):** Porcentaje del tiempo que la señal permanece en nivel alto durante un período.
  * $0\%$ duty cycle $\rightarrow 0\text{ V}$ eficaz (LED apagado).
  * $50\%$ duty cycle $\rightarrow 1.65\text{ V}$ eficaz (brillo medio).
  * $100\%$ duty cycle $\rightarrow 3.3\text{ V}$ eficaz (brillo máximo).
* **Funciones API del ESP32:**
  * `ledcAttach(pin, frecuencia, resolucion)`: Asocia el pin al canal PWM.
  * `ledcWrite(pin, dutyCycle)`: Ajusta el valor del ciclo de trabajo.

---

### D. Interrupciones Externas (Hardware Interrupts)
Mecanismo para suspender temporalmente el flujo normal del programa ante un evento externo urgente (como presionar un botón de parada de emergencia), ejecutar una sub-rutina de atención (**ISR**) y regresar al punto exacto donde estaba:
* **`IRAM_ATTR`:** Macro obligatoria para que el compilador ubique la función en la memoria RAM del ESP32 en lugar de la memoria Flash, asegurando ejecución inmediata en nanosegundos.
* **`volatile`:** Las variables compartidas entre la ISR y el `loop()` deben declararse `volatile` para que el compilador no las guarde en registros temporales y siempre lea su valor real actualizado desde la RAM.
* **Reglas de las ISR:** Deben ser ultra cortas, sin `delay()`, sin llamadas lentas por serial ni operaciones pesadas.
* **Modos de disparo:**
  * `RISING`: Flanco de subida (de LOW a HIGH).
  * `FALLING`: Flanco de bajada (de HIGH a LOW).
  * `CHANGE`: Cualquier cambio de nivel lógico.
  * `LOW` / `HIGH`: Nivel continuo.

---

### E. Sensor Digital de Humedad y Temperatura DHT22
* A diferencia del NTC (que es analógico), el **DHT22 es un sensor digital** que integra un termistor, un sensor capacitivo de humedad y un microcontrolador que digitaliza los datos internamente.
* Utiliza un protocolo de comunicación serie propietario por un único cable de datos (**DATA**).
* Requiere la biblioteca `#include <DHT.h>`.
* Se debe validar la lectura con `isnan(temperatura) || isnan(humedad)` por si ocurre algún fallo de comunicación.

---

### F. Sensor Ultrasónico de Distancia HC-SR04
* **Principio de tiempo de vuelo (Time-of-Flight):**
  1. El ESP32 envía un pulso de $10\ \mu\text{s}$ en el pin **Trigger**.
  2. El sensor emite automáticamente una ráfaga de 8 pulsos ultrasónicos a $40\text{ kHz}$.
  3. El pin **Echo** se pone en nivel `HIGH` y permanece así hasta que el sonido rebota en el objeto y regresa al receptor.
  4. La función `pulseIn(echoPin, HIGH)` mide cuántos microsegundos duró el pulso de eco.
* **Fórmula de distancia:**
  * Velocidad del sonido en el aire: $v = 343\text{ m/s} = 0.0343\text{ cm}/\mu\text{s}$.
  * El sonido viaja ida y vuelta, por lo que se divide entre 2:
  $$\text{Distancia (cm)} = \frac{\text{Duración en }\mu\text{s} \times 0.0343}{2}$$
* ⚠️ **Alimentación:** El HC-SR04 clásico requiere **5V** para sus emisores ultrasónicos (conectar al pin `5V` o `VIN` del ESP32).

---

## 📂 2. Índice de Ejercicios y Códigos Resueltos

| Archivo | Tema / Consigna | Enlace Wokwi Oficial |
| :--- | :--- | :--- |
| [`01-potenciometro-adc.cpp`](./01-potenciometro-adc.cpp) | Lectura de posición de potenciómetro con ADC de 12 bits y mapeo. | [Wokwi 410104863739757569](https://wokwi.com/projects/410104863739757569) |
| [`02-termistor-ntc-temperatura.cpp`](./02-termistor-ntc-temperatura.cpp) | Medición de temperatura con termistor NTC y ecuación Steinhart-Hart ($\beta$). | [Wokwi 442528601802077185](https://wokwi.com/projects/442528601802077185) |
| [`03-tarea-potenciometro-semaforo-leds.cpp`](./03-tarea-potenciometro-semaforo-leds.cpp) | **Consigna Tarea (Pág. 14):** 3 LEDs según potenciómetro ($\ge 1365$ Verde, $\ge 2730$ Amarillo, $= 4095$ Rojo). | Simulación disponible |
| [`04-pwm-led-fade.cpp`](./04-pwm-led-fade.cpp) | Incremento y decremento suave de brillo de LED con PWM (`ledcAttach`). | [Wokwi 410109256783209473](https://wokwi.com/projects/410109256783209473) |
| [`05-pwm-potenciometro-brillo.cpp`](./05-pwm-potenciometro-brillo.cpp) | Modificación de intensidad lumínica de LED según potenciómetro. | [Wokwi 442800733118882817](https://wokwi.com/projects/442800733118882817) |
| [`06-interrupciones-semaforo-emergencia.cpp`](./06-interrupciones-semaforo-emergencia.cpp) | Semáforo normal con pulsador de parada de emergencia (`attachInterrupt` + `IRAM_ATTR`). | [Wokwi 410179671454768129](https://wokwi.com/projects/410179671454768129) |
| [`07-interrupciones-pulsador-asincrono.cpp`](./07-interrupciones-pulsador-asincrono.cpp) | Detección asíncrona de pulsador con bandera `volatile` en tareas con delay bloqueante. | [Wokwi 442816839406691329](https://wokwi.com/projects/442816839406691329) |
| [`08-sensor-dht22-temp-humedad.cpp`](./08-sensor-dht22-temp-humedad.cpp) | Medición de temperatura y humedad con sensor digital DHT22 cada minuto. | [Wokwi 442885860030479361](https://wokwi.com/projects/442885860030479361) |
| [`09-sensor-ultrasonico-hcsr04.cpp`](./09-sensor-ultrasonico-hcsr04.cpp) | Medición continua de distancia con HC-SR04 y alarma de proximidad ($\le 50\text{ cm}$). | [Wokwi 442888513380344833](https://wokwi.com/projects/442888513380344833) |

