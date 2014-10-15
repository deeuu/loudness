#include "RoexBankMod.h"
#include "../Support/AuditoryTools.h"

namespace loudness{

    RoexBankMod::RoexBankMod(Real flo, Real fhi, Real estep)
        : flo_(flo), fhi_(fhi), estep_(estep)
    {
    }

    RoexBankMod::~RoexBankMod()
    {
    }

    int RoexBankMod::InitialiseInternal(const Signal &input)
    {
        //check if we need to interpolate
        if(estep_==0)
            interp_ = false;
        else if(estep_>0)
            interp_ = true;
        else
            cout << "Throw error: estep cannot be < 0" << endl;

        //if input spectrum has adaptive bins, need to interpolate for subsequent processors
        //I may drop this stage all together
        if(input.adaptive_freqs())
        {
            interp_ = true;
            if(estep_==0)
                estep_ = 0.1;
            lOUDNESS_ERROR("Cannot have adaptive center freuncies and no interpolation.")
        }

        //number of input components
        n_channels_ = input.n_channels();
        
        //erb limits
        const Real erb_hi = FreqToERBRate(fhi_);
        const Real erb_lo = FreqToERBRate(flo_);

        k_lo_ = 0;
        k_hi_ = n_channels_;
        //do not place filters on components below flo_ or fhi_
        for(int k=0;k<n_channels_; k++)
        {
            if(input.center_freq(k)<flo_)
                k_lo_ = k+1;
            if(input.center_freq(k)<fhi_)
                k_hi_ = k+1;
        }
        n_erbs_ = k_hi_-k_lo_;

        if(interp_)
        {
            //number of roex filters to use
            n_erbs_ = round((erb_hi-erb_lo)/estep_)+1; //+1 inclusive
            //erbs holds erb numbers ('erb rate' or 'cam' scale)
            erbs_.assign(n_erbs_, 0.0);
        }

        //initialise output signal
        output_.Initialise(n_erbs_, 1, input.fs());
        output_.set_frame_rate(input.frame_rate());

        p51_1k_ = 4000 / FreqToERBWidth(1000.0);

        //p upper is level invariant
        pu_.assign(n_channels_, 0.0);
        
        //p lower is level dependent
        pl_.assign(n_channels_, 0.0);

        //component frequencies in ERB units
        erb_comp_.assign(n_channels_, 0.0);

        //comp_level holds level per ERB on each component
        comp_level_.assign(n_channels_, 0.0);

        //fill input side
        Real erb, erb_w, fc;
        for(int i=0; i<n_channels_; i++)
        {
            fc = input.center_freq(i);
            erb_w = FreqToERBWidth(fc);
            erb_comp_[i] = FreqToERBRate(fc);
            pu_[i] = 4*fc/erb_w;
            pl_[i] = 0.35*(pu_[i]/p51_1k_);
        }

        //fill output side (dependent on if we are interpolating or not)
        for(int i=0; i<n_erbs_; i++)
        {
            if(interp_)
            {       
                erb = erb_lo+(i*estep_);
                erbs_[i] = erb;
                fc = ERBRateToFreq(erb);
            }
            else
                fc = input.center_freq(k_lo_+i);
            output_.set_center_freq(i,fc);
        }
        
        return 1;
    }

