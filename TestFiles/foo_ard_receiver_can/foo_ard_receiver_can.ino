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
    Serial.println("CAN Receiver Initialized Successfully!");

    CAN.setMode(MCP_NORMAL);  // Set to normal mode to receive messages
}

void loop() {
    if (CAN.checkReceive() == CAN_MSGAVAIL) {  
        long unsigned int rxId;
        byte len;
        byte rxBuf[8];

        // Read the received message
        CAN.readMsgBuf(&rxId, &len, rxBuf);

        // Print the received CAN ID
        Serial.print("Received CAN ID: 0x");
        Serial.println(rxId, HEX);

        // Print the data
        Serial.print("Data: ");
        for (int i = 0; i < len; i++) {
            Serial.print(rxBuf[i], HEX);  // Print in hex format
            Serial.print(" ");
        }
        Serial.println();
    }

    delay(100);  // Reduce CPU usage
}
