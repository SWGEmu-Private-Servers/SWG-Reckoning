#ifndef SPAWNPROTECTIONREMOVALTASK_H_
#define SPAWNPROTECTIONREMOVALTASK_H_

#include "server/zone/objects/player/PlayerObject.h"

namespace server {
namespace zone {
namespace objects {
namespace player {
namespace events {

class SpawnProtectionRemovalTask: public Task {
	ManagedWeakReference<CreatureObject*> creature;

public:
	SpawnProtectionRemovalTask(CreatureObject* creo) {
		creature = creo;
	}

	void run() {
		ManagedReference<CreatureObject*> player = creature.get();

		if (player == nullptr)
			return;

		ManagedReference<PlayerObject*> ghost = player->getPlayerObject().get();

		if (ghost == nullptr) {
			return;
		}

		Locker locker(player);

		if (ghost->hasSpawnProtection()) {
			this->reschedule(llabs(ghost->getSpawnProtectionTimestamp().miliDifference()));

		} else {
			player->broadcastPvpStatusBitmask();
		}
	}
	};
}
}
}
}
}

using namespace server::zone::objects::player::events;

#endif /* SPAWNPROTECTIONREMOVALTASK_H_ */
