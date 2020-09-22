/*
 * ReckoningLogManager.cpp
 */

#include "server/zone/managers/log/ReckoningLogManager.h"
#include "server/zone/managers/log/LogType.h"

void ReckoningLogManagerImplementation::initialize() {
	//Staff Command Log
	staffCommandLog.setLoggingName("Staff Command Log");
	staffCommandLog.setFileLogger("log/staff/commands.log", true);
	staffCommandLog.setLogging(true);

	//Staff Tip Log
	staffTipLog.setLoggingName("Staff Tip Log");
	staffTipLog.setFileLogger("log/staff/tips.log", true);
	staffTipLog.setLogging(true);

	//Staff Trade Log
	staffTradeLog.setLoggingName("Staff Trade Log");
	staffTradeLog.setFileLogger("log/staff/trade.log", true);
	staffTradeLog.setLogging(true);

	//Player Tip Log
	playerTipLog.setLoggingName("Player Tip Log");
	playerTipLog.setFileLogger("log/player/tips.log", true);
	playerTipLog.setLogging(true);

	//Player Large Tip Log
	playerLargeTipLog.setLoggingName("Player Large Tip Log");
	playerLargeTipLog.setFileLogger("log/player/largeTips.log", true);
	playerLargeTipLog.setLogging(true);

	//Player Trade Log
	playerTradeLog.setLoggingName("Player Trade Log");
	playerTradeLog.setFileLogger("log/player/trade.log", true);
	playerTradeLog.setLogging(true);

	//Player Credit Trade Log
	playerCreditTradeLog.setLoggingName("Player Credit Trade Log");
	playerCreditTradeLog.setFileLogger("log/player/creditTrade.log", true);
	playerCreditTradeLog.setLogging(true);

	//Player Large Credit Trade Log
	playerLargeCreditTradeLog.setLoggingName("Player Large Credit Trade Log");
	playerLargeCreditTradeLog.setFileLogger("log/player/largeCreditTrade.log", true);
	playerLargeCreditTradeLog.setLogging(true);

	//FRS Kill Log
	frsKillLog.setLoggingName("FRS Kill Log");
	frsKillLog.setFileLogger("log/player/frsKills.log", true);
	frsKillLog.setLogging(true);

	//FRS Fight Club Log
	frsFightClubLog.setLoggingName("FRS Fight Club Log");
	frsFightClubLog.setFileLogger("log/player/frsFightClub.log", true);
	frsFightClubLog.setLogging(true);

	//Structure Packup Log
	structurePackupLog.setLoggingName("Structure Packup Log");
	structurePackupLog.setFileLogger("log/structure/packup.log", true);
	structurePackupLog.setLogging(true);

	//System Structure Packup Log
	systemStructurePackupLog.setLoggingName("System Structure Packup Log");
	systemStructurePackupLog.setFileLogger("log/structure/systemPackup.log", true);
	systemStructurePackupLog.setLogging(true);

	//Structure Destroy Log
	structureDestroyLog.setLoggingName("Structure Destroy Log");
	structureDestroyLog.setFileLogger("log/structure/structureDestroy.log", true);
	structureDestroyLog.setLogging(true);

	//Auction Chat Log
	auctionChatLog.setLoggingName("Auction Chat");
	auctionChatLog.setFileLogger("log/chat/auction.log", true);
	auctionChatLog.setLogging(true);

	//General Chat Log
	generalChatLog.setLoggingName("General Chat");
	generalChatLog.setFileLogger("log/chat/general.log", true);
	generalChatLog.setLogging(true);

	//Planet Chat Log
	planetChatLog.setLoggingName("Planet Chat");
	planetChatLog.setFileLogger("log/chat/planet.log", true);
	planetChatLog.setLogging(true);

	//Spatial Chat Log
	spatialChatLog.setLoggingName("Spatial Chat");
	spatialChatLog.setFileLogger("log/chat/spatial.log", true);
	spatialChatLog.setLogging(true);

	//PvP Chat Log
	pvpChatLog.setLoggingName("PvP Chat");
	pvpChatLog.setFileLogger("log/chat/pvp.log", true);
	pvpChatLog.setLogging(true);

	//PvE Chat Log
	pveChatLog.setLoggingName("PvE Chat");
	pveChatLog.setFileLogger("log/chat/pve.log", true);
	pveChatLog.setLogging(true);

	//Services Chat Log
	servicesChatLog.setLoggingName("Services Chat");
	servicesChatLog.setFileLogger("log/chat/services.log", true);
	servicesChatLog.setLogging(true);

	//Imperial Chat Log
	imperialChatLog.setLoggingName("Imperial Chat");
	imperialChatLog.setFileLogger("log/chat/imperial.log", true);
	imperialChatLog.setLogging(true);

	//Rebel Chat Log
	rebelChatLog.setLoggingName("Rebel Chat");
	rebelChatLog.setFileLogger("log/chat/rebel.log", true);
	rebelChatLog.setLogging(true);

	//Custom Chat Log
	customChatLog.setLoggingName("Custom Chat");
	customChatLog.setFileLogger("log/chat/custom.log", true);
	customChatLog.setLogging(true);
}

void ReckoningLogManagerImplementation::logAction(int logType, const String& message) {
	switch (logType) {
		case LogType::STAFFCOMMAND:
			staffCommandLog.info(message);
			break;
		case LogType::STAFFTIP:
			staffTipLog.info(message);
			break;
		case LogType::STAFFTRADE:
			staffTradeLog.info(message);
			break;
		case LogType::STAFFCREDITTRADE:
			staffCreditTradeLog.info(message);
			break;
		case LogType::PLAYERTIP:
			playerTipLog.info(message);
			break;
		case LogType::PLAYERLARGETIP:
			playerLargeTipLog.info(message);
			break;
		case LogType::PLAYERTRADE:
			playerTradeLog.info(message);
			break;
		case LogType::PLAYERCREDITTRADE:
			playerCreditTradeLog.info(message);
			break;
		case LogType::PLAYERLARGECREDITTRADE:
			playerLargeCreditTradeLog.info(message);
			break;
		case LogType::FRSKILL:
			frsKillLog.info(message);
			break;
		case LogType::FRSFIGHTCLUB:
			frsFightClubLog.info(message);
			break;
		case LogType::STRUCTUREPACKUP:
			structurePackupLog.info(message);
			break;
		case LogType::SYSTEMSTRUCTUREPACKUP:
			systemStructurePackupLog.info(message);
			break;
		case LogType::STRUCTUREDESTROY:
			structureDestroyLog.info(message);
			break;
		case LogType::AUCTIONCHAT:
			auctionChatLog.info(message);
			break;
		case LogType::GENERALCHAT:
			generalChatLog.info(message);
			break;
		case LogType::PLANETCHAT:
			planetChatLog.info(message);
			break;
		case LogType::SPATIALCHAT:
			spatialChatLog.info(message);
			break;
		case LogType::PVPCHAT:
			pvpChatLog.info(message);
			break;
		case LogType::PVECHAT:
			pveChatLog.info(message);
			break;
		case LogType::SERVICESCHAT:
			servicesChatLog.info(message);
			break;
		case LogType::IMPERIALCHAT:
			imperialChatLog.info(message);
			break;
		case LogType::REBELCHAT:
			rebelChatLog.info(message);
			break;
		case LogType::CUSTOMCHAT:
			customChatLog.info(message);
			break;
		default:
			break;
	}
}
