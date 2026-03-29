#include "OledController.hpp"
#include "Dispatcher.hpp"
#include "string.h"
#include "utils/Debug.hpp"
#include <math.h>
#include <stdlib.h>
#include <string.h> // For memcpy

// OLED OLED height in pixels
static uint8_t oled_height = 64;

// OLED width in pixels
static uint8_t oled_width = 128;

// Screenbuffer
static uint8_t OLED_Buffer[OLED_BUFFER_SIZE];

// Screen object
static OLED_t OLED;

void oledDispatch(const CoprocReq_OledReq& request) {
    switch (request.which_oledCmd) {
    case CoprocReq_OledReq_init_tag:
        oledInit(request.oledCmd.init);
        break;

    case CoprocReq_OledReq_fill_tag:
        oledFill(OLED_COLOR(request.oledCmd.fill));
        break;

    case CoprocReq_OledReq_update_tag:
        oledUpdateScreen();
        break;

    case CoprocReq_OledReq_drawPixel_tag: {
        const auto& drawPixel = request.oledCmd.drawPixel;
        oledDrawPixel(drawPixel.x, drawPixel.y, OLED_COLOR(drawPixel.color));
        break;
    }
    case CoprocReq_OledReq_writeString_tag: {

        const auto& writeString = request.oledCmd.writeString;
        char text[33];
        strcpy(text, writeString.text);
        FontDef* font;
        switch (writeString.font) {
        case CoprocReq_OledFont_OLED_FONT_6X8:
            font = &Font_6x8;
            break;
        case CoprocReq_OledFont_OLED_FONT_7X10:
            font = &Font_7x10;
            break;
        case CoprocReq_OledFont_OLED_FONT_11X18:
            font = &Font_11x18;
            break;
        case CoprocReq_OledFont_OLED_FONT_16X26:
            font = &Font_16x26;
            break;
        default:
            font = &Font_7x10;
        }
        oledWriteString(text, *font, OLED_COLOR(writeString.color));
        break;
    }
    case CoprocReq_OledReq_setCursor_tag: {
        const auto& setCursor = request.oledCmd.setCursor;
        oledSetCursor(setCursor.x, setCursor.y);
        break;
    }

    case CoprocReq_OledReq_drawLine_tag: {
        const auto& drawLine = request.oledCmd.drawLine;
        oledDrawLine(drawLine.x1, drawLine.y1, drawLine.x2, drawLine.y2,
            OLED_COLOR(drawLine.color));
        break;
    }

    case CoprocReq_OledReq_drawArc_tag: {
        const auto& drawArc = request.oledCmd.drawArc;
        oledDrawArc(drawArc.x, drawArc.y, drawArc.radius, drawArc.start_angle,
            drawArc.sweep, OLED_COLOR(drawArc.color));
        break;
    }

    case CoprocReq_OledReq_drawCircle_tag: {
        const auto& drawCircle = request.oledCmd.drawCircle;
        oledDrawCircle(drawCircle.x, drawCircle.y, drawCircle.radius,
            OLED_COLOR(drawCircle.color));
        break;
    }

    case CoprocReq_OledReq_drawRectangle_tag: {
        const auto& drawRectangle = request.oledCmd.drawRectangle;
        oledDrawRectangle(drawRectangle.x1, drawRectangle.y1, drawRectangle.x2,
            drawRectangle.y2, OLED_COLOR(drawRectangle.color));
        break;
    }
    };
}

bool oledTestConnection() { return I2Cdev_IsDeviceReady(OLED_I2C_ADDR, 1, 10); }

