#pragma once
//------------------------------------------------------------------------------
/**
@class IDbStubParam

(C) 2016 n.lee
*/
#include <string>

#include "../log/StdLog.h"

struct db_stub_param_t {
	std::string _dbServerId;
	std::string _dbUid;
	std::string _dbPwd;
	std::string _dbName;
	int _poolSize;

	StdLog *_refLog;
};

//------------------------------------------------------------------------------
/**
@brief IDbStubParam
*/
class IDbStubParam {
public:
	virtual ~IDbStubParam() { }

	virtual db_stub_param_t&	DbStubParam() = 0;

};

/*EOF*/