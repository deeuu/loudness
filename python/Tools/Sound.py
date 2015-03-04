from numpy import *
from numpy import max, round
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

    def __init__(self, data, fs, file_path=None):
        '''
        Default constructor. 
        Allows numpy arrays to be used as audio data.
        Samples are stored in data which is of shape (nSamples x nChannels).
        file_path_ for loading and saving audio files.
        '''
        sh = len(data.shape)
        if sh==1:
            self.data = data.reshape((data.shape[0],1))
        else:
            if min(data.shape)>2:
                raise ValueError("Only 1 and 2 channels supported")
            else:
                self.data = data.reshape((max(data.shape), min(data.shape)))
        self.fs = fs;
        self.nSamples, self.nChannels = self.data.shape
        if file_path:
            self.file_path_ = file_path
        self.ref = 1
        self.filepath = file_path

    @staticmethod
    def loadAudioFile(file_path, mono=True):
        '''
        Calls audiolab to generate Sound object from wav and aiff files.
        If mono is true, returns the left channel only.
        '''
        if ".wav" in file_path:
            data,fs,enc= wavread(file_path)
        elif ".aiff" in file_path:
            data,fs,enc= aiffread(file_path)
        else: 
            raise ValueError("File extension not .wav or .aiff")
        if (len(data.shape)==2 and mono):
            return Sound(data[:,0],fs,file_path)
        else:
            return Sound(data,fs,file_path)

    @staticmethod
    def tone(freq=[440], phase=[0], dur=0.1, fs=48e3):
        '''
        Generate a sinusoid with specified frequency, phase, duration and sampling frequency.
        Length of arguments must be 1 or 2 and matching.
        '''
        t = arange(0, dur,1.0/fs)
        if not isinstance(freq, list):
            freq = [freq]
        if not isinstance(phase, list):
            phase = [phase]
        if len(freq)>2:
            raise ValueError("Only 1 and 2 channels supported")
        if len(freq)!=len(phase):
            raise TypeError("arguments freq and phase do not have same length")
            
        data = zeros((t.size, len(freq)))
        for chn, f in enumerate(freq):
            data[:, chn]= sin(2*pi*freq[chn]*t+phase[chn])
        return Sound(data, fs)

    @staticmethod
    def whiteNoise(dur=0.1, fs=48e3, channels=1):
        '''
        Generate a whitenoise with specified duration.
        For two channels of noise set channels=2.
        Noise is peak normalised according to max |value| in either channel.
        '''
        if channels!=1 and channels!=2:
            raise ValueError("Only 1 and 2 channels supported")
        dur    = dur*fs
        data   = random.randn(round(dur), channels)
        data   = data/max(abs(data))
        return Sound(data,fs)

    def getDuration(self):
        return self.nSamples / float(self.fs)

    def writeAudioFile(self, file_path, enc='pcm16'):
        if ".wav" in file_path:
            wavwrite(self.data, file_path, self.fs, enc)
        elif ".aiff" in file_path:
            aiffwrite(self.data, file_path, self.fs, enc)
        else: 
            raise TypeError("File extension not .wav or .aiff")

    def normalise(self, target_db = 0, norm_type = 'RMS', start=0, end=None):
        '''
        Peak or rms normalise the data to a target value in decibels.
        If 'PEAK', data is normalised according to max |value| in either channel.
        If 'RMS', data is normalised by computing the rms acrodd both channels.
        '''
        gain_db = 0
        if norm_type is 'PEAK':
            gain_db = target_db - self.computePeak(start, end)
        elif norm_type is 'RMS':
            gain_db = target_db - self.computeRMS(start, end)
        gain_lin = 10**(gain_db/20.0)
        self.data *= gain_lin

    def computePeak(self, start=0, end=None):
        '''
        Return peak of data across both channels (if two)
        from the specified sample index start to end.
        Value in decibels relative to the reference value.
        '''
        return 20*log10(max(abs(self.data[start:end]))/self.ref)
    
    def computeRMS(self,start=0, end=None):
        '''
        Return the root mean square (rms) of the data across both channels (if two) 
        from the specified sample index start to end.
        Value in decibels relative to the reference value.
        '''
        rms = 10*log10((mean(self.data[start:end,:].flatten()**2,0))/\
                self.ref**2)
        return rms

    def computeCrest(self, start=0, end=None):
        '''
        Return the crest factor (peak-rms) of the data across both channels (if two)
        from the specified sample index start to end.
        '''
        return self.computePeak(start,end)-self.computeRMS(start, end)
    
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
        d = round(dur*self.fs)
        theta = pi*0.5*(arange(d)/float(d))
        nSamplesmiddle = self.nSamples - 2*d
        if nSamplesmiddle<0:
            print "Duration too large"
        else:
            self.data *= concatenate((cos(theta+pi*1.5), ones(nSamplesmiddle),cos(theta))).reshape((self.nSamples,1))

    def modulateAmplitude(self, freq=1, phase=0, min_val=0, max_val=1, linear= True):
        '''
        Multiplies the data with a sinusoid.
        Modulation can be linear (default) or logarithmic.
        '''
        min_val = min(max_val, min_val)
        amp = (max_val-min_val)*0.5
        offset = min_val+amp
        
        mod = offset+amp*sin(phase + 2*pi*freq*arange(self.nSamples)/self.fs).reshape((self.nSamples, 1))
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
            self.data = vstack((self.data, zeros((samps, self.nChannels))))
        else:
            self.data = vstack((zeros((samps, self.nChannels)), self.data))
        self.nSamples, self.nChannels = self.data.shape

    def stackSound(self, sound):
        '''
        Stack another sounds data after this sounds data.
        '''
        self.data = vstack((self.data, sound.data))
        self.nSamples, self.nChannels = self.data.shape

    def plot(self):
        '''
        Plot the data as amplitude vs time.
        '''
        for chn in xrange(self.nChannels):
            plt.plot(arange(self.nSamples, dtype='float32')/self.fs, self.data[:,chn])
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
