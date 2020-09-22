/*
 * VendorManager.cpp
 *
 *  Created on: Mar 23, 2011
 *      Author: polonel
 */

#include "VendorManager.h"
#include "server/chat/ChatManager.h"
#include "server/zone/managers/vendor/sui/DestroyVendorSuiCallback.h"
#include "server/zone/objects/intangible/VendorControlDevice.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/auction/AuctionItem.h"
#include "server/zone/objects/player/sui/inputbox/SuiInputBox.h"
#include "server/zone/objects/player/sui/messagebox/SuiMessageBox.h"
#include "server/zone/objects/structure/StructureObject.h"
#include "server/zone/objects/waypoint/WaypointObject.h"
#include "server/zone/managers/vendor/sui/PackupVendorSuiCallback.h"
#include "server/zone/managers/vendor/sui/RelistItemsSuiCallback.h"
#include "server/zone/managers/vendor/sui/RenameVendorSuiCallback.h"
#include "server/zone/managers/vendor/sui/RegisterVendorSuiCallback.h"
#include "server/zone/managers/auction/AuctionManager.h"
#include "server/zone/managers/auction/AuctionsMap.h"
#include "server/zone/objects/tangible/components/vendor/VendorDataComponent.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/ZoneProcessServer.h"

VendorManager::VendorManager() {
	setLoggingName("VendorManager");
	server = nullptr;
	nameManager = nullptr;
}

void VendorManager::initialize(ZoneProcessServer* zserv) {
	server = zserv;

	loadLuaVendors();
	loadVendorOutfits();
}

void VendorManager::loadLuaVendors() {
	info("Loading Vendor Options...");

	Lua* lua = new Lua();
	lua->init();

	bool res = lua->runFile("custom_scripts/managers/vendor_manager.lua");

	if (!res)
		res = lua->runFile("scripts/managers/vendor_manager.lua");

	LuaObject menu = lua->getGlobalObject("VendorMenu");

	rootNode = new VendorSelectionNode();

	rootNode->parseFromLua(menu);

	menu.pop();

	delete lua;
	lua = nullptr;
}

bool VendorManager::isValidVendorName(const String& name) {

	if(nameManager == nullptr)
		nameManager = server->getNameManager();

	if(nameManager == nullptr) {
		error("Name manager is nullptr");
		return false;
	}

	return nameManager->validateVendorName(name) == NameManagerResult::ACCEPTED;
}

