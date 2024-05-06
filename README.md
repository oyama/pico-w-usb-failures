# Battery-Powered Raspberry Pi Pico W and USB Type-C Connection Issue

This repository investigates a specific issue that occurs when the battery-powered Raspberry Pi Pico W is connected to a host PC via a USB Type-C connection. Specifically, the following connection sequence fails to initiate USB communication:

1. Connect the USB cable to the micro USB connector of the Pico W.
2. Connect the USB cable to the Type-C connector of the host PC.

This phenomenon does not occur if the cable is connected in the reverse order from the host PC to the Pico W, or when using a USB Type-A connection, and interestingly, it does not occur with the standard Raspberry Pi Pico either.


## Technical Investigation

To understand and diagnose this issue, the following methods were employed:

- GPIO: monitored GPIOs connected to VBUS (`GPIO24` for Pico and `LW_GPIO02` for Pico W).
- RP2040 register: monitored the `USBCTRL_REGS` on the RP2040 chip.
- TinyUSB: call `tud_ready()` function to check if USB communication was ready.

The monitoring results matched the USB communication state, but the behavior of the Pico W when connected to the host PC was not as expected. Further behavior is detailed below.

### Circuit Diagram

(diagram)

## Build and Install Instructions

The source code available in this repository monitors the USB connection status of both Pico and Pico W, continuously reporting the USB connection state to UART every second after cable attachment and detachment operations.

### Build Instructions for Pico

```bash
mkdir build; cd build
PICO_SDK_FETCH_FROM_GIT=1 cmake -DPICO_BOARD=pico ..
make
```

### Build Instructions for Pico W

```bash
mkdir build; cd build
PICO_SDK_FETCH_FROM_GIT=1 cmake -DPICO_BOARD=pico_w ..
make
```

The above examples specify the environment variable `PICO_SDK_FETCH_FROM_GIT` to download the pico-sdk from GitHub. If you want to specify a locally deployed pico-sdk, you should set it with the `PICO_SDK_PATH` environment variable.
Once built, the firmware `picow-usb-failures.uf2` will be generated. Simply drag and drop it onto your device to install.

## Monitoring USB Connection Status

To observe the UART output under battery power, use the [Raspberry Pi Debug Probe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html) to monitor the UART.
For the Pico, when connected to the host PC, the status changes as follows:
```
Waiting for USB connection
BOARD=pico, TinyUSB not ready, USBCTRL_REG(value=0x00000015) disconnect, VBUS low
BOARD=pico, TinyUSB not ready, USBCTRL_REG(value=0x00000015) disconnect, VBUS low
BOARD=pico, TinyUSB not ready, USBCTRL_REG(value=0x00000015) disconnect, VBUS low
BOARD=pico, TinyUSB ready, USBCTRL_REG(value=0x40050005) connect, VBUS high
BOARD=pico, TinyUSB ready, USBCTRL_REG(value=0x40050005) connect, VBUS high
BOARD=pico, TinyUSB ready, USBCTRL_REG(value=0x40050005) connect, VBUS high

```
However, for the Pico W, even when the host PC's type-C cable is connected, the status does not change:
```
Waiting for USB connection
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
```

The USB connection status recognized by the devices matches the device recognition status on the host PC side. Interestingly, if you boot the Pico in `BOOTSEL mode` while powered by VSYS and connect it to the host PC, the `RPI RP2` disk will mount, but in the case of Pico W, there is no change in USB connection status, and the disk does not mount.

## References

- [Raspberry Pi Pico Datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)
- [Raspberry Pi Pico W Datasheet](https://datasheets.raspberrypi.com/picow/pico-w-datasheet.pdf)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)