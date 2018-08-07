#pragma once
//------------------------------------------------------------------------------
/**
@class CDbService

(C) 2016 n.lee
*/
#include "PbStubTrunkQueue.h"
#include "PbStubExecQueue.h"

#include "base/IDbService.h"

//------------------------------------------------------------------------------
/**
@brief CDbService
*/
class MY_DB_EXTERN CDbService : public IDbService {
public:
	CDbService(db_stub_param_t *param);
	virtual ~CDbService();

public:
	virtual void				RunOnce() override {
		_trunkQueue->RunOnce();
	}

	//////////////////////////////////////////////////////////////////////////
	/// entity
	//////////////////////////////////////////////////////////////////////////
	virtual void				EntityLoad(const google::protobuf::Message& entity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string>& vKey = {}) override;
	virtual void				EntityAdd(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey = {}) override;
	virtual void				EntitySave(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey = {}) override;
	virtual void				EntityDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey = {}) override;

	// async version
	virtual void				EntityLoadAsync(const google::protobuf::Message& entity, load_cb_t&& cb, const std::string& sTableName, const std::vector<std::string>& vKey = {}) override;
	virtual void				EntityAddAsync(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey = {}) override;
	virtual void				EntitySaveAsync(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey = {}) override;
	virtual void				EntityDeleteAsync(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey = {}) override;

	//////////////////////////////////////////////////////////////////////////
	/// stored proc
	//////////////////////////////////////////////////////////////////////////
	virtual void				StoredProcQuery(google::protobuf::Message& storedProc) override;
	virtual void				StoredProcUpdate(const google::protobuf::Message& storedProc) override;

	// async version
	virtual void				StoredProcQueryAsync(const google::protobuf::Message& storedProc, query_cb_t&& cb) override;
	virtual void				StoredProcUpdateAsync(const google::protobuf::Message& storedProc) override;

	virtual void				Shutdown() override;

private:
	bool _bShutdown = false;

	db_stub_param_t _param;

	CPbStubTrunkQueuePtr _trunkQueue;
	CPbStubExecQueuePtr _execQueue;
};

/*EOF*/