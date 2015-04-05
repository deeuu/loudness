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
#include "../src/thirdParty/cnpy/cnpy.h"
#include "../src/thirdParty/spline/Spline.h"
#include "../src/support/Debug.h"
#include "../src/support/Common.h"
#include "../src/support/UsefulFunctions.h"
#include "../src/support/AuditoryTools.h"
#include "../src/support/SignalBank.h"
#include "../src/support/Module.h"
#include "../src/support/Model.h"
#include "../src/support/FFT.h"
#include "../src/support/Filter.h"
#include "../src/modules/FIR.h"
#include "../src/modules/IIR.h"
#include "../src/modules/Butter.h"
#include "../src/modules/Biquad.h"
#include "../src/modules/AudioFileCutter.h"
#include "../src/modules/FrameGenerator.h"
#include "../src/modules/Window.h"
#include "../src/modules/PowerSpectrum.h"
#include "../src/modules/WeightSpectrum.h"
#include "../src/modules/CompressSpectrum.h"
#include "../src/modules/RoexBankANSIS342007.h"
#include "../src/modules/FastRoexBank.h"
#include "../src/modules/DoubleRoexBank.h"
#include "../src/modules/SpecificLoudnessANSIS342007.h"
#include "../src/modules/InstantaneousLoudness.h"
#include "../src/modules/ARAverager.h"
#include "../src/models/SteadyStateLoudnessANSIS342007.h"
#include "../src/models/DynamicLoudnessGM2002.h"
#include "../src/models/DynamicLoudnessCH2012.h"
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
%include "../src/thirdParty/cnpy/cnpy.h"
%include "../src/thirdParty/spline/Spline.h"
%include "../src/support/Debug.h"
%include "../src/support/Common.h"
%include "../src/support/UsefulFunctions.h"
%include "../src/support/AuditoryTools.h"
%include "../src/support/Module.h"
%include "../src/support/Model.h"
%include "../src/support/FFT.h"
%include "../src/support/Filter.h"
%include "../src/modules/FIR.h"
%include "../src/modules/IIR.h"
%include "../src/modules/Butter.h"
%include "../src/modules/Biquad.h"
%include "../src/modules/AudioFileCutter.h"
%include "../src/modules/FrameGenerator.h"
%include "../src/modules/Window.h"
%include "../src/modules/PowerSpectrum.h"
%include "../src/modules/WeightSpectrum.h"
%include "../src/modules/CompressSpectrum.h"
%include "../src/modules/RoexBankANSIS342007.h"
%include "../src/modules/FastRoexBank.h"
%include "../src/modules/DoubleRoexBank.h"
%include "../src/modules/SpecificLoudnessANSIS342007.h"
%include "../src/modules/InstantaneousLoudness.h"
%include "../src/modules/ARAverager.h"
%include "../src/models/SteadyStateLoudnessANSIS342007.h"
%include "../src/models/DynamicLoudnessGM2002.h"
%include "../src/models/DynamicLoudnessCH2012.h"
