# UTN Santa Fe - Ingeniería en Sistemas de Información
## Tecnologías para la Automatización - Unidad 5: Sistemas Embebidos (Parte 4)
### Guía Práctica 2 con ESP32: Actuadores y Control PID - Resolución Completa

Carpeta dedicada a la resolución, explicación teórica paso a paso y código fuente de cada uno de los ejercicios y consignas de la guía oficial de la cátedra (`U5.4. Práctica 2 con ESP32.pdf` por Mariano Miguel Perdomo).

---

## 📖 1. Marco Teórico y Conceptos Clave

### A. El Relé (Aislamiento Galvánico y Control de Potencia)
* Un microcontrolador como el ESP32 opera a $3.3\text{ V}$ y entrega como máximo $\approx 12\text{ mA}$ por pin, insuficiente para alimentar cargas eléctricas de potencia (motores, calefactores de $220\text{ V}$, etc.).
* El **relé** es un interruptor electromecánico u optoacoplado de estado sólido que permite a una señal débil de control ($3.3\text{ V}$) conmutar un circuito independiente de alta tensión y corriente ($220\text{ VAC}$ / $10\text{ A}$).
* **Terminales de salida del relé:**
  * **COM (Común):** Borne donde entra la fase o la alimentación principal.
  * **NO (Normally Open - Normalmente Abierto):** Permanece desconectado en reposo. Se cierra cuando el ESP32 activa el relé (`HIGH`). En la práctica conecta la carga de potencia (ej: calefactor).
  * **NC (Normally Closed - Normalmente Cerrado):** Permanece conectado en reposo. Se desconecta cuando el relé se activa. Conecta la luz indicadora de "sistema en reposo/apagado".
* 💡 *Nota técnica:* Muchos módulos de relé comerciales son **activos en nivel bajo (`LOW`)** mediante optoacopladores para aislar ruidos parásitos.

---

### B. El Servomotor (Control de Posición Angular por PWM)
* A diferencia de un motor DC común que gira continuamente, un servomotor se posiciona en un ángulo exacto (típicamente entre $0^\circ$ y $180^\circ$).
* **Principio de señal:** Recibe un pulso PWM periódico a $50\text{ Hz}$ (período de $20\text{ ms}$):
  * Pulso de $\approx 1.0\text{ ms} \rightarrow 0^\circ$.
  * Pulso de $\approx 1.5\text{ ms} \rightarrow 90^\circ$ (punto medio).
  * Pulso de $\approx 2.0\text{ ms} \rightarrow 180^\circ$.
* En ESP32 se utiliza la biblioteca `#include <ESP32Servo.h>`, la cual utiliza los temporizadores de hardware del SoC para generar la modulación sin bloquear el microcontrolador.
* ⚠️ **Alimentación:** En el laboratorio o la industria, el servomotor debe alimentarse con una **fuente externa de 5V**, uniendo únicamente las masas (GND común), para evitar que los picos de corriente del motor reinicien el microcontrolador.

---

### C. Motor Paso a Paso Bipolar (Stepper) y Controlador A4988
* Los motores paso a paso dividen una rotación completa en un número determinado de pasos discretos mecánicos.
* **Geometría típica (NEMA 17):**
  * Ángulo por paso: $1.8^\circ$ por cada paso completo.
  * Pasos por revolución: $\frac{360^\circ}{1.8^\circ} = \mathbf{200\text{ pasos}}$ para dar 1 giro completo ($360^\circ$).
* **Controlador (Driver) A4988:**
  * Simplifica el control del motor reduciéndolo a **solo 2 señales digitales del ESP32**:
    1. **DIR (Dirección):** `HIGH` para giro horario, `LOW` para giro antihorario.
    2. **STEP (Paso):** Cada pulso en `HIGH` con su correspondiente flanco hace avanzar el motor exactamente 1 paso ($1.8^\circ$).
  * **Velocidad de giro:** Se controla variando el tiempo de espera entre pulsos (`stepDelay` en microsegundos). Menor tiempo de retardo $\rightarrow$ mayor frecuencia de pulsos $\rightarrow$ mayor velocidad.

---

### D. Controlador PID Discreto (Proporcional - Integral - Derivativo)

#### 1. Ecuación Continua Clásica:
$$u(t) = K_p \, e(t) + K_i \int_0^t e(\tau) \, d\tau + K_d \frac{de(t)}{dt}$$
Donde $e(t) = \text{SetPoint} - \text{Medición}(t)$ es el error del sistema.

