#include "spi.h"
#include "lcd.h"
#include "gpio.h"
#include "uart.h"
#include "timer.h"

int main() {
    timer_init();
    uart_init();
    spi_init();
    lcd_init(320, 480);

    lcd_drawpixel(99, 99, 0x07E0);
    lcd_drawpixel(100, 99, 0x07E0);
    lcd_drawpixel(101, 99, 0x07E0);

    lcd_drawpixel(99, 100, 0x07E0);
    lcd_drawpixel(100, 100, 0x07E0);
    lcd_drawpixel(101, 100, 0x07E0);

    lcd_drawpixel(99, 101, 0x07E0);
    lcd_drawpixel(100, 101, 0x07E0);
    lcd_drawpixel(101, 101, 0x07E0);

    //lcd_fillscreen(0xF800);

    while(1) { /*uart_putchar('a');*/ }
    return 0;
}