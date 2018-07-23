echo "Building haunt manager binary..."
g++ -std=c++0x -Wall -pthread -o haunt_manager include/net_socket.cpp include/net_serial.cpp include/thread.cpp include/timer.cpp src/haunt_manager.cpp src/pigpioServo.cpp src/Scanner.cpp src/rplidar_driver.cpp -lpigpio -lrt

if [ $? -eq 0 ]; then
    echo "Successfully built 'haunt_manager'"
else
    echo "ERROR - Failed to build 'haunt_manager'"
    exit 1
fi

echo ""
echo "Good work. If I were you, first I'd run the calibration script like this:"
echo ""
echo "    'sudo ./calibrate <gpio servo pin>'"
echo ""
