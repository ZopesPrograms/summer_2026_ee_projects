#include "lcd.h"
#include "gpio.h"
#include "spi.h"
#include "timer.h"

static int _width;
static int _height;
static spi_device_t *lcd;

#define WR_STB gpio_write(WR_PIN, 0); timer_delay_ms(5); gpio_write(WR_PIN, 1)
#define WR_H   gpio_write(WR_PIN, 1)

#define CSL_RSL_WRL gpio_write(CS_PIN, 0); gpio_write(RS_PIN, 0); gpio_write(WR_PIN, 0)
#define CSL_RSL_WRH gpio_write(CS_PIN, 0); gpio_write(RS_PIN, 0); gpio_write(WR_PIN, 1)
#define CSL_RSH_WRL gpio_write(CS_PIN, 0); gpio_write(RS_PIN, 1); gpio_write(WR_PIN, 0)
#define CSL_RSH_WRH gpio_write(CS_PIN, 0); gpio_write(RS_PIN, 1); gpio_write(WR_PIN, 1)

// Writes byte to lower 8 data pins (DB0->DB7)
static void write_lower8(uint8_t dat) {
    gpio_write(DB0_PIN+0, c&1);
    c >> 1;
    gpio_write(DB0_PIN+1, c&1);
    c >> 1;
    gpio_write(DB0_PIN+2, c&1);
    c >> 1;
    gpio_write(DB0_PIN+3, c&1);
    c >> 1;
    gpio_write(DB0_PIN+4, c&1);
    c >> 1;
    gpio_write(DB0_PIN+5, c&1);
    c >> 1;
    gpio_write(DB0_PIN+6, c&1);
    c >> 1;
    gpio_write(DB0_PIN+7, c&1);
}

// Writes byte to upper 8 data pins (DB8->DB15)
static void write_upper8(uint8_t dat) {
    gpio_write(DB0_PIN+8, c&1);
    c >> 1;
    gpio_write(DB0_PIN+9, c&1);
    c >> 1;
    gpio_write(DB0_PIN+10, c&1);
    c >> 1;
    gpio_write(DB0_PIN+11, c&1);
    c >> 1;
    gpio_write(DB0_PIN+12, c&1);
    c >> 1;
    gpio_write(DB13_PIN+0, c&1);
    c >> 1;
    gpio_write(DB13_PIN+1, c&1);
    c >> 1;
    gpio_write(DB13_PIN+2, c&1);
}

