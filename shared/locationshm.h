/*	\file
 *	\author Craig Richards
 *	\date 18/11/2017
*/

#pragma once

#include <time.h>

struct shmLocation
{
	volatile unsigned int valid;
	time_t lastUpdated;
	float latitude;
	float longitude;
	float speed;
};

/*	Define the shared memory unit IDs
 *	These hold location information from different sources
*/
enum class LocationUnitIDs
{
	GPS,
	Modem
};

struct shmLocation* getShmLocation(LocationUnitIDs unit);
void cleanupShmLocation(struct shmLocation* shm);

void addLocation(struct shmLocation* shm, float latitude, float longitude, float speed);
