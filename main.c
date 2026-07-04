#include "spi.h"
#include "lcd.h"
#include "gpio.h"
#include "uart.h"
#include "timer.h"

int main() {
    timer_init();
    uart_init();
    spi_init();
    lcd_init(480, 320);

    lcd_drawpixel(99, 99, 0xAC4F);
    lcd_drawpixel(100, 99, 0xAC4F);
    lcd_drawpixel(101, 99, 0xAC4F);

    lcd_drawpixel(99, 100, 0xAC4F);
    lcd_drawpixel(100, 100, 0xAC4F);
    lcd_drawpixel(101, 100, 0xAC4F);

    lcd_drawpixel(99, 101, 0xAC4F);
    lcd_drawpixel(100, 101, 0xAC4F);
    lcd_drawpixel(101, 101, 0xAC4F);

    while(1) { /* SPIN!!! */ }
    return 0;
}