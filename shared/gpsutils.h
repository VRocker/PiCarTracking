#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	void convertNMEAToLatLon(float nmeaLat, char ns, float nmeaLon, char ew, float* outLat, float* outLon);
	// Convert knots to Kilometers per hour
	void convertSpeedToKMH(float speedInKnots, float* speedInKmh);
	// Convert knots to Meters per Second
	void convertSpeedToMPS(float speedInKnots, float* speedInMps);

#ifdef __cplusplus
}
#endif
