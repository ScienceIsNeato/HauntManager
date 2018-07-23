echo "Building calibration binary..."
g++ -Wall -pthread -o calibrate src/calibrate.cpp -lpigpio -lrt
if [ $? -eq 0 ]; then
    echo "Successfully built 'calibrate'"
else
    echo "ERROR - Failed to build 'calibrate'"
    exit 1
fi

echo "Building test binary..."
g++ -std=c++0x -Wall -pthread -o servo_tester src/pigpioServo.cpp src/servo_tester.cpp -lpigpio -lrt

if [ $? -eq 0 ]; then
    echo "Successfully built 'servo_tester'"
else
    echo "ERROR - Failed to build 'servo_tester'"
    exit 1
fi

echo ""
echo "Good work. If I were you, first I'd run the calibration script like this:"
echo ""
echo "    'sudo ./calibrate <gpio servo pin>'"
echo ""
