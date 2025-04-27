// File: color_defines.hpp
// Author: Gus Zhang
// Created on: 22 Apr 2025
// Description: This file contains color definitions for the project.

#pragma once
#ifndef _COLOR_DEFINES_HPP_
#define _COLOR_DEFINES_HPP_

#include <cstdint>

typedef struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

#define COLOR_F_CU \
    Color { 231, 76, 60, 255 }
#define COLOR_F_SILKSCREEN \
    Color { 242, 243, 244, 255 }
#define COLOR_B_CU \
    Color { 52, 152, 219, 255 }
#define COLOR_B_SILKSCREEN \
    Color { 229, 231, 233, 255 }
#define COLOR_F_COURTYARD \
    Color { 255, 192, 203, 255 }
#define COLOR_B_COURTYARD \
    Color { 255, 192, 203, 255 }
#define COLOR_VIA \
    Color { 26, 188, 156, 255 }
#define COLOR_EDGE_CUTS \
    Color { 255, 255, 128, 255 }
#define COLOR_HOLE \
    Color { 0, 0, 0, 255 }
#define COLOR_WHITE \
    Color { 255, 255, 255, 255 }
#define COLOR_RED \
    Color { 255, 0, 0, 255 }
#define COLOR_GREEN \
    Color { 0, 255, 0, 255 }
#define COLOR_BLUE \
    Color { 0, 0, 255, 255 }
#define COLOR_YELLOW \
    Color { 255, 255, 0, 255 }
#define COLOR_MAGENTA \
    Color { 255, 0, 255, 255 }
#define COLOR_CYAN \
    Color { 0, 255, 255, 255 }
#define COLOR_BLACK \
    Color { 0, 0, 0, 255 }
#define COLOR_PINK \
    Color { 255, 192, 203, 255 }

#endif // _COLOR_DEFINES_HPP_