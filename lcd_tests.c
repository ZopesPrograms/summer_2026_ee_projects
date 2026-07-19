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
}
void lcd_device_test() {
    uint8_t  command  = READ_DISP_ID;
    uint8_t *response = malloc(4);
    spi_write_and_read(lcd_get_spi_dev(), &command, 1, response, 4);
    printf("received device id of: %x %x %x %x\n", response[0], response[1], response[2], response[3]);
    free(response);
}