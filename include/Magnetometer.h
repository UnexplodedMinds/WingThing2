class Adafruit_SPIDevice;


class Magnetometer
{
public:
    Magnetometer();
    ~Magnetometer();

    void  begin();
    void getHeading( int32_t *pX, int32_t *pY, int32_t *pZ );

private:
    Adafruit_SPIDevice *m_pMag;
};
