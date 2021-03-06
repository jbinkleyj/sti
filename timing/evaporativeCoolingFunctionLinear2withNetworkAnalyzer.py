include('microwaveKnifeFunction.py')
include('approximateExponentialSweep.py')
from math import pow

### VCAs ######################################################################
## VCA's control RF knife ramps
## ~4.5 is off. Frequencies are in MHz.
## Power should drop and ramps should get faster as you evaporate. 
## Several ramps are defined here, but not all may be executed. See evaporativeCoolingFunction
setvar('vca1', 10)    #6.5

setvar('f0', 120) #120 #117.8
setvar('f1', 60)       #3*4 #3*20

setvar('dtRamp1', 3.5*s) #3.5
#setvar('dtRamp1', 3.5*s*(f0-f1)/(117.8-78.5)) #3.25 #4.5 #3 #6.5
#setvar('dtRamp1', ( (f0-f1)*12.0*s/(120-12)) )

#setvar('dtRamp1', 5*s) #3.5
#setvar('dtHold1', 5*s)
#setvar('vcaHold', 10)
#setvar('dtPulseDelay', 1*s)
#setvar('dtPulseTime', 0.1*s)
#if (dtPulseDelay + dtPulseTime > dtHold1):
#    print 1/0

#sweepRate=(3*4 - 3*(0.01))/(1*s)
#setvar('dtRamp2', (f1-f2)/sweepRate )  #1.5*s

setvar('vca2', 10) 
setvar('f2', 30)    #10
setvar('dtRamp2', 3*s )  #1.5*s #0.9*s


setvar('vca3', 10) #5.75
setvar('f3', 3*4)
#setvar('dtRamp3', (f2-f3)/sweepRate ) #0.6*s
setvar('dtRamp3', 2*s ) #0.6*s #2

setvar('vca4', 10) #10 (first BEC) #5.75
setvar('f4', 3*2) #3*2
setvar('dtRamp4', 2*s ) #3*s (first BEC) #0.6*s #1s


setvar('vca5', 8) #8 #5.75
setvar('f5', 3*0.58)    #0.38
setvar('dtRamp5', 1.7*s ) #0.6*s  #1s

setvar('dtHoldCut', 0*s)





###############################################################################

def evaporate(tStart, rampNumber = 1):

    if (rampNumber < 1):
        return tStart

    rampList=[]
    
    #################### events #######################
    internalTime = tStart


    if (applyFilterRF):
        dtQuadrupoleRamp = 250*ms
    else:
        dtQuadrupoleRamp = 0
        
    ### Turn microwave horn on during evaporation ######
    event(sixPointEightGHzSwitch, internalTime - dtQuadrupoleRamp, 1)

    ddsRbfreq = ddsRbResonanceFreq


    evapTime = internalTime
    tEnd = evapTime
    
    tFrequencyRampStart = internalTime - dtQuadrupoleRamp

    if (applyFilterRF):
        rampList.append((ddsRbfreq - f0, ddsRbfreq - f0, dtQuadrupoleRamp/s))
        event(rfKnifeAmplitude, tFrequencyRampStart +  10*us, 10)
        
    ###  Ramp 1 ###########################################
    if (rampNumber >= 1) :
        event(rfKnifeAmplitude,evapTime +  10*us, vca1)
        rampList.append((ddsRbfreq - f0, ddsRbfreq - f1, dtRamp1/s))
        tEnd = evapTime + dtRamp1
#        event(rfKnifeAmplitude,evapTime +  10*us + dtRamp1, 0)
#        event(rfKnifeAmplitude,evapTime +  10*us + dtRamp1 + dtPulseDelay, vcaHold)
#        event(rfKnifeAmplitude,evapTime +  10*us + dtRamp1 + dtPulseDelay + dtPulseTime, 0)
#        rampList.append((ddsRbfreq - f1, ddsRbfreq - f1, dtHold1/s))
#        tEnd = evapTime + dtRamp1+dtHold1




###        depumpMOT(evapTime + (dtRamp1/2), pumpingTime = 200*us)
    #######################################################

#    event(rfKnifeAmplitude, evapTime +  3*s, 3)



   ### Ramp 2 ############################################
    if (rampNumber >= 2) :
        evapTime = tEnd + 10*ms
        event(rfKnifeAmplitude, evapTime - 4*ms, vca2)
        rampList.append((ddsRbfreq - f1, ddsRbfreq - f2, dtRamp2/s))
#        rampList.append((ddsRbfreq - f2, ddsRbfreq - f2, dtHold2/s))

        #Add a depump during the evaporation
#        event(motZShutter, tEnd-10*ms, 1)                 #open Z shutter
#        depumpMOT(tEnd, pumpingTime = 200*us)
#        event(motZShutter, tEnd, 0)                             #close Z shutter   
#        setvar('triggerTime', tEnd)

        tEnd = evapTime + dtRamp2
