Warning
=======

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

`sudo apt-get install ca-certificates`

Firmware updater:

`sudo wget http://goo.gl/1BOfJ -O /usr/bin/rpi-update && sudo chmod +x /usr/bin/rpi-update
`
And finally run the command for update:

`sudo rpi-update
`
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

Archive files
------
 
As I always forget the right options to pass to tar for different types of archives.  Lots of places on web to find info like: http://www.thegeekstuff.com/2010/04/unix-tar-command-examples/
    
    tar -xvf archive_name.tar
    tar -zxvf  archive_name.tar.gz
    tar -jxvf archive_name.tar.bz2
    gunzip archive_name.gz
    bunzip2 archive_name.bz2

Configure Raspberry Pi
----------------------

Note: On the first boot, I also use the configuration program, to re-size the main partition to
the size of the SD card, plus time zone, keyboard type... If I missed changing anything can always 
bring this utility back up by typing: 
sudo raspi-config

Configure BeagleBone Black
--------------------------

Setup for date and time update
    ntpdate us.pool.ntp.org
    nano /etc/default/ntpdate   (Set us.pool.ntp.org in NTPSERVERS)

    rm /etc/localtime
    ln -s /usr/share/zoneinfo/America/Los_Angeles /etc/localtime
    
    Better yet:
        dpkg-reconfigure tzdata

Nameserver may not be set properly
    echo nameserver 8.8.8.8 > /etc/resolv.conf
    
For the Root, I exported a few things in the ~/.profile

    export SLOTS=/sys/devices/bone_capemgr.8/slots
    export PINS=/sys/kernel/debug/pinctrl/44e10800.pinmux/pins
    export OSTYPE
    ln -s /dev/ttyO1 /dev/ttyXBEE
    ln -s /dev/ttyO2 /dev/ttyRCLAW`

Note in the above the bone_capemgr.8 may change from build to build previously was .9
    
Also as a way to force the date to be set properly, I added at the endo of ~/.bashrc 

    ntpdate-sync

(I also enabled color coding of lines and the like)

Configuring Intel Edison
------------------------
**Warning**: This is the start of my working notes as I Just started with Edison

Follow the steps to configure Edison: https://communities.intel.com/docs/DOC-23147
If new board update firmware to latest: https://communities.intel.com/docs/DOC-23192

Notes also found on forum: http://forums.trossenrobotics.com/showthread.php?7145-Experiment-with-Intel-Edison

wifi did not work after configure: Talked about in thread: https://communities.intel.com/thread/55527
edit: /etc/systemd/system/basic.target.wants/network-gadget-init.service
change: 192.168.2.15 to something like: 192.168.99.15

Configure to use other repos to get stuff: 

MRAA:

    echo "src mraa-upm http://iotdk.intel.com/repos/1.1/intelgalactic" > /etc/opkg/mraa-upm.conf
    opkg update
    opkg install libmraa0

Lots of good stuff: http://alextgalileo.altervista.org/edison-package-repo-configuration-instructions.html  

set the contents of: /etc/opkg/base-feeds.conf 

    src/gz all http://repo.opkg.net/edison/repo/all
    src/gz edison http://repo.opkg.net/edison/repo/edison
    src/gz core2-32 http://repo.opkg.net/edison/repo/core2-32

The Boot segment gets full when you do updates as the whole space allocated for the boot is not actually
used.  Instructions on how to fix up at: http://alextgalileo.altervista.org/blog/install-kernel-from-repo-onto-edison-official-image/


If playing around with my Adafruit code base, may be issue with weird Adafruit_GFX files includes instead of mine.  If so delete the library:   Robot_Control out of the Intel IDE

If playing around with my Adafruit stuff with makefiles useing MRAA, issue with mraa with duplicate symbols, problem with their header files included in multiple source files.  I edited the file: /usr/include/mraa/common.hpp and made all of the functions inline, which appears to have solved this.

Warning: my testAdafruit_ILI9341 test program can cause the Edison to reboot!

Note: did not find any OSTYPE defined to be able to update makefiles to check to for now I am forcing it by creating the file  ~/.profile

    export OSTYPE=Edison
Note: still working on how to do udev rules on this machine may need to add lines like below to the .profile file until I do.

    ln -s /dev/ttyO1 /dev/ttyXBEE
    ln -s /dev/ttyO2 /dev/ttyRCLAW`

