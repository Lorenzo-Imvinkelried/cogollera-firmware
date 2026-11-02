# Problemas y Ejercicios Prácticos de la Cátedra (ESP32)

Carpeta dedicada a resolver y documentar todos los conceptos, circuitos y ejercicios prácticos presentados en la clase teórica/práctica (archivo `video.pdf`).

---

## 📚 1. Resumen Teórico de la Clase

### A. Características Clave del ESP32
* **GPIO:** 32 pines digitales utilizables.
  * **4 pines de SOLO ENTRADA (Input Only):** GPIO 34, 35, 36 (VP), 39 (VN). No tienen circuito interno para escribir en alto/bajo; solo sirven para leer sensores.
  * **Resto:** Entrada o Salida digital configurable.
* **ADC (Conversor Analógico a Digital):**
  * 15 a 18 canales, divididos en **ADC1** y **ADC2**.
  * Rango de voltaje: **0 a 3.3V**.
  * Resolución: **12 bits (0 a 4095 valores discretos)** $\rightarrow$ Escalón de $\approx 0.8\text{ mV}$ por unidad.
* **DAC (Conversor Digital a Analógico):** 2 canales reales (salida analógica pura).
* **PWM (Modulación por Ancho de Pulso):** Modifica el ciclo de trabajo para variar el valor eficaz de salida (control de intensidad lumínica o velocidad).
* **Touch Pins:** 10 entradas táctiles capacitivas (detectan proximidad del dedo variando la capacitancia).
* **Comunicaciones:** UART (UART0 para conexión PC por USB), SPI, I2C, I2S (audio).
* **Procesamiento y Seguridad:** Wi-Fi 2.4 GHz, Bluetooth, aceleradores de hardware para encriptación (AES, SHA, RSA) y sensores internos (temperatura y efecto Hall).

### B. Entradas y Salidas Digitales
* **Niveles Lógicos:** `LOW` = 0V (0 lógico) | `HIGH` = 3.3V (1 lógico).
* **Estado Flotante (Indeterminado):** Un pin de entrada desconectado capta ruido electromagnético ambiental, alternando aleatoriamente entre 0 y 1.
* **Solución: Resistencias Pull-Up / Pull-Down:**
  * **Pull-Up:** Mantiene el pin en `HIGH` (3.3V) en reposo. Al pulsar, se conecta a GND y lee `LOW` (lógica inversa).
  * **Pull-Down:** Mantiene el pin en `LOW` (GND) en reposo. Al pulsar, se conecta a 3.3V y lee `HIGH`.
  * **Enfoques:**
    1. *Externo:* Resistencia física de $10\text{ k}\Omega$ en la protoboard.
    2. *Interno:* El ESP32 posee resistencias pull-up y pull-down incorporadas en el chip (`INPUT_PULLUP` e `INPUT_PULLDOWN`).
* **Efecto Rebote (Debounce):** Los pulsadores mecánicos tienen láminas flexibles que rebotan por unos milisegundos antes de asentarse, produciendo múltiples disparos falsos. Se filtra por software mediante temporización (ej. retardo de $\approx 50\text{ ms}$ a $200\text{ ms}$).

### C. Por qué evitar `delay()` y usar `millis()`
* `delay(ms)` **congela la CPU**: El procesador no hace nada durante ese tiempo, perdiendo lecturas de pulsadores o sensores.
* `millis()` **es no bloqueante**: Devuelve los milisegundos transcurridos desde que arrancó la placa. Permite evaluar condiciones en cada vuelta del `loop()` mientras el procesador sigue atendiendo otros eventos.

---

## 🗂️ 2. Índice de Ejercicios Resueltos

| Archivo | Consigna del Video | Enfoque / Concepto |
| :--- | :--- | :--- |
| [`01-led-blink-2s-delay.cpp`](./01-led-blink-2s-delay.cpp) | Encender y apagar un LED por 2 segundos en cada estado cíclicamente. | Salida digital básica con `delay()`. |
| [`02a-pulsador-pullup-externa.cpp`](./02a-pulsador-pullup-externa.cpp) | Detectar pulsador e indicar por consola cuando ocurra. | Resistencia Pull-Up externa ($10\text{ k}\Omega$) + Antirrebote. |
| [`02b-pulsador-pullup-interna.cpp`](./02b-pulsador-pullup-interna.cpp) | Detectar pulsador e indicar por consola cuando ocurra. | Resistencia Pull-Up interna (`INPUT_PULLUP`). |
| [`02c-pulsador-pulldown.cpp`](./02c-pulsador-pulldown.cpp) | Alternativa explicada en clase con Pull-Down. | Pull-Down externa e interna (`INPUT_PULLDOWN`). |
| [`03-cochera-3-parpadeos-delay.cpp`](./03-cochera-3-parpadeos-delay.cpp) | Luminaria de cochera: pulsador activa 3 parpadeos (1s ON / 1s OFF). | Entradas + Salidas con secuencia fija (`delay()`). |
| [`04-led-blink-2s-millis.cpp`](./04-led-blink-2s-millis.cpp) | Encender y apagar un LED por 2 segundos sin usar `delay()`. | Temporización cíclica con `millis()`. |
| [`05-cochera-avanzada-millis.cpp`](./05-cochera-avanzada-millis.cpp) | **Propuesta para seguir practicando:** Rehacer la cochera sin `delay()`. | Máquina de estados no bloqueante con `millis()`. |

---

## 🌐 3. Simulación en Wokwi
Cada archivo incluye en sus comentarios el enlace o la configuración esquemática para copiar y pegar directamente en [Wokwi ESP32 Simulator](https://wokwi.com/).

