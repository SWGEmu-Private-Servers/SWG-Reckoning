/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef CONFIGMANAGER_H_
#define CONFIGMANAGER_H_

// #define DEBUG_CONFIGMANAGER

#include "engine/engine.h"

namespace conf {

	class ConfigDataItem {
		bool asBool;
		String asString;
		lua_Number asNumber;
		Vector <ConfigDataItem *>* asVector = nullptr;
		Vector <String>* asStringVector = nullptr;
		SortedVector <String>* asSortedStringVector = nullptr;
		Vector <int>* asIntVector = nullptr;
		mutable AtomicInteger usageCounter = 0;

		Mutex mutex;

	public:
		ConfigDataItem(lua_Number value);
		ConfigDataItem(int value);
		ConfigDataItem(bool value);
		ConfigDataItem(float value);
		ConfigDataItem(const String& value);
		ConfigDataItem(Vector <ConfigDataItem *>* value);

		~ConfigDataItem();

		inline bool getBool() const {
			usageCounter.increment();
			return asBool;
		}

		inline float getFloat() const {
			usageCounter.increment();
			return (float)asNumber;
		}

		inline int getInt() const {
			usageCounter.increment();
			return (int)asNumber;
		}

		inline const String& getString() const {
			usageCounter.increment();
			return asString;
		}

		const Vector<String>& getStringVector() {
			Locker guard(&mutex);

			if (asStringVector == nullptr) {
				asStringVector = new Vector<String>();

				if (asStringVector == nullptr)
					throw Exception("Failed to allocate Vector<String> in getStringVector()");

				if (asVector == nullptr) {
					asStringVector->add(getString());
				} else {
					for (int i = 0;i < asVector->size(); i++) {
						ConfigDataItem *curItem = asVector->get(i);

						if (curItem == nullptr)
							continue;

						asStringVector->add(curItem->getString());
					}
				}
			}

			return *asStringVector;
		}

		const SortedVector<String>& getSortedStringVector() {
			Locker guard(&mutex);

			if (asSortedStringVector == nullptr) {
				asSortedStringVector = new SortedVector<String>();
				auto entries = getStringVector();

				for (int i = 0;i < entries.size(); i++) {
					asSortedStringVector->add(entries.get(i));
				}
			}

			return *asSortedStringVector;
		}

		const Vector<int>& getIntVector() {
			Locker guard(&mutex);

			if (asIntVector == nullptr) {
				asIntVector = new Vector<int>();

				if (asIntVector == nullptr)
					throw Exception("Failed to allocate Vector<int> in getIntVector()");

				if (asVector == nullptr) {
					asIntVector->add(getInt());
				} else {
					for (int i = 0;i < asVector->size(); i++) {
						ConfigDataItem *curItem = asVector->get(i);

						if (curItem == nullptr)
							continue;

						asIntVector->add(curItem->getInt());
					}
				}
			}

			return *asIntVector;
		}

		void getAsJSON(JSONSerializationType& jsonData);

		String toString() {
			Locker guard(&mutex);

			usageCounter.increment();

			if (asVector == nullptr)
				return String(asString);

			const Vector<String>& elements = getStringVector();

			StringBuffer buf;

			buf << asString << " = {";

			for (int i = 0; i < elements.size(); ++i) {
				buf << (i == 0 ? " " : ", ")
					<< elements.get(i);
			}

			buf << " }";

			return buf.toString();
		}

		inline int getUsageCounter() const {
			return usageCounter;
		}

		inline int resetUsageCounter() {
			int prevCount = usageCounter.get(std::memory_order_acquire);
			usageCounter.set(0, std::memory_order_release);

			return prevCount;
		}

#ifdef DEBUG_CONFIGMANAGER
	private:
		String debugTag;

	public:
		inline void setDebugTag(const String& tag) {
			debugTag = tag;
		}
#endif // DEBUG_CONFIGMANAGER
	};

