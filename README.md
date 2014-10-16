loudness
========

Loudness is a C++ library with Python bindings for modelling perceived loudness. 
The library consists of processing modules which can be cascaded to form a loudness model.

## Dependencies

To build the C++ library you will need:
  - libsndfile1-dev >= 1.0.25
  - libfftw3-dev >= 3.3.3
  - zlib1g-dev >= 1.2.8

To build the Python bindings you will need:
  - swig >= 3.0.0
  - python-numpy-dev

## Acknowledgments 

The library interface is entirely based on the fantastic AIM-C:
https://code.google.com/p/aimc/

The cnpy library for reading numpy arrays in C++:
https://github.com/rogersce/cnpy

Ricard Marxer for the loudia audio project:
https://github.com/rikrd/loudia
