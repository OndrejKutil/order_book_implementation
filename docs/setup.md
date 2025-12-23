# Setup Guide

## Install Python Dependencies

Install from [pyproject.toml](../pyproject.toml):

```powershell
pip install .
```

## C++ Prerequisites

- **g++** compiler (MinGW-w64 via MSYS2)
- Install from https://www.msys2.org/
- Add `C:\msys64\mingw64\bin` to PATH
- Verify: `g++ --version`

## Compile with g++

For the Python extension:

```powershell
cd src\py
python setup.py build_ext --compiler=mingw32 --inplace
```
