/*
 * Copyright (C) 2014 Dominic Ward <contactdominicward@gmail.com>
 *
 * This file is part of Loudness
 *
 * Loudness is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Loudness is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Loudness.  If not, see <http://www.gnu.org/licenses/>. 
 */

%module loudness
%{
#define SWIG_FILE_WITH_INIT
#include "../src/cnpy/cnpy.h"
#include "../src/Support/Debug.h"
#include "../src/Support/Common.h"
#include "../src/Support/UsefulFunctions.h"
#include "../src/Support/Spline.h"
#include "../src/Support/AuditoryTools.h"
#include "../src/Support/SignalBank.h"
#include "../src/Support/Module.h"
#include "../src/Support/Model.h"
#include "../src/Support/FFT.h"
#include "../src/Support/Filter.h"
#include "../src/Modules/FIR.h"
#include "../src/Modules/IIR.h"
#include "../src/Modules/Butter.h"
#include "../src/Modules/AudioFileCutter.h"
#include "../src/Modules/FrameGenerator.h"
#include "../src/Modules/Window.h"
#include "../src/Modules/PowerSpectrum.h"
#include "../src/Modules/WeightSpectrum.h"
#include "../src/Modules/CompressSpectrum.h"
#include "../src/Modules/RoexBankANSIS3407.h"
#include "../src/Modules/FastRoexBank.h"
#include "../src/Modules/DoubleRoexBank.h"
#include "../src/Modules/SpecificLoudnessGM.h"
#include "../src/Modules/IntegratedLoudnessGM.h"
#include "../src/Models/DynamicLoudnessGM.h"
#include "../src/Models/DynamicLoudnessCH.h"
#include "../src/Models/SteadyStateLoudnessANSIS342007.h"
%}

//Required for integration with numpy arrays
%include "numpy.i"
%init %{
    import_array();
%}

//apply all of the double typemaps to Real
%apply double { Real };
%apply unsigned int { uint };

%include "std_vector.i"
%include "std_string.i"
namespace std {
    //The argument to %template() is the name of the instantiation in the target language
    %template(RealVec) vector<double>;
    %template(IntVec) vector<int>;

    %apply vector<double>& { RealVec& };
    %apply const vector<double>& { const RealVec& };
    %apply vector<int> { IntVec };
    %apply vector<int>& { IntVec& };
    %apply const vector<int>& { const IntVec& };
}

%apply (double* IN_ARRAY1, int DIM1) {(Real* data, int nSamples)}; 
%apply (double* IN_ARRAY1, int DIM1) {(Real* data, int nChannels)}; 
%apply (double* IN_ARRAY3, int DIM1, int DIM2, int DIM3) {(Real* data, int nEars, int nChannels, int nSamples)};

using namespace std;
namespace loudness{
using std::string;
using std::vector;
}

%include "./SignalBank.i"
%include "../src/cnpy/cnpy.h"
%include "../src/Support/Debug.h"
%include "../src/Support/Common.h"
%include "../src/Support/UsefulFunctions.h"
%include "../src/Support/Spline.h"
%include "../src/Support/AuditoryTools.h"
%include "../src/Support/Module.h"
%include "../src/Support/Model.h"
%include "../src/Support/FFT.h"
%include "../src/Support/Filter.h"
%include "../src/Modules/FIR.h"
%include "../src/Modules/IIR.h"
%include "../src/Modules/Butter.h"
%include "../src/Modules/AudioFileCutter.h"
%include "../src/Modules/FrameGenerator.h"
%include "../src/Modules/Window.h"
%include "../src/Modules/PowerSpectrum.h"
%include "../src/Modules/WeightSpectrum.h"
%include "../src/Modules/CompressSpectrum.h"
%include "../src/Modules/RoexBankANSIS3407.h"
%include "../src/Modules/FastRoexBank.h"
%include "../src/Modules/DoubleRoexBank.h"
%include "../src/Modules/SpecificLoudnessGM.h"
%include "../src/Modules/IntegratedLoudnessGM.h"
%include "../src/Models/DynamicLoudnessGM.h"
%include "../src/Models/DynamicLoudnessCH.h"
%include "../src/Models/SteadyStateLoudnessANSIS342007.h"
