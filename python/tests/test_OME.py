import numpy as np
import matplotlib.pyplot as plt
import loudness as ln

def plotResponse(freqPoints, dataPoints, 
        freqsInterp, responseInterp,
        ylim=(-40,10), title = ""):

    if np.any(dataPoints):
        plt.semilogx(freqPoints, dataPoints, 'o')
    plt.semilogx(freqsInterp, responseInterp)
    plt.xlim(20, 20e3)
    plt.ylim(ylim)
    plt.xlabel("Frequency, Hz")
    plt.ylabel("Response, dB")
    plt.title(title)
    plt.show()

def plotMiddleEar(author="", ylim = (-40,0)):
    freqs = np.arange(20, 20000, 2)
    ome = ln.OME(author, "")
    ome.interpolateResponse(freqs)
    response = ome.getResponse()
    freqPoints = ome.getMiddleEarFreqPoints()
    dataPoints = ome.getMiddleEardB()
    plotResponse(freqPoints, dataPoints, \
            freqs, response, ylim,\
            title = "Middle ear : " + author)

def plotOuterEar(author="", ylim = (-40,0)):
    freqs = np.arange(20, 20000, 2)
    ome = ln.OME("", author)
    ome.interpolateResponse(freqs)
    response = ome.getResponse()
    freqPoints = ome.getOuterEarFreqPoints()
    dataPoints = ome.getOuterEardB()
    plotResponse(freqPoints, dataPoints, \
            freqs, response, ylim,\
            title = "Outer ear : " + author)

def plotCombined(author="",fieldType="", ylim = (-40, 10)):
    freqs = np.arange(20,20000,2)
    ome = ln.OME(author, fieldType)
    ome.interpolateResponse(freqs)
    response = ome.getResponse()
    plotResponse(None, None, \
            freqs, response, ylim,\
            title = "Middle ear : " + author + " Outer ear : " + fieldType)

plt.figure(1)
plotMiddleEar("ANSIS342007", (-40, 0))
plt.figure(2)
plotMiddleEar("CHGM2011", (-40, 10))
plt.figure(2)
plotMiddleEar("ANSIS342007_HPF", (-40, 0))
plt.figure(3)
plotOuterEar("ANSIS342007_FREEFIELD", (-5, 20))
plt.figure(4)
plotOuterEar("ANSIS342007_DIFFUSEFIELD", (-5, 20))
plt.figure(5)
plotOuterEar("DT990", (-10, 10))
plt.figure(6)
plotCombined("ANSIS342007", "ANSIS342007_FREEFIELD", (-40, 10))
plt.figure(7)
plotCombined("ANSI_S34_2007", "DT990", (-40, 10))