void VendorManager::handleDisplayStatus(CreatureObject* player, TangibleObject* vendor) {
	DataObjectComponentReference* data = vendor->getDataObjectComponent();
	if (data == nullptr || data->get() == nullptr || !data->get()->isVendorData()) {
		error("Vendor has no data component");
		return;
	}

	VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());
	if (vendorData == nullptr) {
		error("Vendor has wrong data component");
		return;
	}

	if (vendor->getZone() == nullptr && !vendorData->isPackedUp()) {
		error("nullptr zone in VendorManager::handleDisplayStatus and vendor is not packed up");
		return;
	}

	ManagedReference<SuiListBox*> statusBox = new SuiListBox(player, SuiWindowType::STRUCTURE_VENDOR_STATUS);
	statusBox->setPromptTitle("@player_structure:vendor_status");
	statusBox->setPromptText("Vendor Status");

	if (vendorData->isPackedUp())
		statusBox->setUsingObject(vendor->getControlDevice().get());
	else
		statusBox->setUsingObject(vendor);

	ManagedReference<CreatureObject*> owner = server->getZoneServer()->getObject(vendorData->getOwnerId()).castTo<CreatureObject*>();
	String ownerName;

	if (owner == nullptr)
		ownerName = "nullptr";
	else
		ownerName = owner->getFirstName();

	statusBox->addMenuItem("Owner: " + ownerName);

	int condition = (((float)vendor->getMaxCondition() - (float)vendor->getConditionDamage()) / (float)vendor->getMaxCondition()) * 100;
	statusBox->addMenuItem("Condition: " + String::valueOf(condition) + "%");

	float secsRemaining = 0.f;
	if (vendorData->getMaint() > 0){
		secsRemaining = (vendorData->getMaint() / vendorData->getMaintenanceRate())*3600;
	}

	statusBox->addMenuItem("Maintenance Pool: " +
			               String::valueOf(vendorData->getMaint()) +
			               "cr " + getTimeString( (uint32)secsRemaining ) );
	statusBox->addMenuItem("Maintenance Rate: " + String::valueOf((int)vendorData->getMaintenanceRate()) + " cr/hr");

	ManagedReference<AuctionManager*> auctionManager = server->getZoneServer()->getAuctionManager();
	if (auctionManager == nullptr) {
		error("null auction manager");
		return;
	}
	ManagedReference<AuctionsMap*> auctionsMap = auctionManager->getAuctionMap();
	if (auctionsMap == nullptr) {
		error("null auctionsMap");
		return;
	}

	statusBox->addMenuItem("Number of Items on vendor: " + String::valueOf(auctionsMap->getVendorItemCount(vendor, false)));

	if (vendorData->isVendorSearchEnabled())
		statusBox->addMenuItem("@player_structure:vendor_search_enabled");
	else
		statusBox->addMenuItem("@player_structure:vendor_search_disabled");

	if (!vendorData->isOnStrike() && !vendorData->isEmpty())
		statusBox->addMenuItem("\\#32CD32Vendor Operating Normally\\#.");

	player->getPlayerObject()->addSuiBox(statusBox);
	player->sendMessage(statusBox->generateMessage());

}

String VendorManager::getTimeString(uint32 timestamp) {

	if( timestamp == 0 ){
		return "";
	}

	String abbrvs[3] = { "minutes", "hours", "days" };

	int intervals[3] = { 60, 3600, 86400 };
	int values[3] = { 0, 0, 0 };

	StringBuffer str;

	for (int i = 2; i > -1; --i) {
		values[i] = floor((float) timestamp / intervals[i]);
		timestamp -= values[i] * intervals[i];

		if (values[i] > 0) {
			if (str.length() > 0){
				str << ", ";
			}

			str << values[i] << " " << abbrvs[i];
		}
	}

	return "(" + str.toString() + ")";
}

void VendorManager::promptDestroyVendor(CreatureObject* player, TangibleObject* vendor) {

	DataObjectComponentReference* data = vendor->getDataObjectComponent();
	if(data == nullptr || data->get() == nullptr || !data->get()->isVendorData()) {
		error("Vendor has no data component");
		return;
	}

	VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());
	if(vendorData == nullptr) {
		error("Vendor has wrong data component");
		return;
	}

	if (vendorData->getOwnerId() != player->getObjectID())
		return;

	SuiMessageBox* destroyBox = new SuiMessageBox(player, SuiWindowType::STRUCTURE_DESTROY_VENDOR_CONFIRM);
	destroyBox->setCallback(new DestroyVendorSuiCallback(player->getZoneServer()));
	destroyBox->setUsingObject(vendor);
	destroyBox->setPromptTitle("@player_structure:destroy_vendor_t"); //DestroyVendor?
	destroyBox->setPromptText("@player_structure:destroy_vendor_d");
	destroyBox->setOkButton(true, "@yes");
	destroyBox->setCancelButton(true, "@no");

	player->getPlayerObject()->addSuiBox(destroyBox);
	player->sendMessage(destroyBox->generateMessage());

}

void VendorManager::promptRenameVendorTo(CreatureObject* player, TangibleObject* vendor) {
	SuiInputBox* input = new SuiInputBox(player, SuiWindowType::STRUCTURE_NAME_VENDOR);
	input->setCallback(new RenameVendorSuiCallback(player->getZoneServer()));
	input->setUsingObject(vendor);
	input->setCancelButton(true, "@cancel");
	input->setPromptTitle("@player_structure:name_t");
	input->setPromptText("@player_structure:name_d");

	player->sendMessage(input->generateMessage());
	player->getPlayerObject()->addSuiBox(input);
}

