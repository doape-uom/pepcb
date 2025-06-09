# Procedures of PEPCB functionalities

This document describes the behaviours and use cases of the PEPCB framework. It is intended to help developers understand the design paradigm and keep coherency. In the previous development, a traditional OOP approach was used, but a few problems emerged. Data objects used as containers are passed through stages of processing, hard to keep consistent flows. For example, an initial data object describing the extracted circuit information contains the netlist and footprint information, but also undetermined positions that await for the placement algorithm to fill in. Then it may be passed to the DRC stage for validiy checks, which may result in two states of the object: valid and invalid. The further treatment is poorly structured and obscured.

The new approach is to use a functional programming paradigm, where the data objects are immutable and passed through a series of functions that transform them into the next stage. The data objects are not modified in place, but rather new objects are created with the updated information. This allows for better tracking of the data flow and easier debugging.

## Data objects

### Primitive input data
- `netlist`: a list of nets, each net is a list of pins
- `footprint`: a list of footprints, each footprint is a list of pins

### Processed data
- `component position`: a list of components with their positions, rotations, and orientations
- `copper area`: a list of copper areas, each area is a list of polygons and vias

### Presentation data
- `drawables`: a list of drawables, each drawable is a list of shapes and texts

## Data flow

### Step 1: Circuit capture
- The user captures the circuit using a schematic editor, such as KiCad or Eagle.
- Together with footprint information, the netlist is exported to a file.
- The packed information may be suitable for exported into SPICE simulation for analysing proximity. 

*Data structure*
```
DesignInstance
    - GroupTree: TreeObject
    - Netlist: List<{Part, Pin}>
    - Footprint: List<Part>