	class ConfigManager : public Singleton<ConfigManager>, public Object, public Logger {
	protected:
		Lua lua;

		Timer configStartTime;
		bool logChanges = false;

		VectorMap<String, ConfigDataItem *> configData;

		// Each change increments configVersion allowing cached results to auto-reload
		mutable AtomicInteger configVersion = 0;

		ReadWriteLock mutex;

	private:
		ConfigDataItem* findItem(const String& name) const;
		bool updateItem(const String& name, ConfigDataItem* newItem);

		bool parseConfigData(const String& prefix, bool isGlobal = false, int maxDepth = 5);
		bool parseConfigJSONRecursive(const String prefix, JSONSerializationType jsonNode, String& errorMessage, bool updateOnly = true);
		void writeJSONPath(StringTokenizer& tokens, JSONSerializationType& jsonData, const JSONSerializationType& jsonValue);
		bool isSensitiveKey(const String& key);

		void incrementConfigVersion() {
			configVersion.increment();
		}

	public:
		ConfigManager();
		~ConfigManager();

		bool loadConfigData();
		void clearConfigData();
		void cacheHotItems();
		bool parseConfigJSON(const String& jsonString, String& errorMessage, bool updateOnly = true);
		bool parseConfigJSON(const JSONSerializationType jsonData, String& errorMessage, bool updateOnly = true);
		void dumpConfig(bool includeSecure = false);
		bool testConfig(ConfigManager* configManager);

		uint64 getConfigDataAgeMs() const {
			return configStartTime.elapsedMs();
		}

		int getConfigVersion() {
			return configVersion.get();
		}

		// General config functions
		bool contains(const String& name) const;
		int getUsageCounter(const String& name) const;
		int getInt(const String& name, int defaultValue);
		bool getBool(const String& name, bool defaultValue);
		float getFloat(const String& name, float defaultValue);
		const String& getString(const String& name, const String& defaultValue);
		const Vector<String>& getStringVector(const String& name);
		const SortedVector<String>& getSortedStringVector(const String& name);
		const Vector<int>& getIntVector(const String& name);
		bool getAsJSON(const String& target, JSONSerializationType& jsonData);

		bool setNumber(const String& name, lua_Number newValue);
		bool setInt(const String& name, int newValue);
		bool setBool(const String& name, bool newValue);
		bool setFloat(const String& name, float newValue);
		bool setString(const String& name, const String& newValue);
		bool setStringFromFile(const String& name, const String& fileName);

		// Legacy getters
		inline bool getMakeLogin() {
			return getBool("Core3.MakeLogin", true);
		}

		inline bool getMakeZone() {
			return getBool("Core3.MakeZone", true);;
		}

		inline bool getMakePing() {
			return getBool("Core3.MakePing", true);
		}

		inline bool getMakeStatus() {
			return getBool("Core3.MakeStatus", true);
		}

		inline bool getDumpObjFiles() {
			return getBool("Core3.DumpObjFiles", true);
		}

		inline bool shouldUnloadContainers() {
			// Use cached value as this is called often
			static uint32 cachedVersion = 0;
			static bool cachedUnloadContainers;

			if (configVersion.get() > cachedVersion) {
				Locker guard(&mutex);
				cachedUnloadContainers = getBool("Core3.UnloadContainers", true);
				cachedVersion = configVersion.get();
			}

			return cachedUnloadContainers;
		}

		inline bool shouldUseMetrics() {
			// On Basilisk this is called 400/s
			static uint32 cachedVersion = 0;
			static bool cachedUseMetrics;

			if (configVersion.get() > cachedVersion) {
				Locker guard(&mutex);
				cachedUseMetrics = getBool("Core3.UseMetrics", false);
				cachedVersion = configVersion.get();
			}

			return cachedUseMetrics;
		}

