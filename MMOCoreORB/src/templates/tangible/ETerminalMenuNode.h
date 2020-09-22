#ifndef ETERMINALMENUNODE_H_
#define ETERMINALMENUNODE_H_

#include "engine/lua/LuaObject.h"

class ETerminalMenuNode : public Object {
	WeakReference<ETerminalMenuNode*> parentNode;

	String displayName;
	String templatePath;
	uint32 templateCRC;

	SortedVector<Reference<ETerminalMenuNode*> > childNodes;

public:
	ETerminalMenuNode(const String& name) {
		displayName = name;
		parentNode = nullptr;
		templateCRC = 0;
		childNodes.setInsertPlan(SortedVector<ETerminalMenuNode*>::NO_DUPLICATE);
	}

	ETerminalMenuNode(const String& name, const String& tplPath) {
		parentNode = nullptr;
		displayName = name;
		templatePath = tplPath;
		templateCRC = tplPath.hashCode();
		childNodes.setInsertPlan(SortedVector<ETerminalMenuNode*>::NO_DUPLICATE);
	}

	int readLuaObject(LuaObject& luaObject, bool recursive) {
		int tableSize = luaObject.getTableSize();

		if (tableSize % 2 != 0)
			return 0;

		for (int i = 1; i <= tableSize; i += 2) {
			String title = luaObject.getStringAt(i);

			lua_State* L = luaObject.getLuaState();
			lua_rawgeti(L, -1, i + 1);
			LuaObject a(L);

			ETerminalMenuNode* node = new ETerminalMenuNode(title);
			node->setParentNode(this);

			if (a.isValidTable()) {
				node->readLuaObject(a, true);
				a.pop();
			} else {
				a.pop();
				node->setTemplatePath(luaObject.getStringAt(i + 1));
			}

			childNodes.put(node);
		}

		return 0;
	}

	int compareTo(ETerminalMenuNode* obj) {
		return displayName.compareTo(obj->getDisplayName());
	}

	inline void setTemplatePath(const String& tplPath) {
		templatePath = tplPath;
		templateCRC = tplPath.hashCode();
	}

	inline void setParentNode(ETerminalMenuNode* parent) {
		parentNode = parent;
	}

	inline ETerminalMenuNode* getParentNode() {
		return parentNode.get().get();
	}

	inline bool hasParentNode() {
		return parentNode.get() != nullptr;
	}

	inline bool hasChildNodes() {
		return childNodes.size() > 0;
	}

	inline String& getDisplayName() {
		return displayName;
	}

	inline String& getTemplatePath() {
		return templatePath;
	}

	inline uint32 getTemplateCRC() {
		return templateCRC;
	}

	inline int getChildNodeSize() {
		return childNodes.size();
	}

	inline ETerminalMenuNode* getChildNodeAt(int index) {
		if (childNodes.size() < index + 1 || index < 0)
			return nullptr;

		return childNodes.get(index);
	}
};

#endif /* ETERMINALMENUNODE_H_ */
