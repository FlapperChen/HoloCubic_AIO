#ifndef TEMP_H
#define TEMP_H

#include <I2Cdev.h>
#include <SparkFun_Qwiic_Humidity_AHT20.h> //Click here to get the library: http://librarymanager/All#Qwiic_Humidity_AHT20 by SparkFun

// struct TempStatus
// {

// };

class TEMP
{
private:
    AHT20 tempSensor;

public:
    bool ready;
    float temperature;
    float humidity;
    
    TEMP();
    void init();
    int getTemp();
};

#endif
