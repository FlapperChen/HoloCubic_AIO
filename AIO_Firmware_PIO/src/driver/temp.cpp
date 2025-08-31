/**************************************************************************
   Tests the getTemperature and getHumidity functions of the Qwiic Humidity
   library

   Priyanka Makin @ SparkFun Electronics
   Original Creation Date: March 31, 2020

   SparkFun labored with love to create this code. Feel like supporting open
   source hardware? Buy a board from SparkFun! https://www.sparkfun.com/products/16618

   This code is lemonadeware; if you see me (or any other SparkFun employee)
   at the local, and you've found our code helpful, please buy us a round!

   Hardware Connections:
   Attach a RedBoard to computer using micro-B USB cable.
   Attach a Qwiic Humidity board to RedBoard using Qwiic cable.

   Distributed as-is; no warranty is given.
 **************************************************************************/
#include <Wire.h>
#include "common.h"
#include "temp.h"

TEMP::TEMP()
{
    this->ready = false;
    this->temperature = 0.0;
    this->humidity = 0.0;
}

void TEMP::init()
{
    Serial.println("Qwiic Humidity AHT20");

    Wire.begin(TEMP_I2C_SDA, TEMP_I2C_SCL); //Join I2C bus
    Serial.printf("I2C initialized with SDA=%d, SCL=%d\n", TEMP_I2C_SDA, TEMP_I2C_SCL);
    
    delay(100); // 等待I2C稳定

    //Check if the AHT20 will acknowledge
    if (false == tempSensor.begin())
    {
        Serial.println("AHT20 not detected. Please check wiring. Freezing.");
        return;
    }
    Serial.println("AHT20 acknowledged.");
    
    // 等待传感器完全准备好
    delay(500);
    this->ready = true;
    return;
}

//The AHT20 can respond with a reading every ~50ms. However, increased read time can cause the IC to heat around 1.0C above ambient.
//The datasheet recommends reading every 2 seconds.
int TEMP::getTemp()
{
    if (!this->ready)
    {
        Serial.println("AHT20 not ready. Please init first.");
        this->init();
        if (!this->ready)
            return -1;
    }
    
    //If a new measurement is available
    if (tempSensor.available() == true)
    {
        //Get the new temperature and humidity value
        temperature = tempSensor.getTemperature();
        humidity = tempSensor.getHumidity();

        //Print the results
        Serial.print("Temperature: ");
        Serial.print(temperature, 2);
        Serial.print(" C\t");
        Serial.print("Humidity: ");
        Serial.print(humidity, 2);
        Serial.print("% RH");

        Serial.println();

        return 0;
    }
    else
    {
        Serial.println("No new measurement available from AHT20");
        return -1;
    }
}
