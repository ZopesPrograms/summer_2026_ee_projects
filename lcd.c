#include "lcd.h"
#include "gpio.h"
#include "spi.h"
#include "timer.h"

static int _width;
static int _height;
static spi_device_t *lcd;

static const uint8_t COL_ADDR_SET  = 0x2A; // Sets column (x) address for pixel to be written or read
static const uint8_t PAGE_ADDR_SET = 0x2B; // Sets page (y) address for pixel to be written or read

static const uint8_t RAM_WR = 0x2C; // RAM WRITE to LCD display memory
static const uint8_t RAM_RD = 0x2E; // RAM READ from LCD display memory

void lcd_init(int16_t width, int16_t height) {
    if(RAM_RD == RAM_RD) { /* SPIN! */ } // To remove compiler warning
    // Initializes LCD screen as SPI device
    lcd = spi_new(CS_PIN, SPI_MODE_0, 1000000);

    // Assigns static width and height of LCD pixel-map/output to requested values:
    _width = width;
    _height = height;

    // Configures pins we are using (and one we are not, the flash chip select pin) to direct LED to be in output mode!
    gpio_set_output(RS_PIN);
    gpio_set_output(F_CS_PIN);

    // Toggle RST high then low to reset
    gpio_write(RST_PIN, 1);

    // Sets up default values of chip-select data/command select pins when lcd not doing anything:
    gpio_write(RS_PIN, 1);   // Default data/command select setting
    gpio_write(CS_PIN, 1);   // Default chip select setting
    gpio_write(F_CS_PIN, 1); // Default flash chip select setting (so it doesn't float and do weird stuff).

    timer_delay_ms(50);

    gpio_write(RST_PIN, 0);
    timer_delay_ms(10);
    
    gpio_write(RST_PIN, 1);
    timer_delay_ms(10);

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

    lcd_writecommand(LCD_DISPLAY_ON);
    timer_delay_ms(25);
}
void lcd_writecommand(uint8_t c) {
    // Start a communication by dipping to low on chip select line
    gpio_write(CS_PIN, 0);
    // Tells LCD this communication is a COMMAND by dipping on command/data line
    gpio_write(RS_PIN, 0);

    // Writes command byte by SPI
    spi_write(lcd, &c, 1);

    // Resets for next op
    gpio_write(RS_PIN, 1);
    gpio_write(CS_PIN, 1);
}

void lcd_writedata(uint8_t c) {
    // Start a communication by dipping to low on chip select line
    gpio_write(CS_PIN, 0);

    // Writes data byte by SPI
    spi_write(lcd, &c, 1);

    // Resets for next op
    gpio_write(CS_PIN, 1);
}

void lcd_drawpixel(uint16_t x, uint16_t y, uint16_t color) {
    // Faster range checking, possible because x and y are unsigned
    if ((x >= _width) || (y >= _height)) return;

    // Command to set column/x address for pixel to be drawn
    gpio_write(CS_PIN, 0);
    gpio_write(RS_PIN, 0);
    spi_write(lcd, &COL_ADDR_SET, 1);

    // Data for column address set sent to lcd
    gpio_write(RS_PIN, 1);

    // Parameter that identifies x position for display memory RAM to modify (sent by SPI)
    spi_write(lcd, (uint8_t*)&x, 2);

    // Command to set page/y address for pixel to be drawn
    gpio_write(RS_PIN, 0);
    spi_write(lcd, &PAGE_ADDR_SET, 1);

    // Data for page address set sent to lcd
    gpio_write(RS_PIN, 1);

    // Parameter that identifies y position for display memory RAM to modify (sent by SPI)
    spi_write(lcd, (uint8_t*)&y, 2);

    // Command to write pixel color to LCD display RAM
    gpio_write(RS_PIN, 0);
    spi_write(lcd, &RAM_WR, 1);

    // Color data sent to LCD to be written to display RAM
    gpio_write(RS_PIN, 1);
    spi_write(lcd, (uint8_t*)&color, 2);

    // Ends pixel draw operation by releasing chip select
    gpio_write(CS_PIN, 1);
}