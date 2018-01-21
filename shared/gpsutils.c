#include "gpsutils.h"

void convertNMEAToLatLon(float nmeaLat, char ns, float nmeaLon, char ew, float* outLat, float* outLon)
{
	/*
	So as latitude is in format DDSS.SSSSS
	DD = int(float(Lat)/100) = int(3137.36664/100) = int(31.3736664) = 31
	SS = float(lat) - DD * 100 = 3137.36664 - 31 * 100 = 3137.36664 - 3100 = 37.36664

	LatDec = DD + SS/60 = 31 + 37.36664/60 = 31 + 0.6227773333333333 = 31.6227773333333333
	*/

	int latDD= (int)(nmeaLat / 100.0f);
	float latSS = nmeaLat - (latDD * 100);

	int lonDD = (int)(nmeaLon / 100.0f);
	float lonSS = nmeaLon - (lonDD * 100);

	float newLat = latDD + (latSS / 60);
	float newLon = lonDD + (lonSS / 60);

	if (ns == 'S')
		newLat = -newLat;
	if (ew == 'W')
		newLon = -newLon;

	*outLat = newLat;
	*outLon = newLon;
}

void convertSpeedToKMH(float speedInKnots, float* speedInKmh)
{
	*speedInKmh = (speedInKnots * 1.852f);
}

void convertSpeedToMPS(float speedInKnots, float* speedInMps)
{
	*speedInMps = (speedInKnots * 0.514444444444f);
}

