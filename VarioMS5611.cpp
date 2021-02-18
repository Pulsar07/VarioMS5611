/*
VarioMS5611.cpp - Class definition file for the VarioMS5611 Barometric Variometer, Altimeter, Pressure & Temperature Sensor Arduino Library.

(c) 2021 Rainer Stransky
www.so-fa.de

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

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>
#include <math.h>

#include "VarioMS5611.h"

bool VarioMS5611::begin(ms5611_osr_t aSamplingRate) {
    Wire.begin();
    reset();
    setOversampling(aSamplingRate);
    delay(100);
    readPROM();

    myPendingValueType = NONE;
    myPressureSmoothingFactor = 0.9d;

    // set a valid inital value
    for (int i=0; i < 50; i++) {
      mySmoothedPressureVal = readPressure(true);
    }
    myRawTemperatureVal_D2 = readRawTemperature();
    myVerticalSpeed = 0.0d;
    myVerticalSpeedSmoothingFactor = 0.9d;
    myTemperatureVal = readTemperature(true);
    myReferenceHeight = calcAltitude(getSmoothedPressure());     
    myDoSecondOrderCompensation = false;
    myRunCnt = 0;
    myWarmUpPhase = true;
    #ifdef VARIO_EXTENDED_INTERFACE
    myReadsCnt = 0;
    myReadsCntTimer = millis();
    myReadsPerSecond = 0.0f;
    #endif

    return true;
}

void VarioMS5611::setOversampling(ms5611_osr_t osr)
{
    switch (osr)
    {
	case MS5611_ULTRA_LOW_POWER:
	    myct = 1;
	    break;
	case MS5611_LOW_POWER:
	    myct = 2;
	    break;
	case MS5611_STANDARD:
	    myct = 3;
	    break;
	case MS5611_HIGH_RES:
	    myct = 5;
	    break;
	case MS5611_ULTRA_HIGH_RES:
	    myct = 10;
	    break;
    }

    myuosr = osr;
}

ms5611_osr_t VarioMS5611::getOversampling(void)
{
    return (ms5611_osr_t)myuosr;
}

void VarioMS5611::reset(void)
{
    Wire.beginTransmission(MS5611_ADDRESS);

    #if ARDUINO >= 100
	Wire.write(MS5611_CMD_RESET);
    #else
	Wire.send(MS5611_CMD_RESET);
    #endif

    Wire.endTransmission();
}

void VarioMS5611::readPROM(void)
{
    for (uint8_t offset = 0; offset < 6; offset++)
    {
	myCompensationValues[offset] = readRegister16(MS5611_CMD_READ_PROM + (offset * 2));
    }
}

uint32_t VarioMS5611::readRawTemperature(void)
{
  while (!triggerReadValues(DIGITAL_TEMPERATURE_VALUE)) {
    delay(1);
  }
  return myRawTemperatureVal_D2;
}

uint32_t VarioMS5611::getRawTemperature(void) {
  return myRawTemperatureVal_D2;
}

boolean VarioMS5611::triggerReadValues(vario_value_t aRequestType) {
  static unsigned long nextRead = 0;
  boolean retVal = false;

  if (millis() > (nextRead)) {
    // values can be read now !!!
    myRunCnt++;
    if (myRunCnt == 100 ) {
      myWarmUpPhase = false;
      // after a couple (100) of run()'s the temperature of the sensor is more stable
      // so some values has to be fixed finally
      myReferenceHeight = calcAltitude(getSmoothedPressure());     
    }
    #ifdef VARIO_EXTENDED_INTERFACE
    if ( (myReadsCntTimer+1000) < millis() ) {
      myReadsPerSecond = (float) myReadsCnt / ((millis() - myReadsCntTimer)/1000);
      myReadsCntTimer = millis();
      myReadsCnt = 0;
    }
    #endif
    if (myPendingValueType == DIGITAL_PRESSURE_VALUE) {
        #ifdef VARIO_EXTENDED_INTERFACE
        myReadsCnt++;
        #endif
        myRawPressureVal_D1 = readRegister24(MS5611_CMD_ADC_READ);
	myTemperatureVal = calcTemperature(myRawTemperatureVal_D2);
	myPressureVal = calcTemperatureCompensatedPressure(myRawPressureVal_D1, myRawTemperatureVal_D2);
	calcFilter();

    } else if (myPendingValueType == DIGITAL_TEMPERATURE_VALUE) {
        myRawTemperatureVal_D2 = readRegister24(MS5611_CMD_ADC_READ);
    } else {
    } 

    if (aRequestType == myPendingValueType) {
      retVal = true;
    }

    // now an potentially pending read value is read
    // and an new value can be requested
    uint8_t valueAddr;
    if (aRequestType != NONE) {
      myPendingValueType = aRequestType;
      switch(aRequestType) {
        case DIGITAL_TEMPERATURE_VALUE:
          valueAddr = MS5611_CMD_CONV_D2 + myuosr;
	  break;
        case DIGITAL_PRESSURE_VALUE:
          valueAddr = MS5611_CMD_CONV_D1 + myuosr;
	  break;
      }
    } else if (myRunCnt %2 == 0) {
      myPendingValueType = DIGITAL_TEMPERATURE_VALUE;
      valueAddr = MS5611_CMD_CONV_D2 + myuosr;
    } else {
      myPendingValueType = DIGITAL_PRESSURE_VALUE;
      valueAddr = MS5611_CMD_CONV_D1 + myuosr;
    }

    // request data and do not wait for answer
    Wire.beginTransmission(MS5611_ADDRESS);
    #if ARDUINO >= 100
      Wire.write(valueAddr);
    #else
      Wire.send(valueAddr);
    #endif
    Wire.endTransmission();
    nextRead = millis() + myct;
    
  } else {
    // do nothing, there is an pending value requested and we have to wait 
    // till the answer can be read
  } 
  return retVal;
}

uint32_t VarioMS5611::readRawPressure(void)
{
  while (!triggerReadValues(DIGITAL_PRESSURE_VALUE)) {
    delay(1);
  }
  return myRawPressureVal_D1;
}

uint32_t VarioMS5611::getRawPressure(void) {
  return myRawPressureVal_D1;
}


int32_t VarioMS5611::calcTemperature(uint32_t aRawTemperature) {
    uint32_t D2 = aRawTemperature;
    int32_t dT = D2 - (uint32_t)myCompensationValues[4] * 256;

    int32_t TEMP = 2000 + ((int64_t) dT * myCompensationValues[5]) / 8388608;

    myTEMP2 = 0;

    // second order temperature compensation
    if (myDoSecondOrderCompensation) 
    {
	if (TEMP < 2000)
	{
	    myTEMP2 = (dT * dT) / INT32_MAX;
	}
    }

    TEMP = TEMP - myTEMP2;

    // return temperature in 1/100 °C: 2007 = 20.07°C
    return TEMP;
}

int32_t VarioMS5611::calcTemperatureCompensatedPressure(uint32_t aRawPressure, uint32_t aRawTemperature) {

    int32_t dT = aRawTemperature - (uint32_t)myCompensationValues[4] * 256;
    int64_t OFF = (int64_t)myCompensationValues[1] * 65536 + (int64_t)myCompensationValues[3] * dT / 128;
    int64_t SENS = (int64_t)myCompensationValues[0] * 32768 + (int64_t)myCompensationValues[2] * dT / 256;

    if (myDoSecondOrderCompensation) 
    {
	int32_t TEMP = 2000 + ((int64_t) dT * myCompensationValues[5]) / 8388608;

	myOFF2 = 0;
	mySENS2 = 0;

	if (TEMP < 2000)
	{
	    myOFF2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 2;
	    mySENS2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 4;
	}

	if (TEMP < -1500)
	{
	    myOFF2 = myOFF2 + 7 * ((TEMP + 1500) * (TEMP + 1500));
	    mySENS2 = mySENS2 + 11 * ((TEMP + 1500) * (TEMP + 1500)) / 2;
	}

	OFF = OFF - myOFF2;
	SENS = SENS - mySENS2;
    }

    uint32_t result = (aRawPressure * SENS / 2097152 - OFF) / 32768;

    return result;
}

void VarioMS5611::setSecondOrderCompenstation(bool aDoCompensate) {
  myDoSecondOrderCompensation = aDoCompensate;
}

bool VarioMS5611::getSecondOrderCompenstation() {
  return myDoSecondOrderCompensation;
}

double VarioMS5611::getPressureSmoothingFactor(void) {
  return myPressureSmoothingFactor;
}

void VarioMS5611::setPressureSmoothingFactor(double aFactor) {
  myPressureSmoothingFactor = aFactor;
}

void VarioMS5611::calcFilter(void) {
  // Vario Filter
  // IIR Low Pass Filter
  // y[i] := α * x[i] + (1-α) * y[i-1]
  //      := α * x[i] + (1 * y[i-1]) - (α * y[i-1])
  //      := α * x[i] +  y[i-1] - α * y[i-1]
  //      := α * ( x[i] - y[i-1]) + y[i-1]
  //      := y[i-1] + α * (x[i] - y[i-1])
  // mit α = 1- β
  //      := y[i-1] + (1-ß) * (x[i] - y[i-1])
  //      := y[i-1] + 1 * (x[i] - y[i-1]) - ß * (x[i] - y[i-1])
  //      := y[i-1] + x[i] - y[i-1] - ß * x[i] + ß * y[i-1]
  //      := x[i] - ß * x[i] + ß * y[i-1]
  //      := x[i] + ß * y[i-1] - ß * x[i]
  //      := x[i] + ß * (y[i-1] - x[i])
  
  mySmoothedPressureVal = (double) myPressureVal + myPressureSmoothingFactor * (mySmoothedPressureVal - myPressureVal);
  
  calcVerticalSpeed();
}

void VarioMS5611::setVerticalSpeedSmoothingFactor(double aFactor) {
  myVerticalSpeedSmoothingFactor = aFactor;
}

double VarioMS5611::getVerticalSpeedSmoothingFactor(void) {
  return myVerticalSpeedSmoothingFactor;
}

void VarioMS5611::calcVerticalSpeed(void) {
  // Vario calculation
  static unsigned long lastTime = 0;
  unsigned long dT = millis() - lastTime;     // delta time in ms
  static double lastAltitude = 0;

  double altitude = calcAltitude(getSmoothedPressure())*100; // altitude in cm
  if (myWarmUpPhase) {
    lastAltitude = altitude;
  }
  double vspeed = (altitude - lastAltitude) * (1000.0 / dT);
  myVerticalSpeed = vspeed + myVerticalSpeedSmoothingFactor * (myVerticalSpeed - vspeed);
  lastAltitude = altitude;
  lastTime = millis();
}

int VarioMS5611::getVerticalSpeed(void) { 
  return myVerticalSpeed;
}

unsigned int VarioMS5611::getRunCount() {
  return myRunCnt;
}

#ifdef VARIO_EXTENDED_INTERFACE
float VarioMS5611::getReadsPerSecond() {
  return myReadsPerSecond;
}
#endif

double VarioMS5611::getSmoothedPressure(void) {
  return mySmoothedPressureVal;
}

double VarioMS5611::getPressure(void) {
  return myPressureVal;
}

int32_t VarioMS5611::readPressure(bool aCompensation)
{
    uint32_t D1 = readRawPressure();

    uint32_t D2 = readRawTemperature();
    int32_t dT = D2 - (uint32_t)myCompensationValues[4] * 256;

    int64_t OFF = (int64_t)myCompensationValues[1] * 65536 + (int64_t)myCompensationValues[3] * dT / 128;
    int64_t SENS = (int64_t)myCompensationValues[0] * 32768 + (int64_t)myCompensationValues[2] * dT / 256;

    if (aCompensation)
    {
	int32_t TEMP = 2000 + ((int64_t) dT * myCompensationValues[5]) / 8388608;

	myOFF2 = 0;
	mySENS2 = 0;

	if (TEMP < 2000)
	{
	    myOFF2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 2;
	    mySENS2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 4;
	}

	if (TEMP < -1500)
	{
	    myOFF2 = myOFF2 + 7 * ((TEMP + 1500) * (TEMP + 1500));
	    mySENS2 = mySENS2 + 11 * ((TEMP + 1500) * (TEMP + 1500)) / 2;
	}

	OFF = OFF - myOFF2;
	SENS = SENS - mySENS2;
    }

    uint32_t P = (D1 * SENS / 2097152 - OFF) / 32768;

    return P;
}

double VarioMS5611::getTemperature(void) {
  return ((double) myTemperatureVal)/100;
}

double VarioMS5611::readTemperature(bool aCompensation)
{
    uint32_t D2 = readRawTemperature();
    int32_t dT = D2 - (uint32_t)myCompensationValues[4] * 256;

    int32_t TEMP = 2000 + ((int64_t) dT * myCompensationValues[5]) / 8388608;

    myTEMP2 = 0;

    if (aCompensation) {
      // second order compensation
      if (TEMP < 2000) {
        myTEMP2 = (dT * dT) / INT32_MAX;

      }
    }

    TEMP = TEMP - myTEMP2;

    return ((double)TEMP/100);
}

double VarioMS5611::getReferenceHeight(void) {
  return myReferenceHeight;
}

/**
 * return the relative altitude 
 */
