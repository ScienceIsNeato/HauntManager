# HauntManager

TODO: General instuctions

# Installing the restarter into cron
There is some sort of bug that causes the program to occasionally become unresponsive - probably a memory leak. While we want to fix this eventually, there is a script here that will force the pi to always stay up and running. If you experience unresponsiveness, su to root and type `crontab -e` and add the following to the bottom of the file:

	* * * * * /home/pi/HauntManager/monitor_haunt.sh 


