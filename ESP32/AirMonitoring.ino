#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DFRobot_OzoneSensor.h>

#define LED                     4
#define BUTTON                  35
#define COLLECT_NUMBER          20

#define BTN_UNPRESSED           0
#define BTN_DEBOUNCE            1
#define BTN_PRESSED             2

#define SERVICE_UUID            "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX  "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX  "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLEServer          *g_pServer = NULL;
BLECharacteristic  *g_pTxCharacteristic;
bool                g_deviceConnected = false;
DFRobot_OzoneSensor g_OzoneSensor;
unsigned char co2dataReq[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
char col;
unsigned int PMSa = 0,FMHDSa = 0,TPSa = 0,HDSa = 0,PMSb = 0,FMHDSb = 0,TPSb = 0,HDSb = 0;
unsigned int PMS = 0,FMHDS = 0,TPS = 0,HDS = 0,CR1 = 0,CR2 = 0;
unsigned char buffer_RTT[40]={};   //Serial buffer; Received Data
unsigned long       g_ElapsedTimeO3 = 0;
unsigned long       g_ElapsedTimeCO2 = 0;
unsigned long       g_ElapsedTimeSendData = 0;
unsigned long       g_ElapsedButton = 0;
int16_t             g_O3 = 0;
long                g_CO2 = 0;
byte                g_OutputBuffer[15];
float               g_Temperature = 0.0;
float               g_Humidity = 0.0;
bool                g_isLoggerActive = false;
byte                g_btnStatus = BTN_UNPRESSED;
unsigned int        g_LedCounter = 0;
String              dataMessage;

class MyServerCallbacks: public BLEServerCallbacks 
{
    void onConnect(BLEServer* g_pServer) 
    {
      g_deviceConnected = true;
    };

    void onDisconnect(BLEServer* g_pServer) 
    {
      g_deviceConnected = false;
    }
};

void setup() 
{
    Serial.begin(9600);  // PM2.5, Formaldeyde, Temperature and Humidity Sensor
    Serial2.begin(9600); // CO2 Sensor
    delay(500);

    pinMode(LED, OUTPUT);
    pinMode(BUTTON, INPUT);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // SD CARD Configuration
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!SD.begin())
        Serial.println("Card Mount Failed");
    else
    {
        uint8_t cardType = SD.cardType();
        
        if(cardType == CARD_NONE)
            Serial.println("No SD card attached");
            
        Serial.print("SD Card Type: ");
        if(cardType == CARD_MMC)
            Serial.println("MMC");
        else if(cardType == CARD_SD)
            Serial.println("SDSC");
        else if(cardType == CARD_SDHC)
            Serial.println("SDHC");
        else 
            Serial.println("UNKNOWN");
    
        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("SD Card Size: %lluMB\n", cardSize);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Ozone Sensor
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int count = 0;
    while(!g_OzoneSensor.begin(ADDRESS_3) && count++ < 3) 
    {
        Serial.println("Ozone Connection Error !");
        delay(1000);
    }  
    Serial.println("Ozone Sensor Connected !");
    g_OzoneSensor.SetModes(MEASURE_MODE_AUTOMATIC);
    delay(500);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Bluetooth Low Energy Initialization
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BLEDevice::init("AIR MONITOR"); // BLE Device Name
    
    // Create the BLE Server
    g_pServer = BLEDevice::createServer();
    g_pServer->setCallbacks(new MyServerCallbacks());
    
    // Create the BLE Service
    BLEService *pService = g_pServer->createService(SERVICE_UUID);
    
    // Create a BLE Characteristic
    g_pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);            
    g_pTxCharacteristic->addDescriptor(new BLE2902());
    
    // Start the service
    pService->start();
    
    // Start advertising
    g_pServer->getAdvertising()->start();
    Serial.println("Waiting a Bluetooth connection...");

    digitalWrite(LED, LOW);
}

