#!/bin/sh
#
# UPS Pico shutdown script
#

# Send a SIGTERM to the GPS Logger
kill `pidof gpslogger.bin` &
echo "Waiting for GPS Logger to exit..."
while true ; do
        PID=`ps cat | grep gpslogger.bin | grep -v grep`
        if [ -z "$PID" ]; then
		echo "Killed!"
		break
	fi
	sleep 1
done

# Shutdown Realtime reporting (and report current location)
kill `pidof celluploader.bin` &
echo "Waiting for Cell Uploader to exit..."
while true ; do
        PID=`ps cat | grep celluploader.bin | grep -v grep`
        if [ -z "$PID" ]; then
		echo "Killed!"
		break
	fi
	sleep 1
done

# Power off the system
poweroff
