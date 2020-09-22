/*
 * ShutdownTask.h
 *
 *  Created on: 20/05/2012
 *      Author: victor
 */

#ifndef SHUTDOWNTASK_H_
#define SHUTDOWNTASK_H_

#include "server/ServerCore.h"
#include "server/zone/ZoneServer.h"
#include "server/chat/ChatManager.h"

class ShutdownTask : public Task {
	int minutesRemaining;
	ManagedReference<ZoneServer*> zoneServer;
public:
	ShutdownTask(ZoneServer* srv, int minutes) {
		zoneServer = srv;
		minutesRemaining = minutes;
	}

	void run() {
		--minutesRemaining;

		String str = "The server will shutdown in " + String::valueOf(minutesRemaining) + " minutes. Please move your character to a safe place.";

		if (minutesRemaining <= 0)
			str = "THE SERVER IS SHUTTING DOWN NOW!";

		Logger::console.info(str, true);

		if (minutesRemaining % 10 == 0 || minutesRemaining <= 5)
			zoneServer->getChatManager()->broadcastGalaxy(nullptr, str);

		if (minutesRemaining <= 0) {
			ServerCore::getInstance()->signalShutdown();
		} else {
			schedule(60 * 1000);
		}
	}
};


#endif /* SHUTDOWNTASK_H_ */
