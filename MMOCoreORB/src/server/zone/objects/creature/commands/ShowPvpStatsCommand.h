#ifndef SHOWPVPSTATSCOMMAND_H_
#define SHOWPVPSTATSCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"

#include "server/zone/objects/player/sui/messagebox/SuiMessageBox.h"

class ShowPvpStatsCommand : public QueueCommand {
public:

	ShowPvpStatsCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		PlayerManager* playerManager = server->getZoneServer()->getPlayerManager();
		ManagedReference<CreatureObject*> targetObj = nullptr;
		StringTokenizer args(arguments.toString());
		String targetName = "";

		if (args.hasMoreTokens()) {
			args.getStringToken(targetName);
			targetObj = playerManager->getPlayer(targetName);
		} else if (creature->getTargetID() != 0) {
			targetObj = server->getZoneServer()->getObject(creature->getTargetID()).castTo<CreatureObject*>();
		} else {
			targetObj = creature;
		}

		PlayerObject* ghost = creature->getPlayerObject();

		if (targetObj != nullptr) {
			PlayerObject* targetGhost = targetObj->getPlayerObject();

			if (ghost != nullptr && targetGhost != nullptr) {
				StringBuffer msg;

				msg << "---PvP Statistics---\n"
					<< "PvP Rating: " << targetGhost->getPvpRating() << "\n"
					<< "Total PvP Kills: " << targetGhost->getPvpKills() << "\n"
					<< "Total Bounty Kills: " << targetGhost->getBountyKills();

				ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, SuiWindowType::NONE);
				box->setPromptTitle(targetObj->getFirstName() + "'s" + " PvP Statistics");
				box->setPromptText(msg.toString());
				ghost->addSuiBox(box);
			 	creature->sendMessage(box->generateMessage());
			}
		}

		return SUCCESS;
	}

};

#endif //SHOWPVPSTATSCOMMAND_H_
