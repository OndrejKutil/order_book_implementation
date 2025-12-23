
# Order Book Engine (C++ core + Python)

This is a small personal project where I’m building a simple **limit order book (LOB)** matching engine in C++ and exposing it to Python via **pybind11**.

## Why I’m doing this

- I want to learn more about **quant trading / market microstructure** by implementing the core mechanics myself.
- I want to work as a **quant**, and this is a hands-on way to practice building realistic building blocks.
- I also wanted to explore how to **combine C++ (performance/engine)** with **Python (research/simulation/agents)**.

## What’s inside (high level)

- **C++ order book** implementing price-time priority matching.
- **C++ simulator** wrapper that advances time and exposes market data.
- **Python extension module** named `market_simulator` (built from the C++ code).
- A small Python-side scaffold to run a simulation loop and plug in “agents”.

## Project tree

```text
order_book_engine/
├─ docs/
│  ├─ engine.md
│  └─ setup.md
├─ src/
│  ├─ book_implementation/
│  │  ├─ order_book/
│  │  │  ├─ order_book.cpp
│  │  │  ├─ order_book.hpp
│  │  │  └─ types.hpp
│  │  └─ simulation/
│  │     ├─ python_bindings.cpp
│  │     ├─ simulator.cpp
│  │     └─ simulator.hpp
│  └─ py/
│     ├─ setup.py
│     ├─ simulator.py
│     ├─ market_simulator.pyi
│     ├─ helper/
│     │  └─ orders.py
│     └─ agents/
│        └─ test.py
├─ pyproject.toml
└─ README.md
```

## Quickstart (Windows)

### 1) Install Python deps

From the repo root:

```powershell
# Create venv
python -m venv .venv

# activate
./.venv/Scripts/activate

# install dependencies
pip install .
```

or use uv:

```powershell
uv sync
```

### 2) Build the Python extension (pybind11)

This project builds a native module called `market_simulator` from C++ sources.

```powershell
cd src\py
python setup.py build_ext --compiler=mingw32 --inplace
```

After this, you should be able to import the module from Python.

### 3) Run the Python simulation scaffold

```powershell
python simulator.py
```

## Notes

- Setup details and prerequisites (MSYS2/MinGW): see `docs/setup.md`.
- More engine internals (data structures, matching logic): see `docs/engine.md`.
