// File: pepcb_core.hpp
// Author: Gus Zhang
// Created on: 22 Apr 2025
// Description: Core functionality for the PEPCB project.

#pragma once
#ifndef _PEPCB_CORE_HPP_
#define _PEPCB_CORE_HPP_

#include <vector>
#include <map>

#include <string>
#include <iostream>
#include <fstream>

#include <algorithm>

#include "color_defines.hpp"
#include "3rd_party/Clipper2Lib/include/clipper2/clipper.h"

#define SCALE_FACTOR 1e9 // for converting to integer coordinates, resolution = 1 nm

typedef Clipper2Lib::Point64 Point2D;

typedef struct AABBBoundary2D
{
    Point2D start; // point at lower left corner
    Point2D end;  // point at upper right corner
} AABBBoundary2D;

typedef struct Drawing2D
{
    Clipper2Lib::Paths64 paths;
    Color color;
} Drawing2D;




#endif // _PEPCB_CORE_HPP_