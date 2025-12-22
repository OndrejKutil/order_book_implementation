import sys
from setuptools import setup, Extension
import pybind11

# Define compiler flags
# On Windows with MSVC, use /std:c++17. With MinGW (g++), use -std=c++17
is_mingw = False
for arg in sys.argv:
    if 'mingw' in arg:
        is_mingw = True
        break

if sys.platform == "win32" and "MSC" in sys.version and not is_mingw:
    extra_compile_args = ['/std:c++17']
    extra_link_args = []
else:
    extra_compile_args = ['-std=c++17', '-O3']
    if is_mingw:
        # Static link all runtime libraries
        extra_link_args = ['-static']
    else:
        extra_link_args = []

ext_modules = [
    Extension(
        'market_simulator',
        [
            'src/simulation/python_bindings.cpp',
            'src/simulation/simulator.cpp', 
            'src/book/order_book.cpp'
        ],
        include_dirs=[
            pybind11.get_include(), 
            'src'
        ],
        language='c++',
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
    ),
]

setup(
    name='market_simulator',
    version='0.1',
    description='A C++ Order Book Simulator with Python Bindings',
    ext_modules=ext_modules,
)