void VendorManager::destroyVendor(TangibleObject* vendor) {
	DataObjectComponentReference* data = vendor->getDataObjectComponent();
	if (data == nullptr || data->get() == nullptr || !data->get()->isVendorData()) {
		error("Vendor has no data component");
		return;
	}

	VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());
	if (vendorData == nullptr) {
		error("Vendor has wrong data component");
		return;
	}

	ManagedReference<AuctionManager*> auctionManager = server->getZoneServer()->getAuctionManager();
	if (auctionManager == nullptr) {
		error("null auctionManager when deleting vendor");
		return;
	}

	ManagedReference<AuctionsMap*> auctionsMap = auctionManager->getAuctionMap();
	if (auctionsMap == nullptr) {
		error("null auctionsMap");
		return;
	}

	if (vendorData->isRegistered() && vendor->getZone() != nullptr) {
		vendor->getZone()->unregisterObjectWithPlanetaryMap(vendor);
	}

	if (vendorData->isPackedUp()) {
		ManagedReference<VendorControlDevice*> controlDevice = vendor->getControlDevice().get().castTo<VendorControlDevice*>();

		if (controlDevice != nullptr) {
			controlDevice->destroyObjectFromWorld(true);
			controlDevice->destroyObjectFromDatabase(true);
		}
	}

	Locker locker(vendor);

	vendorData->cancelVendorCheckTask();

	vendor->destroyObjectFromWorld(true);
	vendor->destroyObjectFromDatabase(true);

	locker.release();

	auctionsMap->deleteTerminalItems(vendor);
}

void VendorManager::sendRegisterVendorTo(CreatureObject* player, TangibleObject* vendor) {

	SuiListBox* registerBox = new SuiListBox(player, SuiWindowType::STRUCTURE_VENDOR_REGISTER);
	registerBox->setCallback(new RegisterVendorSuiCallback(player->getZoneServer()));
	registerBox->setPromptTitle("@player_structure:vendor_mapcat_t");
	registerBox->setPromptText("@player_structure:vendor_mapcat_d");
	registerBox->setUsingObject(vendor);
	registerBox->setCancelButton(true, "@cancel");

	registerBox->addMenuItem("@player_structure:subcat_armor");
	registerBox->addMenuItem("@player_structure:subcat_clothing");
	registerBox->addMenuItem("@player_structure:subcat_components");
	registerBox->addMenuItem("@player_structure:subcat_droids");
	registerBox->addMenuItem("@player_structure:subcat_equipment");
	registerBox->addMenuItem("@player_structure:subcat_food");
	registerBox->addMenuItem("@player_structure:subcat_housing");
	registerBox->addMenuItem("@player_structure:subcat_medical");
	registerBox->addMenuItem("@player_structure:subcat_pets");
	registerBox->addMenuItem("@player_structure:subcat_resources");
	registerBox->addMenuItem("@player_structure:subcat_ships");
	registerBox->addMenuItem("@player_structure:subcat_tools");
	registerBox->addMenuItem("@player_structure:subcat_weapons");

	player->sendMessage(registerBox->generateMessage());
	player->getPlayerObject()->addSuiBox(registerBox);

}

