import loudness as ln

def plotResponse(freqPoints, dataPoints, freqsInterp, responseInterp,
        ylim=(-40,10)):
    plt.semilogx(freqsInterp, responseInterp)
    if np.any(dataPoints):
        plt.semilogx(freqPoints, dataPoints, 'o')
    plt.xlim(20,20e3)
    plt.ylim(ylim)
    plt.xlabel("Frequency, Hz")
    plt.ylabel("Response, dB")

def plotCombined(author="",fieldType=""):
    freqs = np.arange(20,20000,2)
    ome = ln.OME(author, fieldType)
    ome.interpolateResponse(freqs)
    response = ome.getResponse()
    freqPoints = np.array(ome.getFreqPoints())
    middleEarPoints = np.zeros(freqPoints.size)
    outerEarPoints = np.zeros(freqPoints.size)
    if author:
        middleEarPoints = np.array(ome.getMiddleEardB())
        if author == "CHEN_ETAL":
            ylim = (-40, 10)
        elif "ANSI" in author:
            ylim = (-40, 0)
    else:
        ylim = (-5, 20)
    if fieldType:
        outerEarPoints = np.array(ome.getOuterEardB())
    if fieldType == "DT990":
        plotResponse(freqPoints, None, freqs, response, ylim)
    else:
        plotResponse(freqPoints, outerEarPoints + middleEarPoints, freqs, response, ylim)

plt.figure(1)
plotCombined("ANSI")
plt.figure(2)
plotCombined("CHEN_ETAL")
plt.figure(2)
plotCombined("ANSI_HPF")
plt.figure(3)
plotCombined("", "ANSI_FREEFIELD")
plt.figure(4)
plotCombined("", "ANSI_DIFFUSEFIELD")
plt.figure(5)
plotCombined("", "DT990")
plt.figure(6)
plotCombined("ANSI", "ANSI_FREEFIELD")
plt.figure(7)
plotCombined("ANSI", "DT990")
