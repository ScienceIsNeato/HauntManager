#/bin/bash!

NUM_PROCESSES=`ps -ef | pgrep haunt_manager | wc -l`

echo "DEBUG num processes is $NUM_PROCESSES"

if [ $NUM_PROCESSES = 0 ] 
then
	echo "DEBUG exiting - not running"
	exit 0
fi

# see how long the heartbeat file is now
HEARTBEATS=`cat /tmp/haunt_manager_heartbeat.log | wc -l`
LAST_COUNT=`cat /tmp/last_haunt_heart_count`

echo "DEBUG last heartbeat count was $LAST_COUNT"
echo "DEBUG this heartbeat count is $HEARTBEATS"

echo $HEARTBEATS > /tmp/last_haunt_heart_count

if [ $HEARTBEATS = $LAST_COUNT ]
then
	echo "NEED TO RESTART THE PROGRAM"
	#sudo pkill -9 haunt_manager
	sleep 2 
	echo `date` >> ~/haunt_restarts.log
	#nohup /home/pi/HauntManager/run_haunt &
	sudo reboot
else
	echo "shits good"
fi