void VendorManager::handleRegisterVendorCallback(CreatureObject* player, TangibleObject* vendor, const String& planetMapCategoryName) {

	Zone* zone = vendor->getZone();

	if (zone == nullptr)
		return;

	DataObjectComponentReference* data = vendor->getDataObjectComponent();
	if(data == nullptr || data->get() == nullptr || !data->get()->isVendorData()) {
		error("Vendor has no data component");
		return;
	}

	VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());
	if(vendorData == nullptr) {
		error("Vendor has wrong data component");
		return;
	}

	if (vendorData->isEmpty()) {
		player->sendSystemMessage("You cannot register an empty vendor.");
		return;
	}

	Reference<const PlanetMapCategory*> planetMapCategory = TemplateManager::instance()->getPlanetMapCategoryByName("vendor");
	Reference<const PlanetMapCategory*> planetMapSubCategory = TemplateManager::instance()->getPlanetMapCategoryByName("vendor_" + planetMapCategoryName);

	if (planetMapCategory == nullptr || planetMapSubCategory == nullptr)
		return;

	Locker locker(vendor);

	vendor->setPlanetMapCategory(planetMapCategory);
	vendor->setPlanetMapSubCategory(planetMapSubCategory);
	vendorData->setRegistered(true);

	zone->registerObjectWithPlanetaryMap(vendor);

	player->sendSystemMessage("@player_structure:register_vendor_not");

}

void VendorManager::handleUnregisterVendor(CreatureObject* player, TangibleObject* vendor) {
	if (vendor == nullptr)
		return;

	Zone* zone = vendor->getZone();

	if (zone == nullptr)
		return;

	DataObjectComponentReference* data = vendor->getDataObjectComponent();
	if(data == nullptr || data->get() == nullptr || !data->get()->isVendorData()) {
		error("Vendor has no data component");
		return;
	}

	VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());
	if(vendorData == nullptr) {
		error("Vendor has wrong data component");
		return;
	}

	Locker locker(vendor);

	zone->unregisterObjectWithPlanetaryMap(vendor);

	vendorData->setRegistered(false);

	player->sendSystemMessage("@player_structure:unregister_vendor_not");
}

void VendorManager::handleRenameVendor(CreatureObject* player, TangibleObject* vendor, String& name) {
	if (vendor == nullptr)
		return;

	Zone* zone = vendor->getZone();

	if (zone == nullptr)
		return;

	if (!isValidVendorName(name)) {
		player->sendSystemMessage("@player_structure:obscene");
		promptRenameVendorTo(player, vendor);
		return;
	}


	DataObjectComponentReference* data = vendor->getDataObjectComponent();
	if(data == nullptr || data->get() == nullptr || !data->get()->isVendorData()) {
		error("Vendor has no data component");
		return;
	}

	VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());
	if(vendorData == nullptr) {
		error("Vendor has wrong data component");
		return;
	}

	Locker _locker(vendor);
	vendor->setCustomObjectName("Vendor: " + name, true);


	if (vendorData->isRegistered()) {

		vendorData->setRegistered(false);
		zone->unregisterObjectWithPlanetaryMap(vendor);

		player->sendSystemMessage("@player_structure:vendor_rename_unreg");
	} else
		player->sendSystemMessage("@player_structure:vendor_rename");
}

void VendorManager::promptPackupVendor(CreatureObject* player, TangibleObject* vendor) {
	if (!ConfigManager::instance()->getVendorPackupEnabled())
		return;

	DataObjectComponentReference* data = vendor->getDataObjectComponent();

	if (data == nullptr || data->get() == nullptr || !data->get()->isVendorData()) {
		error("Vendor has no data component");
		return;
	}

	VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());

	if (vendorData == nullptr) {
		error("Vendor has wrong data component");
		return;
	}

	if (vendorData->getOwnerId() != player->getObjectID())
		return;

	SuiMessageBox* packupBox = new SuiMessageBox(player, SuiWindowType::STRUCTURE_PACKUP_VENDOR_CONFIRM);
	packupBox->setCallback(new PackupVendorSuiCallback(player->getZoneServer()));
	packupBox->setUsingObject(vendor);
	packupBox->setPromptTitle("@player_structure:packup_vendor_t");
	packupBox->setPromptText("@player_structure:packup_vendor_d");
	packupBox->setOkButton(true, "@yes");
	packupBox->setCancelButton(true, "@no");

	player->getPlayerObject()->addSuiBox(packupBox);
	player->sendMessage(packupBox->generateMessage());
}

