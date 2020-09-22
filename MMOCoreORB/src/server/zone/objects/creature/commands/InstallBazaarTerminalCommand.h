/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef INSTALLBAZAARERMINALCOMMAND_H_
#define INSTALLBAZAARTERMINALCOMMAND_H_

#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/objects/creature/commands/sui/InstallBazaarTerminalSuiCallback.h"

class InstallBazaarTerminalCommand : public QueueCommand {
public:

	InstallBazaarTerminalCommand(const String& name, ZoneProcessServer* server)
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

		if (!ghost->hasAbility("installbazaarterminal"))
			return GENERALERROR;

		ManagedReference<CityRegion*> city = creature->getCityRegion().get();

		if (city == nullptr)
			return GENERALERROR;

		if (!city->isMayor(creature->getObjectID())) {
			creature->sendSystemMessage("You must be the mayor of this city to add a Bazaar Terminal.");
			return GENERALERROR;
		}

		ManagedReference<SuiListBox*> suiTerminalType = new SuiListBox(creature, SuiWindowType::INSTALL_MISSION_TERMINAL, 0);
		suiTerminalType->setCallback(new InstallBazaarTerminalSuiCallback(server->getZoneServer()));

		suiTerminalType->setPromptTitle("Install Bazaar Terminal");

		StringBuffer promptText;
		promptText << "Select Bazaar Terminal from the list below. The Bazaar Terminal will be placed at your current location and rotation(ie. the way you're facing). "
				<< "If you want to remove the Bazaar Terminal in the future, select the Remove option from the terminal's radial menu. "
				<< "Placing a Bazaar Terminal will cost 10000 credits, withdrawn from the treasury at the time of placement.";

		suiTerminalType->setPromptText(promptText.toString());

		suiTerminalType->addMenuItem("Bazaar Terminal", 0);

		ghost->addSuiBox(suiTerminalType);
		creature->sendMessage(suiTerminalType->generateMessage());

		return SUCCESS;
	}
};

#endif //INSTALLBAZAARTERMINALCOMMAND_H_
