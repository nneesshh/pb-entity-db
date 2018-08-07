#pragma once

//------------------------------------------------------------------------------
/**
@class IProtobufDbStub

(C) 2016 n.lee
*/
#include "IDbStubParam.h"

//------------------------------------------------------------------------------
/**
@brief IProtobufDbStub
*/
class IProtobufDbStub {
public:
	virtual ~IProtobufDbStub() { }

	virtual void				Open(db_stub_param_t& param) = 0;
	virtual void				Close() = 0;

	/* entity */
	virtual void				EntityLoadString(const std::string& strEntityName, const std::string& strEntity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string >& vKey) = 0;
	virtual void				EntityAddString(const std::string& strEntityName, const std::string& strEntity, const std::string& sTableName, const std::vector<std::string >& vKey) = 0;
	virtual void				EntitySaveString(const std::string& strEntityName, const std::string& strEntity, const std::string& sTableName, const std::vector<std::string >& vKey) = 0;
	virtual void				EntityDeleteString(const std::string& strEntityName, const std::string& strEntity, const std::string& sTableName, const std::vector<std::string >& vKey) = 0;

	/* stored proc */
	virtual void				StoredProcQueryString(const std::string& strTypeName, const std::string& strRequest, std::string& strResponse) = 0;
	virtual void				StoredProcUpdateString(const std::string& strTypeName, const std::string& strRequest) = 0;

	/* NOTE: "EntityLoad" and "StoredProcQuery" are not public API because they would modify pb message and it is not thread safe, 
	         use "EntityLoadString" and "StoredProcQueryString" instead. */
	virtual bool				EntityAdd(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string >& vKey) = 0;
	virtual bool				EntitySave(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string >& vKey) = 0;
	virtual bool				EntityDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string >& vKey) = 0;

	virtual bool				StoredProcUpdate(const google::protobuf::Message& storedProc) = 0;
};

/*EOF*/