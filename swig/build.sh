#!/bin/sh
python setup.py build
python setup.py install
rm -rf build core_wrap.cpp core.py
