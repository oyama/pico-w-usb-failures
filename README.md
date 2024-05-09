# Battery-Powered Raspberry Pi Pico W and USB Type-C Connection Issue

This repository investigates a specific issue that occurs when the battery-powered Raspberry Pi Pico W is connected to a host PC via a USB Type-C connection. Specifically, the following connection sequence fails to initiate USB communication:

1. Supply power to Pico W's VSYS.
2. Connect the USB cable to the micro USB connector of the Pico W.
3. Connect the USB cable to the Type-C connector of the host PC.

This phenomenon does not occur if the cable is connected in the reverse order from the host PC to the Pico W, or when using a USB Type-A connection. And interestingly, it does not occur with the standard Raspberry Pi Pico either.

## Technical Investigation

To understand and diagnose this issue, the following methods were employed:

- GPIO: monitored GPIOs connected to VBUS (`GPIO24` for Pico and `WL_GPIO 2` for Pico W).
- RP2040 register: monitored the `USB: SIE_STATUS` register on the RP2040.
- TinyUSB: call `tud_ready()` function to check if USB communication was ready.

The monitoring results matched the USB communication state, but the behavior of the Pico W when connected to the host PC was not as expected. Further behavior is detailed below.

### Build and Install Instructions

The build requires the [pico-sdk](https://github.com/raspberrypi/pico-sdk) build environment; refer to [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) to prepare the toolchain for your platform.

#### Build for Pico

```bash
mkdir build; cd build
PICO_SDK_FETCH_FROM_GIT=1 cmake -DPICO_BOARD=pico ..
make
```

#### Build for Pico W

```bash
mkdir build; cd build
PICO_SDK_FETCH_FROM_GIT=1 cmake -DPICO_BOARD=pico_w ..
make
```

The above examples specify the environment variable `PICO_SDK_FETCH_FROM_GIT` to download the pico-sdk from GitHub. If you want to specify a locally deployed pico-sdk, you should set it with the `PICO_SDK_PATH` environment variable.
Once built, the firmware `picow-usb-failures.uf2` will be generated. Simply drag and drop it onto your device to install.

### Circuit Diagram

![circut_diagram](https://github.com/oyama/pico-w-usb-failures/assets/27072/064b206d-8506-4cf4-a0ed-b6e6422890a5)

## Monitoring USB Connection Status

Firmware from this repository monitors the USB connection status of Pico and Pico W and continuously reports the USB connection status to the UART every second.
To observe the UART output under battery power, use the [Raspberry Pi Debug Probe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html) to monitor the UART.
For the Pico, when connected to the host PC, the status changes as follows:
```
Waiting for USB connection
BOARD=pico, TinyUSB not ready, USB_SIE_STATUS=0x00000015 disconnect, VBUS low
BOARD=pico, TinyUSB not ready, USB_SIE_STATUS=0x00000015 disconnect, VBUS low
BOARD=pico, TinyUSB not ready, USB_SIE_STATUS=0x00000015 disconnect, VBUS low
BOARD=pico, TinyUSB ready, USB_SIE_STATUS=0x40050005 connect, VBUS high
BOARD=pico, TinyUSB ready, USB_SIE_STATUS=0x40050005 connect, VBUS high
BOARD=pico, TinyUSB ready, USB_SIE_STATUS=0x40050005 connect, VBUS high
```
However, for the Pico W, even when the host PC's type-C cable is connected, the status does not change:
```
Waiting for USB connection
BOARD=pico_w, TinyUSB not ready, USB_SIE_STATUS=0x40050015 disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USB_SIE_STATUS=0x40050015 disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USB_SIE_STATUS=0x40050015 disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USB_SIE_STATUS=0x40050015 disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USB_SIE_STATUS=0x40050015 disconnect, VBUS low
BOARD=pico_w, TinyUSB not ready, USB_SIE_STATUS=0x40050015 disconnect, VBUS low
```

The USB connection status recognized by the devices matches the device recognition status on the host PC side. Interestingly, if you boot the Pico in `BOOTSEL mode` while powered by VSYS and connect it to the host PC, the `RPI RP2` disk will mount, but in the case of Pico W, there is no change in USB connection status, and the disk does not mount.

### Actual voltage of VBUS

A multimeter was used to measure the change in voltage on `VBUS`(pin 40) and `GND`(pin 38) of Pico and Pico W. Power is supplied to `VSYS` under all conditions.

Pico:

- USB cable disconnected: `0.42 V`
- USB cable connected to Pico, and finally to host: `5.17 V`
- USB cable connected to host, and finally to Pico: `5.17 V`

Pico W:

- USB cable disconnected: `1.36 V`
- USB cable connected to Pico W, and finally to host: `0.97..1.09 V` (Multimeter readings are not stable)
- USB cable connected to host, and finally to Pico W: `5.16 V`

## Temporary solutions

### Software solution

This problem can be avoided by setting `WL_GPIO 2` to __Low__.
```diff
     cyw43_arch_init();
+    cyw43_arch_gpio_put(2, 0);
```
This is not a good approach, although the behaviour is as expected.

### Hardware solution

This problem can be avoided by pull-down the `VBUS`.

![Pico W USB Pull-down](https://github.com/oyama/pico-w-usb-failures/assets/27072/bc719325-4708-4c03-8ac4-2c62f3b2c0bb)

Pull-down resistor `R1` works as expected at around 1 kOhm. Considering the USB suspend requirements, around 10 kOhm may be appropriate, but this has not been tested.

## Acknowledgements

I would like to express my gratitude to the [Raspberry Pi Pico forum](https://forums.raspberrypi.com/viewtopic.php?t=370292) for their invaluable advice and insights, which greatly assisted in the development of the solutions documented in this README. Your contributions have been instrumental in making these improvements possible.

## References

- [Raspberry Pi Pico Datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)
- [Raspberry Pi Pico W Datasheet](https://datasheets.raspberrypi.com/picow/pico-w-datasheet.pdf)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
