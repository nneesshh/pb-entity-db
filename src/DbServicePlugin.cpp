//------------------------------------------------------------------------------
//  DbServicePlugin.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "DbService.h"

//#include "base/platform_types.h"

#ifdef _WIN32
#pragma comment(lib, "WS2_32.Lib")
#pragma comment(lib, "DbgHelp.Lib")
#pragma comment(lib, "WinMM.Lib")
#pragma comment(lib, "Version.Lib")
#pragma comment(lib, "ShLwApi.Lib")
#endif

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

extern "C" {
	MY_DB_EXTERN IDbService *
		GetPlugin(db_stub_param_t *param) {
		return new CDbService(param);
	}

	MY_DB_EXTERN IDbService *
		GetClass(db_stub_param_t *param) {
		return GetPlugin(param);
	}
}

/** -- EOF -- **/