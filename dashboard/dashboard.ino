// #include <FlexCAN.h>
#include <FlexCAN_T4.h>
/*
  12/02/24 Need to handle receive messages from can to update the dashboard

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

// convention for CAN messages: source_dest_label
CAN_message_t dash_vcu_buzzPlayed;

// used to recieve the temp value
// you might need to change this in a fat 
// if loop to sift through the messages
CAN_message_t incoming_message; 

// CAN message from 

void setup() {
  // Start serial communication with Nextion display
  // Serial.begin(9600);
  Serial2.begin(9600);    // RXTX

  // Can communication
  can1.begin();   // CAN communication
  can1.setBaudRate(500000); //bit rate of 500kbs

  dash_vcu_buzzPlayed.id = 0x110;
  dash_vcu_buzzPlayed.buf[0] = 0x1;
}

void loop() {

  // Send the number to a text component (e.g., "t0" on the Nextion)
  // sendNumberToNextion("t0", 666); // this is not to be changed. However, the down left name have to be changed to Motor Temp (C)
  
  // // Debug info
  // Serial.println("Sending something");
  // Serial2.print(12345);
  // Serial2.write(0xFF);
  
  // Serial.println("Now Sending..."); // See on Teensy end if it is sending now
  Serial.print("Battery: ");
  Serial.println(battery_percentage);

  Serial.print("Speed: ");
  Serial.println(speed);

  Serial.print("Temperature: ");
  Serial.println(ctof(motor_temperature));

  /*
    Actual Sender
  */
  sendNumberToNextion("numbattery", battery_percentage);
  sendNumberToNextion("numspeed", speed);
  sendNumberToNextion("numtemp", ctof(motor_temperature)); // why are we using F instead of C?
  sendNumberToNextion("t4", ctof(motor_controller_temperature));

  /* 
    Check if received the buzzer message from VCU 
    if so, then forward the state and call fxn:
    'sendResponse(blah,blah)'
  */
  if (can1.read(incoming_message)) {
    if (incoming_message.id == 0x109 && incoming_message.buf[0] == 0x1) {
      state = incoming_message.buf[1];  
      buzz_played_response(state);
    }
    else if (incoming_message.id == 0x203){
      speed = incoming_message.buf[0];
      motor_temperature = incoming_message.buf[1];
      battery_percentage = incoming_message.buf[2];
      motor_controller_temperature = incoming_message.buf[3];
    }
  }



  // Delay for a second
  // delay(1000);  delay used for dashboard refresh
}

/*
  ctof(): 
    celcius to farenheit 
  Args: 
    c (int): celcius input 
  Returns:
    float: gives back the farenheit value
*/

float ctof (int c) {
  return c*1.8 + 32;
}

/* 
  sendNumberToNextion(): 
    Function to send a number to a Nextion component

  Args: 
    component (String): first parameter
      specify which component you are sending to  
    value     (float) : second parameter
      value to show on display
  Returns:
    void 
*/

// Function to send a number to a Nextion component
void sendNumberToNextion(String component, float value) {
  // Send the command to update the text component with the value
  Serial2.print(component);     // Component name, e.g., "t0"
  Serial2.print(".txt=\"");     // For a text field
  Serial2.print(value);         // Value to display
  Serial2.print("\"");          // End the string value

  sendEndCommand();
}

/* 
  buzz_played_response(): 
    Sends a message to the VCU that the buzzer has been played 

  Args: 
    state (int): first parameter
      takes the state of the recieved message and sends it back
  Returns:
    void 
*/

void buzz_played_response(int state) {
  dash_vcu_buzzPlayed.buf[1] = state;
  can1.write(dash_vcu_buzzPlayed); // some code to make buzz
  // Serial.write("sent buzz message validation"); // Validation passed
  }

/* 
  sendEndCommand(): 
    Function to send the end command required by Nextion Protocol

  Args: 
    empty

  Returns:
    void 
*/

// Function to send the end command required by Nextion

void sendEndCommand() {
  // Serial.write(0xFF);  // End of command character (0xFF) sent three times
  // Serial.write(0xFF);
  // Serial.write(0xFF);

  Serial2.write(0xFF);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
}