		inline bool getPvpMode() {
			// Use cached value as this is a hot item called in:
			//   CreatureObjectImplementation::isAttackableBy
			//   CreatureObjectImplementation::isAggressiveTo
			static uint32 cachedVersion = 0;
			static bool cachedPvpMode;

			if (configVersion.get() > cachedVersion) {
				Locker guard(&mutex);
				cachedPvpMode = getBool("Core3.PvpMode", false);
				cachedVersion = configVersion.get();
			}

			return cachedPvpMode;
		}

		inline bool setPvpMode(bool val) {
			return setBool("Core3.PvpMode", val);
		}

		inline const String& getORBNamingDirectoryAddress() {
			return getString("Core3.ORB", "");
		}

		inline uint16 getORBNamingDirectoryPort() {
			return getInt("Core3.ORBPort", 44419);
		}

		inline const String& getDBHost() {
			return getString("Core3.DBHost", "127.0.0.1");
		}

		inline bool isProgressMonitorActivated() {
			// Use cached value as this a hot item called in lots of loops
			static uint32 cachedVersion = 0;
			static bool cachedProgressMonitors;

			if (configVersion.get() > cachedVersion) {
				Locker guard(&mutex);
				cachedProgressMonitors = getBool("Core3.ProgressMonitors", false);
				cachedVersion = configVersion.get();
			}

			return cachedProgressMonitors;
		}

		inline int getDBPort() {
			return getInt("Core3.DBPort", 3306);
		}

		inline const String& getDBName() {
			return getString("Core3.DBName", "swgemu");
		}

		inline const String& getDBUser() {
			return getString("Core3.DBUser", "root");
		}

		inline const String& getDBPass() {
			return getString("Core3.DBPass", "Gemeni1");
		}

		inline const String& getDBSecret() {
			return getString("Core3.DBSecret", "swgemusecret");
		}

		inline const String& getMantisHost() {
			return getString("Core3.MantisHost", "127.0.0.1");
		}

		inline int getMantisPort() {
			return getInt("Core3.MantisPort", 3306);
		}

		inline const Vector<String>& getTreFiles() {
			return getStringVector("Core3.TreFiles");
		}

		inline const String& getMantisName() {
			return getString("Core3.MantisName", "swgemu");
		}

		inline const String& getMantisUser() {
			return getString("Core3.MantisUser", "root");
		}

		inline const String& getMantisPass() {
			return getString("Core3.MantisPass", "Gemeni1");
		}

		inline const String& getMantisPrefix() {
			return getString("Core3.MantisPrfx", "");
		}

		inline const String& getMessageOfTheDay() {
			return getString("Core3.MOTD", "Welcome to SWGEmu!");
		}

		inline const String& getRevision() {
			return getString("Core3.Revision", "");
		}

		inline const String& getMetricsHost() {
			return getString("Core3.MetricsHost", "127.0.0.1");
		}

		inline const String& getMetricsPrefix() {
			return getString("Core3.MetricsPrefix", "");
		}

		inline int getMetricsPort() {
			return getInt("Core3.MetricsPort", 8125);
		}

		inline const String& getTrePath() {
			return getString("Core3.TrePath", "tre");
		}

		inline uint16 getLoginPort() {
			return getInt("Core3.LoginPort", 44453);
		}

		inline uint16 getStatusPort() {
			return getInt("Core3.StatusPort", 44455);
		}

		inline uint16 getPingPort() {
			return getInt("Core3.PingPort", 44462);
		}

		inline const String& getLoginRequiredVersion() {
			return getString("Core3.LoginRequiredVersion", "20050408-18:00");
		}

		inline int getLoginProcessingThreads() {
			return getInt("Core3.LoginProcessingThreads", 1);
		}

		inline int getLoginAllowedConnections() {
			return getInt("Core3.LoginAllowedConnections", 30);
		}

		inline int getStatusAllowedConnections() {
			return getInt("Core3.StatusAllowedConnections", 100);
		}

		inline int getPingAllowedConnections() {
			return getInt("Core3.PingAllowedConnections", 3000);
		}

