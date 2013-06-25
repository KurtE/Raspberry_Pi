Warning
#######

This is a Work In Progress!  There are no warrantees or Guarantees 
of any type that this code is useable for anything.  But I hope it is…

This set of directories are some test programs as well as some code for some
different robots, that I am trying out on the Raspberry Pi (RPI).  

Also recently I started to adapt this code to work with the BeagleBone Black (BBBk)

Note: I am in no way an expert on Linux and what little Unix experience I 
have is from quite a long time ago.

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

Likewise I have a thread talking about starting to work with the BeagleBone Black up on 
Trossen Robotics: http://forums.trossenrobotics.com/showthread.php?6158-BeagleBone-Black-starting-to-play...



Quick Start Guide
=================

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

Putty and WinSCP
----------------

Also if setting up a PC to talk to a PI through the network, more information from Kevin:
So for file transfer I use WinSCP. It's a nice little GUI for simple FTP. I still code on my PC 
then when I want to test compile I just transfer over the directories. Here is the direct link. 
It should auto download. Your raspberry Pi should tell you it's IP towards the end of it's boot cycle.

http://winscp.net/download/winscp514setup.exe

I run my rpi headless so I simply ssh into its IP. I use the below program to get a terminal in.

http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html

As long as you are comfortable with a terminal this should work fine.

Configure Raspberry Pi
----------------------

Note: On the first boot, I also use the configuration program, to resize the main partition to
the size of the SD card, plus time zone, keyboard type... If I missed changing anything can always 
bring this utility back up by typing: 
sudo raspi-config


Adding Users
------------

To Add a different user other than pi, you can do something like: 
sudo adduser kurt

It will prompt you for some stuff...
Then to make a super user: 
sudo adduser kurt sudo

if you wish to make the new user act like the user pi, as with regards to sudo command, you can edit the information:
sudo nano /etc/sudoerrs

Also some devices require special permissions or the like to work properly.  Example USB serial devices
need permission.  You can either get this by running the command using sudo, or you in this case you can add the
user to the dialout group: sudo adduser kurt dialout

To list a complete list of users and groups you can try:
cat /etc/passwd | cut -d: -f1
cat /etc/group |cut -d: -f1 

If something works for one user such as pi but not for another user like kurt, you can also try using the
groups command and see what the differences are and maybe add some to the user that is not working.
groups
groups pi


Setup WiFI on RPI
-----------------

