# NeKuNeKoFM

Librería modular en C++ para Arduino (**SAMD21** y **ESP32**) orientada a receptores y transmisores de radio FM con decodificación RDS atómica, interfaz gráfica multi-resolución, mapeo declarativo de controles y consola serie desacoplada.

## Arquitectura

* **`NeKuNeKoFM.h`**: Contratos base `NKFM_Receiver` (RX) y `NKFM_Station` (TX), gestión centralizada de estado, presets, catálogo de acciones (`NKFM_Action`) y sistema de eventos `NKFM_UIEvent`.
* **`NKFM_ActionDispatcher.h`**: Despachador central único que traduce cualquier `NKFM_Action` (pulsaciones cortas, largas o comandos IR) en acciones sobre el sintonizador y la interfaz.
* **`NKFM_BoardConfig.h`**: Presets y preferencias comunes con selección automática de hardware (`NKFM_Board_KittenDisplay.h` para SAMD21 y `NKFM_Board_TTGO_ESP32.h` para ESP32-PICO-D4), completamente sobrescribible mediante `UserBoardConfig.h` en el sketch del usuario.
* **`NKFM_RDSParser`**: Decodificador RDS (Grupos 0A, 2A, 4A) con conversión de caracteres CP437 y tabla PTY europea integrada.
* **`NKFM_ReceiverSi4705`**: Driver I2C nativo para SiLabs Si4705 con soporte de antena por cable de auriculares (`FMI`) o antena dedicada (`ANT`), reloj de referencia de 32.768 kHz y control independiente de LEDs GPIO `IO1`/`IO2`.
* **`NKFM_UILayout`**: Motor gráfico ST7789 con 9 temas de color, reloj interno sincronizable por RDS CT y cambio dinámico de vistas (`uiSetLayout`):
  * **`NKFM_UILayout_240x240`**: Vista principal cuadrada (1.3") con dial dinámico, presets, RDS y telemetría de batería.
  * **`NKFM_UILayout_240x135`**: Vista principal panorámica (1.14") con dial dinámico, marcas de presets y scroll de RadioText.
  * **`NKFM_UILayout_240x135_Sensors`**: Vista secundaria de estación ambiental (BME280 / Si7021) con cálculo de punto de rocío y altitud barométrica.
* **`NKFM_UISerial`**: Consola interactiva por puerto Serie (`Stream*`) completamente desacoplada de la pantalla.
* **Módulos de entrada y energía**:
  * **`NKFM_ButtonPad`**: Lectura de botones directos por GPIO con detección de pulsación corta y larga mediante `NKFM_ButtonDef`.
  * **`NKFM_ButtonPadSX1509`**: Driver I2C con interrupción por hardware para expansores de 16 canales SX1509 (`0x3E`).
  * **`NKFM_IR` / `NKFM_IRSony`**: Receptor infrarrojo multiplataforma (`IRLib2` en SAMD21 e `IRremoteESP8266` en ESP32) con mapa estándar Sony SIRC.
  * **`NKFM_BatteryMAX17048`**: Monitorización I2C de voltaje y porcentaje de batería LiPo.

## Ejemplos incluidos

* **`NeKuNeKoFM_KittenDisplay`**: Implementación completa para **Kitten Display 1.3 (SAMD21)** con pantalla 240×240, 7 botones directos, batería MAX17048 y mando IR Sony.
* **`NeKuNeKoFM_TTGO_ESP32_RadioStation`**: Implementación completa para **TTGO Mini Radio Station (ESP32-PICO-D4)** con pantalla panorámica 240×135, conmutación entre vista de Radio y vista de Sensores (BME280/Si7021), amplificadores PAM8304A + TPA6130A2 (`0x60`), expansor de 16 botones SX1509 y mando IR Sony.
