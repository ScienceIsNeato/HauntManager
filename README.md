# HauntManager

# Prerequisites
Hi there! Welcome to the haunt manager!
First you'll need to install pigpio - http://abyz.me.uk/rpi/pigpio/download.html

I'm not sure if this step is necessary or not, but if the Lidar
throws bugs, you can download the firmware here: http://www.slamtec.com/en/support

# Getting started
1. Run build_calibrate.sh
2. Run the calibration routine as instructed by the build script
3. Run the servo_tester as instructed by the calibration binary
4. Modify main.cpp as instructed by the calibration binary
5. Run build_manager.sh
6. Run haunt_manager as instructed by build script

Have fun!

# Misc Notes

# Installing the restarter into cron
There is some sort of bug that causes the program to occasionally become unresponsive - probably a memory leak. While we want to fix this eventually, there is a script here that will force the pi to always stay up and running. If you experience unresponsiveness, su to root and type `crontab -e` and add the following to the bottom of the file:

	* * * * * /home/pi/HauntManager/monitor_haunt.sh 


