#pragma once
//------------------------------------------------------------------------------
/**
@class CPbStubExecQueue

(C) 2016 n.lee
*/
#include "base/concurrent/blockingconcurrentqueue.h"
#include "base/IDbService.h"

#include "db/ProtobufImporter.h"
#include "db/IDbStubParam.h"

//------------------------------------------------------------------------------
/**
@brief CPbStubExecQueue
*/
class CPbStubExecQueue {
public:
	CPbStubExecQueue(db_stub_param_t& param, int nProducerNum, int nConsumerNum);
	~CPbStubExecQueue();

	using CallbackEntry = IDbService::cmd_t;
	struct MyTraits : public moodycamel::ConcurrentQueueDefaultTraits {
		static const size_t BLOCK_SIZE = 256;
	};

	// entity
	bool						AddEntityLoadString(std::string&& ename, std::string&& entity, IDbService::load_cb_t&& cb, const std::string& sTableName, const std::vector<std::string>& vKey);
	bool						AddEntityAddString(std::string&& ename, std::string&& entity, const std::string& sTableName, const std::vector<std::string>& vKey);
	bool						AddEntitySaveString(std::string&& ename, std::string&& entity, const std::string& sTableName, const std::vector<std::string>& vKey);
	bool						AddEntityDeleteString(std::string&& ename, std::string&& entity, const std::string& sTableName, const std::vector<std::string>& vKey);

	/// synchronize only
	bool						SynchronizeEntityLoad(const google::protobuf::Message& entity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string>& vKey);
	bool						SynchronizeEntityAdd(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey);
	bool						SynchronizeEntitySave(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey);
	bool						SynchronizeEntityDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey);

	// stored proc
	bool						AddStoredProcQueryString(std::string&& spname, std::string&& storedProc, IDbService::query_cb_t&& cb);
	bool						AddStoredProcUpdateString(std::string&& spname, std::string&& storedProc);

	/// synchronize only
	bool						SynchronizeStoredProcQuery(const google::protobuf::Message& storedProc, std::string& strResponse);
	bool						SynchronizeStoredProcUpdate(const google::protobuf::Message& storedProc);
	
	void						Finish();

private:
	void						Start();
	void						Run();

private:
	db_stub_param_t& _refParam;
	
	int _producerNum;
	int _consumerNum;
	bool _done = false;
	moodycamel::BlockingConcurrentQueue<CallbackEntry, MyTraits> _callbacks;

	// threads
	std::vector<std::thread> _threads;
};
using CPbStubExecQueuePtr = std::shared_ptr<CPbStubExecQueue>;

/*EOF*/