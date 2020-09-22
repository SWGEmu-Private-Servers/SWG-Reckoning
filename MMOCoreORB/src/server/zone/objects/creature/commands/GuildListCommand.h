/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef GUILDLISTCOMMAND_H_
#define GUILDLISTCOMMAND_H_

#include "server/zone/ZoneServer.h"
#include "server/zone/managers/guild/GuildManager.h"
#include "server/zone/objects/guild/GuildObject.h"


class GuildListCommand : public QueueCommand {
public:

	GuildListCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return INVALIDPARAMETERS;

		if (!creature->isInGuild()) {
			creature->sendSystemMessage("Error: You are not in a guild.");
			return GENERALERROR;
		}

		ZoneServer* zoneServer = server->getZoneServer();
		if (zoneServer == nullptr)
			return GENERALERROR;

		ManagedReference<GuildObject*> guild = creature->getGuildObject().get();
		ManagedReference<GuildManager*> guildManager = zoneServer->getGuildManager();

		if (guild == nullptr || guildManager == nullptr)
			return GENERALERROR;

		if (!guild->hasMember(creature->getObjectID()))
			return GENERALERROR;

		guildManager->sendGuildMemberListTo(creature, guild, nullptr);

		return SUCCESS;
	}

};

#endif //GUILDLISTCOMMAND_H_
