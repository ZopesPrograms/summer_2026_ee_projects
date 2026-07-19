#include "spi.h"
#include "lcd.h"
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include "lcd_tests.h"

int main() {
    timer_init();
    uart_init();
    spi_init();
    lcd_init(320, 480);

    for(int i = 100; i < 300; i++) {
        for(int j = 100; j < 300; j++) {
            lcd_drawpixel(i, j, 0x0FE0);
        }
    }
    while(1) { lcd_flicker_test(); }
    return 0;
}