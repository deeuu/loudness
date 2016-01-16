import numpy as np
import matplotlib.pyplot as plt
import loudness as ln


def plotResponse(freqPoints, dataPoints,
                 freqsInterp, responseInterp,
                 ylim=(-40, 10), title = ""):

    if np.any(dataPoints):
        plt.semilogx(freqPoints, dataPoints, 'o')
    plt.semilogx(freqsInterp, responseInterp)
    plt.xlim(20, 20e3)
    plt.ylim(ylim)
    plt.xlabel("Frequency, Hz")
    plt.ylabel("Response, dB")
    plt.title(title)
    plt.show()


def plotMiddleEar(filterType, ylim=(-40, 0)):
    freqs = np.arange(20, 20000, 2)
    ome = ln.OME(filterType, ln.OME.NONE)
    ome.interpolateResponse(freqs)
    response = ome.getResponse()
    freqPoints = ome.getMiddleEarFreqPoints()
    dataPoints = ome.getMiddleEardB()
    plotResponse(freqPoints, dataPoints,
                 freqs, response, ylim)


def plotOuterEar(filterType, ylim=(-40, 0)):
    freqs = np.arange(20, 20000, 2)
    ome = ln.OME(ln.OME.NONE, filterType)
    ome.interpolateResponse(freqs)
    response = ome.getResponse()
    freqPoints = ome.getOuterEarFreqPoints()
    dataPoints = ome.getOuterEardB()
    plotResponse(freqPoints, dataPoints,
                 freqs, response, ylim)


def plotCombined(middleFilterType, outerFilterType, ylim=(-40, 10)):
    freqs = np.arange(20, 20000, 2)
    ome = ln.OME(middleFilterType, outerFilterType)
    ome.interpolateResponse(freqs)
    response = ome.getResponse()
    plotResponse(None, None,
                 freqs, response, ylim)

plt.figure(1)
plotMiddleEar(ln.OME.ANSIS342007_MIDDLE_EAR, (-40, 0))
plt.figure(2)
plotMiddleEar(ln.OME.CHGM2011_MIDDLE_EAR, (-40, 10))
plt.figure(2)
plotMiddleEar(ln.OME.ANSIS342007_MIDDLE_EAR_HPF, (-40, 0))
plt.figure(3)
plotOuterEar(ln.OME.ANSIS342007_FREEFIELD, (-5, 20))
plt.figure(4)
plotOuterEar(ln.OME.ANSIS342007_DIFFUSEFIELD, (-5, 20))
plt.figure(5)
plotOuterEar(ln.OME.BD_DT990, (-10, 10))
plt.figure(6)
plotCombined(ln.OME.ANSIS342007_MIDDLE_EAR,
             ln.OME.ANSIS342007_FREEFIELD, (-40, 10))
plt.figure(7)
plotCombined(ln.OME.ANSIS342007_MIDDLE_EAR, ln.OME.BD_DT990, (-40, 10))
