/*
  WingThing (c) 2019 Allen K. Lair, Sky Fun
*/

#include <Wire.h>
#include <utility/imumaths.h>

#include "PitotStatic.h"

static PitotStatic lastPitotSample;


// Get the data from the 4525DO sensor (airspeed and temperature)
// This function is used in place of what would otherwise be done by a library but there doesn't appear to be one for this sensor.
bool getPitotStatic( PitotStatic *pPitot )
{
    char           pitotBuff[4];
    int            iBuffPos;
    char           b1, b2, t1, t2;
    unsigned short usBridge, usTemp;

    memset( pitotBuff, 0x00, sizeof( char ) * 4 );
    if( Wire.requestFrom( 0x28, 4 ) != 4 )
    {
        *pPitot = lastPitotSample;
        return false;
    }
    iBuffPos = 0;
    while( Wire.available() )
    {
        pitotBuff[iBuffPos] = Wire.read();
        iBuffPos++;
    }

    // Bridge and Temp data in the 4-byte return
    b1 = pitotBuff[0];
    b2 = pitotBuff[1];
    t1 = pitotBuff[2];
    t2 = pitotBuff[3];

    b1 &= 0x3F;     // Mask out the two high bits of the pressure
    t2 &= 0xE0;     // Mask out the 5 low bits of the temperature
    usBridge = (static_cast<unsigned short int>( b1 ) << 8);
    usBridge |= (static_cast<unsigned short int>( b2 ) & 0x00FF);
    usTemp = ((static_cast<unsigned short int>( t1 ) << 8) | static_cast<unsigned short int>( t2 )) >> 5;

    pPitot->temp = ((static_cast<float>( usTemp ) * 200.0f) / 2047.0f) - 53.3f;  // 3.3 deg C offset from datasheet
    pPitot->airspeed = (static_cast<float>( usBridge ) - 8192.0f) / 2730.0f * 200.0f; // Airspeed as a ratio of pressure count to pressure range - this very likely needs calibration.
    if( pPitot->airspeed < 0.0f )
        pPitot->airspeed = 0.0f;

    lastPitotSample = *pPitot;

    return true;
}

