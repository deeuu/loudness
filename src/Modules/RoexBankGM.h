#ifndef RoexBankGM_H_
#define RoexBankGM_H_

#include "../Support/Module.h"

/*
 * =====================================================================================
 *        Class:  RoexBankGM
 *  Description:  Applies a set of level dependent rounded exponential (roex) filters
 *                to an input power spectrum to estimate the auditory representation
 *                (excitation pattern). This implementation follows a combination of:
 *
 *                Glasberg, B. R., & Moore, B. C. J. (1990). 
 *                Derivation of Auditory Filter Shapes From Notched-Noise Data.
 *                Hearing Research, 47, 103–138.
 *
 *                ANSI S3.4-2007. (2007). 
 *                Procedure for the Computation of Loudness of Steady Sounds.
 * =====================================================================================
 */

namespace loudness{

    class RoexBankGM : public Module
    {

    public:
        /*
         *--------------------------------------------------------------------------------------
         *       Class:  RoexBankGM
         *      Method:  RoexBankGM :: RoexBankGM
         * Description:  Constructs a RoexBankGM object.
         *  Parameters:    freqLo:  lowest filter center frequency of interest.
         *                 freqHi:  highest filter center frequency of interest.
         *               camStep:  ERB filter step size (reciprocal of filter density).
         *--------------------------------------------------------------------------------------
         */ 
        RoexBankGM(Real camLo = 1.8, Real camHi = 38.9, Real camStep = 0.1, bool approxRoex = false);

        virtual ~RoexBankGM();

    private:
        /*
         *--------------------------------------------------------------------------------------
         *       Class:  RoexBankGM
         *      Method:  RoexBankGM :: initializeInternal
         * Description:  Given the input SignalBank to process, designs the filter bank 
         *               and configures the output SignalBank.
         *               The output is an N channel x 1 sample SignalBank representing excitation 
         *               per filter.
         *  Parameters:  input:  The power spectrum to be filtered.
         *--------------------------------------------------------------------------------------
         */ 
        virtual bool initializeInternal(const SignalBank &input);
        /*
         *--------------------------------------------------------------------------------------
         *       Class:  RoexBankGM
         *      Method:  RoexBankGM :: ProcessInternal
         * Description:  Computes the level per ERB, roex filter shapes
         *               and performs cochlea filtering to estimate the excitation pattern on 
         *               an ERB scale.
         *  Parameters:  input:  The power spectrum to be filtered.
         *--------------------------------------------------------------------------------------
         */  
        virtual void processInternal(const SignalBank &input);

        virtual void resetInternal();

        void generateRoexTable(int size = 1024);

        Real camLo_, camHi_, camStep_;
        bool approxRoex_;
        int nFilters_, roexIdxLimit_;
        Real step_;
        vector<vector<int> > rectBinIndices_;
        RealVec pu_, pl_, compLevel_, roexTable_;
    };
}

#endif