		inline int getStatusInterval() {
			return getInt("Core3.StatusInterval", 60);
		}

		inline int getAutoReg() {
			return getBool("Core3.AutoReg", true);
		}

		inline int getZoneProcessingThreads() {
			return getInt("Core3.ZoneProcessingThreads", 10);
		}

		inline int getZoneAllowedConnections() {
			return getInt("Core3.ZoneAllowedConnections", 300);
		}

		inline int getZoneGalaxyID() {
			return getInt("Core3.ZoneGalaxyID", 2);
		}

		inline int getZoneServerPort() {
			return getInt("Core3.ZoneServerPort", 0);
		}

		const SortedVector<String>& getEnabledZones() {
			return getSortedStringVector("Core3.ZonesEnabled");
		}

		inline int getPurgeDeletedCharacters() {
			return getInt("Core3.PurgeDeletedCharacters", 10); // In minutes
		}

		inline int getMaxNavMeshJobs() {
			return getInt("Core3.MaxNavMeshJobs", 6);
		}

		inline int getMaxAuctionSearchJobs() {
			return getInt("Core3.MaxAuctionSearchJobs", 1);
		}

		inline const String& getLogFile() {
			return getString("Core3.LogFile", "log/core3.log");
		}

		inline int getLogFileLevel() {
			return getInt("Core3.LogFileLevel", Logger::INFO);
		}

		inline int getRotateLogSizeMB() {
			return getInt("Core3.RotateLogSizeMB", 100);
		}

		inline bool getRotateLogAtStart() {
			return getBool("Core3.RotateLogAtStart", false);
		}

		inline void setProgressMonitors(bool val) {
			setBool("Core3.ProgressMonitors", val);
		}

		inline const String& getTermsOfService() {
			return getString("Core3.TermsOfService", "");
		}

		inline int getTermsOfServiceVersion() {
			return getInt("Core3.TermsOfServiceVersion", 0);
		}

		inline bool getJsonLogOutput() {
			return getBool("Core3.LogJSON", false);
		}

		inline bool getSyncLogOutput() {
			return getBool("Core3.LogSync", false);
		}

		inline bool getLuaLogJSON() {
			return getBool("Core3.LuaLogJSON", false);
		}

		inline bool getPathfinderLogJSON() {
			return getBool("Core3.PathfinderLogJSON", false);
		}

		inline int getCleanupMailCount() {
			return getInt("Core3.CleanupMailCount", 25000);
		}

		inline int getRESTPort() {
			return getInt("Core3.RESTServerPort", 0);
		}

		inline int getPlayerLogLevel() {
			return getInt("Core3.PlayerLogLevel", Logger::INFO);
		}

		inline int getMaxLogLines() {
			return getInt("Core3.MaxLogLines", 1000000);
		}

		////////////////////////
		//Custom Server Settings
		////////////////////////

		//Login Error Messages
		inline const String& getRevisionError() {
			return getString("Core3.revisionError", "The client you are attempting to connect with does not match that required by the server.");
		}
		inline const String& getRegisterError() {
			return getString("Core3.registerError", "Automatic registration is currently disabled. Please contact the administrators of the server in order to get an authorized account.");
		}
		inline const String& getActivateError() {
			return getString("Core3.activateError", "The server administrators have disabled your account.");
		}
		inline const String& getPasswordError() {
			return getString("Core3.passwordError", "The password you entered was incorrect.");
		}
		inline const String& getBannedError() {
			return getString("Core3.bannedError", "Your account has been banned from the server by the administrators.\n\n");
		}

