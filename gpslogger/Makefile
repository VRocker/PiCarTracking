OBJS=main.o SerialHandler.o ../shared/Logger.o ../shared/strutils.o ../shared/gpsutils.o ../shared/locationshm.o NmeaParser.o nmea/GprmcMessage.o nmea/GpggaMessage.o ublox/Ublox.o NtpUpdater.o
LIBS=-lpthread
BIN=gpslogger.bin

CFLAGS+=-std=c99
CXXFLAGS+=-fpermissive -std=c++11

include ../Makefile.include

