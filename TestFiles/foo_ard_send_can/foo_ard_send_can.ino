#include <SPI.h>
#include <mcp_can.h>

#define CAN_CS 10  // Chip Select pin for MCP2515
MCP_CAN CAN(CAN_CS);

void setup() {
    Serial.begin(115200);

    // Initialize CAN at 500kbps and 16 MHz clock (adjust as necessary)
    while (CAN_OK != CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ)) {
        Serial.println("Error Initializing CAN Module...");
        delay(1000);
    }
    Serial.println("CAN Sender Initialized Successfully!");
}

void loop() {
    // Sending a simple CAN message
    byte data[] = {0x01, 0x02, 0x03, 0x04};  // Example data
    long unsigned int txId = 0x100;  // CAN ID for the message

    if (CAN.sendMsgBuf(txId, 0, sizeof(data), data) == CAN_OK) {
        Serial.println("Message Sent Successfully");
    } else {
        Serial.println("Error Sending Message");
    }

    delay(1000);  // Send a message every second
}
