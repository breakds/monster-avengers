// dllmain.cpp : Defines the entry point for the DLL application.
#include <cstdio>
#include <memory>
#include <string>
#include <locale>
#include "stdafx.h"
#include "supp/timer.h"
#include "core/armor_up.h"
#include "utils/query.h"

using namespace monster_avengers;
std::string result;
std::unique_ptr<ArmorUp> armor_up;

extern "C"
{
	__declspec(dllexport) void Initialize(const char *dataset) {
		armor_up.reset(new ArmorUp(dataset));
		armor_up->Summarize();
	}
	__declspec(dllexport) const char *DoSearch(const wchar_t* text) {
		std::wstring query_text = text;
		Query query;
		CHECK_SUCCESS(Query::Parse(query_text, &query));
		query.DebugPrint();
		result = armor_up->SearchEncoded(query);
		return result.c_str();
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

