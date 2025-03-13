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
  /*
    It should be known that Serial is the object for communication with the Serial Monitor function on the Arduino IDE.
    This Serial output can also be monitored using a python file.
    Please refer to the readTeensyOutputPython/main.py for more information.
  */

  // This set is included to ensure 
  // that 7 and 8 are the ideal port used
  // Serial2.setTX(14);
  // Serial2.setRX(15);
  // unsure if 14, 15 is serial3
  Serial3.begin(9600);    // RXTX

  // Can communication
  can1.begin();   // CAN communication
  can1.setBaudRate(500000); //bit rate of 500kbs

  dash_vcu_buzzPlayed.id = 0x110;
  dash_vcu_buzzPlayed.buf[0] = 0x1;
}

void loop() {
  sendNumberToNextion("mtrtemp", motor_temperature);  
  sendNumberToNextion("numbat", battery_percentage);  
  sendNumberToNextion("probat", battery_percentage);  
  sendNumberToNextion("numspeed", speed);  
  sendNumberToNextion("mtrctrltemp", motor_controller_temperature);  


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
    else if (incoming_message.id == 0x444){
      speed = incoming_message.buf[0];
      motor_temperature = ctof(incoming_message.buf[1]);
      battery_percentage = incoming_message.buf[2];
      motor_controller_temperature = ctof(incoming_message.buf[3]);
    }
  }
}

/*
  ctof(): 
    celcius to farenheit 
  Args: 
    c (int): celcius input 
  Returns:
    float: gives back the farenheit value
*/

int ctof (int c) {
  return floor(c*1.8 + 32);
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
void sendNumberToNextion(String component, int value) {
  // Send the command to update the text component with the value
  Serial3.print(component);
  Serial3.print(".val=");
  Serial3.print(value);
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
  Serial3.write(0xFF);
  Serial3.write(0xFF);
  Serial3.write(0xFF);
}