This works for the USB2AX in /etc/udev/rules.d/99-usb-serial.rules

	SUBSYSTEM=="tty", ATTRS{idVendor}=="16d0", ATTRS{idProduct}=="06a7", ATTRS{serial}=="74031303437351D02210", SYMLINK+="ttyUSB2AX"

Change Arduino IDE to allow wifi uploads of programs.  Currently I have created the file 
c:\arduino-1.5.3-Intel.1.0.4\hardware\arduino\edison\tools\izmir\clupload_win_hacked.sh that contains:
	#!/bin/sh
	
	echo "starting download script"
	echo "Args to shell:" $*
	
	edison_ip="192.168.2.115"
	edison_pw="*** YourPassword ***"
	SKETCH=/sketch/sketch.elf
	OLD_SKETCH=/sketch/sketch.elf.old
	
	
	$1/plink -pw $edison_pw root@$edison_ip  "mv -f $SKETCH $OLD_SKETCH"
	
	# Execute the target download command
	
	#Download the file.
	$1/pscp -scp -pw $edison_pw $2 root@$edison_ip:$SKETCH
	$1/plink  -pw $edison_pw root@$edison_ip "chmod +x $SKETCH"
	$1/plink  -pw $edison_pw root@$edison_ip "systemctl restart clloader"
	
You then change the file: 
c:\arduino-1.5.3-Intel.1.0.4\hardware\arduino\edison\platform.win.txt
and change the line: 

    tools.izmirdl.cmd.path={runtime.ide.path}/hardware/arduino/edison/tools/izmir/clupload_win.sh

to:

	tools.izmirdl.cmd.path={runtime.ide.path}/hardware/arduino/edison/tools/izmir/clupload_win_hacked.sh






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
user to the dialout group: 

    sudo adduser kurt dialout

To find out which group you may need for a device, do a ls -l for the device.  Example:

    kurt@raspberrypi ~ $ ls -l /dev/ttyUSB*
    crw-rw---T 1 root dialout 188, 0 Dec 31  1969 /dev/ttyUSB0
    crw-rw---T 1 root dialout 188, 1 Jun 25 08:30 /dev/ttyUSB1


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
 
 setup WiFi on BBBK
 ------------------
 There are issues with the reliablility of the Wifi Dongle that comes with the current Angstrom builds.
 
 I have followed others and currently go through the process of rebuilding the driver for the rtl8192...
 Instructions are up at: 
     http://www.codealpha.net/864/how-to-set-up-a-rtl8192cu-on-the-beaglebone-black-bbb/
 Note: There is a step missing in the instructions that you need to build the scripts.

    cd /usr/src/kernel
    make scripts
 
 Change Host Name
 ----------------
 If you wish to change the hostname of your system from the default, you can do this by editing the files:

    /etc/hosts   (the new host name needs to map to 127.0.1.1)
    /etc/hostname

 Note You need to update both devices. 
 

Setup tty Device on RPI
-----------------------

Also My code is setup to use an XBee and an SSC-32, both of which connect up as USB devices.  I did not want my
code to have to depend on the order the devices are added so I wished to setup rules to create an alias for these devices
I found a lot of good information up at: http://hintshop.ludvig.co.nz/show/persistent-names-usb-serial-devices/
Note: I had to experiment as some of the devices would hang, so I had to find the right command sequence to work.
Example: udevadm info --query=property --name=ttyUSB0

From this I created a file named 99-usb-serial.rules in the /etc/udev/rules.d directory.  A copy of mine from the first
Raspberry Pi is contained in the Phoenix directory.  It looks like:
```
    SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A800fclo", SYMLINK+="ttyXBEE"
    SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A4014UWE", SYMLINK+="ttySSC-32"
```
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

