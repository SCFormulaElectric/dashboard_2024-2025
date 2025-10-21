/*
  This is the ULTIMATE program for Teensy.
  Teensy is the communicator between GEVCU and Nextion Display.
  Teensy is connected to the display using RX/TX, pin 8 and 7 respectively.
  When connecting Teensy and display, connect Teensy RX to display TX and vice versa.
  Teensy is connected to the GEVCU using CAN bridge.
  Please refer to Systems Electric for CANBUS.
*/

// #include <FlexCAN.h>
#include <FlexCAN_T4.h>
/*
  12/02/24 Need to handle receive messages from can to update the dashboard
  01/27/25 Fixed the serial port issue. Tested RX/TX com with Nextion Display.
  - Need testings on CAN with GEVCU and BMS
  04/28/25 Migrated reading speed, motor temp, and motor controller temp  
    reading to teensy from gevcu to decrease # of msgs sent on bus
*/

// Define the CAN bus settings
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

//speed, battery percetage, motor temp, motor controller temperature?
int i = 0;
int speed = 0;
int motor_temperature = 0;
int motor_controller_temperature = 0;
int battery_percentage = 0;
int state;
int BMS_PIN = 7;
int IMD_PIN = 8;
int RESET_PIN = 9;
int BMS_LED = 11;
int IMD_LED = 10;
int BUZZER_PIN = 4;
bool LED_ON = false;
int bms_fault = 0;
int imd_fault = 0;
uint32_t fault_code = 0x00000000;
// convention for CAN messages: source_dest_label
CAN_message_t dash_vcu_buzzPlayed;

// used to recieve the temp value
// you might need to change this in a fat 
// if loop to sift through the messages
CAN_message_t incoming_message; 

// LUT for motor temperature
static const struct {
    int32_t value;  
    int16_t tempC; 
} motorTempLookup[] = {
    {7414, -35}, {7687, -30}, {7962, -25}, {8240, -20}, {8520, -15},
    {8802, -10}, {9085, -5},  {9369,  0},  {9654,  5},  {9939, 10},
    {10225, 15}, {10510, 20}, {10795, 25}, {11080, 30}, {11364, 35},
    {11646, 40}, {11927, 45}, {12207, 50}, {12485, 55}, {12762, 60},
    {13036, 65}, {13308, 70}, {13578, 75}, {13846, 80}, {14111, 85},
    {14373, 90}, {14633, 95}, {14890, 100}, {15144, 105}, {15391, 110},
    {15632, 115}, {15852, 120}, {16061, 125}, {16251, 130}, {16421, 135},
    {16569, 140}, {16692, 145}, {16789, 150}, {16857, 155}
};

/************************************************
  motorToCelsius: 
    Function to convert motor temp to celsius
  Args: 
    reading (uint_16_t): first parameter
      raw hex bytes of the motor temperature msg
  Returns:
    double
************************************************/
int motorToCelsius(uint16_t reading) {
    for (int i = 1; i < (int)(sizeof(motorTempLookup) / sizeof(motorTempLookup[0])); ++i) {
        if (reading <= motorTempLookup[i].value) {
            double t1 = motorTempLookup[i-1].tempC;
            double t2 = motorTempLookup[i].tempC;
            double v1 = motorTempLookup[i-1].value;
            double v2 = motorTempLookup[i].value;

            double ratio = (reading - v1) / (v2 - v1);
            double result = t1 + ratio * (t2 - t1);
            return (int)result; // Truncate fractional part
        }
    }
    return (int)motorTempLookup[sizeof(motorTempLookup) / sizeof(motorTempLookup[0]) - 1].tempC;
}

