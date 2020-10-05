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
6. `cp servo_config_template.conf servo_config.conf` and update values based on your previous steps
7. Run haunt_manager as instructed by build script

Have fun!

# Misc Notes
You'll probably eventually want to setup your rPi so that this program runs at startup. Here's
a way to do that by adding the following to the bottom of your ~/.bashrc file:
``` sh
    115 cd /home/pi/dev/HauntManager
    116
    117 NUM_PROCESSES=`ps -ef | pgrep haunt_manager | wc -l`
    118
    119 if [ $NUM_PROCESSES = 0 ]
    120 then
    121         echo "\nLogging in and no haunt_manager started - so gonna start one...\n"
    122 else
    123         echo "\nStarting new session. Was going to start haunt_manager, but one already running...\n"
    124 fi
```

# Installing as a service
- `sudo cp hauntManager.service /etc/systemd/system/hauntManager.service`
- `sudo systemctl daemon-reload`
- `sudo systemctl enable myscript.service` (to automatically start service on boot)
- `sudo service hauntManager start`
- then you can tail the logs of the service using `tail -f /var/log/syslog`

# Installing the restarter into cron
There is some sort of bug that causes the program to occasionally become unresponsive - probably a memory leak. While we want to fix this eventually, there is a script here that will force the pi to always stay up and running. If you experience unresponsiveness, su to root and type `crontab -e` and add the following to the bottom of the file:

	* * * * * /home/pi/HauntManager/monitor_haunt.sh 

If you do above, you'll want to set up a couple files.
`echo "0" > /tmp/last_haunt_heart_count`
`touch ~/haunt_restarts.log`


