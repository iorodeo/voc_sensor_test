#include <Adafruit_GPS.h>
#include <Adafruit_Arcada.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "bsec.h"

#define GPSECHO false

Adafruit_Arcada arcada;
Arcada_FilesystemType foundFS; 
Adafruit_GPS GPS(&Wire);
Bsec iaqSensor;

enum DisplayType {SYS_DISPLAY, IAQ_DISPLAY, GPS_DISPLAY}; 

const uint8_t TEXT_Y_INIT = 10; 
const uint8_t TEXT_Y_INCR = 18; 
const uint8_t FILE_NUM_DEC = 4;
const uint32_t DISPLAY_DURATION = 5000;
const char OUTPUT_FILE[] = "/data.txt";

void updateSysDispay();
void updateIaqDisplay();
void updateGpsDisplay();
void updateLogFile();

float convert_C_to_F(float tmp_C);
float convert_Pa_to_Hg(float p);

void setup() { 

    Serial.begin(115200);
    Wire.begin();


    // Setup IAQ Sensor
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

    // Setup GPS sensor
    GPS.begin(0x10);  
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); 

    // Setup Arcada and Display
    if (!arcada.arcadaBegin()) { 
        while (1); 
    } 
    arcada.displayBegin(); 
    arcada.setBacklight(200);
    arcada.display->fillScreen(ARCADA_BLACK); 
    arcada.display->setTextSize(1);
    arcada.display->setFont(&FreeMono9pt7b);

    // Check for file system
    foundFS = arcada.filesysBegin(ARCADA_FILESYS_SD);

}

void loop() {

    static uint32_t lastTimer = millis();
    static DisplayType currentDisplay = SYS_DISPLAY;

    // Read data from IAQ and GPS sensors
    bool newIaqData = iaqSensor.run();
    GPS.read();
    if (GPS.newNMEAreceived()) { 
        GPS.parse(GPS.lastNMEA()); 
    }

    uint32_t timer = millis();
    if (timer - lastTimer > DISPLAY_DURATION) {
        switch (currentDisplay) {
            case SYS_DISPLAY:
                updateSysDisplay();
                currentDisplay = IAQ_DISPLAY;
                break;
            case IAQ_DISPLAY:
                updateIaqDisplay();
                currentDisplay = GPS_DISPLAY;
                break;
            case GPS_DISPLAY:
                updateGpsDisplay();
                currentDisplay = SYS_DISPLAY;
                break;
            default:
                break;
        }
        lastTimer = timer;
    }

    if (foundFS && newIaqData) {
        updateLogFile();
    }
}

void updateSysDisplay() {
    uint8_t text_y_pos = TEXT_Y_INIT;

    arcada.display->fillRect(0, 0, 160, 128, ARCADA_BLACK);
    arcada.display->setTextColor(ARCADA_GREENYELLOW);

    arcada.display->setCursor(0, text_y_pos);
    arcada.display->print("SYSTEM INFO"); 

    float vbat = arcada.readBatterySensor();
    text_y_pos += 2*TEXT_Y_INCR;
    arcada.display->setCursor(0, text_y_pos);
    arcada.display->print("VBAT "); 
    arcada.display->print(vbat,1);
    arcada.display->print("V");

    text_y_pos += TEXT_Y_INCR;
    arcada.display->setCursor(0, text_y_pos);
    arcada.display->print("FILE "); 
    if (foundFS) {
        arcada.display->print("Y");
    }
    else {
        arcada.display->print("N");
    }

}

void updateIaqDisplay() { 

    uint8_t text_y_pos = TEXT_Y_INIT;

    arcada.display->fillRect(0, 0, 160, 128, ARCADA_BLACK);
    arcada.display->setTextColor(ARCADA_GREENYELLOW);
    
    arcada.display->setCursor(0, text_y_pos);
    arcada.display->print("IAQ  "); 
    arcada.display->print(iaqSensor.iaq,0);
    
    text_y_pos += TEXT_Y_INCR;
    arcada.display->setCursor(0, text_y_pos);
    arcada.display->print("SIAQ "); 
    arcada.display->print(iaqSensor.staticIaq,0);
    
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
    
}