void VendorManager::handlePackupVendor(CreatureObject* player, TangibleObject* vendor, bool sendMail) {
	if (!ConfigManager::instance()->getVendorPackupEnabled())
		return;

	if (player == nullptr || vendor == nullptr)
		return;

	Zone* zone = vendor->getZone();

	if (zone == nullptr)
		return;

	DataObjectComponentReference* data = vendor->getDataObjectComponent();

	if (data == nullptr || data->get() == nullptr || !data->get()->isVendorData()) {
		error("Vendor has no data component");
		return;
	}

	VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());

	if (vendorData == nullptr) {
		error("Vendor has wrong data component");
		return;
	}

	ManagedReference<SceneObject*> datapad = player->getSlottedObject("datapad");

	if (datapad == nullptr || datapad->isContainerFullRecursive()) {
		player->sendSystemMessage("Vendor Pack Up failed! Your datapad is full.");
		return;
	} else {
		ManagedReference<VendorControlDevice*> controlDevice = server->getZoneServer()->createObject(STRING_HASHCODE("object/intangible/vendor/generic_vendor_control_device.iff"), 1).castTo<VendorControlDevice*>();

		if (controlDevice == nullptr) {
			error("controlDevice is null in handlePackupVendor");
			return;
		}

		controlDevice->setCustomObjectName(vendor->getCustomObjectName(), true);
		controlDevice->setControlledObject(vendor);
		controlDevice->updateStatus(1);

		if (datapad->transferObject(controlDevice, -1)) {
			datapad->broadcastObject(controlDevice, true);

			vendorData->setInitialized(false);
			vendorData->setRegistered(false);
			vendorData->setPackedUp(true);
			zone->unregisterObjectWithPlanetaryMap(vendor);

			vendor->setControlDevice(controlDevice);
			vendor->destroyObjectFromWorld(false);

			if (sendMail) {
				ManagedReference<ChatManager*> chatManager = server->getZoneServer()->getChatManager();

				if (chatManager != nullptr) {
					UnicodeString subject("@auction:vendor_status_subject");
					StringBuffer msg;

					msg << "Your vendor " << "(" << vendor->getCustomObjectName().toString() << ")"
							<< " has been packed up due to player structure packup."
							<< " It has been placed in your datapad as a Vendor Control Device.";

					chatManager->sendMail("System", subject, msg.toString(), player->getFirstName());
				}
			} else {
				player->sendSystemMessage("Vendor Pack Up Successful! A vendor control device has been placed in your datapad.");
			}

		} else {
			controlDevice->destroyObjectFromDatabase(true);
			error("Could not transfer Control Device in handlePackupVendor");
			return;
		}
	}
}

void VendorManager::handleUnpackVendor(CreatureObject* player, TangibleObject* vendor) {
	if (player == nullptr || vendor == nullptr)
		return;

	if (!player->hasSkill("crafting_artisan_business_03")) {
		player->sendSystemMessage("You no longer have the skill required to place this vendor.");
		return;
	}

	ManagedReference<CellObject*> cell = player->getParent().get().castTo<CellObject*>();

	if (cell == nullptr) {
		player->sendSystemMessage("@player_structure:must_be_in_building");
		return;
	}

	ManagedReference<SceneObject*> parent = cell->getParent().get();

	if (parent == nullptr || !parent->isBuildingObject()) {
		player->sendSystemMessage("@player_structure:must_be_in_building");
		return;
	}

	Zone* zone = parent->getZone();

	if (zone == nullptr)
		return;

	ManagedReference<BuildingObject*> building = cast<BuildingObject*>(parent.get());

	if (building == nullptr)
		return;

	if (!building->isOnAdminList(player) && !building->isOnPermissionList("VENDOR", player)) {
		player->sendSystemMessage("@player_structure:drop_npc_vendor_perm"); // You don't have vendor permissions
		return;
	}

	if (!building->isPublicStructure()) {
		player->sendSystemMessage("@player_structure:vendor_public_only");
		return;
	}

	vendor->initializePosition(player->getPositionX(), player->getPositionZ(), player->getPositionY());
	vendor->setDirection(0);
	vendor->rotate(player->getDirectionAngle());

	if (cell->transferObject(vendor, -1, true, true)) {
		ManagedReference<VendorControlDevice*> controlDevice = vendor->getControlDevice().get().castTo<VendorControlDevice*>();

		if (controlDevice != nullptr) {
			controlDevice->destroyObjectFromWorld(true);
			controlDevice->destroyObjectFromDatabase(true);
		}

		vendor->setControlDevice(nullptr);
	}
}

