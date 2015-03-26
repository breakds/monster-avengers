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
std::unique_ptr<int> a;
std::string result;
Timer timer;
std::unique_ptr<ArmorUp> armor_up;

extern "C"
{
	__declspec(dllexport) const char *DisplayHelloFromDLL()
	{
		result = std::to_string(timer.Toc());
		return result.c_str();
	}

	__declspec(dllexport) void Initialize(const char *dataset) {
		armor_up.reset(new ArmorUp(dataset));
		armor_up->Summarize();
	}
	__declspec(dllexport) void DoSearch(const wchar_t* text) {
		std::wstring query_text = text;
		Query query;
		CHECK_SUCCESS(Query::Parse(query_text, &query));
		query.DebugPrint();
		armor_up->Search<SCREEN>(query);
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
		setlocale(LC_ALL, "en_US.UTF-8");
		timer.Tic();
		a.reset(new int(24));
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		a.release();
		armor_up.release();
		break;
	}
	return TRUE;
}