On this 2nd Pi I am trying a different USB Wifi adapter that has an external 5dbi gain antenna 
( http://www.amazon.com/GMYLE-Wireless-80 ... pd_cp_pc_0 ) 
that probably will have a longer range. On the first Pi I did a bunch of manual stuff to get it 
up and working with my network. This time I simply had the GUI up (startX) and ran the Wifi Config
 program on the desktop and was able to see my network, choose the right AP, entered in the WPA key
 and so far it appears to work  

Setup tty Device on RPI
-----------------------

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


Setup TTy Device on BBBk
------------------------

I can also use the same type of setup on the BBBk, but in addition to this, the BBBk has multiple 
Usarts available on expansion connectors P8/P9.  However to use these, you need to update the device
tree.  I will put more information in here on how to do that, including files I use to update.
More information up on: http://blog.pignology.net/2013/05/getting-uart2-devttyo1-working-on.html

Download and Build this code.
=============================

Then you can copy or git the sources that I have up on github. The commands I did to do this is:
cd ~
mkdir git
cd git
 git clone git://github.com/KurtE/Raspberry_Pi 
cd Raspberry_Pi/Phoenix 
make

PCM Sound and ESpeak
====================

I want some form of sound capability on my robots, to help with different things like simply 
acknowledgments and also fun to play with speech.  There is now code in the library directory for
doing this.


I have been playing around with installing everything necessary to be able to install my Phoenix code 
on this Pi and be able to compile it. I installed it by using the following commands:

Setup for Raspberry Pi
----------------------

First currently code is dependent on using espeak code. Will probably make this optional later... 
To install the necessary stuff you need to do something like:
sudo apt-get install espeak
sudo apt-get install libespeak-dev

Also the Phoenix code base now has the capability of outputting tones to the speacker using PCM.  To build
using this capability you will need the appropriate header files and the like:  
sudo apt-get libasound2-dev
Note: this maybe should be:
sudo apt-get install libasound2 


Note: you should then try to configure espeak and see if you can get it to work. More information
earlier in this thread, but look at the data up at the link:  http://elinux.org/RPi_Text_to_Speech_%28Speech_Synthesis%29


Setup for BeagleBone Black
--------------------------

The only sound output for the BBBk is using the HDMI, which I don't wish to do.  So on the BBBk I 
have installed a cheap USB sound card:
http://www.amazon.com/dp/B002R33VWW/ref=pe_175190_21431760_M3T1_ST1_dp_1

I did not find any valid packages to install the ALSA, so I did it manually:
    wget ftp://ftp.alsa-project.org/pub/lib/a...1.0.25.tar.bz2 
    tar jxf alsa-lib-1.0.25.tar.bz2
    cd alsa-lib-1.0.25
    ./configure
    make install

This did not set the USB device to be default sound device, I used the command: aplay -L
to list the nodes.  From which I created the configuration file: /etc/asound.conf
    pcm.!default sysdefault:Device
    
Next up, install ESpeak, which also relies on a library portaudio. I downloaded the most recent package for
portaudio from www.portaudio.com.  I extracted it and then:
    ./configure
    make install
    
I downloaded espeak-1.47.11-source.zip from sourceforge.net.  After I extracted the stuff from the zip file and
copied down the sources, I then did:
    ./configure
    make clean
    make
    make install

Tried running and found that the portaudio library was in /usr/local/lib
Could maybe move it, but instead defined:

    LD_LIBRARY_PATH=/usr/local/lib
    export LD_LIBRARY_PATH
in /etc/profile

...  



Other notes
=============

Raspberry Pi
------------

### Overclock ###

Many overclock their RPI up from the default 700mhz up to 1000mhz to do this:
sudo nano /boot/config.txt
add or uncomment lines:
arm_freq=1000
sdram_freq+500 

More details up at: http://www.jeremymorgan.com/tutorials/raspberry-pi/how-to-overclock-raspberry-pi/ and
http://raspberrypi.stackexchange.com/questions/1219/how-do-i-determine-the-current-mhz

More details about using the RPICONFIG and warnings about corrupting SDCards up at:
http://elinux.org/RPiconfig#Overclocking_options


Streaming video
---------------

With the webcam I am playing around with the mjpg-streamer to stream from Pi to PC...
Instructions on how to do this is up at: 
http://www.instructables.com/id/Create-an-internet-controlled-robot-using-Livebots/step5/Get-the-webcam-streamer-for-Raspberry-Pi/

BBB PWM Support
---------------

I am now playing with trying to have Servos supported directly on the Beagle Bone Black.  There are several threads 
up about enabling PWM support up on the BeagleBoard.org forums.  Also a member created a C++ library for it, which
I borrowed the code from as part of my library.  More details up on the 
[thread](https://groups.google.com/forum/embed/?place=forum%2Fbeagleboard&showsearch=true&showpopout=true&showtabs=true&hideforumtitle=true&parenturl=http%3A%2F%2Fbeagleboard.org%2FCommunity%2FForums%3Futm_expid%3D6702460-6%26utm_referrer%3Dhttp%253A%252F%252Fbeagleboard.org%252FSupport%252FHardware%252520Support#!category-topic/beagleboard/C2tzvRYk1Wg)

I have an updated copy of his library code included in mine plus a simple PWM test case, which enables two PWM pins and has 
them pulse from 500-2500us pulses.

There are issues with the default test-pwm driver that did not allow us to properly initialize two pwm devices on the same
logical device.  You need to update the compiled driver for this (/lib/modules/3.8.13/kernel/drivers/pwm/pwm_test.ko). 
Likewise he created duplicates of the device tree overlays, that initialized the values to a state that allowed both
channels period to be set before they are initialized.  They have names like: sc_pwm_P8_13-00A0.dtbo
which must be coppied into: /lib/firmware.

May upload some precompiled ones here such that you don't have to build the kernel to get them. 
    


Warning
=======

This is a work in progress

