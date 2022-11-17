#include <LiquidCrystal.h>

// Arduino Energy Meter V2.0
// This code is for This code is for Wemos(ESP8266) based Energy monitoring Device
// This code is a modified version of sample code from https://github.com/pieman64/ESPecoMon
// Last updated on 30.05.2018

const int Sensor_Pin = A0;
unsigned int Sensitivity = 185;   // 185mV/A for 5A, 100 mV/A for 20A and 66mV/A for 30A Module
float Vpp = 0; // peak-peak voltage 
float Vrms = 0; // rms voltage
float Irms = 0; // rms current
float Supply_Voltage = 233.0;           // reading from DMM
float Vcc = 5.0;         // ADC reference voltage // voltage at 5V pin 
float power = 0;         // power in watt              
float Wh =0 ;             // Energy in kWh
unsigned long last_time =0;
unsigned long current_time =0;
unsigned long interval = 100;
unsigned int calibration = 100;  // V2 slider calibrates this
unsigned int pF = 85;           // Power Factor default 95
float bill_amount = 0;   // 30 day cost as present energy usage incl approx PF
float kwh = 0; 
unsigned int energyTariff = 0.73; // Energy cost in Eur per unit (kWh)
String sendInfoString = "";  
int period = 14000;
unsigned long time_now = 0;

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void getACS712() {  // for AC
  Vpp = getVPP();
  Vrms = (Vpp/2.0) *0.707; 
  Vrms = Vrms - (calibration / 10000.0);     // calibtrate to zero with slider
  Irms = (Vrms * 1000)/Sensitivity ;
  if((Irms > -0.015) && (Irms < 0.008)){  // remove low end chatter
    Irms = 0.0;
  }
  power= (Supply_Voltage * Irms) * (pF / 100.0); 
  last_time = current_time;
  current_time = millis();    
  Wh = Wh+  power *(( current_time -last_time) /3600000.0) ; // calculating energy in Watt-Hour
  bill_amount = Wh * (energyTariff/1000);
  kwh = Wh / 1000;
  //Serial.print("Irms:  "); 
  //Serial.print(String(Irms, 3));
  //Serial.println(" A");
  //Serial.print("Power: ");   
  //Serial.print(String(power, 3)); 
  //Serial.println(" W"); 
  //Serial.print("  Bill Amount: INR"); 
  //Serial.println(String(bill_amount, 2));

  //LCD
  lcd.clear();
  lcd.print(String(power));
  lcd.print("W");
  lcd.setCursor(0, 1);
  lcd.println((String(bill_amount)));
}

float getVPP()
{
  float result; 
  int readValue;                
  int maxValue = 0;             
  int minValue = 1024;          
  uint32_t start_time = millis();
  while((millis()-start_time) < 950) //read every 0.95 Sec
  {
     readValue = analogRead(Sensor_Pin);    
     if (readValue > maxValue) 
     {         
         maxValue = readValue; 
     }
     if (readValue < minValue) 
     {          
         minValue = readValue;
     }
  } 
   result = ((maxValue - minValue) * Vcc) / 1024.0;
  // Print a message to the LCD.  
   return result;
 }

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
}

void SendingData(){
  //String that needs to be send to ESP32
  if(millis() >= time_now + period){
      sendInfoString = "?kwh=" + (String(kwh, 2)) + "&wattage=" + (String(power, 3));
      Serial.println(sendInfoString);
      time_now = millis();
    }
}

void loop() {
  getACS712();
  SendingData();
}
