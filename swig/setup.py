# setup.py

from distutils.core import setup, Extension
from distutils import sysconfig

if __name__ == "__main__":

    debugFlag = "-DDEBUG"

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
                "../src/Modules/FIR.cpp",
                "../src/Modules/IIR.cpp",
                ],

            #include_dirs = [numpy_include, "/usr/include"],
            library_dirs=['/usr/lib', '/usr/local/lib'],
            libraries=['fftw3', 'sndfile'],
            swig_opts=['-c++'],
            extra_compile_args=["-std=c++11", "-fPIC", "-O3", debugFlag])])
