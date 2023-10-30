// Includes
#include "SCD30.h"
#include "TFT_eSPI.h"
#include "SparkFunBQ27441.h"

// Defines
#define max_size 50 //maximum size of data
#define SIZE_LCD_W 320
#define SIZE_LCD_H 240
#define SIZE_BOX_X 150
#define SIZE_BOX_Y 100
#define STRING_X_OFFSET 5
#define STRING_Y_OFFSET 5
#define LCD_BACKLIGHT (72Ul) // Control Pin of LCD

// Global variables
TFT_eSPI tft;

float concentrationOxygen;
float concentrationCO2;
float temperature;
float humidity;
float result[3];
int stateOfBattery;
const unsigned int BATTERY_CAPACITY = 650; // Set Wio Terminal Battery's Capacity 

void setupBQ27441(void)
{
  // Use lipo.begin() to initialize the BQ27441-G1A and confirm that it's
  // connected and communicating.
  lipo.begin(); // begin() will return true if communication is successful

  // Uset lipo.setCapacity(BATTERY_CAPACITY) to set the design capacity
  // of your battery.
  lipo.setCapacity(BATTERY_CAPACITY);
}

void setup() {
    // Serial output Init
    SerialUSB.begin(115200);

    // Use button to control display
    pinMode(WIO_KEY_A, INPUT_PULLUP);
 
    // LCD Init
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    
    tft.drawRect(5,5,SIZE_BOX_X,SIZE_BOX_Y,TFT_WHITE);
    tft.drawString("Temperature [C]", 5+STRING_X_OFFSET, 5+STRING_Y_OFFSET); //prints strings from (0, 0)
    
    tft.drawRect((SIZE_LCD_W/2)+5,5,SIZE_BOX_X,SIZE_BOX_Y,TFT_WHITE);
    tft.drawString("Humidity: [%]", (SIZE_LCD_W/2)+5+STRING_X_OFFSET,5+STRING_Y_OFFSET); //prints strings from (0, 0)
    
    tft.drawRect(5,(SIZE_LCD_H/2)+5,SIZE_BOX_X,SIZE_BOX_Y,TFT_WHITE);
    tft.drawString("CO2: [ppm]", 5+STRING_X_OFFSET,(SIZE_LCD_H/2)+5+STRING_Y_OFFSET); //prints strings from (0, 0)
    
    tft.drawRect((SIZE_LCD_W/2)+5,(SIZE_LCD_H/2)+5,SIZE_BOX_X,SIZE_BOX_Y,TFT_WHITE);
    tft.drawString("O2: [%]", (SIZE_LCD_W/2)+5+STRING_X_OFFSET,(SIZE_LCD_H/2)+5+STRING_Y_OFFSET); //prints strings from (0, 0)

    // CO2, Humidity, Temperature sensor Init
    Wire.begin();
    scd30.initialize();
    concentrationCO2 = 0;
    temperature = 0;
    humidity = 0;
    result[3] = {0};

    // O2 sensor Init
    Serial1.begin(9600);
    concentrationOxygen = 0;

    // Battery init
    setupBQ27441();

    tft.setTextColor(TFT_WHITE);       
    tft.setTextSize(1); 
    tft.drawString("Battery [%]: ", 130, 230); 

    digitalWrite(LCD_BACKLIGHT, LOW);
}

void loop() {
    if (scd30.isAvailable()) {
        scd30.getCarbonDioxideConcentration(result);

        SerialUSB.print("CO2 Concentration: [");
        SerialUSB.print(result[0]);
        SerialUSB.print("] ppm - Temperature: [");
        SerialUSB.print(result[1]);
        SerialUSB.print("] ℃ - Humidity: [");
        SerialUSB.print(result[2]);
        SerialUSB.print("] % - ");
    }

    stateOfBattery = lipo.soc();

    concentrationCO2 = result[0];
    temperature = result[1];
    humidity = result[2];
    concentrationOxygen = getOxygen();

    SerialUSB.print(" Battery: ");
    SerialUSB.print(stateOfBattery);

    // LCD update 
    if (digitalRead(WIO_KEY_A) == LOW) {
      digitalWrite(LCD_BACKLIGHT, HIGH);
      
      updateLCD(temperature, humidity, concentrationCO2, concentrationOxygen, stateOfBattery);
    }
    else {
      digitalWrite(LCD_BACKLIGHT, LOW);
    }
                  
    delay(2000);
}

