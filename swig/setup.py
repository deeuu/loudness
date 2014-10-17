# setup.py

from distutils.core import setup, Extension
from distutils import sysconfig

setup(name="loudness",
        py_modules=['loudness'], 
        ext_modules=[Extension("_loudness",
        [
            "loudness.i",
            "../src/cnpy/cnpy.cpp",
            "../src/Support/SignalBank.cpp",
            "../src/Support/Module.cpp",
            "../src/Support/Model.cpp",
            "../src/Support/Spline.cpp",
            "../src/Support/AuditoryTools.cpp",
            "../src/Support/Timer.cpp",
            "../src/Support/Filter.cpp",
            "../src/Modules/AudioFileCutter.cpp",
            "../src/Modules/FrameGenerator.cpp",
            "../src/Modules/FIR.cpp",
            "../src/Modules/IIR.cpp",
            "../src/Modules/Butter.cpp",
            "../src/Modules/IntegratedLoudnessGM.cpp",
            "../src/Modules/SpecificLoudnessGM.cpp",
            "../src/Modules/RoexBankANSIS3407.cpp",
            "../src/Modules/FastRoexBank.cpp",
            "../src/Modules/DoubleRoexBank.cpp",
            "../src/Modules/PowerSpectrum.cpp",
            "../src/Modules/GoertzelPS.cpp",
            "../src/Modules/CompressSpectrum.cpp",
            "../src/Modules/WeightSpectrum.cpp",
            "../src/Models/DynamicLoudnessGM.cpp",
            "../src/Models/SteadyLoudnessANSIS3407.cpp"
            ],
        #include_dirs = [numpy_include, "/usr/include"],
        library_dirs=['/usr/lib', '/usr/local/lib'],
        libraries=['fftw3', 'sndfile'],
        swig_opts=['-c++'],
        extra_compile_args=["-std=c++11", "-fPIC", "-O3"])])
