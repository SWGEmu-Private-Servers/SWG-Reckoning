#ifndef SHOWPLAYERSTATSCOMMAND_H_
#define SHOWPLAYERSTATSCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"

#include "server/zone/objects/player/sui/messagebox/SuiMessageBox.h"

class ShowPlayerStatsCommand : public QueueCommand {
public:

	ShowPlayerStatsCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		PlayerObject* ghost = creature->getPlayerObject();

		if (ghost == nullptr)
			return GENERALERROR;

		StringBuffer msg;

		msg << "---PvP Statistics---\n"
			<< "PvP Rating: " << ghost->getPvpRating() << "\n"
			<< "Total PvP Kills: " << ghost->getPvpKills() << "\n"
			<< "Total PvP Deaths: " << ghost->getPvpDeaths() << "\n"
			<< "Total Bounty Kills: " << ghost->getBountyKills() << "\n\n"
			<< "---PvE Statistics---\n"
			<< "Total PvE Kills: " << ghost->getPveKills() << "\n"
			<< "Total PvE Deaths: " << ghost->getPveDeaths() << "\n\n"
			<< "---Mission Statistics---\n"
			<< "Total Missions Completed: " << ghost->getMissionsCompleted();

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, SuiWindowType::NONE);
		box->setPromptTitle(creature->getFirstName() + "'s" + " Statistics");
		box->setPromptText(msg.toString());
		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

};

#endif //SHOWPLAYERSTATSCOMMAND_H_