// Initialize the oled screen
void oledInit(const CoprocReq_OledInit& init) {

    if (oledTestConnection()) {
        // Init OLED
        oledSetDisplayOn(false); //display off

        oledWriteCommand(0x20); //Set Memory Addressing Mode
        oledWriteCommand(
            0x00); // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
        // 10b,Page Addressing Mode (RESET); 11b,Invalid

        oledWriteCommand(
            0xB0); //Set Page Start Address for Page Addressing Mode,0-7

        if (init.rotate) {
            oledWriteCommand(0xC0); // Mirror vertically
        } else {
            oledWriteCommand(0xC8); //Set COM Output Scan Direction
        }

        oledWriteCommand(0x00); //---set low column address
        oledWriteCommand(0x10); //---set high column address

        oledWriteCommand(0x40); //--set start line address - CHECK

        oledSetContrast(0xFF);

        if (init.rotate) {
            oledWriteCommand(0xA0); // Mirror horizontally
        } else {
            oledWriteCommand(0xA1); //--set segment re-map 0 to 127 - CHECK
        }

        if (init.inverseColor) {
            oledWriteCommand(0xA7); //--set inverse color
        } else {
            oledWriteCommand(0xA6); //--set normal color
        }

        oledWriteCommand(0xA8); //--set multiplex ratio(1 to 64) - CHECK

        if (init.height == 32) {
            oledWriteCommand(0x1F); //
        } else if (init.height == 64) {
            oledWriteCommand(0x3F); //
        } else {
            //TO DO warning message
        }

        oledWriteCommand(
            0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

        oledWriteCommand(0xD3); //-set display offset - CHECK
        oledWriteCommand(0x00); //-not offset

        oledWriteCommand(
            0xD5); //--set display clock divide ratio/oscillator frequency
        oledWriteCommand(0xF0); //--set divide ratio

        oledWriteCommand(0xD9); //--set pre-charge period
        oledWriteCommand(0x22); //

        oledWriteCommand(0xDA); //--set com pins hardware configuration - CHECK

        if (init.height == 32) {
            oledWriteCommand(0x02);
        } else if (init.height == 64) {
            oledWriteCommand(0x12);
        } else {
            //TO DO warning message
        }

        oledWriteCommand(0xDB); //--set vcomh
        oledWriteCommand(0x20); //0x20,0.77xVcc

        oledWriteCommand(0x8D); //--set DC-DC enable
        oledWriteCommand(0x14); //
        oledSetDisplayOn(true); //--turn on OLED panel

        // Clear screen
        oledFill(Black);

        // Flush buffer to screen
        oledUpdateScreen();

        // Set default values for screen object
        OLED.CurrentX = 0;
        OLED.CurrentY = 0;

    } else {
        DEBUG("Oled not connected-init\n");
        CoprocStat status = {
            .which_payload = CoprocStat_faultStat_tag,
            .payload = {
                .faultStat = {
                    .which_fault = CoprocStat_FaultStat_oledFault_tag,
                },
            },
        };
        dispatcherEnqueueStatus(status);
    }
}

// Fill the whole screen with the given color
void oledFill(OLED_COLOR color) {
    /* Set memory */
    uint32_t i;

    for (i = 0; i < sizeof(OLED_Buffer); i++) {
        OLED_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

// Write the screenbuffer with changed to the screen
void oledUpdateScreen(void) {
    if (!oledTestConnection()) {
        DEBUG("Oled not connected\n");
        CoprocStat status = {
            .which_payload = CoprocStat_faultStat_tag,
            .payload = {
                .faultStat = {
                    .which_fault = CoprocStat_FaultStat_oledFault_tag,
                },
            },
        };
        dispatcherEnqueueStatus(status);
    }
    // Write data to each page of RAM. Number of pages
    // depends on the screen height:
    //
    //  * 32px   ==  4 pages
    //  * 64px   ==  8 pages
    //  * 128px  ==  16 pages
    for (uint8_t i = 0; i < oled_height / 8; i++) {
        oledWriteCommand(0xB0 + i); // Set the current RAM page address.
        oledWriteCommand(0x00);
        oledWriteCommand(0x10);
        oledWriteData(&OLED_Buffer[oled_width * i], oled_width);
    }
}

//    Draw one pixel in the screenbuffer
//    X => X Coordinate
//    Y => Y Coordinate
//    color => Pixel color
void oledDrawPixel(uint8_t x, uint8_t y, OLED_COLOR color) {
    if (x >= oled_width || y >= oled_height) {
        // Don't write outside the buffer
        return;
    }

    // Check if pixel should be inverted
    if (OLED.Inverted) {
        color = (OLED_COLOR)!color;
    }

    // Draw in the right color
    if (color == White) {
        OLED_Buffer[x + (y / 8) * oled_width] |= 1 << (y % 8);
    } else {
        OLED_Buffer[x + (y / 8) * oled_width] &= ~(1 << (y % 8));
    }
}

// Draw 1 char to the screen buffer
// ch       => char om weg te schrijven
// Font     => Font waarmee we gaan schrijven
// color    => Black or White
char oledWriteChar(char ch, FontDef Font, OLED_COLOR color) {
    uint32_t i, b, j;

    // Check if character is valid
    if (ch < 32 || ch > 126)
        return 0;

    // Check remaining space on current line
    if (oled_width < (OLED.CurrentX + Font.FontWidth)
        || oled_height < (OLED.CurrentY + Font.FontHeight)) {
        // Not enough space on current line
        return 0;
    }

    // Use the font to write
    for (i = 0; i < Font.FontHeight; i++) {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++) {
            if ((b << j) & 0x8000) {
                oledDrawPixel(
                    OLED.CurrentX + j, (OLED.CurrentY + i), (OLED_COLOR)color);
            } else {
                oledDrawPixel(
                    OLED.CurrentX + j, (OLED.CurrentY + i), (OLED_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    OLED.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}

// Write full string to screenbuffer
char oledWriteString(const char* str, FontDef Font, OLED_COLOR color) {
    // Write until null-byte
    while (*str) {
        if (oledWriteChar(*str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

// Position the cursor
void oledSetCursor(uint8_t x, uint8_t y) {
    OLED.CurrentX = x;
    OLED.CurrentY = y;
}

// Draw line by Bresenhem's algorithm
void oledDrawLine(
    uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_COLOR color) {
    int32_t deltaX = abs(x2 - x1);
    int32_t deltaY = abs(y2 - y1);
    int32_t signX = ((x1 < x2) ? 1 : -1);
    int32_t signY = ((y1 < y2) ? 1 : -1);
    int32_t error = deltaX - deltaY;
    int32_t error2;

    oledDrawPixel(x2, y2, color);
    while ((x1 != x2) || (y1 != y2)) {
        oledDrawPixel(x1, y1, color);
        error2 = error * 2;
        if (error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        } else {
            /*nothing to do*/
        }

        if (error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        } else {
            /*nothing to do*/
        }
    }
    return;
}
//Draw polyline
void oledPolyline(
    const OLED_VERTEX* par_vertex, uint16_t par_size, OLED_COLOR color) {
    uint16_t i;
    if (par_vertex != 0) {
        for (i = 1; i < par_size; i++) {
            oledDrawLine(par_vertex[i - 1].x, par_vertex[i - 1].y,
                par_vertex[i].x, par_vertex[i].y, color);
        }
    } else {
        /*nothing to do*/
    }
    return;
}
/*Convert Degrees to Radians*/
static float oledDegToRad(float par_deg) { return par_deg * 3.14 / 180.0; }
/*Normalize degree to [0;360]*/
static uint16_t oledNormalizeTo0_360(uint16_t par_deg) {
    uint16_t loc_angle;
    if (par_deg <= 360) {
        loc_angle = par_deg;
    } else {
        loc_angle = par_deg % 360;
        loc_angle = ((par_deg != 0) ? par_deg : 360);
    }
    return loc_angle;
}
/*DrawArc. Draw angle is beginning from 4 quart of trigonometric circle (3pi/2)
 * start_angle in degree
 * sweep in degree
 */
void oledDrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle,
    uint16_t sweep, OLED_COLOR color) {
#define CIRCLE_APPROXIMATION_SEGMENTS 36
    float approx_degree;
    uint32_t approx_segments;
    uint8_t xp1, xp2;
    uint8_t yp1, yp2;
    uint32_t count = 0;
    uint32_t loc_sweep = 0;
    float rad;

    loc_sweep = oledNormalizeTo0_360(sweep);

    count = (oledNormalizeTo0_360(start_angle) * CIRCLE_APPROXIMATION_SEGMENTS)
        / 360;
    approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_degree = loc_sweep / (float)approx_segments;
    while (count < approx_segments) {
        rad = oledDegToRad(count * approx_degree);
        xp1 = x + (int8_t)(sin(rad) * radius);
        yp1 = y + (int8_t)(cos(rad) * radius);
        count++;
        if (count != approx_segments) {
            rad = oledDegToRad(count * approx_degree);
        } else {
            rad = oledDegToRad(loc_sweep);
        }
        xp2 = x + (int8_t)(sin(rad) * radius);
        yp2 = y + (int8_t)(cos(rad) * radius);
        oledDrawLine(xp1, yp1, xp2, yp2, color);
    }

    return;
}
//Draw circle by Bresenhem's algorithm
void oledDrawCircle(
    uint8_t par_x, uint8_t par_y, uint8_t par_r, OLED_COLOR par_color) {
    int32_t x = -par_r;
    int32_t y = 0;
    int32_t err = 2 - 2 * par_r;
    int32_t e2;

    if (par_x >= oled_width || par_y >= oled_height) {
        return;
    }

    do {
        oledDrawPixel(par_x - x, par_y + y, par_color);
        oledDrawPixel(par_x + x, par_y + y, par_color);
        oledDrawPixel(par_x + x, par_y - y, par_color);
        oledDrawPixel(par_x - x, par_y - y, par_color);
        e2 = err;
        if (e2 <= y) {
            y++;
            err = err + (y * 2 + 1);
            if (-x == y && e2 <= x) {
                e2 = 0;
            } else {
                /*nothing to do*/
            }
        } else {
            /*nothing to do*/
        }
        if (e2 > x) {
            x++;
            err = err + (x * 2 + 1);
        } else {
            /*nothing to do*/
        }
    } while (x <= 0);

    return;
}

//Draw rectangle
void oledDrawRectangle(
    uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_COLOR color) {
    oledDrawLine(x1, y1, x2, y1, color);
    oledDrawLine(x2, y1, x2, y2, color);
    oledDrawLine(x2, y2, x1, y2, color);
    oledDrawLine(x1, y2, x1, y1, color);

    return;
}

void oledSetContrast(const uint8_t value) {
    const uint8_t kSetContrastControlRegister = 0x81;
    oledWriteCommand(kSetContrastControlRegister);
    oledWriteCommand(value);
}

void oledSetDisplayOn(const bool on) {
    uint8_t value;
    if (on) {
        value = 0xAF; // Display on
        OLED.DisplayOn = true;
    } else {
        value = 0xAE; // Display off
        OLED.DisplayOn = false;
    }
    oledWriteCommand(value);
}

bool oledGetDisplayOn() { return OLED.DisplayOn; }

// Send a byte to the command register
void oledWriteCommand(uint8_t byte) {
    I2Cdev_Mem_Write(OLED_I2C_ADDR, 0x00, 1, &byte, 1, HAL_MAX_DELAY);
}

// Send data
void oledWriteData(uint8_t* buffer, size_t buff_size) {
    I2Cdev_Mem_Write(OLED_I2C_ADDR, 0x40, 1, buffer, buff_size, HAL_MAX_DELAY);
}

/* Fills the Screenbuffer with values from a given buffer of a fixed length */
OLED_Error_t oledFillBuffer(uint8_t* buf, uint32_t len) {
    OLED_Error_t ret = OLED_ERR;
    if (len <= OLED_BUFFER_SIZE) {
        memcpy(OLED_Buffer, buf, len);
        ret = OLED_OK;
    }
    return ret;
}