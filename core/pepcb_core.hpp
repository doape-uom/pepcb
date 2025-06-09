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
typedef Clipper2Lib::Paths64 Paths2D;
typedef Clipper2Lib::Path64 Path2D;
typedef float Angle; // angle in radians

#define PI 3.14159265358979323846
#define PI_2 1.57079632679489661923
#define PI_4 0.78539816339744830962
#define PI_180 0.01745329251994329577
#define RADIAN_TO_DEGREE(radian) ((radian) * (180.0 / PI))
#define DEGREE_TO_RADIAN(degree) ((degree) * (PI / 180.0))

// Drawing related

typedef struct Drawing2D
{
    Clipper2Lib::Paths64 paths;
    Color color;
} Drawing2D;

// Boundary collision detection related

typedef struct AABBBoundary2D
{
    Point2D start; // point at lower left corner
    Point2D end;  // point at upper right corner
} AABBBoundary2D;

// PCB related

// Layers definition

enum LayerType
{
    LAYER_TYPE_UNKNOWN,
    LAYER_TYPE_TOP,
    LAYER_TYPE_2,
    LAYER_TYPE_3,
    LAYER_TYPE_4,
    LAYER_TYPE_5,
    LAYER_TYPE_6,
    LAYER_TYPE_7,
    LAYER_TYPE_8,
    LAYER_TYPE_9,
    LAYER_TYPE_10,
    LAYER_TYPE_11,
    LAYER_TYPE_12,
    LAYER_TYPE_13,
    LAYER_TYPE_14,
    LAYER_TYPE_15,
    LAYER_TYPE_BOTTOM,
    LAYER_TYPE_SILKSCREEN_TOP,
    LAYER_TYPE_SILKSCREEN_BOTTOM,
    LAYER_TYPE_SOLDER_MASK_TOP,
    LAYER_TYPE_SOLDER_MASK_BOTTOM,
    LAYER_TYPE_EDGE_CUT
};

// Footprint related

// PadPrototype as struct
struct PadPrototype
{
    std::vector<LayerType> layers; // layers
    Path2D copper_path; // assume copper polygon has no holes and will always on all layers
    Path2D hole_path; // assume hole polygon is a single piece of cut off thus no holes, will through all layers
};

// Pad as struct
struct Pad : public PadPrototype
{
    Point2D position; // position relative to the center of the part
    Angle rotation; // rotation of the pad
    AABBBoundary2D aabbb; // bounding box of the pad
};

// PartPrototype as struct
struct PartPrototype
{
    std::string name;
    std::map<std::string, std::vector<Pad>> pads; // pads by net name
    Path2D collision_path_top; // collision path on top layer
    Path2D collision_path_bottom; // collision path on bottom layer
};

// Part as struct
struct Part : public PartPrototype
{
    std::string designator; // designator
    Point2D position; // center of the part
    Angle rotation; // rotation of the part
    LayerType side; // layer type

    AABBBoundary2D aabbb_top; // bounding box of the part
    AABBBoundary2D aabbb_bottom; // bounding box of the part
};

// SuperPart as struct
struct SuperPart : public Part // aggregated part groups that internal placement has been fixed
{
    std::vector<Part> parts;
};

// PCB related

// Grouping tree for parts
struct GroupTreeNode

class PartPlacement
{
    public:
        PartPlacement() = default;
        ~PartPlacement() = default;

        void addPart(const Part& part); // add part to the placement, will also update PartGroupTree
};




#endif // _PEPCB_CORE_HPP_