void updateGpsDisplay() {
    uint8_t text_y_pos = TEXT_Y_INIT;

    arcada.display->fillRect(0, 0, 160, 128, ARCADA_BLACK);
    arcada.display->setTextColor(ARCADA_GREENYELLOW);
    
    arcada.display->setCursor(0, text_y_pos); 
    arcada.display->print("Date ");
    arcada.display->print(GPS.day, DEC); 
    arcada.display->print('/');
    arcada.display->print(GPS.month, DEC); 
    arcada.display->print("/20");
    arcada.display->print(GPS.year, DEC);
    
    text_y_pos += TEXT_Y_INCR;
    arcada.display->setCursor(0, text_y_pos); 
    arcada.display->print("Time "); 
    if (GPS.hour < 10) { 
        arcada.display->print('0'); 
    } 
    arcada.display->print(GPS.hour, DEC); 
    arcada.display->print(':');
    if (GPS.minute < 10) { 
        arcada.display->print('0'); 
    }
    arcada.display->print(GPS.minute, DEC); 
    arcada.display->print(':');
    if (GPS.seconds < 10) { 
        arcada.display->print('0'); 
    }
    arcada.display->print(GPS.seconds, DEC); 
    
    text_y_pos += TEXT_Y_INCR;
    arcada.display->setCursor(0, text_y_pos); 
    arcada.display->print("Fix  "); 
    arcada.display->print((int)GPS.fix);
    arcada.display->print(" Q"); 
    arcada.display->print((int)GPS.fixquality);
    if (GPS.fix) {
        arcada.display->print(" S"); 
        arcada.display->print((int)GPS.satellites);
    }

    text_y_pos += TEXT_Y_INCR;
    arcada.display->setCursor(0, text_y_pos); 
    arcada.display->print("LAT  "); 

    text_y_pos += TEXT_Y_INCR;
    arcada.display->setCursor(0, text_y_pos); 
    if (GPS.fix) {
        arcada.display->print(" "); 
        arcada.display->print(GPS.latitude, 4); 
        arcada.display->print(GPS.lat);
    }
    else {
        arcada.display->print("NA"); 
    }

    text_y_pos += TEXT_Y_INCR;
    arcada.display->setCursor(0, text_y_pos); 
    arcada.display->print("LON  "); 
    
    text_y_pos += TEXT_Y_INCR;
    arcada.display->setCursor(0, text_y_pos); 
    if (GPS.fix) {
        arcada.display->print(" "); 
        arcada.display->print(GPS.longitude, 4); 
        arcada.display->print(GPS.lon);
    }
    else {
        arcada.display->print("NA"); 
    }
}

void updateLogFile() { 
    bool fileExists = arcada.exists(OUTPUT_FILE);
    File f = arcada.open(OUTPUT_FILE, O_WRITE | O_CREAT | O_AT_END); 

    if (!fileExists) {
        // This is the first write so add header
        f.print("datetime, ");
        f.print("lat, ");
        f.print("lon, ");
        f.print("vbat, ");
        f.print("iaq, ");
        f.print("siaq, ");
        f.print("temp (F), ");
        f.print("press (Hg), ");
        f.print("humid (%), ");
        f.println();
    }

    // Add datetime
    f.print(GPS.day, DEC); 
    f.print('/');
    f.print(GPS.month, DEC); 
    f.print("/20");
    f.print(GPS.year, DEC);
    f.print("_");
    if (GPS.hour < 10) { 
        f.print('0'); 
    } 
    f.print(GPS.hour, DEC); 
    f.print(':');
    if (GPS.minute < 10) { 
        f.print('0'); 
    }
    f.print(GPS.minute, DEC); 
    f.print(':');
    if (GPS.seconds < 10) { 
        f.print('0'); 
    }
    f.print(GPS.seconds, DEC); 
    f.print(", ");

    // Add latitude
    if (GPS.fix) {
        f.print(" "); 
        f.print(GPS.latitude, 4); 
        f.print(GPS.lat);
    }
    else {
        f.print("NA"); 
    }
    f.print(", ");

    // Add longitude
    if (GPS.fix) {
        f.print(" "); 
        f.print(GPS.longitude, 4); 
        f.print(GPS.lon);
    }
    else {
        f.print("NA"); 
    }
    f.print(", ");

    // Add battery voltage
    float vbat = arcada.readBatterySensor();
    f.print(vbat,FILE_NUM_DEC);
    f.print(", ");
    
    // Add IAQ value
    f.print(iaqSensor.iaq,FILE_NUM_DEC);
    f.print(", ");

    // Add Static IAQ value
    f.print(iaqSensor.staticIaq,FILE_NUM_DEC);
    f.print(", ");

    // Temp (F)
    f.print(convert_C_to_F(iaqSensor.temperature),FILE_NUM_DEC);
    f.print(", "); 

    // Pressure (in Hg)
    f.print(convert_Pa_to_Hg(iaqSensor.pressure),FILE_NUM_DEC);
    f.print(", "); 

    // Humidity (%)
    f.print(iaqSensor.humidity,FILE_NUM_DEC);

    f.println();

    f.close();
}


float convert_C_to_F(float tmp_C) {
    float tmp_F = tmp_C*(9.0/5.0) + 32.0;
    return tmp_F;
}

float convert_Pa_to_Hg(float p) {
    return 0.0002953*p;
}
