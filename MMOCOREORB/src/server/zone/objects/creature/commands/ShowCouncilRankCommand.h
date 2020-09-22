/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SHOWCOUNCILRANKCOMMAND_H_
#define SHOWCOUNCILRANKCOMMAND_H_

#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/objects/player/sui/callbacks/EnclaveCouncilRankSuiCallback.h"

class ShowCouncilRankCommand : public QueueCommand {
public:

	ShowCouncilRankCommand(const String& name, ZoneProcessServer* server)
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

		FrsData* playerData = ghost->getFrsData();
		int playerCouncil = playerData->getCouncilType();
		int curPlayerRank = playerData->getRank();

		if (curPlayerRank < 0)
			return GENERALERROR;

		if (playerCouncil == 0)
			return GENERALERROR;

		ManagedReference<SuiListBox*> box = new SuiListBox(creature, SuiWindowType::ENCLAVE_VOTING, SuiListBox::HANDLETWOBUTTON);
		box->setCallback(new EnclaveCouncilRankSuiCallback(server->getZoneServer(), playerCouncil));
		box->setPromptText("Select the rank whose members you wish to view.");
		box->setPromptTitle("@force_rank:rank_selection"); // Rank Selection
		box->setUsingObject(creature);
		box->setOkButton(true, "@ok");
		box->setCancelButton(true, "@cancel");

		String councilString;
		String stfRank;

		if (playerCouncil == 1)
			councilString = "light";
		else if (playerCouncil == 2)
			councilString = "dark";

		for (int i = 1; i < 12; i++) {
			if (i >= 1 && i <= 9)
				stfRank = "@skl_n:force_rank_" + councilString + "_rank_0" + String::valueOf(i);
			else if (i == 10)
				stfRank = "@skl_n:force_rank_" + councilString + "_rank_10";
			else if (i == 11)
				stfRank = "@skl_n:force_rank_" + councilString + "_master";
			box->addMenuItem(stfRank);
		}

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

};

#endif //SHOWCOUNCILRANKCOMMAND_H_
