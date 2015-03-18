# setup.py

from distutils.core import setup, Extension
from distutils import sysconfig

if __name__ == "__main__":

    setup(name="loudness",
            py_modules=['loudness'], 
            ext_modules=[Extension("_loudness",
            [
                "loudness.i",
                "../src/Support/SignalBank.cpp",
                "../src/Support/Module.cpp",
                "../src/Modules/FrameGenerator.cpp",
                "../src/Modules/Window.cpp"
                ],

            #include_dirs = [numpy_include, "/usr/include"],
            library_dirs=['/usr/lib', '/usr/local/lib'],
            libraries=['fftw3', 'sndfile'],
            swig_opts=['-c++'],
            extra_compile_args=["-std=c++11", "-fPIC", "-O3"])])
