/*
VarioMS5611.h - Declaration file for the VarioMS5611 Barometric Variometer, Altimeter, Pressure & Temperature Sensor Arduino Library.

(c) 2021 Rainer Stransky
www.so-fa.de

This library is based on the MS5611-Arduino library by Korneliusz Jarzebski
see: https://github.com/jarzebski/Arduino-MS5611

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/**
 * \mainpage VarioMS5611 library, supporting barometric variometer, altimeter, 
 *        pressure & temperature provisioning in a accurate and smoothable manner
 *
 * \section intro_sec_de Summary
 * The VarioMS5611 library provides
 * * access to the raw pressure and temperatur values of the MS5611 
 * * calculation of the effective pressure and temperature values by using the MS5611 
 *   internal factory calibration data
 * * an interface to manage MS5611 internal oversampling rates (OSR)
 * * an interface to manage smoothing factors for pressure and variometer values
 * * an non blocking data aquisition method provided by using cooperative run() method, for sampling the pressure and temperature data
 * * some extra methods to get statistical measure value information
 *
 * \section signal_sec Signal quality
 * Using this library a oversampling rate of about 160000 samples/second can be reached. With according smoothing factors (~0.93) 
 * a standard deviation sigma of about 4-5cm/s can be reached. 
 * a signal-to-noise ratio of about 20 for small climbing rates (70cm/s) and a signal noise about less thn +-10cm/s
 * see:

 * ![Variometer-Plot](https://raw.githubusercontent.com/Pulsar07/VarioMS5611/master/doc/img/PlotOf-Vario-SigmaV-RelHeight.png)
 *
 * \section api_sec API
 * see: 
 * ![Class API reference](https://htmlpreview.github.io/?https://github.com/Pulsar07/VarioMS5611/blob/master/doc/html/classVarioMS5611.html)
 * \section hardware_sec Hardware
 * Specification of the MS5611/GY-63 
 * * https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FMS5611-01BA03%7FB3%7Fpdf%7FEnglish%7FENG_DS_MS5611-01BA03_B3.pdf
 *
 */

/**
 * \file VarioMS5611.h
 *
 * \brief header file of the VarioMS5611 library, supporting barometric variometer, altimeter, 
 *        pressure & temperature provisioning in a accurate and smoothable manner
 *
 * \author Author: Rainer Stransky
 *
 * \copyright This project is released under the GNU Public License v3
 *          see https://www.gnu.org/licenses/gpl.html.
 * Contact: opensource@so-fa.de
 *
 */

// Version history
// V0.1.0 : full functional initial version, 
//          based on MS5611-Arduino library V 1.0.0 by Korneliusz Jarzebski

#ifndef VARIO_MS5611_h
#define VARIO_MS5611_h

#define VARIO_MS5611_VERSION "V0.1.0"

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define MS5611_ADDRESS                (0x77)

#define MS5611_CMD_ADC_READ           (0x00)
#define MS5611_CMD_RESET              (0x1E)
#define MS5611_CMD_CONV_D1            (0x40)
#define MS5611_CMD_CONV_D2            (0x50)
#define MS5611_CMD_READ_PROM          (0xA2)

#define PRESSURE_SEALEVEL         101325

/**
 * over sampling rates used by MS5611 internally
 */
typedef enum
{
    MS5611_ULTRA_HIGH_RES   = 0x08, ///< 4096 samples
    MS5611_HIGH_RES         = 0x06, ///< 2048 samples
    MS5611_STANDARD         = 0x04, ///< 1024 samples
    MS5611_LOW_POWER        = 0x02, ///< 512 samples
    MS5611_ULTRA_LOW_POWER  = 0x00  ///< 256 samples
} ms5611_osr_t;

typedef enum
{
    NONE,
    DIGITAL_PRESSURE_VALUE,
    DIGITAL_TEMPERATURE_VALUE,
    LAST
} vario_value_t;


/// VarioMS5611 non-blocking data aquisition, for large OSR rates and accurate pressure, height and variometer values
/**
 * VarioMS5611 provides 
 * * access to the raw pressure and temperatur values of the MS5611 
 * * calculation of the effective pressure and temperature values by using the MS5611 
 *   internal factory calibration data
 * * an interface to manage MS5611 internal over sampling rates
 * * an non blocking run() method, sampling the pressure and temperature data
 *
 */