Note: It is now a lot easier than what it was earlier to do this.  From the current pignology... You see:
just add you BB-UART2 to /media/BEAGLEBONE/uEnv.txt, with the key capemgr.enable_partno.
```
    This is my current uEnv.txt:
        root@beaglebone:/lib/firmware# cat /media/BEAGLEBONE/uEnv.txt
        optargs=quiet video=HDMI-A-1:1280x1024@60e capemgr.enable_partno=BB-SPI0DEV,BB-UART4,BB-CANBUS1
    As you can see, I have the BB-UART4 enabled. This gives me /dev/ttyO4. (And I have SPIO and CAN enabled, but that is a different story).
```
Note: for debian install, the uEnv.txt file is in /boot/uboot directory.  I now have a line that looks like:

    optargs=capemgr.enable_partno=BB-UART1,BB-UART2

Now trying to figure out how to set up the symbolic links to these devices.  Can get more information about these devices with command:

    sudo udevadm info --name=ttyO1 --query=all --attribute-walk

Have two ways that work:

    SUBSYSTEM=="tty", ATTRS{port}=="0x0", ATTRS{line}=="1", SYMLINK+="ttyXBEE"
    SUBSYSTEM=="tty", ATTRS{port}=="0x0", ATTRS{line}=="2", SYMLINK+="ttyRCLAW"

    SUBSYSTEM=="tty", KERNEL=="ttyO1", SYMLINK+="ttyXBEE"
    SUBSYSTEM=="tty", KERNEL=="ttyO2", SYMLINK+="ttySSC-32"


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

    sudo apt-get install libasound2-dev
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

Recently someone else solved this by uninstalling and re-installing alsa-dev package:

    opkg install alsa-dev

Note: on some systems, the install of libasound did not create the library file:
/usr/lib/libasound.so ,but instead created version 2 specific files.  That is in that directory, there were two files: libasound.so.2 which then points to libasound.so.2.0.0.  In these cases our links with just -lasound will fail.  two ways to fix:

    1) Make our own link: ln -s /dev/lib/libasound.so.2.0.0 /dev/lib/libasound.so
    2) change our link to: -l:libasound.so.2

Update: Not sure if I will need to do this again, but from the beagle bone forum, a better way would be to:

    apt-get source libasound2
    cd alsa-lib-1.0.25
    // if you want debug symbols
    export DEB_BUILD_OPTION=nostrip noopt debug
    dpkg-buildpackage -rfakeroot -uc -us -j8 -aarmhf

    // The dpkg-buildpackage uses the settings a patches in the debian folder and builds the library as intended
    // This commands creates Debian packages in the parent folder.

    cd ..
    sudo dpkg -i libasound2* deb


This did not set the USB device to be default sound device, I used the command: aplay -L
to list the nodes.  From which I created the configuration file: /etc/asound.conf

    pcm.!default sysdefault:Device
    
If linking with -lasound  fails, I have seen on the net two answers.  

The first is to create a link, such that your code will use which ever version of the library you have made your link to (in the case of multiple versions.)

The other is to change the link to be more explicit and and do something like -l:libasound.so.2
 

As for espeak.  I am just missing the file /usr/include/espeak/speak_lib.h

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
I borrowed the code from as part of my library.  More details up on a thread on the beagleBoard.org forums.
Do a search for "c++ pwm" and should be thread by Saad Ahmad.  Will point you through to a github Project:
https://github.com/SaadAhmad/beaglebone-black-cpp-PWM

I have an updated copy of his library code included in mine plus a simple PWM test case, which enables two PWM pins and has 
them pulse from 500-2500us pulses.

There are issues with the default test-pwm driver that did not allow us to properly initialize two pwm devices on the same
logical device.  You need to update the compiled driver for this (/lib/modules/3.8.13/kernel/drivers/pwm/pwm_test.ko). 
Likewise he created duplicates of the device tree overlays, that initialized the values to a state that allowed both
channels period to be set before they are initialized.  They have names like: sc_pwm_P8_13-00A0.dtbo
which must be coppied into: /lib/firmware.

When I reloaded the EmmC with the latest firmware 0620 I used his prebuilt binaries and instructions in his Readme.md and
they appeared to work.    

