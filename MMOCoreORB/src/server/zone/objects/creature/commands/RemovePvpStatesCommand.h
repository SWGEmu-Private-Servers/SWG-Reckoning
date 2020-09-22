/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef REMOVEPVPSTATESCOMMAND_H_
#define REMOVEPVPSTATESCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"

class RemovePvpStatesCommand : public QueueCommand {
public:

	RemovePvpStatesCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ZoneServer* zoneServer = server->getZoneServer();

		Reference<SceneObject*> targetObj = zoneServer->getObject(target);

		if (targetObj == nullptr || !targetObj->isPlayerCreature()) {
			creature->sendSystemMessage("Invalid target. This command only works on players");
			return INVALIDTARGET;
		}

		CreatureObject* targetCreature = targetObj.castTo<CreatureObject*>();
		PlayerObject* targetGhost = targetCreature->getPlayerObject();

		if (targetGhost == nullptr) {
			return GENERALERROR;
		}

		if (targetCreature->getFaction() != 0) {
			targetCreature->setFactionStatus(FactionStatus::COVERT);
		}

		targetGhost->schedulePvpTefRemovalTask(true, true, true);
		targetGhost->setHasCityTef(false);
		targetCreature->broadcastPvpStatusBitmask();

		creature->sendSystemMessage("You have removed all PvP states from " + targetCreature->getFirstName() + ".");

		return SUCCESS;
	}

};

#endif //REMOVEPVPSTATESCOMMAND_H_
