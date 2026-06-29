/**
 * This Library was originally written by Olivier Van den Eede (4ilo) in 2016.
 * Some refactoring was done and SPI support was added by Aleksander Alekseev (afiskon) in 2018.
 *
 * https://github.com/afiskon/stm32-oled
 */

#pragma once

#include <stddef.h>

#include "I2cController.hpp"
#include "rbcx.pb.h"

#include "OledController_fonts.hpp"

#define OLED_I2C_ADDR 0x3C
#define OLED_BUFFER_SIZE 1024

// Enumeration for screen colors
typedef enum {
    Black = 0x00, // Black color, no pixel
    White = 0x01 // Pixel is set. Color depends on OLED
} OLED_COLOR;

typedef enum {
    OLED_OK = 0x00,
    OLED_ERR = 0x01 // Generic error.
} OLED_Error_t;

// Struct to store transformations
typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    bool DisplayOn;
} OLED_t;
typedef struct {
    uint8_t x;
    uint8_t y;
} OLED_VERTEX;

void oledDispatch(const CoprocReq_OledReq& request);

bool oledTestConnection();
void oledInitStm();
void oledInit(const CoprocReq_OledInit& init);

// Procedure definitions
void oledInitOld(void);
void oledFill(OLED_COLOR color);
void oledUpdateScreen(void);
void oledDrawPixel(uint8_t x, uint8_t y, OLED_COLOR color);
char oledWriteChar(char ch, FontDef Font, OLED_COLOR color);
char oledWriteString(const char* str, FontDef Font, OLED_COLOR color);
void oledSetCursor(uint8_t x, uint8_t y);
void oledDrawLine(
    uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_COLOR color);
void oledDrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle,
    uint16_t sweep, OLED_COLOR color);
void oledDrawCircle(
    uint8_t par_x, uint8_t par_y, uint8_t par_r, OLED_COLOR color);
void oledPolyline(
    const OLED_VERTEX* par_vertex, uint16_t par_size, OLED_COLOR color);
void oledDrawRectangle(
    uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_COLOR color);
/**
 * @brief Sets the contrast of the display.
 * @param[in] value contrast to set.
 * @note Contrast increases as the value increases.
 * @note RESET = 7Fh.
 */
void oledSetContrast(const uint8_t value);
/**
 * @brief Set Display ON/OFF.
 * @param[in] on 0 for OFF, any for ON.
 */
void oledSetDisplayOn(const bool on);
/**
 * @brief Reads DisplayOn state.
 * @return  0: OFF.
 *          1: ON.
 */
bool oledGetDisplayOn();

// Low-level procedures
void oledWriteCommand(uint8_t byte);
void oledWriteData(uint8_t* buffer, size_t buff_size);
OLED_Error_t oledFillBuffer(uint8_t* buf, uint32_t len);