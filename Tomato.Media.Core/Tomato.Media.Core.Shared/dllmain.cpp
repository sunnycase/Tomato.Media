#include "pch.h"

//BOOL WINAPI DllMain(__in_opt HINSTANCE hInstance, __in DWORD dwReason, __in_opt LPVOID lpReserved)
//{
//	if (DLL_PROCESS_ATTACH == dwReason)
//	{
//		//DisableThreadLibraryCalls(hInstance);
//
//		//Microsoft::WRL::Module<Microsoft::WRL::InProc>::GetModule().Create();
//	}
//	else if (DLL_PROCESS_DETACH == dwReason)
//	{
//		//Microsoft::WRL::Module<Microsoft::WRL::InProc>::GetModule().Terminate();
//	}
//
//	return TRUE;
//}

//STDAPI DllGetActivationFactory(_In_ HSTRING activatibleClassId, _COM_Outptr_ IActivationFactory** factory)
//{
//	auto &module = Microsoft::WRL::Module<Microsoft::WRL::InProc>::GetModule();
//	return module.GetActivationFactory(activatibleClassId, factory);
//}
//
//STDAPI DllCanUnloadNow()
//{
//	auto &module = Microsoft::WRL::Module<Microsoft::WRL::InProc>::GetModule();
//	return module.Terminate() ? S_OK : S_FALSE;
//}
//
//STDAPI DllGetClassObject(__in REFCLSID rclsid, __in REFIID riid, __deref_out LPVOID FAR* ppv)
//{
//	auto &module = Microsoft::WRL::Module<Microsoft::WRL::InProc>::GetModule();
//	return module.GetClassObject(rclsid, riid, ppv);
//}