// LUT for motor controller temperature 
static const struct {
      int16_t tempC;
      uint16_t value;
  } controllerTempLookup[] = {
      { 125, 28480 }, { 120, 28179 }, { 115, 27851 }, { 110, 27497 },
      { 105, 27114 }, { 100, 26702 }, {  95, 26261 }, {  90, 25792 },
      {  85, 25296 }, {  80, 24775 }, {  75, 24232 }, {  70, 23671 },
      {  65, 23097 }, {  60, 22515 }, {  55, 21933 }, {  50, 21357 },
      {  45, 20793 }, {  40, 20250 }, {  35, 19733 }, {  30, 19247 },
      {  25, 18797 }, {  20, 18387 }, {  15, 18017 }, {  10, 17688 },
      {   5, 17400 }, {   0, 17151 }, {  -5, 16938 }, { -10, 16757 },
      { -15, 16609 }, { -20, 16487 }, { -25, 16387 }, { -30, 16308 }
  };
  //LUT for BMS Fault codes
  static const struct {
    uint32_t hex_fault;
    const char* description_fault;
  } faultCodeMap[] = {
    {0x80000000, "Cell Bank Fault"},
    {0x40000000, "Cell Voltage Over 5V Fault"},
    {0x20000000, "Current Sensor Fault"},
    {0x10000000, "Cell Open Wiring Fault"},
    {0x08000000, "Low Cell Volage Fault"},
    {0x04000000, "Weak Cell Fault"},
    {0x02000000, "Cell Balancing Stuck Off Fault"},
    {0x01000000, "Internal Cell Communication Fault"},
    {0x00800000, "Charge-Enable Relay Fault"},
    {0x00400000, "Invalid Input Supply Fault"},
    {0x00200000, "High Voltge Isolation Fault"},
    {0x00100000, "Redundant Power Supply Fault"},
    {0x00080000, "CAN Communication Fault"},
    {0x00040000, "Thermistor Fault"},
    {0x00020000, "Fan Monitor Fault"},
    {0x00010000, "Weak Pack Fault"},
    {0x00008000, "Pack Too Hot Fault"},
    {0x00004000, "Lowest Cell Voltage Too Low Fault"},
    {0x00002000, "Highest Cell Voltage Too High Fault"},
    {0x00001000, "Internal Logic Fault"},
    {0x00000800, "Internal Heatsink Thermistor Fault"},
    {0x00000400, "Internal Hardware Fault"},
    {0x00000200, "Charger Safety Relay Fault"},
    {0x00000100, "Discharge-Enable Relay Fault"},
    {0x00000080, "High Voltage Interlock Signal Fault"},
    {0x00000040, "Precharge Circuit Malfunction"},
    {0x00000020, "Abnormal State of Charge Behavior"},
    {0x00000010, "Charge Interlock"},
};

/*decode_fault function
Args:
  hex_fault -> 64bit fault code in hex
Returns:
  fault code -> Corresponding fault code description string

*/
/*const char* decode_fault(uint64_t hex_fault){
    size_t tableSize = sizeof(faultCodeMap) / sizeof(faultCodeMap[0]);

    for (size_t i=0; i < tableSize; i++){
      if (faultCodeMap[i].hex_fault == hex_fault){
        return faultCodeMap[i].description_fault;
      }
    }
    return NULL;


}*/

String decode_all_faults(uint32_t hex_code){
  String faults = "";
  size_t tableSize = sizeof(faultCodeMap) / sizeof(faultCodeMap[0]);

    for (size_t i=0; i < tableSize; i++){
      if (hex_code & faultCodeMap[i].hex_fault){
        faults +=faultCodeMap[i].description_fault;
        faults += " | "; //separator for sliding text between each fault
      }
    }
    return faults;

}



/************************************************
  motorControllerToCelsius: 
    Function to convert motor controller temp to celsius
  Args: 
    reading (uint_16_t): first parameter
      raw hex bytes of the motor controller temperature msg
  Returns:
    double
************************************************/
int motorControllerToCelsius(uint16_t reading) {
    for (int i = 1; i < (int)(sizeof(controllerTempLookup) / sizeof(controllerTempLookup[0])); ++i) {
        if (reading >= controllerTempLookup[i].value) {
            double t1 = controllerTempLookup[i-1].tempC;
            double t2 = controllerTempLookup[i].tempC;
            double v1 = controllerTempLookup[i-1].value;
            double v2 = controllerTempLookup[i].value;

            double ratio = (reading - v2) / (v1 - v2);
            double result = t2 + ratio * (t1 - t2);
            return (int)result; // Truncate fractional part
        }
    }
    return (int)controllerTempLookup[sizeof(controllerTempLookup) / sizeof(controllerTempLookup[0]) - 1].tempC;
}


int speedToMPH(uint16_t reading) {
    const int NMAX_IN_NDRIVE = 5000; 
    const double gearRatio = 3.25;                
    const int wheelDiameterInInches = 15;    
    const double effeciency = 0.93;
    double percentage = (double) reading / 32767.0;
    double motorRPM = percentage * NMAX_IN_NDRIVE;
    double wheelRPM = motorRPM / gearRatio;
    double mph = (wheelRPM * 3.14159265359 * wheelDiameterInInches) / 1056.0 * effeciency;

    return (int)mph; 
}