double VarioMS5611::calcRelAltitude(double aPressure) {
  double retVal = calcAltitude(aPressure) - myReferenceHeight;
  return retVal;
} 

double VarioMS5611::calcAltitude(double aPressure, double aSeaLevelPressure)
{
    return (44330.0f * (1.0f - pow((double)aPressure / (double)aSeaLevelPressure, 0.1902949f)));
}

// Read 16-bit from register (oops MSB, LSB)
uint16_t VarioMS5611::readRegister16(uint8_t reg)
{
    uint16_t value;
    Wire.beginTransmission(MS5611_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
    #else
        Wire.send(reg);
    #endif
    Wire.endTransmission();

    Wire.beginTransmission(MS5611_ADDRESS);
    Wire.requestFrom(MS5611_ADDRESS, 2);
    while(!Wire.available()) {};
    #if ARDUINO >= 100
        uint8_t vha = Wire.read();
        uint8_t vla = Wire.read();
    #else
        uint8_t vha = Wire.receive();
        uint8_t vla = Wire.receive();
    #endif
    Wire.endTransmission();

    value = vha << 8 | vla;

    return value;
}

// Read 24-bit from register (oops XSB, MSB, LSB)
uint32_t VarioMS5611::readRegister24(uint8_t reg)
{
    uint32_t value;
    Wire.beginTransmission(MS5611_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
    #else
        Wire.send(reg);
    #endif
    Wire.endTransmission();

    Wire.beginTransmission(MS5611_ADDRESS);
    Wire.requestFrom(MS5611_ADDRESS, 3);
    while(!Wire.available()) {};
    #if ARDUINO >= 100
        uint8_t vxa = Wire.read();
        uint8_t vha = Wire.read();
        uint8_t vla = Wire.read();
    #else
        uint8_t vxa = Wire.receive();
        uint8_t vha = Wire.receive();
        uint8_t vla = Wire.receive();
    #endif
    Wire.endTransmission();

    value = ((int32_t)vxa << 16) | ((int32_t)vha << 8) | vla;

    return value;
}

void VarioMS5611::run() {
  triggerReadValues();
}
