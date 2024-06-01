#include <SoftwareSerial.h>

SoftwareSerial espSerial(2, 3); // RX, TX

String apiKey = "5O3SSU8LTSX5RM7S";  //Change this key to your "Write API key"

int gas_sensor = A0; //Sensor pin 
float m = -0.3376; //Slope 
float b = 0.7165; //Y-Intercept 
float R0 = 2.82; //Sensor Resistance in fresh air from previous code

int CO_sensor = A1; //Sensor pin 
float m1 = -0.6527; //Slope 
float b1 = 1.30; //Y-Intercept 
float R01 = 7.22; //Sensor Resistance


void setup() {
  Serial.begin(9600);      // PC to Arduino Serial Monitor
  espSerial.begin(115200); // Arduino to ESP01 Communication
  pinMode(gas_sensor, INPUT);
  pinMode(CO_sensor, INPUT);
}

void loop() { 
  // Gas sensor calculations
  float sensor_volt; // Define variable for sensor voltage 
  float RS_gas; // Define variable for sensor resistance  
  float ratio; // Define variable for ratio
  float sensorValue = analogRead(gas_sensor); // Read analog values of gas sensor  
  sensor_volt = sensorValue * (5.0 / 1023.0); // Convert analog values to voltage 
  RS_gas = ((5.0 * 10.0) / sensor_volt) - 10.0; // Get value of RS in a gas
  ratio = RS_gas / R0;  // Get ratio RS_gas/RS_air
  double ppm_log = (log10(ratio) - b) / m; // Get ppm value in linear scale according to the ratio value  
  double ppm = pow(10, ppm_log); // Convert ppm value to log scale 

  // CO sensor calculations
  float sensor_volt1; // Define variable for sensor voltage 
  float RS_gas1; // Define variable for sensor resistance  
  float ratio1; // Define variable for ratio
  float sensorValue1 = analogRead(CO_sensor); // Read analog values of CO sensor  
  sensor_volt1 = sensorValue1 * (5.0 / 1023.0); // Convert analog values to voltage 
  RS_gas1 = ((5.0 * 10.0) / sensor_volt1) - 10.0; // Get value of RS in a gas
  ratio1 = RS_gas1 / R01;  // Get ratio RS_gas/RS_air
  double ppm_log1 = (log10(ratio1) - b1) / m1; // Get ppm value in linear scale according to the ratio value  
  double ppm1 = pow(10, ppm_log1); // Convert ppm value to log scale 

  // ESP01 communication
  espSerial.println("AT+CIPMUX=0\r\n"); // To Set MUX = 0
  delay(2000); // Wait for 2 sec

  // TCP connection 
  String cmd = "AT+CIPSTART=\"TCP\",\""; // TCP connection with https://thingspeak.com server
  cmd += "184.106.153.149"; // IP addr of api.thingspeak.com
  cmd += "\",80\r\n\r\n"; // Port No. = 80

  espSerial.println(cmd); // Display above Command on PC
  Serial.println(cmd); // Send above command to ESP01

  delay(1000);                         

  if (espSerial.find("ERROR")) { // If returns error in TCP connection
    Serial.println("AT+CIPSTART error"); // Display error msg to PC
    //return; 
  }

  // Prepare GET string 
  String getStr = "GET /update?api_key=";   
  getStr += apiKey;
  getStr +="&field1=";
  getStr += ppm;
  getStr +="&field2=";
  getStr += ppm1; 
  getStr += "\r\n\r\n"; 

  Serial.println(getStr); // Display GET String on PC

  cmd = "AT+CIPSEND="; // Send data length 
  cmd += String(getStr.length());
  cmd += "\r\n";

  Serial.println(cmd); // Display Data length on PC
  espSerial.println(cmd); // Send Data length command to ESP01

  if (espSerial.find(">")) { // If prompt opens //verify connection with cloud
    Serial.println("Pushed whole data TO CLOUD"); // Display confirmation msg to PC
    espSerial.print(getStr); // Send GET String to ESP01
  }
  else { 
    espSerial.println("AT+CIPCLOSE\r\n"); // Send Close Connection command to ESP01
    Serial.println("AT+CIPCLOSE"); // Display Connection closed command on PC
  } 

  // thingspeak free version needs 15-20 sec delay between every push
  delay(15000); // wait for 16sec
}
