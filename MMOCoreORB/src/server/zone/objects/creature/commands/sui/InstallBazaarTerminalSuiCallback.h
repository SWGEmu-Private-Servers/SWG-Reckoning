/*
 * InstallBazaarTerminalSuiCallback.h
 */

#ifndef INSTALLBAZAARTERMINALCALLBACK_H_
#define INSTALLBAZAARTERMINALCALLBACK_H_

#include "server/zone/objects/player/sui/SuiCallback.h"
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/Zone.h"
#include "server/zone/managers/city/CityManager.h"

class InstallBazaarTerminalSuiCallback : public SuiCallback {
public:
	InstallBazaarTerminalSuiCallback(ZoneServer* server)
	: SuiCallback(server) {
	}

	void run(CreatureObject* player, SuiBox* suiBox, uint32 eventIndex, Vector<UnicodeString>* args) {
		bool cancelPressed = (eventIndex == 1);

		if (cancelPressed)
			return;

		if (args->size() < 1)
			return;

		if (player->getParent() != nullptr)
			return;

		ManagedReference<CityRegion*> city = player->getCityRegion().get();
		CityManager* cityManager = player->getZoneServer()->getCityManager();
		if (city == nullptr || cityManager == nullptr)
			return;

		PlayerObject* ghost = player->getPlayerObject();
		if (ghost == nullptr)
			return;

		if (!city->isMayor(player->getObjectID()))
			return;

		if (!cityManager->canSupportMoreBazaarTerminals(city)) {
			player->sendSystemMessage("Your city does not meet the required rank or it can't support any more Bazaar Terminals.");
			return;
		}

		Zone* zone = player->getZone();

		if (zone == nullptr)
			return;

		if (!ghost->hasAbility("installbazaarterminal"))
			return;

		int option = Integer::valueOf(args->get(0).toString());

		String terminalTemplatePath = "";

		switch (option) {
			case 0: terminalTemplatePath = "object/tangible/terminal/terminal_bazaar.iff";
			break;
		}

		if (terminalTemplatePath != "") {
			Locker clocker(city, player);

			if (city->getCityTreasury() < 10000){
				StringIdChatParameter msg;
				msg.setStringId("@city/city:action_no_money");
				msg.setDI(1000);
				player->sendSystemMessage(msg); //"The city treasury must have %DI credits in order to perform that action.");
				return;
			}

			StructureObject* cityHall = city->getCityHall();
			if (cityHall == nullptr)
				return;

			ManagedReference<SceneObject*> sceneObject = ObjectManager::instance()->createObject(terminalTemplatePath.hashCode(), 1, "sceneobjects");

			city->addBazaarTerminal(sceneObject);
			city->subtractFromCityTreasury(10000);

			clocker.release();

			Locker locker(sceneObject);

			sceneObject->initializePosition(player->getWorldPositionX(), player->getWorldPositionZ(),player->getWorldPositionY());
			sceneObject->rotate(player->getDirectionAngle());

			Locker clocker2(cityHall, player);

			cityHall->addChildObject(sceneObject);
			sceneObject->initializeChildObject(cityHall);

			zone->transferObject(sceneObject, -1, true);
		}
	}
};

#endif /* INSTALLBAZAARTERMINALCALLBACK_H_ */
