# Introduction to SR250 UWBIOT release for Zephyr OS

UWB IoT Middleware provides API interface for communicating with Trimension SR250 UWB controller. This middleware will run on an embedded host and communicate over UCI protocol with underlying UWB devices.
The current release targets RW612 NXP MCU running zephyr OS.

The reference platform is composed of:
- [SR250-ARD Development board](https://www.nxp.com/SR250UWBSHIELD)
- [FRDM-RW612 Development Board](https://www.nxp.com/FRDM-RW612) 

⚠️ The FRDM-RW612 board requires a small rework to expose the SPI interface on the Arduino headers. You can find relevant instructions [here](https://github.com/nxp-appcodehub/an-sr250-uwb-plug-and-play-demo/FRDM-RW612_rework_for_SPI.md).

## Revision history
| Version | Description / Update |
|:--------|:---------------------|
| SR250_UWBIOT_IOT4_RFP | Initial release |

## UWB IoT Middleware details
- Information about the UWB IoT Middleware architecture and components can be found in [UWBIoT_Middleware_Guide.pdf](UWBIoT_Middleware_Guide.pdf)
- Description of the UWB IoT Middleware API can be found in [UWBIoT_API_Reference.pdf](UWBIoT_API_Reference.pdf)


## Prerequisites
1. Git latest version (https://git-scm.com/download)
2. CMake version >= 3.21.1 (https://cmake.org/download/)
3. Python 3.8 or later
4. Ninja build system (install with `pip install ninja`)
5. West Zephyr meta-tool (install with `pip install west`)
6. Zephyr SDK 0.16.1 or later (https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html#zephyr-sdk-installation)

## Setting up build environment and compliling
Follow the steps below to set up the build environment for Zephyr-based projects.

1. Retrieve UWBIoT middleware for SR250
```bash
git clone https://github.com/nxp-uwb/sr250-uwbiot-zephyr.git
```
2. Initialize and update Zephyr workspace
```bash
cd ~/sr250-uwbiot-zephyr
west init -l --mf west.yml uwbiot-top
west update
```
3. Install Zephyr Python dependencies
```bash
pip install -r zephyr/scripts/requirements.txt
```
4. Set up Zephyr environment variables
```bash
zephyr\zephyr-env.cmd # On Linux, use: source zephyr/zephyr-env.sh
```
5. Build the application
```bash
west build -b <board_name> -p auto <path_to_demo>
```
Where `<board_name>` is the target board (e.g., `frdm-rw612`) and `<path_to_demo>` is the path to the demo application directory (e.g., `uwbiot-top/demos/SR2xx/demo_sr2xx_fw_update/zephyr`).

6. Running the application
Flash the built application to the target board:
```bash
west flash
```
Log outputs during application execution can be observed using a serial terminal application (e.g., PuTTY, Tera Term) with the following settings:
- Baudrate: 3000000
- Data: 8 bits
- Parity: None
- Stop bits: 1 bit
- Flow control: None


## Support
For more details about Trimension SR250 UWB controller refer to the related to the related page on NXP.com: https://www.nxp.com/products/SR250.

To contact NXP or to report issues, please use https://www.nxp.com/support/support:SUPPORTHOME
