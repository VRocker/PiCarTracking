#pragma once

/*
 *	Upload the data via the cellular connection
 */
void uploadData();

/*
 * Wait for the GPS to report that it has a fix on the satellites before uploading the initial coordinates
 */
void waitForInitialFix();

bool checkForPPPConnection();
