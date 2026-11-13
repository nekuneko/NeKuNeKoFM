# NeKuNeKoFM

Librería modular en C++ para Arduino (SAMD21 y ESP32) orientada a receptores y transmisores de radio FM con decodificación RDS atómica, interfaz gráfica multi-resolución y consola serie desacoplada.

## Arquitectura
* **`NeKuNeKoFM.h`**: Contratos base `NKFM_Receiver` (RX) y `NKFM_Station` (TX), gestión centralizada de estado, presets y sistema de eventos `NKFM_UIEvent`.
* **`NKFM_BoardConfig.h`**: Configuración por defecto para Kitten Display 1.3 (SAMD21) y ESP32-PICO-D4, sobrescribible mediante `UserBoardConfig.h` en el sketch del usuario.
* **`NKFM_RDSParser`**: Decodificador RDS (Grupos 0A, 2A, 4A) con conversión de caracteres CP437 y tabla PTY europea.
* **`NKFM_ReceiverSi4705`**: Driver I2C nativo para SiLabs Si4705 (incluye control independiente de LEDs GPIO `IO1`/`IO2`).
* **`NKFM_UILayout` / `_240x240` / `_240x135`**: Motor gráfico ST7789 con 9 temas de color, reloj interno, dial dinámico y scroll de RadioText.
* **`NKFM_UISerial`**: Consola interactiva por puerto Serie (`Stream*`) completamente desacoplada de la pantalla.
* **Módulos opcionales**: `NKFM_IR` / `NKFM_IRSony`, `NKFM_ButtonPad` y `NKFM_BatteryMAX17048`.
