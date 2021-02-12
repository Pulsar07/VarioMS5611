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

#include <Wire.h>
#include <VarioMS5611.h>

VarioMS5611 varioMS5611;

void setup() 
{
  Serial.begin(115200);
  Serial.println("# Simple VarioMS5611 usage ... ");

  while(!varioMS5611.begin(MS5611_ULTRA_HIGH_RES))
  {
    Serial.println("# waiting for varioMS5611");
    delay(500);
  }
  varioMS5611.setOversampling(MS5611_ULTRA_HIGH_RES);
  varioMS5611.setVerticalSpeedSmoothingFactor(0.92);
  varioMS5611.setPressureSmoothingFactor(0.93);

}

void loop()
{
  varioMS5611.run();

  static unsigned long lastTime = 0;
  unsigned long now;
  now = millis();
  if ( now - lastTime > 180) {
    Serial.print("time: ");
    Serial.print(now);  // #2
    Serial.print(" pressure: ");
    Serial.print(varioMS5611.getPressure()); // #4
    Serial.print(" temperature: ");
    Serial.print(varioMS5611.getTemperature()); // #6
    Serial.print(" abs.height: ");
    Serial.print(varioMS5611.calcAltitude(varioMS5611.getSmoothedPressure())); // #8
    Serial.print(" rel.height: ");
    Serial.print(varioMS5611.calcRelAltitude(varioMS5611.getSmoothedPressure())); // #10
    Serial.print(" vario: ");
    Serial.print(varioMS5611.getVerticalSpeed()); // #12
    Serial.println();
    lastTime = now;
  }
} 

    
