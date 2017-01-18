#!/bin/sh
python3 setup.py build
python3 setup.py install
rm -rf build core_wrap.cpp core.py
