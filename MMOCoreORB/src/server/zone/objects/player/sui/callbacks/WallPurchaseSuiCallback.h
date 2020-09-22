/*
 * WallPurchaseSuiCallback.h
 */

#ifndef WALLPURCHASESUICALLBACK_H_
#define WALLPURCHASESUICALLBACK_H_

#include "server/zone/objects/player/sui/SuiCallback.h"

class WallPurchaseSuiCallback : public SuiCallback {

public:
	WallPurchaseSuiCallback(ZoneServer* server)
		: SuiCallback(server) {
	}

	void run(CreatureObject* player, SuiBox* suiBox, uint32 eventIndex, Vector<UnicodeString>* args) {
		bool cancelPressed = (eventIndex == 1);

		if (!suiBox->isListBox() || cancelPressed)
			return;

		int wallCost = ConfigManager::instance()->getWallPurchaseCost();

		if (wallCost > player->getCashCredits()) {
			player->sendSystemMessage("You do not have enough credits to purchase this wall.");
			return;
		}

		ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

		if (inventory == nullptr || inventory->isContainerFullRecursive()) {
			player->sendSystemMessage("Your inventory is full.");
			return;
		}

		int index = Integer::valueOf(args->get(0).toString());
		Vector<String> walls = ConfigManager::instance()->getStructureWalls();

		ManagedReference<SceneObject*> wallObj = server->createObject(walls.get(index).hashCode(), 1);

		if (wallObj == nullptr)
			return;

		if (inventory->transferObject(wallObj, -1, true)) {
			wallObj->sendTo(player, true);
			player->subtractCashCredits(wallCost);
			player->sendSystemMessage("Wall Purchase Successful.");
		} else {
			wallObj->destroyObjectFromDatabase(true);
		}
	}
};

#endif /* WALLPURCHASESUICALLBACK_H_ */
