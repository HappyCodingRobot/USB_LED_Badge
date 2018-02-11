# XANES X1 Programmable LED light badge protocoll reverse engineering


## General informations
* VID:PID - 0416:5020
* Product ID: LS32 Custm HID
* uses HID Protocoll
* 2 endpoints, uses endpoint 1 for data, all with report 0
* 64 byte paket size

## The Protocol

**Badge Protocol (Report #0)**

Report size: 64 byte (rep_num+data[64])
 
**Header (first report to send):**
```
	id[5]:  "wang",0x00
    byte:   Brightness in 4 steps
   	byte:   bit-coded: flash messages
   	byte:   bit-coded: border messages
   	byte:   Message 0 speed and effect
   	byte:   Message 1 speed and effect
   	byte:   Message 2 speed and effect
   	byte:   Message 3 speed and effect
   	byte:   Message 4 speed and effect
   	byte:   Message 5 speed and effect
   	byte:   Message 6 speed and effect
   	byte:   Message 7 speed and effect
   	word:   Message 0 length (big endian)
   	word:   Message 1 length (big endian)
   	word:   Message 2 length (big endian)
   	word:   Message 3 length (big endian)
   	word:   Message 4 length (big endian)
   	word:   Message 5 length (big endian)
   	word:   Message 6 length (big endian)
   	word:   Message 7 length (big endian)
```
The length counts as the 8x11 characters/elements.

**Brightness coding:**
```
   	0x00xx0000
       	^^ ----- 0: 100%, 1: 75%, 2: 50%, 3: 25%
```
Brightness setting is global for all messages.

**Flash & Border effect coding:**
```
   0bxxxxxxxx
            ^ - 1: Message 1 flash/border, 0: no effect
           ^ -- 1: Message 2 flash/border, 0: no effect
                   ...
```
Per bit setting for the messages.

**Speed & effect/pattern coding:**
```
   0bxxxxnnnn
         ^^^^ - Effect number [0..8], see enum effects
     ^^^^ ----- Scrolling/effect speed [1..8]
```
Effect|Number
------|------
left  |0
right |1
up    |2
down  |3
freeze|4
animation|5
snow  |6
volume|7
laser |8

**Data frames/messages**

Data follows (second to nth report to send, all use Report #0)
char[11]:   characters as bitmasks (8x11), stuffed together to fill the reports.

```
Display:
 0 {8bit}    | 11    | 22    | ...
 1 {8bit}    | 12    | 23    | ...
  ...
 10 {8bit}   | 21    | 32    | ...
```


Up to 6 'chars' are displayed directly on the display. Here the last 4 rows/bits are ignored. On fewer 'chars', the content is centered on the display. On more than 6 'chars', the 6th is fully used, depending on the chosen effect. <br>
This behavior is important if bitmaps should be displayed.


