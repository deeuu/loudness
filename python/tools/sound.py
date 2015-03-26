import numpy as np
from scipy.signal import lfilter
from audiolab import aiffread, wavread, wavwrite
import matplotlib.pyplot as plt

'''
A class for generating, manipulating and loading audio files.

Currently works with 1 or 2 channel files only.

Requires audiolab for reading and writing audio files.

TODO(dominic.ward@bcu.ac.uk):
    * Implement power law noise
    * Allow for variable onset and offset ramp duration
    * Check playback using audiolab
    * Implement spectrogram plot
'''

class Sound:

    def __init__(self, data, fs, filename=None):
        '''
        Default constructor. 
        Allows numpy arrays to be used as audio data.
        Samples are stored in data which is of shape (nSamples x nChannels).
        filename_ for loading and saving audio files.
        '''
        sh = len(data.shape)
        if sh==1:
            self.data = data.reshape((data.shape[0],1))
        else:
            if min(data.shape)>2:
                raise ValueError("Only 1 and 2 channels supported")
            else:
                self.data = data.reshape((np.max(data.shape), np.min(data.shape)))
        self.fs = fs;
        self.nSamples, self.nChannels = self.data.shape
        if filename:
            self.filename_ = filename
        self.ref = 1.0
        self.filepath = filename

    @staticmethod
    def readFromAudioFile(filename, mono=True):
        '''
        Calls audiolab to generate Sound object from wav and aiff files.
        If mono is true, returns the left channel only.
        '''
        if ".wav" in filename:
            data,fs,enc = wavread(filename)
        elif ".aiff" in filename:
            data,fs,enc = aiffread(filename)
        else: 
            raise ValueError("File extension not .wav or .aiff")
        if (len(data.shape)==2 and mono):
            return Sound(data[:,0],fs,filename)
        else:
            return Sound(data,fs,filename)

    @staticmethod
    def tone(freq = 440, phase = 0, dur = 0.1, fs = 48e3):
        '''
        Generate a sinusoid with specified frequency, phase, duration and sampling frequency.
        'freq' and 'phase' can be lists holding up to two values.
        '''
        time = np.arange(0, dur, 1.0/fs)
        if not isinstance(freq, list):
            freq = [freq]
        if not isinstance(phase, list):
            phase = [phase]
        if len(freq)>2:
            raise ValueError("Only 1 and 2 channels supported")
        if len(freq) > len(phase):
            phase = phase * len(freq)
            
        data = np.zeros((time.size, len(freq)))
        for chn, f in enumerate(freq):
            data[:, chn]= np.sin(2 * np.pi * freq[chn] * time + phase[chn])

        return Sound(data, fs)

    @staticmethod
    def whiteNoise(dur = 0.1, fs = 48e3, channels = 1):
        '''
        Generate a whitenoise with specified duration.
        For two channels of noise set channels = 2.
        Noise is peak normalised according to max |value| in either channel.
        '''
        if channels!=1 and channels!=2:
            raise ValueError("Only 1 and 2 channels supported")
        dur    = int(np.round(dur*fs))
        data   = np.random.randn(dur, channels)
        data   = data/np.max(np.abs(data))
        return Sound(data,fs)

    def getDuration(self):
        return self.nSamples / float(self.fs)

    def writeToAudioFile(self, filename, enc='float32'):
        if ".wav" in filename:
            wavwrite(self.data, filename, self.fs, enc)
        elif ".aiff" in filename:
            aiffwrite(self.data, filename, self.fs, enc)
        else: 
            raise TypeError("File extension not .wav or .aiff")

    def normalise(self, target_db = 0, norm_type = 'RMS', start=0, end=None):
        '''
        Peak or rms normalise the data to a target value in decibels.
        If 'PEAK', data is normalised according to max |value| in either channel.
        If 'RMS', data is normalised by computing the rms acrodd both channels.
        '''
        gain_db = 0
        if norm_type in ['PEAK', 'peak', 'pk']:
            gain_db = target_db - self.computePeak(start, end)
        elif norm_type in ['RMS', 'rms']:
            gain_db = target_db - self.computeRMS(start, end)
        gain_lin = 10**(gain_db/20.0)
        self.data *= gain_lin

    def computePeak(self, start=0, end=None):
        '''
        Return peak of data across both channels (if two)
        from the specified sample index start to end.
        Value in decibels relative to the reference value.
        '''
        return 20*np.log10(np.max(np.abs(self.data[start:end]))/self.ref)
    
    def computeRMS(self,start=0, end=None):
        '''
        Return the root mean square (rms) of the data across both channels (if two) 
        from the specified sample index start to end.
        Value in decibels relative to the reference value.
        '''
        rms = 10*np.log10((np.mean(self.data[start:end,:].flatten()**2,0))/\
                self.ref**2)
        return rms

    def computeCrest(self, start=0, end=None):
        '''
        Return the crest factor (peak-rms) of the data across both channels (if two)
        from the specified sample index start to end.
        '''
        return self.computePeak(start,end) - self.computeRMS(start, end)
    
    def applyGain(self, gain_db, start=0, end=None):
        '''
        Apply gain (in decibels) to the data from the specified sample index start to end.
        '''
        gain_lin = 10**(gain_db/20.0)
        self.data[start:end, :] *= gain_lin

    def applyRamp(self, dur):
        '''
        Apply a cosine ramp of a specified duration to the onset and offset of the data.
        '''
        d = np.round(dur*self.fs)
        theta = np.pi * 0.5 * np.arange(d) / float(d)
        nSamplesmiddle = self.nSamples - 2 * d
        if nSamplesmiddle<0:
            print "Duration too large"
        else:
            self.data *= np.concatenate((\
                np.cos(theta + np.pi * 1.5),\
                np.ones(nSamplesmiddle),\
                np.cos(theta))).reshape((self.nSamples,1))

    def modulateAmplitude(self, freq=1, phase=0, minVal=0, maxVal=1, linear= True):
        '''
        Multiplies the data with a sinusoid.
        Modulation can be linear (default) or logarithmic.
        '''
        minVal = np.min(maxVal, minVal)
        amp = (maxVal-minVal)*0.5
        offset = minVal+amp
        
        mod = offset + amp *\
            np.sin(phase + 2*np.pi*freq*np.arange(self.nSamples)/\
                    self.fs).reshape((self.nSamples, 1))
        if linear:
            self.data *= mod
        else:
            self.data *= 10**(mod/20.0)

    def filter(self, b, a=None):
        '''
        Apply a linear filter to the data using feedforward coefficients b
        and feedback coefficients a.
        '''
        if a is None:
            self.data = lfilter(b, [1.0], self.data, 0)
        else:
            self.data = lfilter(b, a, self.data, 0)

    def pad(self, samps,end=True):
        '''
        Pad the data with zeros.
        '''
        if end:
            self.data = np.vstack((self.data, zeros((samps, self.nChannels))))
        else:
            self.data = np.vstack((np.zeros((samps, self.nChannels)), self.data))
        self.nSamples, self.nChannels = self.data.shape

    def stackSound(self, sound):
        '''
        Stack another sounds data after this sounds data.
        '''
        self.data = np.vstack((self.data, sound.data))
        self.nSamples, self.nChannels = self.data.shape

    def plot(self):
        '''
        Plot the data as amplitude vs time.
        '''
        for chn in xrange(self.nChannels):
            plt.plot(np.arange(self.nSamples)/float(self.fs), self.data[:,chn])
        plt.xlabel("Time, s")
        plt.ylabel("Amplitude, linear")

    def __mul__(self, sound):
        '''
        Multiply the data of two sounds and return the resulting sound object.
        '''
        if (sound.data.shape == self.data.shape) and (sound.fs==self.fs):
            return Sound(self.data * sound.data, self.fs)

    def mul(self, sound):
        '''
        Muliply the data by another sounds of the same shape.
        '''
        if (sound.data.shape == self.data.shape):
            self.data *= sound.data

    def segment(self, start=0, end=None):
        self.data = self.data[start:end]
        self.nSamples, self.nChannels = self.data.shape
