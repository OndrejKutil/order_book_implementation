import sys
from setuptools import setup, Extension
import pybind11

# Define compiler flags
# On Windows with MSVC, use /std:c++17. With MinGW (g++), use -std=c++17
if sys.platform == "win32" and "MSC" in sys.version:
    extra_compile_args = ['/std:c++17']
else:
    extra_compile_args = ['-std=c++17', '-O3']

ext_modules = [
    Extension(
        'market_simulator',
        [
            'src/simulation/simulator.cpp', 
            'src/book/order_book.cpp'
        ],
        include_dirs=[
            pybind11.get_include(), 
            'src'
        ],
        language='c++',
        extra_compile_args=extra_compile_args,
    ),
]

setup(
    name='market_simulator',
    version='0.1',
    description='A C++ Order Book Simulator with Python Bindings',
    ext_modules=ext_modules,
)
