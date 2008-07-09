/*************************************************************************
 *
 * Name:   GetLock.h
 *
 * C++ Windows header for scanning and locking the Vortex
 *
 * Susannah Dickerson 7/8/2008
 * Dave Johnson
 * Kasevich Group - Stanford University
 *
 *
 **************************************************************************/

#include "GetLock.h"


// Constructor
GETLOCK::GETLOCK()
{
	lockVoltage = 0;
	start_voltage = 0; // start point in GHz
	voltage_incr = .001; //increment frequency in GHz
	end_voltage = .260; // endpoint in GHz   
	usb_input_channel = 3;
	usb_output_channel = 0;

	GHzToV = .046/.700; // .700 GHz == .049 V
}

// Destructor
GETLOCK::~GETLOCK()
{
}


bool GETLOCK::lock (double* offsetGHz_p, MATLABPLOTTER &matlabplotter, USB1408FS &usb1408fs, AGILENT8648A &agilent8648a)
{
	double coolingPeakV;
	double rangeV = .4 * GHzToV; // roughly twice the expected width of the Rb 85 cooling transitions.
	double globalMaxV;
	int findMaxStartV;
	int findMaxEndV;
	bool noScan;

	std::vector <double> voltage_vector;
	std::vector <double> DAQ_vector;

	getParameters();

	noScan = scan(voltage_vector, DAQ_vector, usb1408fs);
	if (noScan) {return (1);}
	plot(voltage_vector, DAQ_vector, matlabplotter);

	coolingPeakV = findCoolingPeak(voltage_vector, DAQ_vector);
	std::cerr <<std::endl << coolingPeakV <<std::endl;

	std::cout << "Offset Lock frequency: " << *offsetGHz_p << " GHz" << std::endl;

	std::cout << "Enter desired offset lock frequency in GHz. Default value is: " << *offsetGHz_p << " GHz" << std::endl << "frequency = ";
	std::cin >> *offsetGHz_p;

	agilent8648a.output_on();
	agilent8648a.set_frequency(*offsetGHz_p);

	findMaxStartV = position(voltage_vector, coolingPeakV + *offsetGHz_p * GHzToV - rangeV / 2);
	findMaxEndV = position(voltage_vector, coolingPeakV + *offsetGHz_p * GHzToV + rangeV / 2);
	std::cerr << voltage_vector.at(findMaxStartV) << std::endl;
	std::cerr << voltage_vector.at(findMaxEndV) <<std::endl;


	voltage_vector.clear();
	DAQ_vector.clear();
		
	noScan = scan(voltage_vector, DAQ_vector, usb1408fs);
	if (noScan) {return (1);}
	plot(voltage_vector, DAQ_vector, findMaxStartV, findMaxEndV, matlabplotter);



	globalMaxV = findGlobalMax(voltage_vector, DAQ_vector, findMaxStartV, findMaxEndV);

	setLockVoltage(globalMaxV, usb1408fs);

	return(0);
}

void GETLOCK::getParameters ()
{
    bool change_vals = true; // have user defined values

	std::cout << "default values are as follows:" << std::endl;
	std::cout << "USB input channel: " << usb_input_channel << std::endl;
	std::cout << "USB output channel: " << usb_output_channel << std::endl;
    std::cout << "Start Voltage: " << start_voltage << " GHz" << std::endl;
    std::cout << "End Voltage: " << end_voltage << " GHz" << std::endl;
    std::cout << "Voltage Increment: " << voltage_incr << " GHz" << std::endl;
    std::cout << std::endl << "Do you want to change (1/0)? ";
    std::cin >> change_vals; 

    if(change_vals) {

        // user defined start frequency
        std::cout << "Enter desired start voltage in volts. Default value is: " << start_voltage << " V" << std::endl << "start voltage = ";
        std::cin >> start_voltage;
   
        std::cout << "Enter desired end voltage in volts. Default value is: " << end_voltage << " V" << std::endl << "end voltage = ";
        std::cin >> end_voltage;
   
        std::cout << "Enter desired voltage increment in volts. Default value is: " << voltage_incr << " V" << std::endl << "voltage increment = ";
        std::cin >> voltage_incr;
    }
}


