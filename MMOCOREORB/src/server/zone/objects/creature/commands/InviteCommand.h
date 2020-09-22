/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef INVITECOMMAND_H_
#define INVITECOMMAND_H_


#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/managers/group/GroupManager.h"
#include "server/zone/ZoneServer.h"

class InviteCommand : public QueueCommand {
public:

	InviteCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		PlayerObject* playerObject = creature->getPlayerObject();
		bool godMode = false;

		if (playerObject)
		{
			if (playerObject->hasGodMode())
				godMode = true;
		}

		GroupManager* groupManager = GroupManager::instance();

		ManagedReference<SceneObject*> object = server->getZoneServer()->getObject(target);

		ManagedReference<CreatureObject*> player = nullptr;
		StringTokenizer args(arguments.toString());
		String firstName;

		if (args.hasMoreTokens()) {
			args.getStringToken(firstName);
			player = server->getZoneServer()->getPlayerManager()->getPlayer(firstName);
		}

		if (player == nullptr) {
			if (object != nullptr && object->isPlayerCreature())
				player = cast<CreatureObject*>(object.get());
		}

		if (player == nullptr) {
			creature->sendSystemMessage("Group Invite Error: The specified player does not exist or your target is invalid. Usage: /invite <firstName> or /invite your target,");
			return GENERALERROR;
		}

		if (!player->getPlayerObject()->isIgnoring(creature->getFirstName().toLowerCase()) || godMode)
			groupManager->inviteToGroup(creature, player);


		return SUCCESS;
	}

};

#endif //INVITECOMMAND_H_
