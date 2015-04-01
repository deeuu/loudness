# setup.py

from distutils.core import setup, Extension
from distutils import sysconfig

# Third-party modules - we depend on numpy for everything
import numpy

# Obtain the numpy include directory.  This logic works across numpy versions.
try:
    numpy_include = numpy.get_include()
except AttributeError:
    numpy_include = numpy.get_numpy_include()

if __name__ == "__main__":

    setup(name="loudness",
            py_modules=['loudness'], 
            ext_modules=[Extension("_loudness",
            [
                "loudness.i",
                "../src/cnpy/cnpy.cpp",
                "../src/Support/AuditoryTools.cpp",
                "../src/Support/Spline.cpp",
                "../src/Support/SignalBank.cpp",
                "../src/Support/Module.cpp",
                "../src/Support/Model.cpp",
                "../src/Support/FFT.cpp",
                "../src/Support/Filter.cpp",
                "../src/Modules/FIR.cpp",
                "../src/Modules/IIR.cpp",
                "../src/Modules/Butter.cpp",
                "../src/Modules/AudioFileCutter.cpp",
                "../src/Modules/FrameGenerator.cpp",
                "../src/Modules/Window.cpp",
                "../src/Modules/PowerSpectrum.cpp",
                "../src/Modules/WeightSpectrum.cpp",
                "../src/Modules/CompressSpectrum.cpp",
                "../src/Modules/RoexBankANSIS342007.cpp",
                "../src/Modules/FastRoexBank.cpp",
                "../src/Modules/DoubleRoexBank.cpp",
                "../src/Modules/SpecificLoudnessGM.cpp",
                "../src/Modules/InstantaneousLoudnessGM.cpp",
                "../src/Modules/ARAverager.cpp",
                "../src/Models/SteadyStateLoudnessANSIS342007.cpp",
                "../src/Models/DynamicLoudnessGM.cpp",
                "../src/Models/DynamicLoudnessCH.cpp"
            ],
            include_dirs = [numpy_include, "/usr/include"],
            library_dirs=['/usr/lib', '/usr/local/lib'],
            libraries=['fftw3', 'sndfile'],
            swig_opts=['-c++'],
            extra_compile_args=["-std=c++11", "-fPIC", "-O3"])])
