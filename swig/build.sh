#!/bin/sh
#python setup.py build_ext --inplace
python setup.py install
rm -rf build loudness_wrap.cpp _loudness.so
