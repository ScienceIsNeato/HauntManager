#/bin/bash!

NUM_PROCESSES=`ps -ef | pgrep haunt_manager | wc -l`

if [ $NUM_PROCESSES = 0 ] 
then
	exit 0
fi

# see how long the heartbeat file is now
HEARTBEATS=`cat /tmp/haunt_manager_heartbeat.log | wc -l`
LAST_COUNT=`cat /tmp/last_haunt_heart_count`

echo $HEARTBEATS > /tmp/last_haunt_heart_count

if [ $HEARTBEATS = $LAST_COUNT ]
then
	echo "NEED TO RESTART THE PROGRAM"
	echo `date` >> ~/haunt_restarts.log
	sudo reboot
fi

