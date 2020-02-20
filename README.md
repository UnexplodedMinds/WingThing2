![SkyFun](http://skyfun.space/wp-content/uploads/2019/07/SkyFunLogo2-1.png)

# BADASP
Backup Air Data And Sensor Platform - a name I'm considering 8D

Firmware for second WingThing prototype

**What this is not:**

This should only ever be used as a backup or to augment purely VFR flight.  This is not a commerical product.  It is not certified and has not gone through the rigorous testing that commercial and certified products use.  This is purely experimental meant as a proof of concept to use inexpensive hobby sensors and microcontrollers as backup instrumentation.

**What this is:**

BADASP is a microcontroller-based platform using inexpensive sensors to provide air, magnetic and orientation data to an Android-based application in the cockpit.  You can write your own software but it is meant to transmit to the broadcast address on the Stratux ADS-B receiver acting as an access point, received by the Stratofier Android App (source code also available here).  You could also modify the Stratux source code to use the data directly if you want to only use your favorite EFB that supports Stratux.

This repository contains the source code to build the firmware for an ESP8266 microcontroller connected to an Adafruit BMP388 pressure altimiter, a 4525DO pitot-static sensor, Adafruit BNO055 9DOF orientation sensor and a PNI RM3100 3-axis magnetometer.

All sensors besides the magnetometer use the I2C bus.  The PNI RM3100 3-axis magnetometer uses the SPI interface.  PNI sells an I2C breakout board that appears to be a little larger than the version that supports both.

**This is what you need:**

A microcontroller with at least one SPI and one I2C interface.  This repository assumes you are using a Wemos D1 Mini or compatible with built-in wifi.  The specific model I used also has an integrated battery charger and 18650 battery holder.

4525DO-based pitot-static sensor with an I2C interface.  I got mine from 3D Robotics.

PNI RM3100 Magnetometer.  Note that the BNO055 also has an integrated magnetometer but it isn't nearly as accurate as this one.  It uses inductive sensors instead of hall effect sensors that are far more accurate.  It's a little more expensive but it's worth it.

Adafruit BNO055 9DOF orientation sensor.  This project only uses the linear acceleration and orientation data.  You could use a simpler cheaper sensor that only handles acceleration and orientation but you may need to adjust the source code if you use something different.

Adafruit BMP388.  As of this writing this is the most accurate inexpensive pressure sensor.  In limited testing it appears to be within about 2 or 3 feet of actual.  The MPL3115A2 was also very accurate but it has a very long sampling time.

The prototype uses a proto board mounted on a riser directly over the ESP8266 microcontroller with the sensors all mounted in a row.  For details go to the [SkyFun Website](http://skyfun.space/)
