import numpy as np
import matplotlib.pyplot as plt
import loudness as ln

def plotResponse(freqsInterp, responseInterp,
        ylim=(-40,10), title = ""):

    plt.semilogx(freqsInterp, responseInterp)
    plt.xlim(20, 20e3)
    plt.ylim(ylim)
    plt.xlabel("Frequency, Hz")
    plt.ylabel("Response, dB")
    plt.title(title)
    plt.show()

def plotCombined(author="",fieldType=""):
    freqs = np.arange(20,20000,2)
    ome = ln.OME(author, fieldType)
    ome.interpolateResponse(freqs)
    response = ome.getResponse()
    if author:
        if author == "CHEN_ETAL_2011":
            ylim = (-40, 10)
        elif "ANSI" in author:
            ylim = (-40, 0)
            if "ANSI" in fieldType:
                ylim = (-40, 10)
    elif "ANSI" in fieldType:
        ylim = (-5, 20)
    if "DT990" in fieldType:
        ylim = (-40, 10)
    plotResponse(freqs, response, ylim,\
            title = "Middle : " + author + " Outer: " + fieldType)

plt.figure(1)
plotCombined("ANSI_S34_2007")
plt.figure(2)
plotCombined("CHEN_ETAL_2011")
plt.figure(2)
plotCombined("ANSI_S34_2007_HPF")
plt.figure(3)
plotCombined("", "ANSI_S34_2007_FREEFIELD")
plt.figure(4)
plotCombined("", "ANSI_S34_2007_DIFFUSEFIELD")
plt.figure(5)
plotCombined("", "DT990")
plt.figure(6)
plotCombined("ANSI_S34_2007", "ANSI_S34_2007_FREEFIELD")
plt.figure(7)
plotCombined("ANSI_S34_2007", "DT990")