// CAN message from 

void setup() {
  /*
    It should be known that Serial is the object for communication with the Serial Monitor function on the Arduino IDE.
    This Serial output can also be monitored using a python file.
    Please refer to the readTeensyOutputPython/main.py for more information.
  */
  Serial.begin(9600);

  // unsure if 14, 15 is serial3
  Serial3.begin(9600);    // RXTX

  // Can communication
  can1.begin();   // CAN communication
  can1.setBaudRate(500000);

  pinMode(BMS_PIN, INPUT);
  pinMode(IMD_PIN, INPUT);
  pinMode(RESET_PIN, INPUT);
  pinMode(13, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BMS_LED, OUTPUT);
  digitalWrite(BMS_LED, HIGH);
  pinMode(IMD_LED, OUTPUT);
  digitalWrite(IMD_LED, HIGH);

  dash_vcu_buzzPlayed.id = 0x110;
  dash_vcu_buzzPlayed.buf[0] = 0x1;

  // added pin mode
}

void loop() {
  sendNumberToNextion("mtrtemp", motor_temperature);  
  sendNumberToNextion("numbat", battery_percentage);  
  sendNumberToNextion("probat", battery_percentage);  
  sendNumberToNextion("numspeed", speed);  
  sendNumberToNextion("mtrctrltemp", motor_controller_temperature);  
  updateMotorTemperatureColor(motor_temperature);
  updateMotorControllerTemperatureColor(motor_controller_temperature);


  /* 
    Check if received the buzzer message from VCU 
    if so, then forward the state and call fxn:
    'sendResponse(blah,blah)'
  */
  if (can1.read(incoming_message)) {
    digitalWrite(13, LED_ON);
    LED_ON = !LED_ON;
    if (incoming_message.id == 0x109 && incoming_message.buf[0] == 0x1) {
      state = incoming_message.buf[1];  
      buzz_played_response(state);

    
    } else if (incoming_message.id == 0x181){      //from the bamocar                  
      if (incoming_message.buf[0] == 0x30){// speed
        speed = decode_hex(incoming_message.buf[1], incoming_message.buf[2]); 
        speed = speedToMPH(speed);
      }
      else if (incoming_message.buf[0] == 0x4a){                        // motor controller temp
      motor_controller_temperature = decode_hex(incoming_message.buf[1], incoming_message.buf[2]);  
      motor_controller_temperature = (motorControllerToCelsius(motor_controller_temperature));
      } else if (incoming_message.buf[0] == 0x49){                        // motor temp 
        motor_temperature = decode_hex(incoming_message.buf[1], incoming_message.buf[2]); 
        motor_temperature = (motorToCelsius(motor_temperature));
      
      } 
    } else if (incoming_message.id == 0x301) {
      battery_percentage = incoming_message.buf[4]/2;
    } else if (incoming_message.id == 0x303){ // Fault Code Display
      uint64_t received_fault_code = 0;
      for (int i =0; i < 8; i++){
        received_fault_code |= (uint64_t)incoming_message.buf[i] << (8*i);
      }
      String all_fault_descriptions = decode_all_faults(received_fault_code);

      display_all_faults(all_fault_descriptions);
      

    }else {                                                      // Errors from the Car to Display
      // switch (incoming_message.id){
      //   case 0x500: //PotBrake error messages
      //     sendPotbrakeError(incoming_message.buf[0]);
      //     break;
      //   case 0x501: //PotThrottle error messages 
      //     break;
      // }
    }
  }

  if(!digitalRead(BMS_PIN)){
    bms_fault = 1; 
  }

  if(bms_fault){
    digitalWrite(BMS_LED, LOW);
  }

  if(digitalRead(IMD_PIN)){
    imd_fault = 1;
  }

  if(imd_fault){
      digitalWrite(IMD_LED, LOW);
  }

  if(bms_fault){
    if(!digitalRead(RESET_PIN) && digitalRead(BMS_PIN)){
      bms_fault = 0;
      digitalWrite(BMS_LED, HIGH);
    }
  }

  if(imd_fault){
    if(!digitalRead(RESET_PIN) && !digitalRead(IMD_PIN)){
      imd_fault = 0;
      digitalWrite(IMD_LED, HIGH);
    }
  }

}
/************************************************
  decode_hex(): 
    celcius to farenheit 
  Args: 
    first_half  (int_64_t): first half of the bytes 
    second_half (int_64_t): second half of the bytes
  Returns:
    float: gives back the farenheit value
************************************************/

