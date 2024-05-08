#include <hardware/regs/addressmap.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <tusb.h>

#if defined(RASPBERRYPI_PICO_W)
#include <pico/cyw43_arch.h>
#define TARGET_PICO_BOARD  "pico_w"
#else
#define TARGET_PICO_BOARD  "pico"
#endif

#define USB_SIE_STATUS_REG        ((volatile uint32_t *)(USBCTRL_REGS_BASE + 0x50))  // SIE status register
#define USB_SIE_STATUS_SUSPENDED  0b00000000000000000000000000010000                 // Bus in suspended state

#define COLOR_RED(format)    ("\e[31m" format "\e[0m")
#define COLOR_GREEN(format)  ("\e[32m" format "\e[0m")


static void init_vbus_gpio(void) {
#if defined(RASPBERRYPI_PICO_W)
    cyw43_arch_init();
#else
    gpio_init(PICO_VBUS_PIN);
    gpio_set_dir(PICO_VBUS_PIN, GPIO_IN);
#endif
}

static bool is_usb_sie_status_suspended(void) {
    return *USB_SIE_STATUS_REG & USB_SIE_STATUS_SUSPENDED;
}

static bool vbus_supplied(void) {
#if defined(RASPBERRYPI_PICO_W)
    return cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
#else
    return gpio_get(PICO_VBUS_PIN);
#endif
}

int main(void) {
    stdio_init_all();
    init_vbus_gpio();

    printf("Waiting for USB connection\n");
    while (true) {
        printf("BOARD=%s, ", TARGET_PICO_BOARD);
        printf("TinyUSB %s, ",
            tud_ready() ? COLOR_GREEN("ready") : COLOR_RED("not ready"));
        printf("USB_SIE_STATUS=0x%08lX %s, ",
            *USB_SIE_STATUS_REG,
            (!is_usb_sie_status_suspended() ? COLOR_GREEN("connect") : COLOR_RED("disconnect")));
        printf("VBUS %s\n",
            vbus_supplied() ? COLOR_GREEN("high") : COLOR_RED("low"));
        sleep_ms(1000);
    }
}
