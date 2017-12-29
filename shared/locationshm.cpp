#include "locationshm.h"
#include <sys/shm.h>
#include <time.h>

struct shmLocation* getShmLocation(LocationUnitIDs unit)
{
	int shmid = shmget(0x44495030 + (unsigned int)unit, sizeof(struct shmLocation), IPC_CREAT | 0666);
	if (shmid == -1)
		return nullptr;

	struct shmLocation* p = (struct shmLocation*)shmat(shmid, 0, 0);
	if ((int)(long)p != -1)
		return p;

	return nullptr;
}

void cleanupShmLocation(struct shmLocation* shm)
{
	if (shm)
	{
		shmdt(shm);
		shm = nullptr;
	}
}

void addLocation(struct shmLocation* shm, float latitude, float longitude, float speed)
{
	if (!shm)
		return;

	shm->valid = 0;

	shm->latitude = latitude;
	shm->longitude = longitude;
	shm->speed = speed;
	shm->lastUpdated = time(0);

	shm->valid = 1;
}
