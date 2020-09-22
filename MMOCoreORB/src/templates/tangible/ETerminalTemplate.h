#ifndef ETERMINALTEMPLATE_H_
#define ETERMINALTEMPLATE_H_

#include "templates/SharedTangibleObjectTemplate.h"
#include "templates/tangible/ETerminalMenuNode.h"

class ETerminalTemplate : public SharedTangibleObjectTemplate {
	Reference<ETerminalMenuNode*> rootNode;

public:
	ETerminalTemplate() : rootNode(nullptr) {
	}

	~ETerminalTemplate() {
		if (rootNode != nullptr) {
			//delete rootNode;
			rootNode = nullptr;
		}
	}

	void readObject(LuaObject* templateData) {
		SharedTangibleObjectTemplate::readObject(templateData);

		LuaObject luaItemList = templateData->getObjectField("itemList");

		//Ensure that the luaItemList root level is of an even order.
		if (luaItemList.getTableSize() % 2 != 0) {
			System::out << "[ETerminalTemplate] Dimension mismatch in itemList. Item count must be a multiple of 2." << endl;
			luaItemList.pop();
			return;
		}

		rootNode = new ETerminalMenuNode("root");
		rootNode->readLuaObject(luaItemList, true);

		luaItemList.pop();
    }

    inline ETerminalMenuNode* getItemList() const {
        return rootNode;
    }
};

#endif /* ETERMINALTEMPLATE_H_ */
