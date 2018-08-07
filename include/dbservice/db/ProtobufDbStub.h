#pragma once
//------------------------------------------------------------------------------
/**
    @class CProtobufDbStub
    
    (C) 2016 n.lee
*/
#include "Protobuf2DbEngine.h"
#include "ProtobufImporter.h"

#include "IProtobufDbStub.h"

//------------------------------------------------------------------------------
/** 
	@brief CProtobufDbStub
*/
class CProtobufDbStub : public IProtobufDbStub {
public:
	CProtobufDbStub(StdLog *pLog);
	virtual ~CProtobufDbStub();

	virtual void				Open(db_stub_param_t& param);
	virtual void				Close();

	/* entity */
	virtual void				EntityLoadString(const std::string& strEntityName, const std::string& strEntity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string >& vKey) override;
	virtual void				EntityAddString(const std::string& strEntityName, const std::string& strEntity, const std::string& sTableName, const std::vector<std::string >& vKey) override;
	virtual void				EntitySaveString(const std::string& strEntityName, const std::string& strEntity, const std::string& sTableName, const std::vector<std::string >& vKey) override;
	virtual void				EntityDeleteString(const std::string& strEntityName, const std::string& strEntity, const std::string& sTableName, const std::vector<std::string >& vKey) override;

	/* stored proc */
	virtual void				StoredProcQueryString(const std::string& strProcName, const std::string& strStoredProc, std::string& strResult) override;
	virtual void				StoredProcUpdateString(const std::string& strProcName, const std::string& strStoredProc) override;

	/* NOTE: "EntityLoad" and "StoredProcQuery" are not public API because they would modify pb message and it is not thread safe,
	         use "EntityLoadString" and "StoredProcQueryString" instead. */
	virtual bool				EntityAdd(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string >& vKey) override;
	virtual bool				EntitySave(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string >& vKey) override;
	virtual bool				EntityDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string >& vKey) override;

	virtual bool				StoredProcUpdate(const google::protobuf::Message& storedProc) override;

private:
	bool						EntityLoad(google::protobuf::Message& entity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string >& vKey);

	bool						StoredProcQuery(google::protobuf::Message& storedProc);

private:
	StdLog *_refLog;

	CProtobufImporter *_importer = nullptr;
	Protobuf2DbEngine *_pb2dbEngine = nullptr;

};

/*EOF*/