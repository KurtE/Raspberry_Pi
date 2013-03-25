Warning, this is a Work In Progress!  There are no warrantees or Guarantees 
of any type that this code is useable for anything.  But I hope it is…

This set of directories are some test programs as well as some code for some
different robots, that I am trying out on the Raspberry Pi.  Note: I am in no
way an expert on Linux and what little Unix experience I have is from quite
a long time ago.

At some point I will probably merge the code back into the main code bases
That I have for those robots.  Example the Phoenix code may get merged back
Into the Arduino_Phoenix_parts code base.

Not sure if it will help anyone, but I have a thread up on Lynxmotion 
( http://www.lynxmotion.net/viewtopic.php?f=25&t=8607 ), that talks
about different things I did as part of this project. Also there is a lot of
good information up on a thread on Trossenrobotics.com 
(http://forums.trossenrobotics.com/showthread.php?6040-PhantomX-controlled-by-a-Raspberry-Pi )

In case it helps anyone, here is some information I put together (up on Lynxmotion Thread), when
I was setting up my second Pi to run this code.

---------------------------------------------------------------------------------------------------------------------
First link to Quick start: http://www.raspberrypi.org/quick-start-guide

Next Initial information from Kevin that I used to do some initial setup.
as far as linux cuts I suggest the following.

"wheezy" it has hard point and is the most popular.

http://www.raspberrypi.org/downloads

After installing check for any updates that are not in the latest distro by running:

sudo apt-get update && sudo apt-get upgrade

Next thing will install git... nearly everything needs it.  You can also push directly into your git hub online with it:

sudo apt-get install git-core

And finally you might as well make sure your firmware is updated so run the below commands.

For proper time:
sudo apt-get install ntpdate
sudo ntpdate -u ntp.ubuntu.com

For SSL certs:
sudo apt-get install ca-certificates

Firmware updater:
sudo wget http://goo.gl/1BOfJ -O /usr/bin/rpi-update && sudo chmod +x /usr/bin/rpi-update

And finally run the command for update:
sudo rpi-update

The above will pretty much get your RPI up to date with all the latest and greatest.

---------------------------------------------------------------------------------------------------------------------

Also if setting up a PC to talk to a PI through the network, more information from Kevin:
So for file transfer I use WinSCP. It's a nice little GUI for simple FTP. I still code on my PC 
then when I want to test compile I just transfer over the directories. Here is the direct link. 
It should auto download. Your raspberry Pi should tell you it's IP towards the end of it's boot cycle.

http://winscp.net/download/winscp514setup.exe

I run my rpi headless so I simply ssh into its IP. I use the below program to get a terminal in.

http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html

As long as you are comfortable with a terminal this should work fine.

---------------------------------------------------------------------------------------------------------------------

Note: On the first boot, I also use the configuration program, to resize the main partition to
the size of the SD card, plus time zone, keyboard type... If I missed changing anything can always 
bring this utility back up by typing: 
sudo raspi-config


To Add a different user other than pi, you can do something like: 
sudo adduser kurt

It will prompt you for some stuff...
Then to make a super user: 
sudo adduser kurt sudo

---------------------------------------------------------------------------------------------------------------------

On this 2nd Pi I am trying a different USB Wifi adapter that has an external 5dbi gain antenna 
( http://www.amazon.com/GMYLE-Wireless-80 ... pd_cp_pc_0 ) 
that probably will have a longer range. On the first Pi I did a bunch of manual stuff to get it 
up and working with my network. This time I simply had the GUI up (startX) and ran the Wifi Config
 program on the desktop and was able to see my network, choose the right AP, entered in the WPA key
 and so far it appears to work  

---------------------------------------------------------------------------------------------------------------------

Side note: made progress in being able to do speech output from my Phoenix Code.

I have been playing around with installing everything necessary to be able to install my Phoenix code 
on this Pi and be able to compile it. I installed it by using the following commands:

First currently code is dependent on using espeak code. Will probably make this optional later... 
To install the necessary stuff you need to do something like:
sudo apt-get install espeak
sudo apt-get install libespeak-dev

Note: you should then try to configure espeak and see if you can get it to work. More information
earlier in this thread, but look at the data up at the link:  http://elinux.org/RPi_Text_to_Speech_%28Speech_Synthesis%29

Then you can copy or git the sources that I have up on github. The commands I did to do this is:
cd ~
mkdir git
cd git
 git clone git://github.com/KurtE/Raspberry_Pi 
cd Raspberry_Pi/Phoenix 
make

---------------------------------------------------------------------------------------------------------------------

Also My code is setup to use an XBee and an SSC-32, both of which connect up as USB devices.  I did not want my
code to have to depend on the order the devices are added so I wished to setup rules to create an alias for these devices
I found a lot of good information up at: http://hintshop.ludvig.co.nz/show/persistent-names-usb-serial-devices/
Note: I had to experiment as some of the devices would hang, so I had to find the right command sequence to work.
Example: udevadm info --query=property --name=ttyUSB0

From this I created a file named 99-usb-serial.rules in the /etc/udev/rules.d directory.  A copy of mine from the first
Raspberry Pi is contained in the Phoenix directory.  It looks like:
    SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A800fclo", SYMLINK+="ttyXBEE"
    SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A4014UWE", SYMLINK+="ttySSC-32"

Note: You will have to change the {Serial} value (and maybe some others if not FTDI) to the actual device on your machine.

Aflter I installed this file, if I type something like:
ls -l /dev/ttyXBEE

I see something like:
lrwxrwxrwx 1 root root 7 Dec 31  1969 /dev/ttyXBEE -> ttyUSB0
---------------------------------------------------------------------------------------------------------------------



