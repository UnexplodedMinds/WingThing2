#include "Adafruit_SPIDevice.h"

#include "Magnetometer.h"


Magnetometer::Magnetometer()
    : m_pMag( new Adafruit_SPIDevice( SS ) )
{
}


Magnetometer::~Magnetometer()
{
    delete m_pMag;
    m_pMag = nullptr;
}


void Magnetometer::begin()
{
    uint8_t regBuff[8];
    uint8_t addr[2];

    m_pMag->begin();

    // Set Cycle count to 200
    regBuff[0] = 0x04;
    regBuff[1] = 0x00;
    regBuff[2] = 0xC8;
    regBuff[3] = 0x00;
    regBuff[4] = 0xC8;
    regBuff[5] = 0x00;
    regBuff[6] = 0xC8;
    m_pMag->write( regBuff, 7 );

    // Set continuous measurement mode
    regBuff[0] = 0x00;
    regBuff[1] = 0b1110001;
    m_pMag->write( regBuff, 2 );
}


void Magnetometer::getHeading( int32_t *pX, int32_t *pY, int32_t *pZ )
{
    uint8_t regBuff[9];
    uint8_t addr[2];
    float   head;

    // Poll the magnetometer
    regBuff[0] = 0x00;
    regBuff[1] = 0b0111000;
    m_pMag->write( regBuff, 2 );
    delay( 10 );
    addr[0] = 0xA4;
    m_pMag->write_then_read( addr, 1, regBuff, 9 );

    // Create 32-bit signed integers from the 24-bit return for each axis 
    *pX = ((regBuff[0] << 24) | (regBuff[1] << 16) | (regBuff[2] << 8)) >> 8; 
    *pY = ((regBuff[3] << 24) | (regBuff[4] << 16) | (regBuff[5] << 8)) >> 8;
    *pZ = ((regBuff[6] << 24) | (regBuff[7] << 16) | (regBuff[8] << 8)) >> 8;
/* This is being calculated on the Stratofier side - it allows for implementing calibration, etc. that can't be done with just a raw heading number
    // Calculate the heading from the sensor returns
    head = atan2( *pY, *pX ) * 57.29578;
    head -= 90.0;  // Magnetometer on WingThing is mounted 90 degrees counter-clockwise

    // Convert -180~180 to 0~360
    if( head < 0.0f )
        head += 360.0f;

    return head;
*/
}