#        rampList.append((ddsRbfreq - f2, ddsRbfreq - f2, dtHold2/s))
#        tEnd = evapTime + dtRamp2 + dtHold2

        if (decompressAfterRamp == 2):
            setvar('dtIntermediateRamp', dtRampToImaging*intermediateRampRatio)
            setvar('tRampStart', tEnd)

#        event(motZShutter, tEnd-10*ms, 1)                 #open Z shutter
#        depumpMOT(tEnd, pumpingTime = 200*us)
#        event(motZShutter, tEnd, 0)                             #close Z shutter   
    #######################################################

    
#    tEnd = max(tEnd, tRampFinished)    


    ### Ramp 3 ############################################
    if (rampNumber >= 3):

        if (decompressAfterRamp == 2):
            rampList.append((ddsRbfreq - f2, ddsRbfreq - f2, dtIntermediateRamp/s))

#        evapTime = tEnd + 20*ms
#        event(rfKnifeAmplitude, evapTime - 10*ms, vca3)
        evapTime = tEnd + 1.4*ms
        event(rfKnifeAmplitude, evapTime - 5*us, vca3)
        rampList.append((ddsRbfreq - f2, ddsRbfreq - f3, dtRamp3/s))
        tEnd = evapTime + dtRamp3

        if (decompressAfterRamp == 2):
            tEnd += dtIntermediateRamp

        if (decompressAfterRamp == 3):
            setvar('dtIntermediateRamp', dtRampToImaging*intermediateRampRatio)
            setvar('tRampStart', tEnd)

#        event(motZShutter, tEnd-10*ms, 1)                 #open Z shutter
#        depumpMOT(tEnd, pumpingTime = 200*us)
#        event(motZShutter, tEnd, 0)                             #close Z shutter   

    #######################################################


    ### Ramp 4 ############################################
    if (rampNumber >= 4):
        
        if (decompressAfterRamp == 3):
            rampList.append((ddsRbfreq - f3, ddsRbfreq - f3, dtIntermediateRamp/s))

        evapTime = tEnd + 1.4*ms
        event(rfKnifeAmplitude, evapTime - 5*us, vca4)
        rampList.append((ddsRbfreq - f3, ddsRbfreq - f4, dtRamp4/s))
#        rampList.append((ddsRbfreq - f4, ddsRbfreq - f4, dtRamp4/s))
        tEnd = evapTime   + dtRamp4

        if (decompressAfterRamp == 3):
            tEnd += dtIntermediateRamp

        if (decompressAfterRamp == 4):
            setvar('dtIntermediateRamp', dtRampToImaging*intermediateRampRatio)
            setvar('tRampStart', tEnd)

#        event(motZShutter, tEnd-10*ms, 1)                 #open Z shutter
#        depumpMOT(tEnd-10*us, pumpingTime = 200*us)
#        event(motZShutter, tEnd, 0)                             #close Z shutter   

#        setvar('tRampStart', tEnd - (dtIntermediateRamp + 20*ms) )    # end of ramp 4

        
    #######################################################


#    tRampDown=tEnd+ 0*0.1*ms
#
#    tRampFinished = rampDownQuadCoils(tRampDown+1*ms, fullMagneticTrapCurrent = varFullMagneticTrapCurrent, dischargeCurrent = 100, rapidOff = False, quadRampRate = 1.0)
#
#    tEnd = tRampFinished


    ### Ramp 5 ############################################
    if (rampNumber >= 5):

        if (decompressAfterRamp == 4):
            rampList.append((ddsRbfreq - f4, ddsRbfreq - f4, dtIntermediateRamp/s))
#        setvar('dtIntermediateRamp', 00*ms)
#        rampList.append((ddsRbfreq - f4, ddsRbfreq - f4, dtIntermediateRamp/s))

        evapTime = tEnd + 1.4*ms
        event(rfKnifeAmplitude, evapTime - 5*us, vca5)
        rampList.append((ddsRbfreq - f4, ddsRbfreq - f5, dtRamp5/s))
        tEnd = evapTime + dtRamp5
        if (decompressAfterRamp == 4):
            tEnd += dtIntermediateRamp

#        tEnd = evapTime + dtIntermediateRamp+17*ms
#        setvar('tRampStart', evapTime + 20*ms)    # end of ramp 4
    #######################################################

    ### leave for microwave repump
#    rampList.append((ddsRbfreq, ddsRbfreq, .001))

    ## Run the DDS sweep for all linear segments
    event(ddsRfKnife, tFrequencyRampStart, rampList)

    tEnd += dtHoldCut

    event(rfKnifeAmplitude, tEnd + 0*0.1*ms, 0)
    #### Turn off microwave horn at end of evaporation)
    event(sixPointEightGHzSwitch, tEnd + 0*0.1*ms, 0)
   

#    tRampDown=tEnd
#    tRampFinished = rampDownQuadCoils(tRampDown+1*ms, fullMagneticTrapCurrent = varFullMagneticTrapCurrent, dischargeCurrent = 100, rapidOff = False, quadRampRate = 0.250)
#    tEnd = tRampFinished
#    tEnd += 10*ms    #For imagining

    return tEnd
