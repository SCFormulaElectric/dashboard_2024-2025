11/17/25
The main change was displaying multiple faults from the BMS to support accumulator debugging.
Main logic:
Check for the right can id 0x303 (orionbms can id)
Aggregate all 4 bytes into 1 32-bit current_bms_faults
Call updateBmsFaultDisplay();
Clear old faults if they’ve gone inactive
loop over possible error codes, if there’s a match, check if it’s displayed
if it isn’t displayed, find first available textbox
if there is an available textbox, send that String to Nextion
sendTextToNextion():
Take component name & message, format into serial nextion command
Added global variable current_bms_faults
 represents all fault codes or’d together,
Added global var uint32_t displayed_bms_faults[3] = {0, 0, 0};
Represents textboxes that can display fault codes, 1=taken, 0=free
Added String bms_error_slots[3] = {"err1", "err2", "err3"};
component names in nextion, used to send error descriptions
