#include "server/zone/objects/area/FactionalArea.h"
#include "templates/params/creature/CreatureFlag.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/Zone.h"

void FactionalAreaImplementation::notifyEnter(SceneObject* player) {
	if (!player->isPlayerCreature())
		return;

	CreatureObject* creature = cast<CreatureObject*>(player);
	PlayerObject* ghost = creature->getPlayerObject();

	if (creature == nullptr || ghost == nullptr)
		return;

	if (creature->getFaction() == 0 || getAreaFaction() == 0)
		return;

	if (getAreaFaction() != creature->getFaction()) {
		ghost->setIsInOpposingArea(true);
		ghost->updateLastGcwPvpCombatActionTimestamp();
	}
}

void FactionalAreaImplementation::notifyExit(SceneObject* player) {
	if (!player->isPlayerCreature())
		return;

	CreatureObject* creature = cast<CreatureObject*>(player);
	PlayerObject* ghost = creature->getPlayerObject();

	if (creature == nullptr || ghost == nullptr)
		return;

	if (creature->getFaction() == 0 || getAreaFaction() == 0)
		return;

	if (getAreaFaction() != creature->getFaction()) {
		ghost->setIsInOpposingArea(false);
		ghost->updateLastGcwPvpCombatActionTimestamp();
	}
}