void GETLOCK::setLockVoltage (double voltage, USB1408FS &usb1408fs)
{
	lockVoltage = (float) voltage;

	usb1408fs.set_output_voltage(usb_output_channel, lockVoltage);
}

void GETLOCK::plot(std::vector <double>& voltage_vector, std::vector <double>& DAQ_vector, MATLABPLOTTER &matlabplotter)
{
	bool save_data = true;

	matlabplotter.plotfreqscan(voltage_vector, DAQ_vector);

	std::cout << "Do you want to save the data (1/0)?";
    std::cin >> save_data;

	matlabplotter.savedata(save_data);
}

void GETLOCK::plot(std::vector <double>& voltage_vector, std::vector <double>& DAQ_vector, int start, int end, MATLABPLOTTER &matlabplotter)
{
	std::vector <double> voltageBound_vector;
	std::vector <double> DAQBound_vector;
	bool save_data = true;

	voltageBound_vector.push_back(voltage_vector.at(start));
	voltageBound_vector.push_back(voltage_vector.at(end));
	DAQBound_vector.push_back(DAQ_vector.at(start));
	DAQBound_vector.push_back(DAQ_vector.at(end));
	
	matlabplotter.plotfreqscan(voltage_vector, DAQ_vector);
	matlabplotter.plotlockpoints(voltageBound_vector,DAQBound_vector);

	std::cout << "Do you want to save the data (1/0)?";
    std::cin >> save_data;

	matlabplotter.savedata(save_data);
}


bool GETLOCK::scan (std::vector <double>& voltage_vector, std::vector <double>& DAQ_vector, USB1408FS &usb1408fs)
{
	double voltage = start_voltage;

	while(voltage <= end_voltage) {

		usb1408fs.set_output_voltage(usb_output_channel, (float) voltage); //set the output voltage to the back of the vortex

		voltage_vector.push_back(voltage); //record DAQ output voltage

		DAQ_vector.push_back(usb1408fs.read_input_channel(usb_input_channel)); //take data

		// change the frequency
		voltage = voltage + voltage_incr;

		Sleep(25); //wait for the DAQ to settle. spec'd rate is 10 KS/s
	}

	if (voltage_vector.size() == 0 || DAQ_vector.size() == 0) {
		std::cerr <<std::endl << "Error in GETLOCK::scan-- no data received" << std::endl;
		return (1);
	}

	return (0);
}




/*************************************************************************
 * findGlobalMin-- Public
 * Description-- return the laser voltage corresponding to the smallest 
 *             element of the photodiode voltage.
 * Input-- voltage_vector, a vector of laser voltages.
 *         DAQ_vector, a vector of corresponding photodiode voltages.
 * Return-- the value of LASERV_vector corresponding to the smallest
 *             element of PDV_vector.
 *************************************************************************/
int GETLOCK::findGlobalMin(std::vector <double>& voltage_vector, std::vector <double>& DAQ_vector, int start, int end)
{
	unsigned int i;
	unsigned int tempMinPos = start;
	double tempMin = DAQ_vector.at(tempMinPos);

	for (i = start; i < (unsigned) end; i++)
	{
		if (DAQ_vector.at(i) < tempMin)
		{
			tempMin = DAQ_vector.at(i);
			tempMinPos = i;
		}
	}

	return (tempMinPos);
}


