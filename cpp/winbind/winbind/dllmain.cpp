// dllmain.cpp : Defines the entry point for the DLL application.
#include <cstdio>
#include <memory>
#include <string>
#include "stdafx.h"
#include "supp/timer.h"
#include "core/armor_up.h"

std::unique_ptr<int> a;
std::string result;
Timer timer;

extern "C"
{
	__declspec(dllexport) const char *DisplayHelloFromDLL()
	{
		result = std::to_string(timer.Toc());
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
		timer.Tic();
		a.reset(new int(24));
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		a.release();
		break;
	}
	return TRUE;
}

