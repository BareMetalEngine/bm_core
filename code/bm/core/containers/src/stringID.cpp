/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "stringView.h"
#include "stringID.h"
#include "stringIDPrv.h"

BEGIN_INFERNO_NAMESPACE()

//---

struct StringIDGlobalState
{
	SpinLock globalMapLock;
	prv::StringIDMap globalMap;

	static StringIDGlobalState& GetInstance()
	{
		static StringIDGlobalState theGlobalState;
		return theGlobalState;
	}
};

static TYPE_TLS prv::StringIDMap* GStringIDLocalMap = nullptr;

static StringID GEmptyStringID;

const char* StringID::st_StringTable[1024];

StringID StringID::EMPTY()
{
    return GEmptyStringID;
}

StringID StringID::Find(StringView txt)
{
	if (!txt)
		return StringID();

	const auto hash = CalcHash(txt);

	// allocate local map on first use
	if (!GStringIDLocalMap)
		GStringIDLocalMap = new prv::StringIDMap();

	// find in local map
	auto index = GStringIDLocalMap->find(hash, txt.data(), txt.length());
	if (index)
		return StringID(index);

	// get global state
	auto& globalState = StringIDGlobalState::GetInstance();

	// find in global map
	{
		auto lock = CreateLock(globalState.globalMapLock);
		index = globalState.globalMap.find(hash, txt.data(), txt.length());
	}

	// if found in global map store in local as well
	if (index)
		GStringIDLocalMap->insert(hash, index);

	return StringID(index);
}

StringIDIndex StringID::Alloc(StringView txt)
{
	if (!txt)
		return 0;

	const auto hash = CalcHash(txt);

	// allocate local map on first use
	if (!GStringIDLocalMap)
		GStringIDLocalMap = new prv::StringIDMap();

	// find in local map
	auto index = GStringIDLocalMap->find(hash, txt.data(), txt.length());
	if (index)
		return index;

	// get global state
	auto& globalState = StringIDGlobalState::GetInstance();

	// find in global map
	{
		auto lock = CreateLock(globalState.globalMapLock);

		uint32_t freeGlobalBucket = 0;
		index = globalState.globalMap.find(hash, txt.data(), txt.length());

		// allocate in global map
		if (index == 0)
		{
			// place the string in the storage
			index = prv::StringIDDataStorage::GetInstance().place(txt);

			// store in global map
			globalState.globalMap.insert(hash, index);
		}
	}

	// place in local map as well
	GStringIDLocalMap->insert(hash, index);
	return index;
}

END_INFERNO_NAMESPACE()
