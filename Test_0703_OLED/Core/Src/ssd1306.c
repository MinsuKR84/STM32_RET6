#include "ssd1306.h"

#include <string.h>

static uint8_t ssd1306_buffer[SSD1306_BUFFER_SIZE];
static SSD1306_CURSOR ssd1306_cursor;

void ssd1306_write_command(uint8_t command)
{
    uint8_t data[2] = {SSD1306_CONTROL_BYTE_COMMAND, command};
    HAL_I2C_Master_Transmit(SSD1306_I2C, SSD1306_I2C_SA_WRITE, data, sizeof(data), HAL_MAX_DELAY);
}

void ssd1306_write_data(uint8_t *buffer, uint16_t size)
{
    uint8_t data[17];
    uint16_t offset = 0;

    data[0] = SSD1306_CONTROL_BYTE_DATA;
    while (offset < size)
    {
        uint16_t chunk = (uint16_t)((size - offset) > 16 ? 16 : (size - offset));
        memcpy(&data[1], &buffer[offset], chunk);
        HAL_I2C_Master_Transmit(SSD1306_I2C, SSD1306_I2C_SA_WRITE, data, (uint16_t)(chunk + 1), HAL_MAX_DELAY);
        offset = (uint16_t)(offset + chunk);
    }
}

void charge_bump_setting(uint8_t charge_bump)
{
    ssd1306_write_command(CHARGE_BUMP_SETTING);
    ssd1306_write_command(charge_bump);
}

void set_contrast_control(uint8_t value)
{
    ssd1306_write_command(SET_CONTRAST_CONTROL);
    ssd1306_write_command(value);
}

void entire_display_off()
{
    ssd1306_write_command(ENTIRE_DISPLAY_OFF);
}

void entire_display_on()
{
    ssd1306_write_command(ENTIRE_DISPLAY_ON);
}

void set_normal_display()
{
    ssd1306_write_command(SET_NORMAL_DISPLAY);
}

void set_inverse_display()
{
    ssd1306_write_command(SET_INVERSE_DISPLAY);
}

void set_display_off()
{
    ssd1306_write_command(SET_DISPLAY_OFF);
}

void set_display_on()
{
    ssd1306_write_command(SET_DISPLAY_ON);
}

void set_lower_column_start_address_for_page_addressing_mode(uint8_t addr)
{
    ssd1306_write_command((uint8_t)(0x00 | (addr & 0x0F)));
}

void set_higher_column_start_address_for_page_addressing_mode(uint8_t addr)
{
    ssd1306_write_command((uint8_t)(0x10 | (addr & 0x0F)));
}

void set_memory_addressing_mode(uint8_t mode)
{
    ssd1306_write_command(SET_MEMORY_ADDRESSING_MODE);
    ssd1306_write_command(mode);
}

void set_column_address(uint8_t start, uint8_t end)
{
    ssd1306_write_command(SET_COLUMN_ADDRESS);
    ssd1306_write_command(start);
    ssd1306_write_command(end);
}

void set_page_address(uint8_t start, uint8_t end)
{
    ssd1306_write_command(SET_PAGE_ADDRESS);
    ssd1306_write_command(start);
    ssd1306_write_command(end);
}

void set_page_start_address_for_page_addressing_mode(uint8_t page)
{
    ssd1306_write_command((uint8_t)(0xB0 | (page & 0x07)));
}

void set_display_start_line(uint8_t start_line)
{
    ssd1306_write_command((uint8_t)(0x40 | (start_line & 0x3F)));
}

void set_segment_remap(uint8_t mapping)
{
    ssd1306_write_command(mapping ? 0xA1 : 0xA0);
}

void set_multiplex_ratio(uint8_t mux)
{
    ssd1306_write_command(SET_MULTIPLEX_RATIO);
    ssd1306_write_command(mux);
}

void set_com_output_scan_direction(uint8_t mode)
{
    ssd1306_write_command(mode ? 0xC8 : 0xC0);
}

void set_display_offset(uint8_t vertical_shift)
{
    ssd1306_write_command(SET_DISPLAY_OFFSET);
    ssd1306_write_command(vertical_shift);
}

void set_com_pins_hardware_config(uint8_t com_pin_config, uint8_t com_left_right_remap)
{
    ssd1306_write_command(SET_COM_PINS_HARDWARE_CONFIG);
    ssd1306_write_command((uint8_t)(0x02 | ((com_pin_config & 0x01) << 4) | ((com_left_right_remap & 0x01) << 5)));
}

