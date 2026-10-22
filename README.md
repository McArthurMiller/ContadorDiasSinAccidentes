# Contador de Días Sin Accidentes — ESP32

Cartel electrónico para entornos industriales que muestra cuántos días han pasado sin accidentes laborales. Funciona con un ESP32, un reloj de tiempo real (RTC) que incrementa el contador automáticamente a medianoche, y memoria no volátil para que el conteo sobreviva a cortes de energía.

Todo el control se hace con botones físicos: el proyecto no usa comunicación serial.

---

## Características

- **Contador automático:** cuando el RTC detecta un cambio de día, el contador suma 1.
- **Persistencia:** el valor se guarda en la memoria flash del ESP32 (librería `Preferences`), así que no se pierde al apagar el equipo.
- **Ajuste de fecha y hora desde botones**, con parpadeo del campo que se está editando.
- **Modo de prueba** que recorre los dígitos 0–9 en todo el display para verificar segmentos.
- **LED de estado** que parpadea mientras el equipo está funcionando.
- **Temporización no bloqueante:** todo el programa usa `millis()`; no hay ningún `delay()`.
- **Supresión de ceros a la izquierda** en el display (muestra `  42`, no `0042`).

---

## Hardware

| Componente | Descripción |
|---|---|
| Microcontrolador | ESP32-WROOM en tarjeta controladora |
| Display | Tarjeta LED de 4 dígitos de 7 segmentos, controlada por registros de desplazamiento (DATA, CLOCK, LATCH) |
| RTC | PCF85063TP por I2C |
| Entradas | 5 botones |
| Indicador | LED de estado |

### Asignación de pines

| Función | GPIO | Notas |
|---|---|---|
| Boton1 | 18 | `INPUT_PULLUP` |
| Boton2 | 5 | `INPUT_PULLUP` |
| Boton3 | 17 | `INPUT_PULLUP` |
| Boton4 | 16 | `INPUT_PULLUP` |
| Boton5 (test) | 36 | `INPUT` — GPIO36 solo es entrada y no tiene pull-up interno; requiere resistencia externa |
| Display DATA | 26 | |
| Display CLOCK | 0 | |
| Display LATCH | 32 | |
| LED de estado | 13 | |
| RTC (I2C) | SDA/SCL por defecto | `Wire.begin()` |

---

## Funcionamiento de los botones

### Modo normal (muestra el contador)

| Botón | Acción |
|---|---|
| **Boton1** | Entra al modo de ajuste de fecha y hora |
| **Boton3** (mantener 5 s) | Reinicia el contador a 0 y pone el RTC en una fecha base |
| **Boton4** | Suma 1 día al contador manualmente (un incremento por pulsación) |
| **Boton5** | Inicia el modo de prueba |

### Modo de ajuste

| Botón | Acción |
|---|---|
| **Boton2** | Avanza al siguiente campo; en el último campo, sale del modo de ajuste y reinicia el contador a 0 |
| **Boton3** | Incrementa el campo activo y escribe la nueva fecha/hora en el RTC |

El campo que se está editando parpadea cada 300 ms. Orden de los campos:

| # | Campo | Lo que muestra el display |
|---|---|---|
| 0 | Hora | `HH MM` (parpadea la hora) |
| 1 | Minuto | `HH MM` (parpadea el minuto) |
| 2 | Día de la semana | `D DD` (parpadea el día de la semana, 1–7) |
| 3 | Fecha | `D DD` (parpadea el día del mes) |
| 4 | Mes | `MM AA` (parpadea el mes) |
| 5 | Año | `MM AA` (parpadea el año) |
| 6 | Confirmación | `HH MM` fijo; Boton2 sale del ajuste |

### Modo de prueba

Muestra el mismo dígito en las cuatro posiciones, del 0 al 9, cambiando cada 3 segundos. Al terminar regresa al modo normal.

---

## Estructura del código

| Función | Propósito |
|---|---|
| `setup()` | Inicializa I2C, RTC, memoria `Preferences` y pines |
| `loop()` | Lee botones, consulta el RTC, gestiona modos y actualiza el display |
| `mostrarContador(valor)` | Separa el número en dígitos y lo envía al display |
| `mostrarFecha()` | Muestra el campo de ajuste activo con parpadeo |
| `mostrarNumeroTest(valor)` | Muestra un dígito en las 4 posiciones |
| `enviarDigito(dato)` | Convierte un número (0–9, 10 = apagado) a su patrón de 7 segmentos y lo envía con `shiftOut()` |

La detección de pulsaciones usa flanco descendente (estado actual `LOW` y anterior `HIGH`) con antirrebote basado en `millis()`.

---

## Compilación

1. **Arduino IDE** (probado con 1.8.19) con soporte para placas ESP32.
2. Instalar la librería **Grove - High Precision RTC** (Seeed Studio) desde *Programa → Incluir Librería → Administrar Bibliotecas*.
   - El archivo de cabecera es `PCF85063TP.h`, pero la clase se llama `PCD85063TP` (con D). Es así en la librería original.
3. `Wire.h` y `Preferences.h` vienen incluidas con el soporte de ESP32.
4. Seleccionar la placa ESP32 Dev Module y compilar.

Uso de memoria: ~23 % del almacenamiento de programa y ~7 % de la memoria dinámica.

---

## Posibles mejoras

- Al cambiar el día del mes desde el modo de ajuste, o al hacer el reinicio de 5 s, el RTC cambia de día y el contador puede sumar 1. Se podría actualizar `ultimoDiaVisto` al escribir el RTC manualmente.
- El reinicio de 5 s con Boton3 se ejecuta en cada ciclo mientras el botón sigue presionado; una bandera haría que ocurra una sola vez por pulsación.
- GPIO0 es un pin de arranque del ESP32; moverlo a otro pin evitaría posibles problemas al encender.

---

## Autor

**Arturo Jiménez Hernández** — [@McArthurMiller](https://github.com/McArthurMiller)
