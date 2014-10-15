#ifndef RoexBankMod_H_
#define RoexBankMod_H_

#include "../Support/Module.h"

/*
 * =====================================================================================
 *        Class:  RoexBankMod
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

    class RoexBankMod : public Module
    {

    public:
        /*
         *--------------------------------------------------------------------------------------
         *       Class:  RoexBankMod
         *      Method:  RoexBankMod :: RoexBankMod
         * Description:  Constructs a RoexBankMod object.
         *  Parameters:    flo:  lowest filter center frequency of interest.
         *                 fhi:  highest filter center frequency of interest.
         *               estep:  ERB filter step size (reciprocal of filter density).
         *--------------------------------------------------------------------------------------
         */ 
        RoexBankMod(Real flo=50, Real fhi=15e3, Real estep=0.25);

        virtual ~RoexBankMod();

    protected:
        /*
         *--------------------------------------------------------------------------------------
         *       Class:  RoexBankMod
         *      Method:  RoexBankMod :: InitialiseInternal
         * Description:  Given the input signal to process, designs the filter bank 
         *               and configures the output signal.
         *               The output is an N channel x 1 sample signal representing excitation 
         *               per filter.
         *  Parameters:  input:  The power spectrum to be filtered.
         *  TODO:        need to check interpolation with k_lo_ and k_hi_ indexing
         *--------------------------------------------------------------------------------------
         */         
        virtual int InitialiseInternal(const Signal &input);

        virtual void ProcessInternal(const Signal &input);

        virtual void ResetInternal();

    private:
        /*
         * See cpp file.
         */
        int n_erbs_, n_channels_;
        Real flo_, fhi_, estep_;
        bool adapt_fc_, interp_;
        int k_lo_, k_hi_;
        Real erb_lo_, erb_hi_, p51_1k_;
        RealVec erbs_, pu_, pl_, erb_comp_, comp_level_;
    };
}

#endif