    void RoexBankMod::ProcessInternal(const Signal &input)
    {
        Real erb_comp_p, erb_comp_p_prev, erb_comp_0, elo, ehi, fc, erb_w, g_dev, p, shape;
        Real ratio, expo, den;
        int e, k, interp_idx;

        //if bin frequencies change per frame,
        //need to re-calculate filter parameters
        if(input.adaptive_freqs())
        {
            for(int k=0; k<input.n_channels(); k++)
            {
                fc = input.center_freq(k);
                erb_w = FreqToERBWidth(fc);
                pu_[k] = 4*fc/erb_w;
                pl_[k] = 0.35*(pu_[k]/p51_1k_);
                erb_comp_[k] = FreqToERBRate(fc);
            }
        }

        for(e=0;e<n_channels_;e++)
        {
            erb_comp_p = 0;
            k=0;

            //rectangular ERB band edges
            ehi = erb_comp_[e]+0.5;
            elo = erb_comp_[e]-0.5;

            while(k<n_channels_)
            {
                if(erb_comp_[k]>ehi) break;
                if(erb_comp_[k]>=elo)
                    erb_comp_p += input.sample(k,0);
                k++;
            }

            //convert to dB, subtract 51 here to save operations later
            if (erb_comp_p < LOW_LIMIT_POWER)
                comp_level_[e] = -151.0;
            else
                comp_level_[e] = 10*log10(erb_comp_p)-51;

            //cout << "comp_level : " << comp_level_[e] << endl;
        }	 

        interp_idx = 0;
        for(e=k_lo_;e<k_hi_;e++)
        {
            erb_comp_p = 0;
            k=0;
            fc = input.center_freq(e); //note this is input!

            while(k<n_channels_)
            {
                //normalised deviation
                g_dev = (input.center_freq(k)-fc)/fc;

                if (g_dev<=2)
                {
                    if(g_dev<0) //lower value 
                    {
                        //checked out 2.4.14
                        p = pu_[e]-(pl_[e]*comp_level_[k]); //51dB subtracted above
                        shape = -p*g_dev; //p*abs(g)
                    }
                    else //upper value
                    {
                        shape = pu_[e]*g_dev;
                    }

                    shape = (1+shape)*exp(-shape); //roex magnitude response
                    erb_comp_p += shape*input.sample(k,0); //excitation per erb
                }
                else
                    break;
                k++;
            }
            erb_comp_p = erb_comp_p < LOW_LIMIT_POWER ? LOW_LIMIT_POWER : erb_comp_p; 

            /*  
            cout << "Comp dB " << 10*log10(input.sample(e,0)) << endl;
            cout << "ERB dB " << 10*log10(erb_comp_p) << endl;
            */

            //interpolating exponentiated power
            if(interp_)
            {
                if(e>k_lo_)
                {
                    /*  
                    cout << "lo comp " << erb_comp_0 << "hi comp " << erb_comp_[e] << endl;
                    cout << "lo comp dB " << 10*log10(erb_comp_p_prev) << "hi comp dB " << 10*log10(erb_comp_p) << endl;
                    */
                    //Real slope;
                    //slope = (erb_comp_p-erb_comp_p_prev)/(erb_comp_[e]-erb_comp_0);
                    //interpolation checked out 26.5.14
                    ratio = erb_comp_p/erb_comp_p_prev;
                    den = (erb_comp_[e]-erb_comp_0);
                    while(interp_idx<n_erbs_)
                    {
                        //interpolate
                        if(erbs_[interp_idx] > erb_comp_[e])
                            break;
                        else if(erbs_[interp_idx] > erb_comp_0)
                        {
                            //output_.set_sample(interp_idx, 0, erb_comp_p_prev + (erbs_[interp_idx]-erb_comp_0)*slope);
                            expo = (erbs_[interp_idx]-erb_comp_0)/den;
                            output_.set_sample(interp_idx, 0, erb_comp_p_prev*pow(ratio,expo));
                        }
                        else
                            output_.set_sample(interp_idx, 0, erb_comp_p_prev);
                        /*  
                        cout << "ERB : " << erbs_[interp_idx] << " power db : " 
                        << 10*log10(output_.sample(interp_idx,0)) << endl;
                        */
                        interp_idx++;
                    }
                }
                erb_comp_p_prev = erb_comp_p;
                erb_comp_0 = erb_comp_[e];    
            }
            else
                output_.set_sample(interp_idx++, 0, erb_comp_p);
        }

        //upper clipping
        if(interp_)
        {
            while(interp_idx<n_erbs_)
                output_.set_sample(interp_idx++, 0, erb_comp_p);
        }
    }

    void RoexBankMod::ResetInternal(){};
}

