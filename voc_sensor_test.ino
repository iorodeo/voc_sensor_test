#include <Adafruit_Arcada.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "bsec.h"

Adafruit_Arcada arcada;
Arcada_FilesystemType foundFS; 
Bsec iaqSensor;

const uint8_t TEXT_Y_INIT = 10; 
const uint8_t TEXT_Y_INCR = 18; 
const uint8_t FILE_NUM_DEC = 4;

float convert_C_to_F(float tmp_C);
float convert_Pa_to_Hg(float p);

void setup() { 

    //Serial.begin(115200);
    //while (!Serial) delay(10); // wait for console

    Wire.begin();
    iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
    bsec_virtual_sensor_t sensorList[10] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_IAQ,
        BSEC_OUTPUT_STATIC_IAQ,
        BSEC_OUTPUT_CO2_EQUIVALENT,
        BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    };
    iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);

    if (!arcada.arcadaBegin()) { 
        while (1); 
    } 
    arcada.displayBegin(); 
    arcada.setBacklight(200);
    arcada.display->fillScreen(ARCADA_BLACK); 
    arcada.display->setTextSize(1);
    arcada.display->setFont(&FreeMono9pt7b);

    //arcada.display->setFont(&FreeSerif9pt7b);
    //arcada.display->setFont(&FreeSans9pt7b);
    //arcada.display->setFont(&FreeSansBold9pt7b);
    //arcada.display->setFont(&FreeMonoBold9pt7b);

    foundFS = arcada.filesysBegin(ARCADA_FILESYS_SD);
    //if (foundFS) {
    //    Serial.println(arcada.filesysListFiles("/"));
    //}
}

void loop() {
    uint8_t text_y_pos = TEXT_Y_INIT;
    static uint32_t count = 0;

    if (iaqSensor.run()) { // If new data is available

        arcada.display->fillRect(0, 0, 160, 128, ARCADA_BLACK);
        //arcada.display->setTextColor(ARCADA_GREEN);
        arcada.display->setTextColor(ARCADA_GREENYELLOW);

        //arcada.display->setCursor(0, text_y_pos);
        //arcada.display->print("CNT  "); 
        //arcada.display->print(count);

        float vbat = arcada.readBatterySensor();
        arcada.display->setCursor(0, text_y_pos);
        arcada.display->print("BAT  "); 
        arcada.display->print(vbat,1);
        arcada.display->print("V");

        text_y_pos += TEXT_Y_INCR;
        arcada.display->setCursor(0, text_y_pos);
        arcada.display->print("IAQ  "); 
        arcada.display->print(iaqSensor.iaq,0);

        text_y_pos += TEXT_Y_INCR;
        arcada.display->setCursor(0, text_y_pos);
        arcada.display->print("SIAQ "); 
        arcada.display->print(iaqSensor.staticIaq,0);

        //text_y_pos += TEXT_Y_INCR;
        //arcada.display->setCursor(0, text_y_pos);
        //arcada.display->print("GASR "); 
        //arcada.display->print(1.0e-6*iaqSensor.gasResistance,1);
        //arcada.display->print(" MOhm"); 

        text_y_pos += TEXT_Y_INCR;
        arcada.display->setCursor(0, text_y_pos);
        arcada.display->print("TEMP "); 
        arcada.display->print(convert_C_to_F(iaqSensor.temperature),0);
        arcada.display->print(" F"); 

        text_y_pos += TEXT_Y_INCR;
        arcada.display->setCursor(0, text_y_pos);
        arcada.display->print("PRES "); 
        arcada.display->print(convert_Pa_to_Hg(iaqSensor.pressure),1);
        arcada.display->print(" Hg"); 

        text_y_pos += TEXT_Y_INCR;
        arcada.display->setCursor(0, text_y_pos);
        arcada.display->print("HUMI "); 
        arcada.display->print(iaqSensor.humidity,0);
        arcada.display->print(" %"); 

        text_y_pos += TEXT_Y_INCR;
        arcada.display->setCursor(0, text_y_pos);
        arcada.display->print("FILE "); 
        if (foundFS) {
           arcada.display->print("Y");
        }
        else {
            arcada.display->print("N");
        }

        if (foundFS) {
            File f = arcada.open("/data.txt", O_WRITE | O_CREAT | O_AT_END); 

            f.print(count);
            f.print(", ");

            f.print(vbat,FILE_NUM_DEC);
            f.print(", ");
            
            f.print(iaqSensor.iaq,FILE_NUM_DEC);
            f.print(", ");

            f.print(iaqSensor.staticIaq,FILE_NUM_DEC);
            f.print(", ");

            f.print(convert_C_to_F(iaqSensor.temperature),FILE_NUM_DEC);
            f.print(", "); 

            f.print(convert_Pa_to_Hg(iaqSensor.pressure),FILE_NUM_DEC);
            f.print(", "); 

            f.print(iaqSensor.humidity,FILE_NUM_DEC);

            f.println();

            f.close();
        }
        count += 1;

    }
}


float convert_C_to_F(float tmp_C) {
    float tmp_F = tmp_C*(9.0/5.0) + 32.0;
    return tmp_F;
}

float convert_Pa_to_Hg(float p) {
    return 0.0002953*p;
}