int decode_hex( uint8_t first_half,  uint8_t second_half) {
    //second_half has 256 more weight since it is in the 2nd place of base 16, 16^2 = 256.
    return second_half * 256 + first_half;
}

/************************************************
  ctof(): 
    celcius to farenheit 
  Args: 
    c (int): celcius input 
  Returns:
    float: gives back the farenheit value
************************************************/

int ctof (int c) {
  return floor(c*1.8 + 32);
}

/************************************************
  sendNumberToNextion(): 
    Function to send a number to a Nextion component

  Args: 
    component (String): first parameter
      specify which component you are sending to  
    value     (float) : second parameter
      value to show on display
  Returns:
    void 
************************************************/

// Function to send a number to a Nextion component
void sendNumberToNextion(String component, int value) {
  // Send the command to update the text component with the value
  Serial3.print(component);
  Serial3.print(".val=");
  Serial3.print(value);
  sendEndCommand();
}

/************************************************
  buzz_played_response(): 
    Sends a message to the VCU that the buzzer has been played 

  Args: 
    state (int): first parameter
      takes the state of the recieved message and sends it back
  Returns:
    void 
************************************************/
void buzz_played_response(int state) {
  dash_vcu_buzzPlayed.buf[1] = state;
  can1.write(dash_vcu_buzzPlayed); // some code to make buzz
  digitalWrite(BUZZER_PIN, HIGH);
  delay(3000);
  digitalWrite(BUZZER_PIN, LOW);
}


/*
Displays an error message for 3 seconds, before clearing
and displaying nothing.

  Args:
    msg (String): first parameter
        the message to display on the screen
  Returns:
    void
  
  Example msg: "Error:\r\nBMS TOO HOT"  (the \r\n is for a new line)
*/
void setErrorMessage(const String& msg) {
  Serial3.print("errormsg.txt=\"" + msg + "\"");
  sendEndCommand();
  Serial3.print("tm_errormsg.en=1"); // Enables the timer so that after 3 seconds it will stop displaying.
  sendEndCommand();
}

/* 
  sendEndCommand(): 
    Function to send the end command required by Nextion Protocol

  Args: 
    empty

  Returns:
    void 
************************************************/

// Function to send the end command required by Nextion

void sendEndCommand() {
  Serial3.write(0xFF);
  Serial3.write(0xFF);
  Serial3.write(0xFF);
}

void updateMotorControllerTemperatureColor(int motor_controller_temperature){
  if (motor_controller_temperature > 85){ // RED
    Serial3.print("mtrctrltemp.pco=63488");
  }
  else if (motor_controller_temperature > 70){ // YELLOW
    Serial3.print("mtrctrltemp.pco=50688");
  }
  else if  (motor_controller_temperature > 60){ // Black
    Serial3.print("mtrctrltemp.pco=0");
  }
  sendEndCommand();
}

void updateMotorTemperatureColor(int motor_temperature){
  if (motor_temperature > 85){ // RED
    Serial3.print("mtrtemp.pco=63488");
  }
  else if (motor_temperature > 70){ // YELLOW
    Serial3.print("mtrtemp.pco=50688");
  }
  else if  (motor_temperature > 60){ // Black
    Serial3.print("mtrtemp.pco=0");
  }
  sendEndCommand();
}

void updateLights(){
  if (!digitalRead(BMS_PIN)){
      bms_fault = 1;
  }

  if (bms_fault){
      digitalWrite(BMS_LED, LOW);
  }

  if (digitalRead(IMD_PIN)){
      imd_fault = 1;
  }

  if (imd_fault){
      digitalWrite(IMD_LED, LOW);
  }

  if (bms_fault || imd_fault){
    if (!digitalRead(RESET_PIN) && digitalRead(BMS_PIN) && !digitalRead(IMD_PIN))
    {
      bms_fault = 0;
      imd_fault = 0;
      digitalWrite(BMS_LED, HIGH);
      digitalWrite(IMD_LED, HIGH);
    }
  }
}

/*void display_fault_description(const char* description){
    if (description == NULL)
        return;
    Serial3.print("faultcode.txt=\"");
    Serial3.print(description);
    Serial3.print("\"");
    sendEndCommand();
}*/

void display_all_faults(const String& descriptions){
  if (descriptions == "")
    return;
    Serial3.print("faultcode.txt=\"");
    Serial3.print(descriptions);
    Serial3.print("\"");
    sendEndCommand();

}


