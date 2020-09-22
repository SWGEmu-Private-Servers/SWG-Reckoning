/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef STRUCTURELISTCOMMAND_H_
#define STRUCTURELISTCOMMAND_H_

#include "server/zone/managers/stringid/StringIdManager.h"
#include "server/zone/objects/installation/InstallationObject.h"
#include "server/zone/objects/player/sui/SuiBox.h"

class StructureListCommand : public QueueCommand {
public:

	StructureListCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		auto ghost = creature->getPlayerObject();

		if (ghost == nullptr)
			return GENERALERROR;

		StringBuffer body;
		body << "Player Structures:" << endl;

		for (int i = 0; i < ghost->getTotalOwnedStructureCount(); i++) {
			ManagedReference<StructureObject*> structure = creature->getZoneServer()->getObject(ghost->getOwnedStructure(i)).castTo<StructureObject*>();

			int num = i + 1;
			body << endl << String::valueOf(num) << ".) ";

			if (structure == nullptr) {
				body << "nullptr Structure" << endl << endl;
				continue;
			}

			String structureName = StringIdManager::instance()->getStringId(structure->getObjectName()->getFullPath().hashCode()).toString();
			String customName = structure->getCustomObjectName().toString();

			if (customName.isEmpty())
				body << structureName << endl;
			else
				body << customName << endl;

			body << "Maintenance: " << String::valueOf(structure->getSurplusMaintenance()) << endl;

			if (structure->isInstallationObject()) {
				InstallationObject* installation = cast<InstallationObject*> (structure.get());

				if (installation == nullptr)
					continue;

				body << "Power: " << String::valueOf(structure->getSurplusPower()) << endl;

				if (installation->isOperating()) {
					body << "Status: " << "\\#00e604ON\\#96f4fc" << endl;
					body << "Extracting: " << installation->getCurrentSpawnName() << endl;
				} else {
					body << "Status: " << "\\#e60000OFF\\#96f4fc" << endl;
				}
			}

			body << "Zone: ";

			Zone* zone = structure->getZone();

			if (structure->isPackedUp()) {
				body << "Packed Up" << endl;
			} else if (zone == nullptr) {
				body << "nullptr" << endl;
			} else {
				body << zone->getZoneName() << endl;
				body << "World Position: " << structure->getWorldPositionX() << ", " << structure->getWorldPositionY() << endl;
			}
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle("Structure List");
		box->setPromptText(body.toString());
		box->setUsingObject(creature);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

};

#endif //STRUCTURELISTCOMMAND_H_
