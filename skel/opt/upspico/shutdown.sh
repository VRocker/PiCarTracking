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
# TODO

# Power off the system
poweroff
