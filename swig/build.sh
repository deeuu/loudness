#!/bin/sh

python setup.py build_ext --inplace
rm -rf build loudness_wrap.cpp
