#include <hardware/regs/addressmap.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <tusb.h>

#if defined(PICO_BOARD_PICO_W)
#include <pico/cyw43_arch.h>
#define TARGET_PICO_BOARD  "pico_w"
#else
#define TARGET_PICO_BOARD  "pico"
#endif

#define PICO_VBUS_GPIO     24  // Pico   IP VBUS sense - high if VBUS is present, else low
#define PICOW_VBUS_WL_GPIO  2  // Pico-W IP VBUS sense - high if VBUS is present, else low
#define USBCTRL_REG        ((volatile uint32_t *)(USBCTRL_REGS_BASE + 0x50))
#define ANSI_RED           "\e[31m"
#define ANSI_GREEN         "\e[32m"
#define ANSI_CLEAR         "\e[0m"


static void init_vbus_gpio(void) {
#if defined(PICO_BOARD_PICO_W)
    cyw43_arch_init();
#else
    gpio_init(PICO_VBUS_GPIO);
    gpio_set_dir(PICO_VBUS_GPIO, GPIO_IN);
#endif
}

static bool is_usbctrl_reg_connected(void) {
    return (*USBCTRL_REG & 0x10) == 0x00;
}

static bool vbus_supplied(void) {
#if defined(PICO_BOARD_PICO_W)
    return cyw43_arch_gpio_get(PICOW_VBUS_WL_GPIO);
#else
    return gpio_get(PICO_VBUS_GPIO);
#endif
}

int main(void) {
    stdio_init_all();
    init_vbus_gpio();

    printf("Waiting for USB connection\n");
    while (true) {
        printf("BOARD=%s, ", TARGET_PICO_BOARD);
        printf("TinyUSB %s, ",
               (tud_ready()
                   ? ANSI_GREEN "ready" ANSI_CLEAR
                   : ANSI_RED "not ready" ANSI_CLEAR));
        printf("USBCTRL_REG(value=0x%08lX) %s, ",
               *USBCTRL_REG,
               (is_usbctrl_reg_connected()
                   ? ANSI_GREEN "connect" ANSI_CLEAR
                   : ANSI_RED "disconnect" ANSI_CLEAR));
        printf("VBUS %s\n",
               (vbus_supplied()
                   ? ANSI_GREEN "high" ANSI_CLEAR
                   : ANSI_RED "low" ANSI_CLEAR));
        sleep_ms(1000);
    }
}