BBB Debian Update builds. 
---------------
As the Debian builds are progressing, there have been new images that come out.  Sometimes don't want to have to start fresh
and redo everything, as reflashing starts you at square one.  Sometimes they setup some script files that allow you to update
to the newer builds.  Example from one of the posts:

    cd /opt/scripts/fixes
    git pull

    sudo ./debian-2014-03-04-to-2014-03-19.sh
    sudo ./debian-2014-03-19-to-2014-03-27.sh

The only thing it doesn't do is update the bootloader from v2013.10 in
2014-03-04 to v2014.04-rc in 2014-03-19+

For that:

    cd /opt/scripts/tools/
    sudo ./update_bootloader.sh 

# Installing Bluez5 #
I am currently playing with some configurations that is using the Linux Joystick device as the input device.  Currently using this with ODroid XU3-Lite and I am using it with either Playstation DS3 or DS4 controllers.  These controllers appear to use a slightly modified version of Bluetooth, which support was addded in later versions of Bluez5.  Unfortionatly Ubuntu still ships with Bluez4.   

I have had some some luck now installing Bluez5, at least on Odroid Xu3.  I follow the stesp in the link: https://www.raspberrypi.org/forums/viewtopic.php?p=619713
Which works off of the stuff in: http://www.linuxfromscratch.org/blfs/view/svn/general/bluez.html

Once I go through make and make install it still does not work as the bluetooth was not started, as I found by using the command:

	odroid@odroid:~$ hciconfig
	hci0:   Type: BR/EDR  Bus: USB
        BD Address: 00:1A:7D:DA:71:11  ACL MTU: 310:10  SCO MTU: 64:8
        DOWN
        RX bytes:547 acl:0 sco:0 events:27 errors:0
        TX bytes:384 acl:0 sco:0 commands:27 errors:0

So I was able to get the device up:

	odroid@odroid:~$ sudo hciconfig hci0 up
	[sudo] password for odroid:

	odroid@odroid:~$ hciconfig
	hci0:   Type: BR/EDR  Bus: USB
        BD Address: 00:1A:7D:DA:71:11  ACL MTU: 310:10  SCO MTU: 64:8
        UP RUNNING PSCAN
        RX bytes:1816192 acl:95463 sco:0 events:126 errors:0
        TX bytes:6200 acl:50 sco:0 commands:66 errors:0

For the DS4 I then was able to get it to bind, by:  

	odroid@odroid:~$ hcitool scan
	Scanning ...
        D0:27:88:70:B2:9B       n/a
        00:3C:7F:F0:F0:0A       n/a

After that I hit PS button on DS4 and was able to get it to bind (lite stays on), and the device /dev/input/js0 was created.

For PS3 I think you still have to bind it using USB.  I use the sixpair program for it.  I am currently using the program from the HR-OS1 project but program is easy to find.

After that I was able to get it to bind, by using the bluetoothctl program using sudo .  
When I press PS button see message like:

	[NEW] Device FC:62:B9:39:87:AB FC-62-B9-39-87-AB

While still in the program I tried:

	connect FC:62:B9:39:87:AB
	agent on 
	trust FC:62:B9:39:87:AB

After that when I pressed the PS button, the lite came on and the /dev/input/js0 device was created.

Warning: 
---
It now appears that Bluez5 is hanging the Odroid XU3 during the shutdown process. Not sure why yet, but I have tried a few different things so far to get around it. 

But for PS3, the Odroid forum member Meveric has a repository where you can install the sixad package, like I did for Intel NUC. 

The posting: http://forum.odroid.com/viewtopic.php?f=52&t=5908 Gives instructions on how to setup to use his repositories. 

Once they are installed, you can then do: 

	sudo apt-get install sixad

After that completes, you can plug the PS3 into usb and type: sudo sixpair 
Which should do the pairing of the PS3 to your Odroid.  You can then unplug the PS3 and start up the sixad daemon.
I did this with the command:

	sixad --boot-yes

After you boot hopefull you just have to press the PS button to activate the PS3. 







Warning
=======

This is a work in progress

