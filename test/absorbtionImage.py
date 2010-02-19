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
camera=dev('Andor iXon 885','ep-timing1.stanford.edu',0)


#setvar('signal0',     ch(fastAnalogOut, 0)) # The only input channel right now

shutter = ch(digitalOut,1)
#cameraTrigger=ch(digitalOut,0)
takeImage=ch(camera,0)

TA2 = ch(fastAnalogOut, 0)
TA3 = ch(fastAnalogOut, 1)
quadCoil = ch(fastAnalogOut, 1)
current1530 = ch(fastAnalogOut6,0)
aomSwitch0 = ch(dds, 0)
repumpVCO=dev('ADF4360-0', 'eplittletable.stanford.edu', 0)
coolingVCO=dev('ADF4360-6', 'eplittletable.stanford.edu', 3)
#testDevice = ch(slowAnalogOut, 0)

# Define different blocks of the experiment
def MOT(Start):

    ## Camera Settings ##
    dtCameraShutter = 0*ms
    dtCameraPulseWidth = 1000*us  + dtCameraShutter
    dtCameraDelay = 5*us
    expTime = 100*us



    
    ## 1530 Shutter Settings ##
    dtShutterOpenHoldOff = 2.04*ms

    #Initialization Settings
    tStart =1.1*s +dtShutterOpenHoldOff

    ## throwaway image settings ##
    tThrowaway = tStart
    filename1 = 'throwaway image'
    description1 = 'throwaway image'

    #AOM settings
#    absorptionFreq = 1067 
#    aomFreq0 = absorptionFreq / 8
    aomFreq0 = 110
    aomAmplitude0 = 100
    aomHoldOff = 10*us

    ## TA Settings ##
    voltageTA2 = 1.4
    voltageTA3 = 1.5
    dtMOTLoad = 500*ms
    tTAOff =  tStart + dtMOTLoad 

    ## Quad Coil Settings ##
    quadCoilVoltage = 3.01

    ## 1530 current settings ##
    voltage1530 = 0.9

    ## Imaging Settings ##
    dtDriftTime = .1*ms   #see flourescence settings

    dtAbsorbtionLight = 50*us
    tAbsorptionImage = tTAOff + dtDriftTime - dtCameraShutter
    tAomOn = tTAOff + dtDriftTime - aomHoldOff
    tQuadCoilOff = tTAOff
    tAbsorptionCamera = tAbsorptionImage - dtCameraDelay
    filename = 'absorption image'
    description2 = 'absorption image'

    ## Calibration Absorbtion Image Settings ##
    dtDeadMOT = 100*ms
    tCalibrationImage = tAbsorptionImage + dtDeadMOT
    tCalibrationCamera = tCalibrationImage - dtCameraDelay
    tAomCalibration = tCalibrationImage - aomHoldOff
    description3 = 'calibration image'

    ## Dark background imaging settings ##
    dtWait = 100*ms
    tDarkBackground = tCalibrationImage + dtWait
    description4 = 'background image'

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
    event(takeImage, tThrowaway, (expTime,description1))                #take throwaway image

    ## Load the MOT ##    
    event(TA2, tStart, voltageTA2)                   # TA on
    event(TA3, tStart, voltageTA3)                   # TA on
    event(TA2, tTAOff, 0)    # TA off
    event(TA3, tTAOff, 0)    # TA off

    ## Take an absorbtion image ##
    event(aomSwitch0, tAomOn, (aomFreq0, aomAmplitude0, 0)) #turn on absorbtion light
    event(aomSwitch0, tAomOn + dtAbsorbtionLight, (aomFreq0, 0, 0)) #turn off absorbtion light

    event(takeImage, tAbsorptionCamera, (expTime,description2,filename))                #take absorption image

    ## Take an abosorbtion calibration image after the MOT has decayed away ##

    event(aomSwitch0, tAomCalibration, (aomFreq0, aomAmplitude0, 0)) #turn on absorbtion light
    event(aomSwitch0, tAomCalibration + dtAbsorbtionLight, (aomFreq0, 0, 0)) #turn off absorbtion light 

    event(takeImage, tCalibrationCamera, (expTime,description3,filename))                #take absorption image

    ## Take a dark background image ##
    event(takeImage, tDarkBackground, (expTime,description4,filename))                #take absorption image

    event(TA2, tTAEndOfSequence, voltageTA2)
    event(TA3, tTAEndOfSequence, 0)
#    event(aomSwitch0, tTAEndOfSequence, (aomFreq0, aomAmplitude0, 0)) #turn on absorbtion light 
#    event(current1530, t1530EndOfSequence, voltage1530)
#    event(quadCoil, tQuadCoilEndOfSequence, quadCoilVoltage)
#    event(ch(repumpVCO, 1), tTAEndOfSequence, "-6 dBm")
#    event(ch(repumpVCO, 0), tTAEndOfSequence + 10*ms, 2562)
#    event(shutter, tTAEndOfSequence, 1)                             #1530 shutter open

  
    return Start


# Global definitions

t0 = 10*us

time = t0
time = MOT(time)
