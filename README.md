# PEPCB – Power Electronics PCB Automation Toolkit

**PEPCB** is an open-source toolkit designed to automate PCB design workflows in **power electronics**. It provides a modular system for layout solving, parasitic evaluation, and integration with existing EDA tools like KiCad, Altium, and Eagle. PEPCB aims to simplify and accelerate the design process of high-power, high-performance circuit boards.

---

## 🚧 Project Status
Note: This project is currently under active development and is not yet feature-complete.
Expect frequent updates, breaking changes, and evolving documentation.

Contributions and feedback are welcome as the project takes shape!

---

## 🔧 Features

- **Layout Solver**  
  Automated component placement and routing engine for power electronics, written in C/C++ and callable from Python.

- **Parasitic Evaluation**  
  Tools to estimate and analyze parasitic effects (e.g., trace impedance, coupling) during design stages.

- **Visualization UI**  
  Lightweight C/C++ UI for inspecting and debugging polygon-based layout data.

- **EDA Connectors**  
  Interfaces to read and write data from/to popular PCB design tools (such as KiCad, Altium, Eagle).

- **Gerber Conversion**  
  Utilities for generating Gerber files from intermediate formats.

- **Plugin Support**  
  Extend functionality directly within EDA tools via custom plugins.

---

## 📂 Repository Structure

```
PEPCB/
├── core/              # C++ layout + parasitic solvers
├── ui/                # Polygon visualization interface
├── scripts/           # Python automation scripts
├── connectors/        # Interfaces to EDA software
├── converters/        # Format converters, e.g., Gerber
├── plugins/           # Plugin code for KiCad, Eagle, etc.
├── docs/              # Documentation and diagrams
└── ...
```

---

## 🚀 Getting Started

### Prerequisites

- C++17 compatible compiler
- CMake ≥ 3.15
- Python ≥ 3.8

### Building Core Libraries

```bash
mkdir build && cd build
cmake ..
make
```

### Running a Script Example

```bash
python scripts/run_layout_solver.py --input design.json
```

---

## 📦 License

This project is licensed under the [MIT License](LICENSE).  
You're free to use, modify, and distribute it — just give credit.

---

## 🤝 Contributing

Contributions, bug reports, and suggestions are welcome!  
Please see [docs/CONTRIBUTING.md](docs/CONTRIBUTING.md) for guidelines.

---

## 🌐 Contact & Community

- GitHub Issues – for bugs and feature requests
- Discussions – coming soon
- Contact – [Gus Zhang](mailto://cheng.zhang@manchester.ac.uk), The Unviversity of Manchester, United Kingdom
- Contact - [Ali Sep](mailto://ali.sep@innovoltive.com), Innovoltive, United States of America

---

*Made with 💡 and ⚡ for power electronics engineers.*