float getOxygen() {
    static float O2_val;
  
    if (Serial1.available()) {
      uint8_t begin_code = Serial1.read();
      delay(10);
      uint8_t state_code = Serial1.read();
      delay(10);
      uint8_t high_code = Serial1.read();
      delay(10);
      uint8_t low_code = Serial1.read();
      delay(10);
      uint8_t check_code = Serial1.read();
      delay(10);
      uint8_t checkk_code = Serial1.read();
      delay(10);
      uint8_t checkkk_code = Serial1.read();
      delay(10);
      uint8_t checkkkk_code = Serial1.read();
      delay(10);
      uint8_t checkkkkk_code = Serial1.read();
      delay(10);
   
      if(begin_code == 255 && state_code == 134){
   
        O2_val = ((high_code * 256) + low_code) * 0.1 ;
        SerialUSB.print("O2: ");
        SerialUSB.print(O2_val);
        SerialUSB.println(" % ");
      }
      else {
        SerialUSB.println("[ERROR] - O2 Data corrupted");
      }
    }
    else {
      SerialUSB.println("[ERROR] - O2 No Serial");
    }
    
    while(Serial1.read()>=0);    //clear buffer  

    return O2_val;
}

void updateLCD(float temp, float rh, float co2, float o2, int battery)  {
    int co2_int = (int)co2;
    String s_concentrationCO2 = String(co2_int);
    String s_temperature = String(temp, 2);
    String s_humidity = String(rh, 2);
    String s_concentrationOxygen = String(o2, 2);
    String s_battery = String(battery);

    // Temperature
    tft.setTextColor(TFT_BLACK);    
    tft.drawString("℃℃℃℃℃", 5+STRING_X_OFFSET+15, 5+STRING_Y_OFFSET+35); 
    tft.setTextColor(TFT_WHITE);       
    tft.setTextSize(4); 
    tft.drawString(s_temperature, 5+STRING_X_OFFSET+15, 5+STRING_Y_OFFSET+35); 

    // Humidity    
    tft.setTextColor(TFT_BLACK);    
    tft.drawString("℃℃℃℃℃", (SIZE_LCD_W/2)+5+STRING_X_OFFSET+15, 5+STRING_Y_OFFSET+35); 
    if(rh < 30.0 | rh > 50.0) {
      tft.setTextColor(TFT_CYAN);
    }
    else {
      tft.setTextColor(TFT_GREEN);
    }
    tft.setTextSize(4); 
    tft.drawString(s_humidity, (SIZE_LCD_W/2)+5+STRING_X_OFFSET+15, 5+STRING_Y_OFFSET+35);
           
    // CO2
    tft.setTextColor(TFT_BLACK);    
    tft.drawString("℃℃℃℃℃", 5+STRING_X_OFFSET+15, (SIZE_LCD_H/2)+5+STRING_Y_OFFSET+35); 
    if(co2 > 500.00) {
      tft.setTextColor(TFT_ORANGE);
    }
    else if(co2 > 750.00) {
      tft.setTextColor(TFT_CYAN);
    } 
    else {
      tft.setTextColor(TFT_GREEN);     
    }
    tft.setTextSize(4); 
    tft.drawString(s_concentrationCO2, 5+STRING_X_OFFSET+15, (SIZE_LCD_H/2)+5+STRING_Y_OFFSET+35); 

    // O2
    tft.setTextColor(TFT_BLACK);    
    tft.drawString("℃℃℃℃℃", (SIZE_LCD_W/2)+5+STRING_X_OFFSET+15, (SIZE_LCD_H/2)+5+STRING_Y_OFFSET+35); 
    tft.setTextColor(TFT_WHITE);       
    tft.setTextSize(4); 
    tft.drawString(s_concentrationOxygen, (SIZE_LCD_W/2)+5+STRING_X_OFFSET+15, (SIZE_LCD_H/2)+5+STRING_Y_OFFSET+35);     

    // Battery
    tft.setTextColor(TFT_BLACK);       
    tft.setTextSize(1); 
    tft.drawString("℃℃℃", 230, 230); 
    tft.setTextColor(TFT_WHITE); 
    tft.drawString(s_battery, 230, 230); 

    tft.setTextSize(4);
    
}
