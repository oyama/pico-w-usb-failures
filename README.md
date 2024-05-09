# Battery-Powered Raspberry Pi Pico W and USB Type-C Connection Issue

This repository investigates a specific issue that occurs when the battery-powered Raspberry Pi Pico W is connected to a host PC via a USB Type-C connection. Specifically, the following connection sequence fails to initiate USB communication:

1. Supply power to Pico W's VSYS.
2. Connect the USB cable to the micro USB connector of the Pico W.
3. Connect the USB cable to the Type-C connector of the host PC.

This phenomenon does not occur if the cable is connected in the reverse order from the host PC to the Pico W, or when using a USB Type-A connection. And interestingly, it does not occur with the standard Raspberry Pi Pico either.

## Investigation Methodology

To understand and diagnose this issue, the following methods were employed:

- GPIO: monitored GPIOs connected to VBUS (`GPIO24`[^1] for Pico and `WL_GPIO 2`[^2] for Pico W).
- RP2040 register: monitored the `USB: SIE_STATUS` register[^3] on the RP2040.
- TinyUSB: call `tud_ready()` function to check if USB communication was ready.
- Voltage on VBUS: the voltage on `VBUS` (between pins 40 VBUS and 38 GND) was measured with a multimeter.

Identical tests were also carried out on six Pico Ws to rule out the possibility of the problem being a problem with a particular individual Pico W.
The monitoring results matched the USB communication state, but the behavior of the Pico W when connected to the host PC was not as expected. Further behavior is detailed below.

### Build and Install Instructions

The pico-sdk[^4] build environment is required to build the firmware used for this monitoring, see  _Getting started with Raspberry Pi Pico_[^5] to prepare the toolchain for your platform.

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

## Results

The firmware in this repository monitors the USB connection status of Pico and Pico W and continuously reports the USB connection status to the UART and onboard LEDs every second.
To observe the UART output under battery power, use the _Raspberry Pi Debug Probe_[^6] to monitor the UART.
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

The USB connection status recognized by the devices matches the device recognition status on the host PC side.

### Actual voltage of VBUS

A multimeter was used to measure the change in voltage on `VBUS`(pin 40) and `GND`(pin 38) of Pico and Pico W. Power is supplied to `VSYS` under all conditions.

| Device  | Voltage with USB Disconnected | Voltage after connecting to device first, then host | Voltage after connecting to host first, then device |
|---------|-------------------------------|----------------------------------------------------|-----------------------------------------------------|
| Pico    | 0.42 V                        | 5.17 V                                             | 5.17 V                                               |
| Pico W  | 1.36 V                        | 0.97..1.09 V (unstable)                            | 5.16 V                                               |

Under the conditions in question, the host PC does not supply VBUS power to the Pico W when the cable is connected.
Furthermore, the VBUS on the Pico W showed a voltage of `1.36 V` when no cable was connected. This exceeds the `vSafe0V` (0-0.8V) defined by USB Power Delivery[^7].

## Conclusion

The VBUS of the Pico W during battery operation exceeds `vSafe0V` (0-0.8V). It is therefore suspected that the USB controller of the host PC accepting the connection has deactivated the VBUS supply for safety reasons. micro USB and Type-C cable circuits for mixed use are defined in USB Type-C Spec[^8] as _Legacy Cable Assemblies_ which is defined as. The circuits and states expected when connecting devices using these cables are then explicitly stated. A device with a VBUS in excess of `vSafe0V`, as in this Issue, connecting to a host PC with Type-C would have unintended consequences for the user.

It would be desirable if Pico W could address this issue by changing the circuitry or firmware, and it would be useful to mention this issue in the Pico W datasheet. I have already reported this issue to pico-feedback[^9]. Fortunately, there are two measures that end-users can adopt now.

### Software solution

This problem can be avoided by setting `WL_GPIO 2` to __Low__.
```diff
     cyw43_arch_init();
+    cyw43_arch_gpio_put(2, 0);
```
This method shows the expected behaviour, but with the limitation that the use case where you want to get the VBUS status via GPIOs will not work.

### Hardware solution

This problem can be avoided by pull-down the `VBUS`.

![Pico W USB Pull-down](https://github.com/oyama/pico-w-usb-failures/assets/27072/bc719325-4708-4c03-8ac4-2c62f3b2c0bb)

Pull-down resistor `R1` works as expected at around 1 kOhm. Considering the USB suspend requirements, around 10 kOhm may be appropriate, but this has not been tested.

### Limitation

These investigations and solutions are limited to using Pico W as a USB device; using Pico W as a USB host has not been investigated.

## Acknowledgements

I would like to express my gratitude to the Raspberry Pi Pico forum[^10] for their invaluable advice and insights, which greatly assisted in the development of the solutions documented in this README. Your contributions have been instrumental in making these improvements possible.

## References

[^1]: [Raspberry Pi Pico Datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)
[^2]: [Raspberry Pi Pico W Datasheet](https://datasheets.raspberrypi.com/picow/pico-w-datasheet.pdf)
[^3]: [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
[^4]: [pico-sdk](https://github.com/raspberrypi/pico-sdk)
[^5]: [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
[^6]: [Raspberry Pi Debug Probe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html) 
[^7]: [Universal Serial Bus Power Delivery Specification](https://usb.org/document-library/usb-power-delivery)
[^8]: [USB Type-C® Cable and Connector Specification](https://www.usb.org/document-library/usb-type-cr-cable-and-connector-specification-release-23)
[^9]: [raspberrypi/pico-feedback](https://github.com/raspberrypi/pico-feedback/issues/391)
[^10]: [Raspberry Pi Pico forum](https://forums.raspberrypi.com/viewtopic.php?t=370292)

