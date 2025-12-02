##12/1/25
'CAN PACKETS DROPPING' 
Added code to reduce display refresh rate (5hz), aiming to limit serial messages and eliminate serial buffer overflow. Should help w/ CAN PACKETS
[BR is 9600 bps, teensy operates at 480MHz, so, even if the for loop took 10,000 Clock cycles, which shouldn't be because there is minimal division and only a state machine, can parsing, there will be
serial messages sent way more frequently than the BR can handle, causing buffer to fill, causing teensy to freeze, causing CAN PACKETS DROPPING]



## 11/17/25

The main change was displaying multiple faults from the BMS to support accumulator debugging.

### Main logic:

1.  Check for the right can id `0x303` (orionbms can id)
2.  Aggregate all 4 bytes into 1 32-bit `current_bms_faults`
3.  Call `updateBmsFaultDisplay();`
    a.  Clear old faults if they've gone inactive
    b.  loop over possible error codes, if there's a match, check if it's displayed
    c.  if it isn't displayed, find first available textbox
    d.  if there is an available textbox, send that String to Nextion
    e.  `sendTextToNextion():`
        1.  Take component name & message, format into serial nextion command

### Additions

* Added global variable `current_bms_faults`
    * represents all fault codes or'd together,
* Added global var `uint32_t displayed_bms_faults[3] = {0, 0, 0};`
    * Represents textboxes that can display fault codes, 1=taken, 0=free
* Added `String bms_error_slots[3] = {"err1", "err2", "err3"};`
    * component names in nextion, used to send error descriptions

Link to library used in the .ino file: https://github.com/tonton81/FlexCAN_T4/tree/master


