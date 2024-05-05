# Raspberry Pi Pico W Battery-Powered USB Connection Issue

This repository demonstrates an issue with the Raspberry Pi Pico W when powered by battery and failing to detect a USB connection to a host PC. While the Raspberry Pi Pico initializes and communicates over USB under the same conditions, the Pico W does not.

Both _Pico_ and _Pico W_ can be powered by two power sources: __VBUS__ (USB power) and __VSYS__ (battery power). The issue arises specifically when the Pico W is connected to the host PC via USB while powered by VSYS. It does not initiate communication unless VSYS is disconnected, a limitation not observed with the standard Pico.

## Build and Install Instructions

Clone this repository and build the project as follows:

### For Raspberry Pi Pico

```bash
mkdir build; cd build
PICO_SDK_FETCH_FROM_GIT=1 cmake -DPICO_BOARD=pico ..
make
```

### For Raspberry Pi Pico W

```bash
mkdir build; cd build
PICO_SDK_FETCH_FROM_GIT=1 cmake -DPICO_BOARD=pico_w ..
make
```

In the instructions above, `PICO_SDK_FETCH_FROM_GIT` is set to ensure that the latest pico-sdk is fetched from GitHub. Alternatively, you can use a locally deployed SDK by setting the `PICO_SDK_PATH` environment variable.
After compilation, the firmware `picow-usb-failures.uf2` will be generated. Simply drag and drop it onto the device to install.


## Monitoring the Behavior

This demonstration outputs multiple states to the UART to indicate the USB connection status:

- Calling TinyUSB's `tud_ready()` function.
- Reading the RP2040's `USBCTRL_REGS` register.
- Reading the GPIO VBUS status (`GPIO24` for Pico and `WL_GPIO02` for Pico W).

These outputs help track the differences in USB handling between the devices.

During operation, status updates are output to the UART every second. To observe these outputs when USB is not connected, connect to the UART using the [Raspberry Pi Debug Probe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html).

When the Pico, powered by battery, is connected to a host PC via USB, the following changes in status occur:

```
Waiting for USB connection
BOARD=pico, TinyUSB not ready, USBCTRL_REG(value=0x00000015) disconnect, VBUS low
BOARD=pico, TinyUSB not ready, USBCTRL_REG(value=0x00000015) disconnect, VBUS low
BOARD=pico, TinyUSB not ready, USBCTRL_REG(value=0x00000015) disconnect, VBUS low
BOARD=pico, TinyUSB ready, USBCTRL_REG(value=0x40050005) connect, VBUS high
BOARD=pico, TinyUSB ready, USBCTRL_REG(value=0x40050005) connect, VBUS high
BOARD=pico, TinyUSB ready, USBCTRL_REG(value=0x40050005) connect, VBUS high

```

In contrast, when the Pico W, also powered by battery, is connected to the host PC via USB, it does not exhibit the same changes as the Pico:

```
Waiting for USB connection
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
```

When disconnected from the USB host while running on battery power, both Pico and Pico W change state as expected:

```
BOARD=pico, TinyUSB ready, USBCTRL_REG(value=0x40050005) connect, VBUS high
BOARD=pico, TinyUSB ready, USBCTRL_REG(value=0x40050005) connect, VBUS high
BOARD=pico, TinyUSB ready, USBCTRL_REG(value=0x40050005) connect, VBUS high
BOARD=pico, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
BOARD=pico, TinyUSB not ready, USBCTRL_REG(value=0x40050015) disconnect, VBUS low
```

The states of both Pico and Pico W correspond to the device connection status as seen from the host PC.
These differences in behavior between the devices can also be observed when they are powered by battery, booted in BOOTSEL mode, and then connected to a host PC. The battery-powered Pico will mount as an RPI RP2 disk on the host PC, but nothing happens with the Pico W.

## Circuit Diagram

Pending.

## References

- [Raspberry Pi Pico Datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)
- [Raspberry Pi Pico W Datasheet](https://datasheets.raspberrypi.com/picow/pico-w-datasheet.pdf)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
