# USB LED Badge Programmer
## Word of warning
This is just a very early development state / work in progress.<br>
No fancy GUI, not even a usable commandline


## Why ?
I got this neat little LED badge on a promotion sale. Unfortunately i didn't found a software to use it under Linux. So I decided to make one.

## Info
This project provides a very basic way to programm a USB LED badge with text and graphic messages under Linux.<br>
It is developed under [code:blocks](http://www.codeblocks.org/), so there is no makefile available at the moment.<br>
It works for the **XANES X1 DIY Bicycle Taillight**. The **Leadbike LD45** seems to use the same hardware.
This badges are rechargeable and connects to the PC via USB as HID class devices. They support up to 8 messages that can be displayed all one after another or each one allone.

## Software

**What is done so far:**
* [Protocol reverse engineered](./doc/XANESX1ProgrammableLEDlightbadgeprotocollreverseengineering.md) (see ./doc/)
* udev rule created (see ./doc/)
* core routine to communicate with badge 
* example programm with fixed text and graphic messages

**DoDo:** <br>
*a lot..*


#### Usage
*tbd*

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
[the badge @Banggood](https://www.banggood.com/XANES-X1-DIY-Bicycle-Taillight-Programmable-LED-Electronic-Advertising-Display-Bicycle-TailLight-USB-p-1220458.html)
[LD45](http://en.leadbike.cn/index.php?id=2154)


## Copyrights
See the [LICENSE](./LICENSE) file for details.