#### 2. Discretización para Microcontroladores (con período de muestreo $T_s = \Delta t$):
1. **Modo Proporcional (P):** Responde instantáneamente a la magnitud del error actual.
   $$P(k) = K_p \cdot e(k)$$
2. **Modo Integral (I):** Elimina el error en régimen permanente acumulando los errores pasados.
   * Se aproxima la integral mediante la **sumatoria de trapecios (Regla Trapezoidal)**:
     $$\int_0^t e(\tau) \, d\tau \approx \sum_{h=0}^k \frac{e(h) + e(h-1)}{2} \, T_s \approx \text{cumError} + e(k) \cdot \Delta t$$
     $$I(k) = K_i \cdot \text{cumError}$$
3. **Modo Derivativo (D):** Anticipa el comportamiento futuro evaluando la velocidad de cambio del error.
   * Se aproxima mediante la **derivada hacia atrás de Euler (Backward Difference)**:
     $$\frac{de(t)}{dt} \approx \frac{e(k) - e(k-1)}{\Delta t} = \text{rateError}$$
     $$D(k) = K_d \cdot \text{rateError}$$

#### 3. Control de Saturación y Anti-Windup:
* **Integrator Windup:** Si el error persiste durante mucho tiempo (por ejemplo, el calefactor tarda en calentar), el término integral sigue creciendo indefinidamente. Cuando la temperatura por fin llega al setpoint, el acumulador es tan grande que el sistema sigue calentando de más (overshoot masivo).
* **Solución (Anti-Windup):** Se limita el error acumulado entre $[-iMax, +iMax]$:
  $$iMax = \frac{\text{outMax}}{2 \cdot K_i}$$
* **Saturación de Salida:** La señal de control $u(k)$ se acota físicamente al rango del actuador PWM ($0\text{ a }255$).

---

## 📂 2. Índice de Ejercicios y Tareas Resueltas

| Archivo | Actuador / Tema | Consigna Resuelta |
| :--- | :--- | :--- |
| [`01-rele-control-calefaccion.cpp`](./01-rele-control-calefaccion.cpp) | Relé + DHT22 | Control termostático On/Off de calefacción ($\le 10^\circ\text{C}$). |
| [`02a-servo-posiciones-basicas.cpp`](./02a-servo-posiciones-basicas.cpp) | Servomotor | Prueba de límites y rango: 0°, 90° y 180° cada 1 segundo. |
| [`02b-servo-barrido-suave.cpp`](./02b-servo-barrido-suave.cpp) | Servomotor | **Consigna 2:** Barrido suave continuo de 0° a 180° paso a paso de 1° cada 20 ms. |
| [`02c-servo-control-potenciometro.cpp`](./02c-servo-control-potenciometro.cpp) | Servomotor | **Consigna 3:** Control proporcional de posición angular (0° a 180°) con potenciómetro. |
| [`03a-stepper-a4988-giros-velocidad.cpp`](./03a-stepper-a4988-giros-velocidad.cpp) | Stepper + A4988 | 1 giro en un sentido, pausa de 1s, 2 giros al doble de velocidad en reversa. |
| [`03b-stepper-dos-pulsadores-giros.cpp`](./03b-stepper-dos-pulsadores-giros.cpp) | Stepper + A4988 | **Consigna 2:** Pulsador 1 da 1 vuelta horaria completa (200 pasos); Pulsador 2 da 1 vuelta antihoraria. |
| [`03c-stepper-un-paso-y-direccion.cpp`](./03c-stepper-un-paso-y-direccion.cpp) | Stepper + A4988 | **Consigna 3:** Pulsador 1 avanza 1 paso exacto ($1.8^\circ$); Pulsador 2 conmuta el sentido de giro. |
| [`04a-controlador-pid-temperatura.cpp`](./04a-controlador-pid-temperatura.cpp) | Control PID | Implementación completa de PID discreto desde cero con Anti-Windup, DHT22 y salida PWM. |
| [`04b-controlador-pid-clase-modular.cpp`](./04b-controlador-pid-clase-modular.cpp) | Control PID (POO) | Implementación orientada a objetos (clase `ControladorPID` reusable para cualquier planta). |
| [`wokwi-enlaces.md`](./wokwi-enlaces.md) | Simulación | Enlaces directos a los proyectos oficiales de Wokwi de las diapositivas. |