		//Character Creation
		inline int getCharactersPerGalaxy() {
			return getInt("Core3.charactersPerGalaxy", 10);
		}
		inline const String& getCharactersPerGalaxyError() {
			return getString("Core3.charactersPerGalaxyError", "You are limited to 10 characters per galaxy.");
		}
		inline int getCreateTime() {
			return getInt("Core3.createTime", 86400000);
		}
		inline const String& getCreateTimeError() {
			return getString("Core3.createTimeError", "You are limited to creating one character every 24 hours. Attempting to create another character or deleting your character before the 24 hour timer expires will reset the timer.");
		}
		inline const String& getCreateSuiTitle() {
			return getString("Core3.createSuiTitle", "PLEASE NOTE");
		}
		inline const String& getCreateSuiMessage() {
			return getString("Core3.createSuiMessage", "You are limited to creating one character every 24 hours. Attempting to create another character or deleting your character before the 24 hour timer expires will reset the timer.");
		}

		//Status Server Population Display
		inline bool getPopDisplayEnabled() {
			return getBool("Core3.popDisplayEnabled", true);
		}
		inline bool getLoadDisplayEnabled() {
			return getBool("Core3.loadDisplayEnabled", false);
		}
		inline int getMediumLoad() {
			return getInt("Core3.mediumLoad", 0);
		}
		inline int getHeavyLoad() {
			return getInt("Core3.heavyLoad", 0);
		}
		inline int getVeryHeavyLoad() {
			return getInt("Core3.veryHeavyLoad", 0);
		}

		//Chat Logging
		inline bool getChatLoggingEnabled() {
			return getBool("Core3.chatLoggingEnabled", false);
		}

		//General Chat
		inline bool getGeneralChatEnabled() {
			return getBool("Core3.generalChatEnabled", false);
		}

		//Custom Chat
		inline bool getCustomRoomsEnabled() {
			return getBool("Core3.customRoomsEnabled", false);
		}

		//Enhancement Terminal
		inline bool getEnhancementTerminalEnabled() {
			return getBool("Core3.enhancementTerminalEnabled", false);
		}
		inline int getBuffCost() {
			return getInt("Core3.buffCost", 500);
		}
		inline int getCleanseCost() {
			return getInt("Core3.cleanseCost", 500);
		}
		inline int getRemoveCost() {
			return getInt("Core3.removeCost", 500);
		}
		inline int getTerminalPerformanceBuff() {
			return getInt("Core3.terminalPerformanceBuff", 500);
		}
		inline int getTerminalMedicalBuff() {
			return getInt("Core3.terminalMedicalBuff", 500);
		}
		inline int getTerminalPerformanceDuration() {
			return getInt("Core3.terminalPerformaceDuration", 3600);
		}
		inline int getTerminalMedicalDuration() {
			return getInt("Core3.terminalMedicalDuration", 3600);
		}

		//Character Builder
		inline bool getCharacterBuilderEnabled() {
			return getBool("Core3.characterBuilderEnabled", false);
		}
		inline int getBuilderPerformanceBuff() {
			return getInt("Core3.builderPerformanceBuff", 1000);
		}
		inline int getBuilderMedicalBuff() {
			return getInt("Core3.builderMedicalBuff", 1900);
		}
		inline int getBuilderPerformanceDuration() {
			return getInt("Core3.builderPerformanceDuration", 7200);
		}
		inline int getBuilderMedicalDuration() {
			return getInt("Core3.builderMedicalDuration", 7200);
		}

		//Jedi Unlock
		inline bool getCustomUnlockEnabled() {
			return getBool("Core3.customUnlockEnabled", false);
		}
		inline int getRandJedi1() {
			return getInt("Core3.randJedi1", 0);
		}
		inline int getRandJedi2() {
			return getInt("Core3.randJedi2", 0);
		}
		inline int getRandJedi3() {
			return getInt("Core3.randJedi3", 0);
		}
		inline int getRandJedi4() {
			return getInt("Core3.randJedi4", 0);
		}
		inline int getRandJedi5() {
			return getInt("Core3.randJedi5", 0);
		}
		inline int getRandJedi6() {
			return getInt("Core3.randJedi6", 0);
		}

