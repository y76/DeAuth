# DeAuth
## Hardware Requirements
- [NXP LPC55S69-EVK](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/lpcxpresso-boards/lpcxpresso55s69-development-board:LPC55S69-EVK) development board
- (2) [Quorvo QM33120WDK1](https://www.qorvo.com/products/p/QM33120) 
- [ESP32-C3-DevKitC-02](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitc-02.html) Bluetooth board
  - Connected via UART4
 
## Software Requirements

### NXP Board
1. IDE: [MCUXpresso IDE v11.6.1](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE) (released on 2022-10-03)
2. SDK: v2.12.0 (released 2022-07-14)

SDK can be built using [MCUXpresso SDK Builder](https://mcuxpresso.nxp.com/en/welcome), or it can be downloaded via MCUXpresso IDE.
Note that the LOCO implementation on the NXP board is based on the secure_gpio example, provided by NXP.

### ESP32-C3 Boards
Using VSCode with the Espressif IDF extension:
1. Open the Command Palette (Ctrl+Shift+P)
2. Select 'ESP-IDF: Select port to use'
   - Choose the port for the ESP Board (typically /dev/ttyUSB* in Ubuntu)
   - Select the directory containing the ESP source code
3. Select 'ESP-IDF: Set Espressif Device Target'
   - Choose the ESP source code directory
   - Select 'esp32c3'
   - Select 'ESP32-C3 chip (via ESP-PROG)'
4. Use 'ESP-IDF: SDK Configuration editor (menuconfig)' to confirm UART settings
   - Port number: 1
   - Communication speed: 115200
   - RXD pin: 7
   - TXD pin: 6
5. Run 'ESP-IDF: Build your project'
6. Run 'ESP-IDF: Flash your project'


### UWB Boards
- IDE: [Segger Embedded Studio 8.22a](https://www.segger.com/products/development-tools/embedded-studio/)
- Project locations:
  - UT: `~/API/Build_Platforms/nRF52840-DK/IoT_Uwb.emProject`
  - WS: `~/API/Build_Platforms/nRF52840-DK/user_uwb.emProject`
 
## Tamarin Security Models

The security protocol of LOCO has been formally verified using the [Tamarin Prover](https://tamarin-prover.github.io/). The formal models are available in the `tamarin` folder:
