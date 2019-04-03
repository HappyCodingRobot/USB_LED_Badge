# USB LED Badge Programmer
## Word of warning
This is just a very early development state / work in progress.<br>
No fancy GUI, just a simple commandline


## Why ?
I got this neat little LED badge on a promotion sale. Unfortunately i didn't found a software to use it under Linux. So I decided to make one.

## Info
This project provides a very basic way to programm a USB LED badge with text and graphic messages under Linux.<br>
It is developed under [code:blocks](http://www.codeblocks.org/), ~~so there is no makefile available at the moment~~.<br>
It works for the **XANES X1 DIY Bicycle Taillight**. The **Leadbike LD45** seems to use the same hardware.
This badges are rechargeable and connects to the PC via USB as HID class devices. They support up to 8 messages that can be displayed all one after another or each one alone.

## Software

**What is done so far:**
* [Protocol reverse engineered](./doc/XANESX1ProgrammableLEDlightbadgeprotocollreverseengineering.md) (see ./doc/)
* udev rule created (see ./doc/)
* core routine to communicate with badge
* example programm (cli only, no graphic import yet)

**ToDo:** <br>
*a lot.. maybe a GUI*


#### Usage
Commandline options:<br>

    -h  Show help.
    -i  Set message index [0..7].
    -m  Set message text.
    -s  Set the message speed [1..8].
    -e  Set the message effect [0..8].
        0: to left
        1: to right
        2: scroll up
        3: scroll down
        4: Freeze
        5: Animation
        6: Snow
        7: Volume
        8: Laser
    -b  Blink message.
    -f  Set frame for message.
    -B  Set the badge brightness [0..3]. This is for all messages.
    -D  Demo mode. Use as the only argument.

```
badge -i[0..7] -m <text for message num i> <parameter for message num i> (next index with message and parameter ...)
```

Examples:<br>
```bash
badge -i0 -m "Hello World!" -e2 -s6
```
- writes one up scrolling message to slot 0 of the badge with speed 6
```bash
badge -i0 -m "Hello World!" -e2 -s6 -i6 -m "there is more.." -e1 -f
```
- writes 2 messages to slot 0 and 6
```bash
badge -i0 -m "This" -e1 -s6 -i1 -m "is a" -e0 -f -i2 -mTest
```
- writes 3 messages, slots 0,1,2 to the badge

#### Installation and software setup
(*not much at the moment*)

**udev**<br>
Copy the udev rule to the rules directory, e.g.<br>
`sudo cp ./doc/99-led_badge_test.rules /etc/udev/rules.d/`<br>
Depending on your system, it might be necessary to trigger/reload udev<br>
`sudo sudo udevadm trigger` <br>
`sudo reload udev` <br>
Mostly, it should work to copy the rule and then plug in the device.<p>


## Links
[the badge @Banggood](https://www.banggood.com/XANES-X1-DIY-Bicycle-Taillight-Programmable-LED-Electronic-Advertising-Display-Bicycle-TailLight-USB-p-1220458.html) <br>
[LD45](http://en.leadbike.cn/index.php?id=2154)


## Copyrights
See the [LICENSE](./LICENSE) file for details.
