NAME=loudness
CC=g++

MAJOR=0
MINOR=1
REVISION=0
BASENAME=lib$(NAME).so
EXECUTABLE=$(BASENAME).$(MAJOR).$(MINOR).$(REVISION)
TARGET_DIR=/usr/local

CFLAGS = -I/usr/local/include -std=c++11 -c -fPIC -g -Wall -O3

#Debug mode or not
ifeq ($(DEBUG),1)
    CFLAGS += -DDEBUG
endif

LDFLAGS=-shared -L/usr/local/lib -L/usr/local/include
LIBS=-lfftw3 -lsndfile #-lrt
INCS=-I.

SOURCES=../src/thirdParty/cnpy/cnpy.cpp \
../src/thirdParty/spline/Spline.cpp \
../src/support/AuditoryTools.cpp \
../src/support/SignalBank.cpp \
../src/support/Module.cpp \
../src/support/Model.cpp \
../src/support/Filter.cpp \
../src/support/FFT.cpp \
../src/support/AudioFileProcessor.cpp \
../src/modules/AudioFileCutter.cpp \
../src/modules/FIR.cpp \
../src/modules/IIR.cpp \
../src/modules/Butter.cpp \
../src/modules/Biquad.cpp \
../src/modules/ARAverager.cpp \
../src/modules/PeakFollower.cpp \
../src/modules/SMA.cpp \
../src/modules/FrameGenerator.cpp \
../src/modules/Window.cpp \
../src/modules/HoppingGoertzelDFT.cpp \
../src/modules/PowerSpectrum.cpp \
../src/modules/WeightSpectrum.cpp \
../src/modules/CompressSpectrum.cpp \
../src/modules/RoexBankANSIS342007.cpp \
../src/modules/FastRoexBank.cpp \
../src/modules/DoubleRoexBank.cpp \
../src/modules/SpecificLoudnessANSIS342007.cpp \
../src/modules/BinauralInhibitionMG2007.cpp \
../src/modules/InstantaneousLoudness.cpp \
../src/models/StationaryLoudnessANSIS342007.cpp \
../src/models/StationaryLoudnessCHGM2011.cpp \
../src/models/DynamicLoudnessGM2002.cpp \
../src/models/DynamicLoudnessCH2012.cpp

OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@echo "Creating shared library"
	@$(CC) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)
.cpp.o:
	@echo "Compiling: " $<
	@$(CC) $(CFLAGS) $(INCS) $< -o $@

clean:
	@rm -rf $(OBJECTS) $(EXECUTABLE)

install:
	@#install the library and link soname
	@install -c $(EXECUTABLE) $(TARGET_DIR)/lib
	@ln -sf $(TARGET_DIR)/lib/$(EXECUTABLE) $(TARGET_DIR)/lib/$(BASENAME)
	@ldconfig
	@#copy src folder and delete all but headers
	@mkdir -p $(TARGET_DIR)/include/$(NAME)
	@cd ../src; find -name '*.h' -exec cp --parents '{}' $(TARGET_DIR)/include/$(NAME) \;

uninstall:
	@rm -rf $(TARGET_DIR)/include/$(NAME)
	@rm -rf $(TARGET_DIR)/lib/$(EXECUTABLE)
