#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	void convertNMEAToLatLon(float nmeaLat, char ns, float nmeaLon, char ew, float* outLat, float* outLon);

#ifdef __cplusplus
}
#endif
