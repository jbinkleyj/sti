from stipy import *
 
ns = 1.0
us = 1000.0
ms = 1000000.0
s = 1000000000.0

# Set description used by program
setvar('desc','''Take a picture.''')

digitalOut=dev('Digital Out','ep-timing1.stanford.edu',2)
slowAnalogOut=dev('Slow Analog Out', 'ep-timing1.stanford.edu', 4)
fastAnalogOut = dev('Fast Analog Out', 'ep-timing1.stanford.edu', 1)
fastAnalogOut6 = dev('Fast Analog Out', 'ep-timing1.stanford.edu', 6)
trigger = dev('FPGA_Trigger', 'ep-timing1.stanford.edu', 8)
dds = dev('DDS', 'ep-timing1.stanford.edu', 0)
vco0=dev('ADF4360-0', 'ep-timing1.stanford.edu', 0)
vco1=dev('ADF4360-5', 'ep-timing1.stanford.edu', 1)
vco2=dev('ADF4360-5', 'ep-timing1.stanford.edu', 2)


#setvar('signal0',     ch(fastAnalogOut, 0)) # The only input channel right now

shutter = ch(digitalOut,1)
cameraTrigger=ch(digitalOut,0)
TA2 = ch(fastAnalogOut, 0)
quadCoil = ch(fastAnalogOut, 1)
current1530 = ch(fastAnalogOut6,0)
aomSwitch0 = ch(dds, 0)
repumpVCO=dev('ADF4360-0', 'eplittletable.stanford.edu', 0)
coolingVCO=dev('ADF4360-6', 'eplittletable.stanford.edu', 3)
#testDevice = ch(slowAnalogOut, 0)

# Define different blocks of the experiment
def MOT(Start):

    ## 1530 Shutter Settings ##
    dtShutterOpenHoldOff = 2.04*ms

    #Initialization Settings
    tStart = 1000*us +dtShutterOpenHoldOff

    ## Camera Settings ##
    dtCameraDelay = 5*us
    dtCameraPadding = 0*us


    #AOM settings
#    absorptionFreq = 1067 
#    aomFreq0 = absorptionFreq / 8
    aomFreq0 = 110
    aomAmplitude0 = 100
    aomHoldOff = 10*us

    ## TA Settings ##
    voltageTA = 1.4
    dtMOTLoad = 1000*ms
    tTAOff =  tStart + dtMOTLoad 

    ## Quad Coil Settings ##
    quadCoilVoltage = 3.01

    ## 1530 current settings ##
    voltage1530 = 0.9

    ## Camera Settings ##
    dtCameraShutter = 0*ms
    dtCameraPulseWidth = 1000*us  + dtCameraShutter

    ## Imaging Settings ##
    dtDriftTime = 3*ms   #see flourescence settings

    dtAbsorbtionLight = 1000*us
    tImage = tTAOff + dtDriftTime - dtCameraShutter
    tAomOn = tTAOff + dtDriftTime - aomHoldOff
#    tTAOn = tImage
    tQuadCoilOff = tTAOff
    tCamera = tImage - dtCameraDelay - dtCameraPadding

    ## Calibration Absorbtion Image Settings ##
    dtDeadMOT = 100*ms
    tCalibrationImage = tImage + dtDeadMOT
    tCameraCalibration = tCalibrationImage - dtCameraDelay - dtCameraPadding
    tAomCalibration = tCalibrationImage - aomHoldOff

    ## Dark background imaging settings ##
    dtWait = 100*ms
    tDarkBackground = tCalibrationImage + dtWait

    ## 1530 Shutter Settings ##
    t1530Off = tTAOff
    dtShutterCloseHoldOff = 2.055*ms
    tShutterOpen = tStart - dtShutterOpenHoldOff
    tShutterClose = t1530Off - dtShutterCloseHoldOff


    ## End of Sequence Settings ##
#    tQuadCoilEndOfSequence = tDarkBackground + tWait
#    t1530EndOfSequence = tDarkBackground + 2*tWait - dtShutterCloseHoldOff
    tTAEndOfSequence = tDarkBackground +2*ms

    #################### events #######################

    event(ch(trigger, 0), 10*us, "Stop" )
    event(ch(trigger, 0), 30*us, "Play" )

    event(aomSwitch0,tStart, (aomFreq0, 0 ,0)) # AOM is off, so no imaging light
    event(cameraTrigger, tStart, 0)                # initialize Camera Trigger

    ## Load the MOT ##    
    event(TA2, tStart, voltageTA)                   # TA on
    event(TA2, tTAOff, 0)    # TA off

    ## Take a fluorescence image ##
    event(TA2, tAomOn,voltageTA) #turn on fluorescence light
    event(TA2, tAomOn + dtAbsorbtionLight,0) #turn off fluorescence light

    event(cameraTrigger, tCamera, 1)
    event(cameraTrigger, tCamera + dtCameraPulseWidth, 0)

    ## Take a fluorescence calibration image after the MOT has decayed away ##

    event(TA2,  tAomCalibration,voltageTA) #turn on fluorescence light
    event(TA2, tAomCalibration + dtAbsorbtionLight,0) #turn off fluorescence light    

    event(cameraTrigger, tCameraCalibration, 1)
    event(cameraTrigger, tCameraCalibration + dtCameraPulseWidth, 0)

    ## Take a dark background image ##
    event(cameraTrigger, tDarkBackground, 1)
    event(cameraTrigger,  tDarkBackground + dtCameraPulseWidth, 0)

    event(TA2, tTAEndOfSequence, 0)
  
    return Start


# Global definitions

t0 = 10*us

time = t0
time = MOT(time)