class VarioMS5611
{
    public:

	/// for initialzation
	/** has to be called once before the VarioMS5611 instance can be used */
	bool begin(ms5611_osr_t aSamplingRate = MS5611_ULTRA_HIGH_RES );

	/// read the raw tempeature value (blocking)
	/** returns the raw temperature value given by the MS5611 chip 
	 * readXXX() means here reading in a blocking manner (~ 0-30ms)
	 */
	uint32_t readRawTemperature(void);

	/// read the tempeature value (blocking) in °C
	/** returns the temperature value in °C (-40...85°C with 0.01°C resolution) 
	 * the returned value is an internal representation without an unit
	 * readXXX() means here reading in a blocking manner (~ 0-30ms)
	 * @param aCompensation if true a second order compensation is done (more accurate at T<20°C)
	 */
	double readTemperature(bool aCompensation = false);

	/// get raw temperature value (non-blocking)
	/**
	 * returns the raw temperature value given by the MS5611, of the last prefetched value 
	 * getXXX() means non-blocking get of pre fetched values/calculations/smoothings within run()
	 */
	uint32_t getRawTemperature(void);

	/// get temperature value in Pa (non-blocking)
	/**
	 * returns the temperature value in Pa , of the last prefetched calculated value 
	 * second order temperature compensation is manged by setSecondOrderCompenstation(val)
	 * getXXX() means non-blocking get of pre fetched values/calculations/smoothings within run()
	 */
	double getTemperature(void);

	/// read the raw pressure value (blocking)
	/** returns the raw pressure value given by the MS5611 chip
	 * readXXX() means here reading in a blocking manner (~ 0-30ms)
	 */
	uint32_t readRawPressure(void);


	/// read the pressure value (blocking) in Pa
	/** returns the pressure value in Pa 
	 * readXXX() means here reading in a blocking manner (~ 0-30ms)
	 * @param aCompensation if true a second order temperature compensation is done (more accurate at T<20°C)
	 */
	int32_t readPressure(bool aCompensation = false);

	/// get raw pressure value (non-blocking)
	/**
	 * returns the raw pressure value given by the MS5611, of the last prefetched value 
	 * getXXX() means non-blocking get of pre fetched values/calculations/smoothings within run()
	 */
	uint32_t getRawPressure(void);

	/// get pressure value in Pa (non-blocking)
	/**
	 * returns the pressure value in Pa , of the last prefetched calculated value 
	 * second order temperature compensation is manged by setSecondOrderCompenstation(val)
	 * getXXX() means non-blocking get of pre fetched values/calculations/smoothings within run()
	 */
	double getPressure();


	/// get the smoothed pressure value in Pa (non-blocking)
	/**
	 * returns the  the smoothed pressure value in Pa 
	 * second order temperature compensation is manged by setSecondOrderCompenstation(val)
	 * getXXX() means non-blocking get of pre fetched values/calculations/smoothings within run()
	 */
	double getSmoothedPressure(void);

	/// get the vertial speed value (variometer) in cm/s (non-blocking)
	/**
	 * returns the vertial speed value (variometer) in cm/s of the last prefetched pressure values
	 * second order temperature compensation is manged by setSecondOrderCompenstation(val)
	 * getXXX() means non-blocking get of pre fetched values/calculations/smoothings within run()
	 */
        int getVerticalSpeed(void);

	/// calculate the absolute altitude of the given pressure
	/**
	 * returns the calculated absolute altitude in meter, for the given pressure
	 * @aPressure pressure in Pa for which the altitude has to be calculated
	 * @aSeaLevelPressure reference pressure in Pa the altitude has to be related to
	 */
	double calcAltitude(double aPressure, double aSeaLevelPressure = 101325);


	/// calculate the relative altitude of the given pressure
	/**
	 * returns the calculated relative altitude in meter, for the given pressure
	 * @aPressure pressure in Pa for which the altitude has to be calculated
	 */
        double calcRelAltitude(double aPressure);

	/// set the IIR smoothing factor for the vertical speed (variometer) value
	/**
	 * for smoothing the verical speed value a IIR Low Pass Filter is used. 
	 * factor near to 1 means more smoothing 
	 * factor near to 0 means less smoothing
	 */
        void setVerticalSpeedSmoothingFactor(double aFactor);

