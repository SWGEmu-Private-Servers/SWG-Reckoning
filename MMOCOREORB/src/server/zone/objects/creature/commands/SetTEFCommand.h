/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SETTEFCOMMAND_H_
#define SETTEFCOMMAND_H_


class SetTEFCommand : public QueueCommand {
public:

	SetTEFCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

		//Check privileges
		if (ghost == nullptr || !ghost->isPrivileged())
			return INSUFFICIENTPERMISSION;

		//Explain syntax
		if (arguments.isEmpty() && creature->getTargetID() == 0) {
			creature->sendSystemMessage("Syntax: /setTEF <duration in minutes> OR /setTEF <-area range> <duration in minutes>");
			return GENERALERROR;
		}

		StringTokenizer args(arguments.toString());

		bool area = false;
		float range = 64;
		int duration = 5;

		while (args.hasMoreTokens()) {

			String arg;
			args.getStringToken(arg);
			bool validOption = false;

			//Command Options
			if (arg.charAt(0) == '-') {
				//Make command area affect with optional range
				if (arg.toLowerCase() == "-area" || arg == "-a") {
					validOption = true;
					area = true;

					if (args.hasMoreTokens()) {
						range = args.getFloatToken();
						if (range <= 0) {
							creature->sendSystemMessage("Invalid range.");
							return INVALIDPARAMETERS;
						}
					}
				}

				if (!validOption) {
					creature->sendSystemMessage("Invalid option " + arg);
					return INVALIDPARAMETERS;
				}
			} else {
				try {
					//Test if value is integer
					for (int i = 0; i < arg.length(); i++) {
						if (!Character::isDigit(arg.charAt(i)))
							throw Exception("Invalid Duration");
					}

					duration = Integer::valueOf(arg);

					if (args.hasMoreTokens()) {
						throw Exception("Too many arguments.");
						}
					}
				catch (Exception& e) {
					creature->sendSystemMessage(e.getMessage());
					return INVALIDPARAMETERS;
				}
			}
		}

		if (area) {
			SortedVector<QuadTreeEntry*> closeObjects;
			Zone* zone = creature->getZone();

			if (creature->getCloseObjects() == nullptr) {
				zone->getInRangeObjects(creature->getPositionX(), creature->getPositionY(), range, &closeObjects, true);
			} else {
				CloseObjectsVector* closeVector = (CloseObjectsVector*) creature->getCloseObjects();
				closeVector->safeCopyReceiversTo(closeObjects, CloseObjectsVector::PLAYERTYPE);
			}

			for (int i = 0; i < closeObjects.size(); i++) {
				SceneObject* targetObject = cast<SceneObject*>(closeObjects.get(i));

				if (targetObject->isPlayerCreature() && creature->isInRange(targetObject, range)) {
					CreatureObject* targetPlayer = cast<CreatureObject*>(targetObject);
					if (targetPlayer == nullptr)
						continue;

					PlayerObject* targetGhost = targetPlayer->getPlayerObject();
					if (targetGhost == nullptr)
						continue;

					targetGhost->updateLastGcwPvpCombatActionTimestamp();
				}
			}
			creature->sendSystemMessage("You have given all players in range a PvP TEF.");
			return SUCCESS;
		} else {
			ManagedReference<CreatureObject*> targetCreature = nullptr;
			ManagedReference<PlayerObject*> targetGhost = nullptr;
			ManagedReference<SceneObject*> targetObject = server->getZoneServer()->getObject(target);

			if (targetObject != nullptr) {
				if (targetObject->isPlayerCreature()) {
					targetCreature = cast<CreatureObject*>(targetObject.get());
					targetGhost = targetCreature->getPlayerObject();
				} else {
					creature->sendSystemMessage("Invalid target.");
					return INVALIDTARGET;
				}
			}

			if (targetCreature == nullptr || targetGhost == nullptr) {
				creature->sendSystemMessage("Invalid target.");
				return INVALIDTARGET;
			}

			targetGhost->updateLastGcwPvpCombatActionTimestamp();
			creature->sendSystemMessage("You have given " + targetCreature->getFirstName() + " a PvP TEF.");
			return SUCCESS;
		}
		return SUCCESS;
	}
};

#endif //SETTEFCOMMAND_H_
