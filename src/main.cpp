/*
  WingThing (c) 2019 Allen K. Lair, Sky Fun
*/

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_BNO055.h>

// If you plan on using an 8266-based board, specifically the Wemos D1 Mini, these are a MUST.  The Arduino/PlatformIO versions will crash the microcontroller
#include <ESP8266WiFi.h>
#include <ESPAsyncUDP.h>

// These are simplified drivers for only this application
// There are no publicly available drivers for the 4525DO or the RM3100 using SPI (there is an Rm3100 I2C driver)
#include "PitotStatic.h"
#include "Magnetometer.h"

// Undotted firmware revision that gets sent with every packet
#define FIRMWARE_VERSION "200"

// Uncomment to enable debug prints
// #define SERIAL_OUTPUT


Adafruit_BMP3XX bmp;        // Adafruit BMP388 driver
PitotStatic     pitot;      // 4525DO pitot static driver struct
Magnetometer    mag;        // PNI RM3100 Magnetometer
AsyncUDP        udp;        // UDP to/from Stratofier (using Stratux as an AP)
Adafruit_BNO055 bno = Adafruit_BNO055( 55, 0x29 );

float           seaPress = 1013.25;    // Sea Level pressure in hPa
float           altitude;
int             iFromStratofier;
char            fromString[128];
uint8_t         udpString[128];
int32_t        mX, mY, mZ;
int             iSendLen;
IPAddress       stratux( 192, 168, 10, 255 );
sensors_event_t orient, gravity;
int             iHB = 0;


void udpPacketHandler( AsyncUDPPacket packet )
{
    char fromStratofier[16];

    memcpy( fromStratofier, (char *)packet.data(), packet.length() );
    fromStratofier[packet.length()] = '\0';
    seaPress = atof( fromStratofier ) * 33.8638;
}


void setup()
{
    pinMode( D0, OUTPUT );
    digitalWrite( D0, HIGH );

#ifdef SERIAL_OUTPUT
    Serial.begin( 921600 );
    Serial.println( "WingThing Started" );
#endif
    // Initialize the altimiter
    if( !bmp.begin() )
    {
#ifdef SERIAL_OUTPUT
        Serial.println( "BMP388 Altitude Sensor not found." );
#endif
        while( true )
            delay( 1000 );
    }
    bmp.setTemperatureOversampling( BMP3_OVERSAMPLING_32X );
    bmp.setPressureOversampling( BMP3_OVERSAMPLING_32X );
    bmp.setIIRFilterCoeff( BMP3_IIR_FILTER_DISABLE );

    // Initialize the magnetometer
    mag.begin();

    // Initialize the orientation sensor
    bno.begin();
    bno.setMode( Adafruit_BNO055::OPERATION_MODE_NDOF );
    // This is the correct orientation as it is mounted in the WingThing - if you did something different consult the datasheet for the correct orientation
    bno.setAxisRemap( Adafruit_BNO055::REMAP_CONFIG_P0 );
    bno.setAxisSign( Adafruit_BNO055::REMAP_SIGN_P0 );
    if( !bno.begin() )
    {
#ifdef SERIAL_OUTPUT
        Serial.println("Orientation init Failed");
#endif
        while( true )
            delay( 1000 );
    }

    // There is no init required for the airspeed sensor

    // Connect to the stratux
    WiFi.begin( "stratux" );
    if( WiFi.waitForConnectResult() != WL_CONNECTED )
    {
#ifdef SERIAL_OUTPUT
        Serial.println("WiFi Failed");
#endif
        while( true )
            delay( 1000 );
    }

    // Handler for incoming UDP data - currently only barometric pressure.
    if( udp.listen( 45678 ) )
        udp.onPacket( udpPacketHandler );
}

void loop()
{
    // Poll the altimiter
    if( !bmp.performReading() )
    {
#ifdef SERIAL_OUTPUT
        Serial.println( "Cannot read BMP388 Altitude Sensor" );
#endif
        return;
    }
    altitude = bmp.readAltitude( seaPress ) + 363.0;    // Calibration offset is a simple addition


    // Poll the airspeed
    if( !getPitotStatic( &pitot ) )
    {
#ifdef SERIAL_OUTPUT
        Serial.println( "Missed read of 4525DO Airspeed Sensor (not necessarily an error)" );
#endif
        return;
    }

    // Poll the magnetometer
    mag.getHeading( &mX, &mY, &mZ ); // Currently not using heading; Stratofier will calculate since it needs the microtesla measurements for calibration anyway

    // Poll the accelerometer, gyro and linear acceleration values
    bno.getEvent( &orient, Adafruit_BNO055::VECTOR_EULER );
    bno.getEvent( &gravity, Adafruit_BNO055::VECTOR_LINEARACCEL );

    // Send the data over UDP
    sprintf( fromString,
             "%s,"
             "%.2f,"
             "%.2f,"
             "%.2f,"
             "%d,%d,%d,"
             "%.4f,%.4f,%.4f,"
             "%.4f,%.4f,%.4f",
             FIRMWARE_VERSION,
             pitot.airspeed,
             altitude,
             bmp.temperature,
             mX, mY, mZ,
             orient.orientation.x, orient.orientation.y, orient.orientation.z,
             gravity.acceleration.x, gravity.acceleration.y, gravity.acceleration.z );
    iSendLen = strlen( fromString );
    memcpy( udpString, fromString, iSendLen );

    AsyncUDPMessage msg( strlen( fromString ) );

    msg.write( udpString, iSendLen );
    udp.sendTo( msg, stratux, 45678 );

#ifdef SERIAL_OUTPUT
    sprintf( fromString,
             "FW: %s\n"
             "S: %.2f, "
             "A: %.2f, "
             "T: %.2f\n"
             "MAG: %d, %d, %d\n"
             "ORIENT: %.4f,%.4f,%.4f\n"
             "GFORCE: %.4f,%.4f,%.4f",
             FIRMWARE_VERSION,
             pitot.airspeed,
             bmp.readAltitude( seaPress ),
             bmp.temperature,
             mX, mY, mZ,
             orient.orientation.x, orient.orientation.y, orient.orientation.z,
             gravity.acceleration.x, gravity.acceleration.y, gravity.acceleration.z );
    Serial.println( fromString );
#endif

    // Heartbeat
    if( iHB == 8 )
    {
        digitalWrite( D0, LOW );
        delay( 50 );
        digitalWrite( D0, HIGH );
        delay( 450 );
        iHB = 0;
    }
    else
        delay( 500 );

    iHB++;
}
