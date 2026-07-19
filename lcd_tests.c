#include "lcd.h"
#include "timer.h"
#include "spi.h"
#include "malloc.h"
#include "printf.h"
#include <stdint.h>

void lcd_flicker_test() {
    lcd_writecommand(DISP_INV_ON);
    timer_delay_ms(100);
    lcd_writecommand(DISP_INV_OFF);
    timer_delay_ms(100);
}