void loop() 
{
    ////////////////////////////////////////////////////////////////
    // Button and led Management
    ////////////////////////////////////////////////////////////////
    if(millis() > g_ElapsedButton + 10)
    {
        g_ElapsedButton = millis();
        switch(g_btnStatus)
        {
            case BTN_UNPRESSED:
            {
                if(digitalRead(BUTTON) == LOW)                       // Button is Pressed
                {
                    g_btnStatus = BTN_DEBOUNCE;
                }
            } break;

            case BTN_DEBOUNCE:
            {
                if(digitalRead(BUTTON) == LOW)                       // Button is still Pressed
                {
                    g_btnStatus = BTN_PRESSED;
                    if(g_isLoggerActive == false)
                    {
                        File file = SD.open("/data.txt");
                        if(!file) 
                        {
                            Serial.println("File doens't exist");  
                        } else
                        {
                            Serial.println("Remove Existing file");
                            SD.remove("/data.txt");
                        }
                        
                        Serial.println("Creating file...");
                        writeFile(SD, "/data.txt", "Temperature [°C], Humidity [%], PM2.5 [ug/m3], Formaldehyde [ug/m3], O3 [ppb], CO2 [ppm] \r\n");
                        Serial.println("START LOGGING!!!");
                        g_isLoggerActive = true;
                    } else                                          // Button is still Released
                    {
                        g_isLoggerActive = false;
                        Serial.println("STOP LOGGING!!!");
                        digitalWrite(LED, LOW);
                        g_LedCounter = 0;
                    }
                    
                } else
                    g_btnStatus = BTN_UNPRESSED;
            } break;

            case BTN_PRESSED:
            {
                if(digitalRead(BUTTON) == HIGH)                       // Button is Released
                    g_btnStatus = BTN_DEBOUNCE;
            } break;
        }

        /// LED BLINK
        if(g_isLoggerActive == true)
        {
            if(++g_LedCounter >= 50)
            {
                g_LedCounter = 0;
                digitalWrite(LED, !digitalRead(LED));   // Toggle Led            
            }
        }
    }    

    ////////////////////////////////////////////////////////////////
    // Ozone Sensor
    ////////////////////////////////////////////////////////////////
    if(millis() > g_ElapsedTimeO3 + 3000)
    {
        g_ElapsedTimeO3 = millis();
        g_O3 = g_OzoneSensor.ReadOzoneData(COLLECT_NUMBER);
    }


    ////////////////////////////////////////////////////////////////
    // CO2 Sensor
    ////////////////////////////////////////////////////////////////
    if(millis() > g_ElapsedTimeCO2 + 1000)
    {
        g_ElapsedTimeCO2 = millis();
        Serial2.flush();
        Serial2.write(co2dataReq, 9);
        
    }

    if(millis() > g_ElapsedTimeCO2 + 300)
    {
        for(int i = 0, j = 0; i < 9; i++)
        {
            if (Serial2.available() > 0)
            {
                long hi, lo;
                int ch = Serial2.read();
                
                if(i == 2)     
                    hi = ch;   // High byte concentration
                
                if(i == 3)
                    lo = ch;   // Low byte concentration
    
                if(i == 8) 
                {
                    g_CO2 = (hi * 256) + lo;  //CO2 concentration
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////
    // Temperature Humidity Formaldeyde PM2.5 Sensor
    ////////////////////////////////////////////////////////////////
    while(Serial.available() > 0)   //Data check: weather there is any Data in Serial
    {
        for(int i = 0; i < 40; i++)
        {
            col = Serial.read();
            buffer_RTT[i] = (char)col;
            delay(2);
        }

        Serial.flush();

        CR1 =(buffer_RTT[38] << 8) + buffer_RTT[39];
        CR2 = 0;
        for(int i = 0; i < 38; i++)
            CR2 += buffer_RTT[i];
            
        if(CR1 == CR2)                  // Check
        {
            PMSa=buffer_RTT[12];        // Read PM2.5 High 8-bit
            PMSb=buffer_RTT[13];        // Read PM2.5 Low 8-bit
            PMS=(PMSa<<8)+PMSb;         // PM2.5 value
            FMHDSa=buffer_RTT[28];      // Read Formaldehyde High 8-bit
            FMHDSb=buffer_RTT[29];      // Read Formaldehyde Low 8-bit
            FMHDS=(FMHDSa<<8)+FMHDSb;   // Formaldehyde value
            TPSa=buffer_RTT[30];        // Read Temperature High 8-bit
            TPSb=buffer_RTT[31];        // Read Temperature Low 8-bit
            TPS=(TPSa<<8)+TPSb;         // Temperature value
            HDSa=buffer_RTT[32];        // Read Humidity High 8-bit
            HDSb=buffer_RTT[33];        // Read Humidity Low 8-bit
            HDS=(HDSa<<8)+HDSb;         // Humidity value
        } else
        {
            PMS = 0;
            FMHDS = 0;
            TPS = 0;
            HDS = 0;
        }
    }

    ////////////////////////////////////////////////////////////////
    // Print and Save Measured Data
    ////////////////////////////////////////////////////////////////
    if(millis() > g_ElapsedTimeSendData + 5000)
    {
        g_ElapsedTimeSendData = millis();

        if(g_deviceConnected) 
        {
            byte dataToSend = 0;
            g_OutputBuffer[dataToSend++] = (TPS & 0xFF00) >> 8;
            g_OutputBuffer[dataToSend++] = (TPS & 0x00FF);
            g_OutputBuffer[dataToSend++] = (HDS & 0xFF00) >> 8;
            g_OutputBuffer[dataToSend++] = (HDS & 0x00FF);
            g_OutputBuffer[dataToSend++] = (PMS & 0xFF00) >> 8;
            g_OutputBuffer[dataToSend++] = (PMS & 0x00FF);
            g_OutputBuffer[dataToSend++] = (FMHDS & 0xFF00) >> 8;
            g_OutputBuffer[dataToSend++] = (FMHDS & 0x00FF);
            g_OutputBuffer[dataToSend++] = (g_O3 & 0xFF00) >> 8;
            g_OutputBuffer[dataToSend++] = (g_O3 & 0x00FF);
            g_OutputBuffer[dataToSend++] = (g_CO2 & 0xFF000000) >> 24;
            g_OutputBuffer[dataToSend++] = (g_CO2 & 0x00FF0000) >> 16;
            g_OutputBuffer[dataToSend++] = (g_CO2 & 0x0000FF00) >> 8;
            g_OutputBuffer[dataToSend++] = (g_CO2 & 0x000000FF);

            g_pTxCharacteristic->setValue(g_OutputBuffer, dataToSend);
            g_pTxCharacteristic->notify();
        }

        g_Temperature = (float)(TPS) / 10.0;
        g_Humidity = (float)(HDS) / 10.0;

        Serial.println("----------- Data Measured -----------");

        Serial.print("Temperature: ");
        Serial.print(g_Temperature);
        Serial.println(" °C");

        Serial.print("Humidity: ");
        Serial.print(g_Humidity);
        Serial.println(" %");

        Serial.print("PM2.5: ");
        Serial.print(PMS);
        Serial.println(" ug/m3");

        Serial.print("Formaldehyde: ");
        Serial.print(FMHDS);
        Serial.println(" ug/m3");

        Serial.print("Ozone Concentration: ");
        Serial.print(g_O3);
        Serial.println(" ppb");

        Serial.print("CO2 concentration: ");
        Serial.print(g_CO2);
        Serial.println(" ppm");

        Serial.println("  ");

        if(g_isLoggerActive)    // If logger is acrivated
            logSDCard();        // Save data
    }

}

void logSDCard() 
{
    dataMessage = String(g_Temperature) + "," + String(g_Humidity) + "," + String(PMS) + "," + String(FMHDS) + "," + String(g_O3) + "," + String(g_CO2) + "\r\n";
    Serial.print("Save data: ");
    Serial.println(dataMessage);
    appendFile(SD, "/data.txt", dataMessage.c_str());
}

void writeFile(fs::FS &fs, const char * path, const char * message) 
{
    Serial.printf("Writing file: %s\n", path);
    File file = fs.open(path, FILE_WRITE);
    if(!file) 
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)) 
    {
        Serial.println("File written");
    } else 
    {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) 
{
    File file = fs.open(path, FILE_APPEND);
    if(!file) 
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)) 
    {
        Serial.println("Message appended");
    } else 
    {
        Serial.println("Append failed");
    }
    file.close();
}