		//Shrine Jedi Progression Check
		inline bool getShrineProgressionEnabled() {
			return getBool("Core3.shrineProgressionEnabled", false);
		}

		//Galaxy Jedi Unlock Message
		inline bool getUnlockMessageEnabled() {
			return getBool("Core3.unlockMessageEnabled", false);
		}
		inline const String& getUnlockMessage() {
			return getString("Core3.unlockMessage", "IMPERIAL COMMUNICATION FROM THE REGIONAL GOVERNOR: Lord Vader has detected a vergence in the Force.\n Be on the lookout for any suspicious persons displaying unique or odd abilities. Lord Vader authorizes all citizens to use deadly force to eliminate this threat to the Empire.");
		}

		//Fight Clubbing Prevention
		inline bool getFightClubbingPreventionEnabled() {
			return getBool("Core3.fightClubbingPreventionEnabled", false);
		}

		//Bounty Hunter PvP Traps
		inline bool getPvpTrapEnabled() {
			return getBool("Core3.pvpTrapEnabled", false);
		}
		inline int getPvpTrapDuration() {
			return getInt("Core3.pvpTrapDuration", 0);
		}
		inline int getPvpTrapImmunityDuration() {
			return getInt("Core3.pvpTrapImmunityDuration", 0);
		}

		//Spawn Protection
		inline bool getSpawnProtectionEnabled() {
			return getBool("Core3.spawnProtectionEnabled", false);
		}
		inline int getSpawnProtectionTime() {
			return getInt("Core3.spawnProtectionTime", 0);
		}

		//TEF System
		inline bool getTefEnabled() {
			return getBool("Core3.tefEnabled", false);
		}
		inline bool getCityTefEnabled() {
			return getBool("Core3.cityTefEnabled", false);
		}

		//City Alignment
		inline int getCityAlignRankReq() {
			return getInt("Core3.cityAlignRankReq", 0);
		}

		//Player Bounty
		inline bool getPlayerBountyEnabled() {
			return getBool("Core3.playerBountyEnabled", false);
		}

		//Faction Base TEF Scanner
		inline bool getTefScannersEnabled() {
			return getBool("Core3.tefScannersEnabled", false);
		}

		//Increased House Storage
		inline bool getIncreasedStorageEnabled() {
			return getBool("Core3.increasedStorageEnabled", false);
		}
		inline int getTwoLots() {
			return getInt("Core3.twoLots", 0);
		}
		inline int getThreeLots() {
			return getInt("Core3.threeLots", 0);
		}
		inline int getFourLots() {
			return getInt("Core3.fourLots", 0);
		}
		inline int getFiveLots() {
			return getInt("Core3.fiveLots", 0);
		}

		//Post 14.1 Furniture Rotation
		inline bool getAllRotationEnabled() {
			return getBool("Core3.allRotationEnabled", false);
		}

		//Crafted Item Renaming
		inline bool getItemRenamingEnabled() {
			return getBool("Core3.itemRenamingEnabled", false);
		}

		//Event System
		inline bool getEventSystemEnabled() {
			return getBool("Core3.eventSystemEnabled", false);
		}
		inline bool getAwardBadgeEnabled() {
			return getBool("Core3.awardBadgeEnabled", false);
		}
		inline bool getAwardItemEnabled() {
			return getBool("Core3.awardItemEnabled", false);
		}
		inline int getAwardedBadge1() {
			return getInt("Core3.awardedBadge1", 0);
		}
		inline int getAwardedBadge2() {
			return getInt("Core3.awardedBadge2", 0);
		}
		inline const String& getAwardedItem1() {
			return getString("Core3.awardedItem1", "");
		}
		inline const String& getAwardedItem2() {
			return getString("Core3.awardedItem2", "");
		}

		//Reckoning Credits
		inline bool getReckoningCreditsEnabled() {
			return getBool("Core3.reckoningCreditsEnabled", false);
		}
		inline int getReckoningCreditsDropRate() {
			return getInt("Core3.reckoningCreditsDropRate", 0);
		}

