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

#include "../src/Support/SignalBank.h"
#include "../src/Support/Module.h"
#include "../src/Support/Model.h"
#include "../src/Support/Spline.h"
#include "../src/Support/AuditoryTools.h"
#include "../src/Support/Timer.h"
#include "../src/Support/Filter.h"
#include "../src/Modules/FIR.h"
#include "../src/Modules/IIR.h"
#include "../src/Modules/Butter.h"
#include "../src/Modules/AudioFileCutter.h"
#include "../src/Modules/FrameGenerator.h"
#include "../src/Modules/IntegratedLoudnessGM.h"
#include "../src/Modules/SpecificLoudnessGM.h"
#include "../src/Modules/RoexBankANSIS3407.h"
#include "../src/Modules/FastRoexBank.h"
#include "../src/Modules/DoubleRoexBank.h"
#include "../src/Modules/PowerSpectrum.h"
#include "../src/Modules/GoertzelPS.h"
#include "../src/Modules/CompressSpectrum.h"
#include "../src/Modules/WeightSpectrum.h"
#include "../src/Models/DynamicLoudnessGM.h"
#include "../src/Models/SteadyLoudnessANSIS3407.h"
%}

//Required for integration with numpy arrays
%include "numpy.i"

%init %{
import_array();
%}

//apply all of the double typemaps to Real
%apply double { Real };

%include "std_vector.i"
namespace std {
    //The argument to %template() is the name of the instantiation in the target language
    %template(RealVec) vector<double>;
    //apply all of the double vector typemaps to RealVec
    %apply vector<double> { RealVec };
    //apply all of the double vector reference typemaps to const RealVec&
    %apply const vector<double>& { const RealVec&};
}

using namespace std;
%include "std_string.i"

namespace loudness{
using std::string;
}

%include "../src/Support/SignalBank.h"
%include "../src/Support/Module.h"
%include "../src/Support/Model.h"
%include "../src/Support/Spline.h"
%include "../src/Support/AuditoryTools.h"
%include "../src/Support/Timer.h"
%include "../src/Support/Filter.h"
%include "../src/Modules/AudioFileCutter.h"
%include "../src/Modules/FrameGenerator.h"
%include "../src/Modules/FIR.h"
%include "../src/Modules/IIR.h"
%include "../src/Modules/Butter.h"
%include "../src/Modules/PowerSpectrum.h"
%include "../src/Modules/GoertzelPS.h"
%include "../src/Modules/CompressSpectrum.h"
%include "../src/Modules/WeightSpectrum.h"
%include "../src/Modules/IntegratedLoudnessGM.h"
%include "../src/Modules/SpecificLoudnessGM.h"
%include "../src/Modules/RoexBankANSIS3407.h"
%include "../src/Modules/FastRoexBank.h"
%include "../src/Models/DynamicLoudnessGM.h"
%include "../src/Models/SteadyLoudnessANSIS3407.h"



