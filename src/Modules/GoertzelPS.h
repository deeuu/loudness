#ifndef GoertzelPS_H_ 
#define GoertzelPS_H

#include "../Support/Module.h"

/*  
Power spectrum computed using a bank of Goertzel resonators.

The power spectrum is computed following:
http://www.dadisp.com/webhelp/dsphelp.htm#mergedprojects/refman2/SPLGROUP/POWSPEC.htm
such that
sum(power_spec(x)) = mean(x*x)

Thus the area under the curve represents the average power of x, rather than the total power of x.

In the current implementation, DC and nyquist are not computed.

Have checked the resulting power spectrum and gives same result as PowerSpectrum when no windowing is used.
However, frequency domain windowing seems to give small deviations at low levels which I attribute to numerical
differences. See the source for PowerSpectrum for further notes.

*/

namespace loudness{

    class GoertzelPS : public Module
    {

    public:
        GoertzelPS(const RealVec& bandFreqsHz, const RealVec& windowSizeSecs, Real hopSizeSecs);
        virtual ~GoertzelPS();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual void processInternal(const SignalBank &input);
        virtual void resetInternal();
        void windowedPS();

        RealVec bandFreqsHz_, windowSizeSecs_;
        Real hopSizeSecs_, temporalCentre_;
        RealVecVec sine_, cosineTimes2_, vPrev_, vPrev2_;
        RealVec delayLine_, norm_;
        vector<int> windowSizeSamps_, startIdx_, endIdx_;
        int nWindows_, hop_, delayLineSize_, delayWriteIdx_, ready_, blockSize_;
    };
}

#endif
