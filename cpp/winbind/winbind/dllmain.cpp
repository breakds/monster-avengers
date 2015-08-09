// dllmain.cpp : Defines the entry point for the DLL application.
#include <cstdio>
#include <memory>
#include <string>
#include <locale>
#include "stdafx.h"
#include "supp/timer.h"
#include "core/armor_up.h"
#include "utils/query.h"
#include "utils/formatter.h"

using namespace monster_avengers;
std::vector<ArmorSet> result;
std::unique_ptr<DexFormatter> formatter;
std::unique_ptr<ArmorUp> armor_up;
std::string text_response;

extern "C"
{
	__declspec(dllexport) void Initialize(const char *dataset) {
		Data::LoadBinary(dataset);
		armor_up.reset(new ArmorUp);
		formatter.reset(new DexFormatter(&armor_up->GetArsenal()));
	}

	__declspec(dllexport) const char *DoSearch(const wchar_t* text) {
		std::wstring query_text = text;
		Query query;
		CHECK_SUCCESS(Query::Parse(query_text, &query));
		result = std::move(armor_up->Search(query));
		text_response = formatter->StringBatchFormat(result);
		return text_response.c_str();
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
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

