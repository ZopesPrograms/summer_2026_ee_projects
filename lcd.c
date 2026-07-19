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

void lcd_init(int16_t width, int16_t height) {
    // Initializes LCD screen as SPI device
    lcd = spi_new(CS_PIN, SPI_MODE_0, 1000000);  // Try 10 MHz (faster than 1 MHz)

    // Assigns static width and height of LCD pixel-map/output to requested values:
    _width = width;
    _height = height;

    // Configures pins we are using
    //gpio_set_output(F_CS_PIN);
    gpio_set_output(RS_PIN);
    gpio_set_output(RST_PIN);

    // Initial pin states - deselect flash chip
    //gpio_write(F_CS_PIN, 1);
    gpio_write(RS_PIN, 1);
    gpio_write(CS_PIN, 1);

    // Hardware reset sequence
    gpio_write(RST_PIN, 1);
    timer_delay_ms(5);
    gpio_write(RST_PIN, 0);
    timer_delay_ms(20);
    gpio_write(RST_PIN, 1);
    timer_delay_ms(150);

    // Software reset
    lcd_writecommand(LCD_RESET_SOFT);
    timer_delay_ms(120);

    // Sleep out
    lcd_writecommand(LCD_SLEEP_OUT);
    timer_delay_ms(120);

    // Interface Pixel Format - 16-bit color (RGB565)
    lcd_writecommand(INTF_PIX_FORM);
    lcd_writedata(0x55);

    // Memory Access Control
    lcd_writecommand(MEM_ACC_CTRL);
    lcd_writedata(0x48);  // Changed from 0x0a - sets rotation and BGR order

    // Display Inversion Off
    lcd_writecommand(DISP_INV_OFF);

    // Power Control 1
    lcd_writecommand(LCD_POWER_CTRL_1);
    lcd_writedata(0x01); // +3.6250 V for positive gamma
    lcd_writedata(0x01); // -3.6250 V for negative gamma

    /*

    // Power Control 2
    lcd_writecommand(LCD_POWER_CTRL_2);
    lcd_writedata(0x43); // Step up by 5 from VCL1? Gamma bias control of 1.00X?
    lcd_writedata(0x00); // External VCL for voltage regulator output voltage

    // Power Control 3
    lcd_writecommand(LCD_POWER_CTRL_3);
    lcd_writedata(0x00); // Slow step-up-cycle (not stepping up here I guess).

    */

    // VCOM Control
    lcd_writecommand(VCOM_CTRL);
    lcd_writedata(0x00);
    lcd_writedata(0x48);

    // Display Function Control
    lcd_writecommand(DISP_FUNC_CTRL);
    lcd_writedata(0x00);
    lcd_writedata(0x22);  // Changed from 0x22
    lcd_writedata(0x3B);  // 480 lines

    // Positive Gamma Control
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

    // Negative Gamma Control
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

    // Display On
    lcd_writecommand(LCD_DISPLAY_ON);
    timer_delay_ms(100);
}

void lcd_writecommand(uint8_t c) {
    gpio_write(CS_PIN, 0);
    gpio_write(RS_PIN, 0);  // Command mode
    spi_write(lcd, &c, 1);
    gpio_write(CS_PIN, 1);  // Raise CS after command
}

void lcd_writedata(uint8_t c) {
    gpio_write(CS_PIN, 0);
    gpio_write(RS_PIN, 1);  // Data mode
    spi_write(lcd, &c, 1);
    gpio_write(CS_PIN, 1);  // Raise CS after data
}

void lcd_drawpixel(uint16_t x, uint16_t y, uint16_t color) {
    // Faster range checking, possible because x and y are unsigned
    if ((x >= _width) || (y >= _height)) return;

    // Set column address (x)
    gpio_write(CS_PIN, 0);
    gpio_write(RS_PIN, 0);
    spi_write(lcd, &COL_ADDR_SET, 1);
    gpio_write(CS_PIN, 1);

    // Send column start and end (big-endian)
    uint8_t x_data[4];
    x_data[0] = (x >> 8) & 0xFF;  // Start column MSB
    x_data[1] = x & 0xFF;          // Start column LSB
    x_data[2] = (x >> 8) & 0xFF;  // End column MSB
    x_data[3] = x & 0xFF;          // End column LSB

    gpio_write(CS_PIN, 0);
    gpio_write(RS_PIN, 1);
    spi_write(lcd, x_data, 4);
    gpio_write(CS_PIN, 1);

    // Set page address (y)
    gpio_write(CS_PIN, 0);
    gpio_write(RS_PIN, 0);
    spi_write(lcd, &PAGE_ADDR_SET, 1);
    gpio_write(CS_PIN, 1);

    // Send page start and end (big-endian)
    uint8_t y_data[4];
    y_data[0] = (y >> 8) & 0xFF;  // Start page MSB
    y_data[1] = y & 0xFF;          // Start page LSB
    y_data[2] = (y >> 8) & 0xFF;  // End page MSB
    y_data[3] = y & 0xFF;          // End page LSB

    gpio_write(CS_PIN, 0);
    gpio_write(RS_PIN, 1);
    spi_write(lcd, y_data, 4);
    gpio_write(CS_PIN, 1);

    // Write pixel color to RAM
    gpio_write(CS_PIN, 0);
    gpio_write(RS_PIN, 0);
    spi_write(lcd, &RAM_WR, 1);
    gpio_write(CS_PIN, 1);

    // Send color data (big-endian RGB565)
    uint8_t color_data[2];
    color_data[0] = (color >> 8) & 0xFF;  // Color MSB
    color_data[1] = color & 0xFF;         // Color LSB

    gpio_write(CS_PIN, 0);
    gpio_write(RS_PIN, 1);
    spi_write(lcd, color_data, 2);
    gpio_write(CS_PIN, 1);
}

spi_device_t *lcd_get_spi_dev() {
    return lcd;
}

void lcd_fillscreen(uint16_t color) {
    // TODO!
}