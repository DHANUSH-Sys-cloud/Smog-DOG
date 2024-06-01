/*****************************************************************
 * Board: Arduino Uno
 * Developed by: Dhanush V    Email: dhanush.v@cme.christuniversity.in
 * 
 * ESP01 WiFi Module is used to connect to Internet 
 * and send data to https://thingspeak.com server 
 * 
 * Output is shown using GRAPH on thinkspeak server.
 *  
 * NOTE: 
 * Internal 10 bit ADC (A0) is used to read the Analog output of the Temperature Sensor.
********************************************************************/
/****************************************************************** 
 * 
 * PIN Diagram of ESP01: (This module works on 3.3v) (NOT 5v)
 * +---------------------------+
 * | |=| |=| |=|============== |
 * | | | | | | |    ANTENA     |
 * | | |=| |=| | Facing ur side|
 * | ========================= |
 * |                           |
 * | GND   GPIO0   GPIO2   RxD |
 * |  o      o      o       o  |
 * |                           |
 * |  o      o      o       o  |
 * | TxD   CH_PD    RST    Vcc |
 * +---------------------------+
 *
 * Connections for Communication:
 * ESP01   ->  Arduino
 * Vcc     ->  3.3V
 * GND     ->  GND
 * TxD     ->  Rx1 (Pin 19)
 * RxD     ->  Tx1 (Pin 18)
 * CH_PD   ->  3.3V
 * 
 * Serial Communication with PC through USB cable
 *****************************************************************/

 /****************************************************************
 * STEPS:
 * 1. Sign up at https://thingspeak.com
 * 2. Channels > New Channel > Update "Name" and "Field 1", "Field2" so on...-> Save Channel
 * 3. Click On API keys > Copy "Write API key"
 * 4. Make all the Connections to Arduino Mega board mentioned Above.
 * 5. Change Following in below written program.
 *    a. apiKey by above copied "Write API key" (in step 3)
 *    b. NOTE: ESP-01 is connected to wifi hotspot in a different approach using AT commands
 *       AT, AT+CWMODE=1, AT+CWJAP="BKS","sai123456", AT+CWJAP?
 * 6. Upload Program to Arduino Mega Board
 * 7. Open Arduino Serial Monitor on PC (Set Baud Rate to 9600 and set "Both NL & CR"
 * 8. Go to https://thingspeak.com and login  
 *    Channels > My Channels > Click on Channel Name (created in step 2) > Private View
 *    Here you can observe the Grapth of Temperature Vs Day.
 ****************************************************************/



String apiKey = "001A32KQE6LCMXA7";  //Change this key to your "Write API key"

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
  Serial1.begin(115200);   // Arduino to ESP01 Communication
  pinMode(gas_sensor, INPUT);
  pinMode(CO_sensor,INPUT);
  //pinMode(A2,INPUT); //For DHT Sensor
 } 

void loop() { 
  // put your main code here, to run repeatedly:
  float sensor_volt; //Define variable for sensor voltage 
  float RS_gas; //Define variable for sensor resistance  
  float ratio; //Define variable for ratio
  float sensorValue = analogRead(gas_sensor); //Read analog values of sensor  
  sensor_volt = sensorValue*(5.0/1023.0); //Convert analog values to voltage 
    RS_gas = ((5.0*10.0)/sensor_volt)-10.0; //Get value of RS in a gas
  ratio = RS_gas/R0;  // Get ratio RS_gas/RS_air
  double ppm_log = (log10(ratio)-b)/m; //Get ppm value in linear scale according to the the ratio value  
  double ppm = pow(10, ppm_log); //Convert ppm value to log scale 
  Serial.print("Our desired PPM = ");
  Serial.println(ppm);
  

  float sensor_volt1; //Define variable for sensor voltage 
  float RS_gas1; //Define variable for sensor resistance  
  float ratio1; //Define variable for ratio
  float sensorValue1 = analogRead(CO_sensor); //Read analog values of sensor  
  sensor_volt1 = sensorValue1*(5.0/1023.0); //Convert analog values to voltage 
  RS_gas1 = ((5.0*10.0)/sensor_volt1)-10.0; //Get value of RS in a gas
  ratio1 = RS_gas1/R01;  // Get ratio RS_gas/RS_air
  double ppm_log1 = (log10(ratio1)-b1)/m1; //Get ppm value in linear scale according to the the ratio value  
  double ppm1 = pow(10, ppm_log1); //Convert ppm value to log scale 
  Serial.print("CO PPM = ");
  Serial.println(ppm1);


  Serial1.println("AT+CIPMUX=0\r\n");      // To Set MUX = 0
  delay(2000);                             // Wait for 2 sec

  // TCP connection 
  String cmd = "AT+CIPSTART=\"TCP\",\"";   // TCP connection with https://thingspeak.com server
  cmd += "184.106.153.149";                // IP addr of api.thingspeak.com
  cmd += "\",80\r\n\r\n";                  // Port No. = 80

  Serial1.println(cmd);                    // Display above Command on PC
  Serial.println(cmd);                     // Send above command to Rx1, Tx1

  delay(1000);                         

  if(Serial1.find("ERROR"))                // If returns error in TCP connection
  { 
    Serial.println("AT+CIPSTART error");   // Display error msg to PC
    //return; 
  }

  // prepare GET string 
  String getStr = "GET /update?api_key=";   
  getStr += apiKey;
  getStr +="&field1=";
  getStr += ppm;
  getStr +="&field2=";
  getStr += ppm1; 
  getStr += "\r\n\r\n"; 

  Serial.println(getStr);                 // Display GET String on PC

  cmd = "AT+CIPSEND=";                    // send data length 
  cmd += String(getStr.length());
  cmd+="\r\n";

  Serial.println(cmd);                   // Display Data length on PC
  Serial1.println(cmd);                  // Send Data length command to Tx1, Rx1
  if(Serial1.find(">"))                    // If prompt opens //verify connection with cloud
  {
    Serial.println("Pushed whole data TO CLOUD");  // Display confirmation msg to PC
    Serial1.print(getStr);                 // Send GET String to Rx1, Tx1
  }
  else
  { 
    Serial1.println("AT+CIPCLOSE\r\n");    // Send Close Connection command to Rx1, Tx1
    Serial.println("AT+CIPCLOSE");         // Display Connection closed command on PC
  } 
  // thingspeak free version needs 15-20 sec delay between every push
  delay(15000);                            // wait for 16sec
 }