void lcd_init(int16_t width, int16_t height) {
    // Initializes LCD screen as SPI device
    lcd = spi_new(CS_PIN, SPI_MODE_0, 1000000);

    // Assigns static width and height of LCD pixel-map/output to requested values:
    _width = width;
    _height = height;

    // Configures pins we are using (and one we are not, the flash chip select pin) to direct LED to be in output mode!
    gpio_set_output(RS_PIN);
    gpio_set_output(F_CS_PIN);

    // Toggle RST low to reset
    gpio_write(RST_PIN, 1);

    // Sets up default values of chip-select data/command select, and write pins when lcd not doing anything:
    gpio_write(RS_PIN, 1);   // Default data/command select setting
    gpio_write(CS_PIN, 1);   // Default chip select setting
    gpio_write(WR_PIN, 1);   // Default write pin setting
    gpio_write(F_CS_PIN, 1); // Default flash chip select setting (so it doesn't float and do weird stuff).

    timer_delay_ms(50);

    gpio_write(RST_PIN, 0);
    timer_delay_ms(10);
    
    gpio_write(RST_PIN, 1);
    timer_delay_ms(10);

    #ifdef ILI9486

    // Resets lcd software + all previous commands! Sets everything to default params.
    lcd_writecommand(LCD_RESET_SOFT);
    lcd_writedata(0x00);
    timer_delay_ms(50);

    // Turns display off for setup.
    lcd_writecommand(LCD_DISPLAY_OFF);
    lcd_writedata(0x00);

    lcd_writecommand(LCD_POWER_CTRL_1);
    lcd_writedata(0x0d);
    lcd_writedata(0x0d);

    lcd_writecommand(LCD_POWER_CTRL_2);
    lcd_writedata(0x43);
    lcd_writedata(0x00);

    lcd_writecommand(LCD_POWER_CTRL_3);
    lcd_writedata(0x00);

    lcd_writecommand(VCOM_CTRL);
    lcd_writedata(0x00);
    lcd_writedata(0x48);

    lcd_writecommand(DISP_FUNC_CTRL);
    lcd_writedata(0x00);
    lcd_writedata(0x22);
    lcd_writedata(0x3b);

    lcd_writecommand(POS_GAMMA_CTRL);
    lcd_writedata(0x0f);
    lcd_writedata(0x24);
    lcd_writedata(0x1c);
    lcd_writedata(0x0a);
    lcd_writedata(0x0f);
    lcd_writedata(0x08);
    lcd_writedata(0x43);
    lcd_writedata(0x88);
    lcd_writedata(0x32);
    lcd_writedata(0x0f);
    lcd_writedata(0x10);
    lcd_writedata(0x06);
    lcd_writedata(0x0f);
    lcd_writedata(0x07);
    lcd_writedata(0x00);

    lcd_writecommand(NEG_GAMMA_CTRL);
    lcd_writedata(0x0f);
    lcd_writedata(0x38);
    lcd_writedata(0x30);
    lcd_writedata(0x09);
    lcd_writedata(0x0f);
    lcd_writedata(0x0f);
    lcd_writedata(0x4e);
    lcd_writedata(0x77);
    lcd_writedata(0x3c);
    lcd_writedata(0x07);
    lcd_writedata(0x10);
    lcd_writedata(0x05);
    lcd_writedata(0x23);
    lcd_writedata(0x1b);
    lcd_writedata(0x00);

    lcd_writecommand(DISP_INV_OFF);
    lcd_writecommand(MEM_ACC_CTRL);
    lcd_writedata(0x0a);

    lcd_writecommand(INTF_PIX_FORM);
    lcd_writedata(0x55);

    lcd_writecommand(LCD_SLEEP_OUT);    
    timer_delay_ms(150);

    lcd_writecommand(LCD_DISPLAY_OUT);
    timer_delay_ms(25);

    #endif
}
void lcd_writecommand(uint8_t c) {
    // Start a communication by dipping to low on chip select line
    gpio_write(CS_PIN, 0);
    // Tells LCD this communication is a COMMAND by dipping on command/data line
    gpio_write(RS_PIN, 0);

    // Writes command bits to gpio pins
    write_upper8(0);
    write_lower8(c);

    // Dips write pin to send command
    WR_STB;

    // Resets for next op
    gpio_write(RS_PIN, 1);
    gpio_write(CS_PIN, 1);
}

void lcd_writedata(uint8_t c) {
    // Start a communication by dipping to low on chip select line
    gpio_write(CS_PIN, 0);

    // Writes data
    write_upper8(c >> 8);
    write_lower8(c);

    // Dips write pin to send data
    WR_STB;

    // Resets for next op
    gpio_write(CS_PIN, 1);
}

void lcd_drawpixel(uint16_t x, uint16_t y, uint16_t color) {
    // Faster range checking, possible because x and y are unsigned
    if ((x >= _width) || (y >= _height)) return;

    // Command to set column/x address for pixel to be drawn
    gpio_write(CS_PIN, 0);
    gpio_write(RS_PIN, 0);
    PORTC = HX8357_CASET;
    WR_STB;

    // Data for column address set sent to lcd
    gpio_write(RS_PIN, 1);
    PORTC = x>>8; 
    WR_STB;
    PORTC = x; 
    WR_STB;
    PORTC = x>>8; 
    WR_STB;
    PORTC = x;
    WR_STB;

    // Command to set page/y address for pixel to be drawn
    gpio_write(RS_PIN, 0);
    PORTC = HX8357_PASET; 
    WR_STB;

    // Data for page address set sent to lcd
    gpio_write(RS_PIN, 1);
    PORTC = y>>8;
    WR_STB;
    PORTC = y; 
    WR_STB;
    PORTC = y>>8; 
    WR_STB;
    PORTC = y; 
    WR_STB;

    // Command to write pixel color to LCD display RAM
    gpio_write(RS_PIN, 0);
    PORTC = HX8357_RAMWR; 
    WR_STB;

    // Color data sent to LCD to be written to display RAM
    gpio_write(RS_PIN, 1);
    PORTC = color; 
    PORTA = color>>8;
    WR_STB;
}