		//Reckoning Reward Npc
		inline int getRenameCost() {
			return getInt("Core3.renameCost", 0);
		}

		//Update Galaxy Harvester
		inline bool getUpdateGhEnabled() {
			return getBool("Core3.updateGhEnabled", false);
		}

		//Structure Packup
		inline bool getStructurePackupEnabled() {
			return getBool("Core3.structurePackupEnabled", false);
		}

		//Inactive Structure Packup
		inline bool getInactivePackupEnabled() {
			return getBool("Core3.inactivePackupEnabled", false);
		}
		inline int getInactivePackupDays() {
			return getInt("Core3.inactivePackupDays", 0);
		}

		//Vendor Packup
		inline bool getVendorPackupEnabled() {
			return getBool("Core3.vendorPackupEnabled", false);
		}

		//Character Stat Tracking
		inline bool getCharacterStatsEnabled() {
			return getBool("Core3.characterStatsEnabled", false);
		}

		//Vendor Expired Item Relist
		inline bool getItemRelistEnabled() {
			return getBool("Core3.itemRelistEnabled", false);
		}
		inline int getItemRelistFee() {
			return getInt("Core3.itemRelistFee", 0);
		}

		//Structure Wall Purchase
		inline bool getWallPurchaseEnabled() {
			return getBool("Core3.wallPurchaseEnabled", false);
		}
		inline int getWallPurchaseCost() {
			return getInt("Core3.wallPurchaseCost", 0);
		}
		inline const Vector<String>& getStructureWalls() {
			return getStringVector("Core3.StructureWalls");
		}

		inline int getSessionStatsSeconds() {
			static uint32 cachedVersion = 0;
			static int cachedSessionStatsSeconds;

			if (configVersion.get() > cachedVersion) {
				Locker guard(&mutex);
				cachedSessionStatsSeconds = getInt("Core3.SessionStatsSeconds", 3600);
				cachedVersion = configVersion.get();
			}

			return cachedSessionStatsSeconds;
		}

		inline int getOnlineLogSeconds() {
			return getInt("Core3.OnlineLogSeconds", 300);
		}

		inline int getOnlineLogSize() {
			static uint32 cachedVersion = 0;
			static int cachedOnlineLogSize;

			if (configVersion.get() > cachedVersion) {
				Locker guard(&mutex);
				cachedOnlineLogSize = getInt("Core3.OnlineLogSize", 100000000);
				cachedVersion = configVersion.get();
			}

			return cachedOnlineLogSize;
		}

		inline String getNoTradeMessage() {
			static uint32 cachedVersion = 0;
			static String cachedNoTradeMessage;

			if (configVersion.get() > cachedVersion) {
				Locker guard(&mutex);
				cachedNoTradeMessage = getString("Core3.TangibleObject.NoTradeMessage", "");
				cachedVersion = configVersion.get();
			}

			return cachedNoTradeMessage;
		}

		inline String getForceNoTradeMessage() {
			static uint32 cachedVersion = 0;
			static String cachedForceNoTradeMessage;

			if (configVersion.get() > cachedVersion) {
				Locker guard(&mutex);
				cachedForceNoTradeMessage = getString("Core3.TangibleObject.ForceNoTradeMessage", "");
				cachedVersion = configVersion.get();
			}

			return cachedForceNoTradeMessage;
		}

		inline String getForceNoTradeADKMessage() {
			static uint32 cachedVersion = 0;
			static String cachedForceNoTradeADKMessage;

			if (configVersion.get() > cachedVersion) {
				Locker guard(&mutex);
				cachedForceNoTradeADKMessage = getString("Core3.TangibleObject.ForceNoTradeADKMessage", "");
				cachedVersion = configVersion.get();
			}

			return cachedForceNoTradeADKMessage;
		}
	};
}

using namespace conf;

#endif // #ifndef CONFIGMANAGER_H_
