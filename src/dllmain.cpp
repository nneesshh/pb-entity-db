#include <objbase.h>
#include <mysql.h>

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, void* lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		//std::cout << "Dll is attached!" << std::endl;
		break;
	
	case DLL_PROCESS_DETACH:
		//std::cout << "Dll is detached!" << std::endl;
		mysql_library_end();
		break;
	}
	return true;
}