void set_display_clock_divide_ratio_and_osc_freq(uint8_t divide_ratio, uint8_t osc_freq)
{
    ssd1306_write_command(SET_DISPLAY_CLOCK_DIVIDE_RATIO_AND_OSC_FREQ);
    ssd1306_write_command((uint8_t)((divide_ratio & 0x0F) | ((osc_freq & 0x0F) << 4)));
}

void set_pre_charge_period(uint8_t phase_1, uint8_t phase_2)
{
    ssd1306_write_command(SET_PRE_CHARGE_PERIOD);
    ssd1306_write_command((uint8_t)((phase_1 & 0x0F) | ((phase_2 & 0x0F) << 4)));
}

void set_v_comh_deselect_level(uint8_t deselect_level)
{
    ssd1306_write_command(SET_V_COMH_DESELECT_LEVEL);
    ssd1306_write_command(deselect_level);
}

void ssd1306_init()
{
    HAL_Delay(100);

    set_display_off();
    set_display_clock_divide_ratio_and_osc_freq(0x00, 0x08);
    set_multiplex_ratio(SSD1306_HEIGHT - 1);
    set_display_offset(0x00);
    set_display_start_line(0x00);
    charge_bump_setting(0x14);
    set_memory_addressing_mode(0x00);
    set_segment_remap(1);
    set_com_output_scan_direction(1);
    set_com_pins_hardware_config(1, 0);
    set_contrast_control(0x7F);
    set_pre_charge_period(0x01, 0x0F);
    set_v_comh_deselect_level(0x20);
    entire_display_off();
    set_normal_display();
    set_display_on();

    ssd1306_black_screen();
    ssd1306_update_screen();
}

void ssd1306_update_screen()
{
    set_column_address(0, SSD1306_WIDTH - 1);
    set_page_address(0, SSD1306_PAGE - 1);
    ssd1306_write_data(ssd1306_buffer, SSD1306_BUFFER_SIZE);
}

void ssd1306_black_screen()
{
    memset(ssd1306_buffer, 0x00, sizeof(ssd1306_buffer));
    ssd1306_set_cursor(0, 0);
}

void ssd1306_white_screen()
{
    memset(ssd1306_buffer, 0xFF, sizeof(ssd1306_buffer));
    ssd1306_set_cursor(0, 0);
}

void ssd1306_black_pixel(uint8_t x, uint8_t y)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    {
        return;
    }

    ssd1306_buffer[x + (y / 8) * SSD1306_WIDTH] &= (uint8_t)~(1 << (y % 8));
}

void ssd1306_white_pixel(uint8_t x, uint8_t y)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    {
        return;
    }

    ssd1306_buffer[x + (y / 8) * SSD1306_WIDTH] |= (uint8_t)(1 << (y % 8));
}

char ssd1306_write_char(SSD1306_FONT font, char ch)
{
    if (ch < ' ' || ch > '~')
    {
        ch = '?';
    }

    if ((ssd1306_cursor.x + font.width) > SSD1306_WIDTH ||
        (ssd1306_cursor.y + font.height) > SSD1306_HEIGHT)
    {
        return 0;
    }

    uint16_t index = (uint16_t)(ch - ' ') * font.height;
    for (uint8_t row = 0; row < font.height; row++)
    {
        uint16_t line = font.data[index + row];
        for (uint8_t col = 0; col < font.width; col++)
        {
            if (line & (0x8000 >> col))
            {
                ssd1306_white_pixel((uint8_t)(ssd1306_cursor.x + col), (uint8_t)(ssd1306_cursor.y + row));
            }
            else
            {
                ssd1306_black_pixel((uint8_t)(ssd1306_cursor.x + col), (uint8_t)(ssd1306_cursor.y + row));
            }
        }
    }

    ssd1306_cursor.x = (uint8_t)(ssd1306_cursor.x + font.width);
    return ch;
}

char ssd1306_write_string(SSD1306_FONT font, char *str)
{
    while (*str)
    {
        if (ssd1306_write_char(font, *str) == 0)
        {
            return 0;
        }
        str++;
    }

    return 1;
}

void ssd1306_set_cursor(uint8_t x, uint8_t y)
{
    ssd1306_cursor.x = x;
    ssd1306_cursor.y = y;
}

void ssd1306_enter()
{
    ssd1306_cursor.x = 0;
    ssd1306_cursor.y = (uint8_t)(ssd1306_cursor.y + 8);
}

void ssd1306_space()
{
    ssd1306_cursor.x = (uint8_t)(ssd1306_cursor.x + 4);
}