double GETLOCK::findCoolingPeak(std::vector <double>& voltage_vector, std::vector <double>& DAQ_vector)
{
	unsigned int i;
	std::vector <int> minPositions;
	int oldTempMinPos = 0;
	int tempMinPos;
	int tempMinPos1;
	int tempMinPos2;
	double tempMin;
	double tempMin1;
	double tempMin2;
	int count = 1;
	double windowV = .005;
	int window = (int) ceil(windowV / fabs(voltage_vector.at(1)-voltage_vector.at(0)));

	for (i = 0; i < DAQ_vector.size()-window; i++)
	{
		tempMinPos = findGlobalMin(voltage_vector, DAQ_vector, i, i + window);
		if (tempMinPos == oldTempMinPos)
		{
			count++;
			if (count == window) {
				count = 1;
				minPositions.push_back(tempMinPos);
			}
		}
		else {
			oldTempMinPos = tempMinPos;
		}
	}

	if (minPositions.size() <= 1) {
		std::cerr << "Error in findCoolingPeak-- not enough minima found" << std::endl;
		return (0);
	}

	tempMinPos1 = minPositions.at(0);
	tempMinPos2 = minPositions.at(1);

	if (DAQ_vector.at(tempMinPos1) < DAQ_vector.at(tempMinPos2)){
		tempMinPos = tempMinPos1;
		tempMinPos1 = tempMinPos2;
		tempMinPos2 = tempMinPos1;
	}

	tempMin1 = DAQ_vector.at(tempMinPos1);
	tempMin2 = DAQ_vector.at(tempMinPos2);

	for (i = 0; i < minPositions.size();i++)
	{
		tempMinPos = minPositions.at(i);
		tempMin = DAQ_vector.at(tempMinPos);
		if (tempMin <= tempMin1){
			if (tempMin <= tempMin2) {
				tempMin1 = tempMin2;
				tempMinPos1 = tempMinPos2;
				tempMin2 = tempMin;
				tempMinPos2 = tempMinPos;
			}
			else {
				tempMin1 = tempMin;
				tempMinPos1 = tempMinPos;
			}
		}

	}

	if(tempMinPos1 > tempMinPos2) {
		tempMinPos = tempMinPos2;
		tempMinPos2 = tempMinPos1;
		tempMinPos1 = tempMinPos;
	}
	std::cerr << voltage_vector.at(tempMinPos1) << std::endl;
	std::cerr << voltage_vector.at(tempMinPos2) << std::endl;

	return (findGlobalMax(voltage_vector, DAQ_vector, tempMinPos1, tempMinPos2));
}


/*************************************************************************
 * findGlobalMax-- Public
 * Description-- return the laser voltage corresponding to the greatest 
 *             element of the photodiode voltage.
 * Input-- LASERV_vector, a vector of laser voltages.
 *         PDV_vector, a vector of corresponding photodiode voltages.
 * Return-- the value of LASERV_vector corresponding to the greatest
 *             element of PDV_vector.
 *************************************************************************/
double GETLOCK::findGlobalMax(std::vector <double>& voltage_vector, std::vector <double>& DAQ_vector, int start, int end)
{
	unsigned int i;
	unsigned int tempMaxPos = start;
	double tempMax = DAQ_vector.at(tempMaxPos);
	double nearEnd = .1;

	for (i = start; i < (unsigned) end; i++)
	{
		if (DAQ_vector.at(i) > tempMax)
		{
			tempMax = DAQ_vector.at(i);
			tempMaxPos = i;
		}
	}

	if ((double) i/DAQ_vector.size() > 1 - nearEnd || (double) i/DAQ_vector.size() < nearEnd) {
		std::cerr << "Warning: Transition found near edge of scan" << std::endl;
	}

	return (voltage_vector.at(tempMaxPos));
}

/*************************************************************************
 * position-- Private
 * Description-- Finds the first element in the vector that most nearly 
 *         matches the given element.
 * Input-- myVector, a vector of elements to be searched
 *         element, the element to be looked for.
 * Return-- the position in myVector of the element.
 * NOTE-- this function works only when myVector increases or decreases
 *         monotonically
 *************************************************************************/
int GETLOCK::position(std::vector <double>& myVector, double element)
{
	double oldDiff = fabs(myVector.at(0)-element);
	double newDiff = fabs(myVector.at(1)-element);
	int i = 1;	

	while(newDiff < oldDiff && i < (signed int) myVector.size() - 1)
	{
		i++;
		oldDiff = newDiff;
		newDiff = fabs(myVector.at(i)-element);
	}

	if(i == myVector.size() + 1){
		std::cerr << "WHICHLOCK::position--Warning: nearest value at end of vector" << std::endl;
	}

	return (i - 1);
}