void VendorManager::promptRelistItems(CreatureObject* player, TangibleObject* vendor) {
	if (!ConfigManager::instance()->getItemRelistEnabled())
		return;

	ManagedReference<AuctionManager*> auctionManager = server->getZoneServer()->getAuctionManager();
	if (auctionManager == nullptr)
		return;

	ManagedReference<AuctionsMap*> auctionsMap = auctionManager->getAuctionMap();
	if (auctionsMap == nullptr)
		return;

	int expiredItemsCount = auctionsMap->getExpiredItemList(vendor, player).size();

	if (expiredItemsCount > 0){
		SuiMessageBox* confirmationWindow = new SuiMessageBox(player, SuiWindowType::NONE);
		confirmationWindow->setCallback(new RelistItemsSuiCallback(player->getZoneServer()));
		confirmationWindow->setUsingObject(vendor);
		confirmationWindow->setPromptTitle("Restock Items");
		confirmationWindow->setPromptText("The service fee for re-listing the " + String::valueOf(expiredItemsCount)  + " items in the stockroom is "
				+ String::valueOf(expiredItemsCount * ConfigManager::instance()->getItemRelistFee()) + " credits.\n\nContinue?");
		confirmationWindow->setOkButton(true, "@yes");
		confirmationWindow->setCancelButton(true, "@no");

		player->getPlayerObject()->addSuiBox(confirmationWindow);
		player->sendMessage(confirmationWindow->generateMessage());
	} else {
		player->sendSystemMessage("There are no items in the stockroom");
	}
}

void VendorManager::handleRelistItems(CreatureObject* player, TangibleObject* vendor) {
	ManagedReference<AuctionManager*> auctionManager = server->getZoneServer()->getAuctionManager();
	if (auctionManager == nullptr)
		return;

	ManagedReference<AuctionsMap*> auctionsMap = auctionManager->getAuctionMap();
	if (auctionsMap == nullptr)
		return;

	Vector<uint64> expiredItems = auctionsMap->getExpiredItemList(vendor, player);

	int availableCredits = player->getBankCredits();
	int expiredItemsCount = expiredItems.size();
	int totalFees = expiredItemsCount * ConfigManager::instance()->getItemRelistFee();

	if (totalFees > availableCredits) {
		player->sendSystemMessage("You do not have enough credits for the relisting fee.");
		return;
	}

	for (int i = 0; i < expiredItemsCount; i++) {
		ManagedReference<AuctionItem*> item = Core::getObjectBroker()->lookUp(expiredItems.get(i)).castTo<AuctionItem*>();

		if (item != nullptr) {
			Locker locker(item);
			int salePrice = item->getPrice() > 99999990 ? 99999990 : item->getPrice();

			if (item->getStatus() == AuctionItem::EXPIRED && item->getOwnerID() == player->getObjectID()) {
				auctionManager->addSaleItem(player, item->getAuctionedItemObjectID(), vendor, item->getItemDescription(), salePrice, AuctionManager::VENDOREXPIREPERIOD, false, false, true);
			}
		}
	}

	// charge the fees after the entire transaction is complete
	player->subtractBankCredits(totalFees);

	// if in a player city add a percentage to the treasury
	ManagedReference<CityRegion*> city = vendor->getCityRegion().get();
	if (city != nullptr) {
		Locker clocker(city);
		city->addToCityTreasury((double)(totalFees * 0.25));
	}

	player->sendSystemMessage("Stockroom items have been relisted");
}