	/// get the IIR smoothing factor for the vertical speed (variometer) value
	/**
	 * for smoothing the verical speed value a IIR Low Pass Filter is used. 
	 * factor near to 1 means more smoothing 
	 * factor near to 0 means less smoothing
	 */
        double getVerticalSpeedSmoothingFactor(void);

	/// set the IIR smoothing factor for the pressure value
	/**
	 * for smoothing the pressure value a IIR Low Pass Filter is used. 
	 * factor near to 1 means more smoothing 
	 * factor near to 0 means less smoothing
	 */
	void setPressureSmoothingFactor(double aFactor);

	/// get the IIR smoothing factor for the pressure value
	/**
	 * for smoothing the pressure value a IIR Low Pass Filter is used. 
	 * factor near to 1 means more smoothing 
	 * factor near to 0 means less smoothing
	 */
	double getPressureSmoothingFactor(void);

	/// get the reference height (stored at initialization)
	/**
	 * get the reference height (stored at initialization)
	 */
        double getReferenceHeight(void);

	/// set the oversampling rates setup used by the MS5611
	/** sets the MS5611 internal oversampling rates */
	void setOversampling(ms5611_osr_t osr);

	/// get the oversampling rate set to the MS5611
	/** gets the current used MS5611 internal oversampling rates */
	ms5611_osr_t getOversampling(void);

	/// non blocking data aquisition method provided by using cooperative run() method, for sampling the pressure and temperature data
	/** 
	 * this method has to be called in the loop. run() is not blocking or delaying.
	 * - receiving a 24-bit temperature or pressure value via I2C from the MS5611
	 * - sending a temperature or pressure request via I2C to the MS5611
	 * - do nothing and return, if the wait time for receving requested values is not reached
	 */
	void run();


	/// get the number of reads of the pressure and temperature values
	/** returns the number of read of the pressure and temperature values (1 means both are read one time)
	 */
        unsigned int getRunCount(void);

	/// get the number of reads per second of the pressure and temperature value
	/** returns the  number of reads per seconds of the raw MS5611 pressure and temperature value. 
	 * This depends on the oversampling rate and the CPU load (how often the run()-method is called.
	 * For OSR=MS5611_ULTRA_HIGH_RES a rate > 40 should be possible
	 */
        float getReadsPerSecond(void);

	/// get the second order temperature compensation setting for the prefetching data handling
	/** return true if the second order temperature compensation calculation (MS5611 raw values) of the temperature and pressure value is set
	 */
	bool getSecondOrderCompenstation();

	/// set the second order temperature compensation setting for the prefetching data handling
	/** controls the second order temperature compensation calculation (MS5611 raw values) of the temperature and pressure value.
	 * The default setting is false
	 */
	void setSecondOrderCompenstation(bool aDoCompensate);
    private:
	bool myDoSecondOrderCompensation;
	bool myWarmUpPhase;
        unsigned int myRunCnt;
        unsigned int myReadsCnt;
        unsigned long myReadsCntTimer;
        float myReadsPerSecond;
	double myPressureSmoothingFactor;
	double myReferenceHeight;
	vario_value_t myPendingValueType;
	boolean triggerReadValues(vario_value_t aRequestType = NONE);
	int myVerticalSpeed;
	double myVerticalSpeedSmoothingFactor;
	void calcFilter(void);
	void calcVerticalSpeed(void);
        int32_t calcTemperature(uint32_t aRawTemperature);
	int32_t calcTemperatureCompensatedPressure(uint32_t aRawPressure, uint32_t aRawTemperature);
	uint16_t myCompensationValues[6];
        uint32_t myRawPressureVal_D1;
        uint32_t myRawTemperatureVal_D2;
        int32_t myPressureVal;
        double  mySmoothedPressureVal;
        int16_t myPressureHistoryIdx;
        int16_t myRawPressureHistoryIdx;
        int32_t myTemperatureVal;

	uint8_t myct;
	uint8_t myuosr;
	int32_t myTEMP2;
	int64_t myOFF2, mySENS2;

	void reset(void);
	void readPROM(void);

	uint16_t readRegister16(uint8_t reg);
	uint32_t readRegister24(uint8_t reg);
};

#endif
