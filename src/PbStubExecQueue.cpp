//------------------------------------------------------------------------------
//  PbStubExecQueue.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "PbStubExecQueue.h"

#include <future>

#include "db/ProtobufDbStub.h"

//------------------------------------------------------------------------------
/**

*/
CPbStubExecQueue::CPbStubExecQueue(db_stub_param_t& param, int nProducerNum, int nConsumerNum)
	: _refParam(param)
	, _producerNum(nProducerNum)
	, _consumerNum(nConsumerNum)
	, _callbacks(nProducerNum * MyTraits::BLOCK_SIZE) {
	//
	Start();
}

//------------------------------------------------------------------------------
/**

*/
CPbStubExecQueue::~CPbStubExecQueue() {

}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::AddEntityLoadString(std::string&& ename, std::string&& entity, IDbService::load_cb_t&& cb, const std::string& sTableName, const std::vector<std::string>& vKey) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddEntityLoadString()] can't enqueue, callback is dropped!!!");
		return false;
	}

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_E_LOAD_STRING;
	cmd._typename = std::move(ename);
	cmd._request = std::move(entity);
	cmd._tablename = sTableName;
	cmd._key_list = vKey;
	cmd._pbmsg_ptr = nullptr;
	cmd._load_cb = std::move(cb);
	cmd._dispose_cb = nullptr;

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddEntityLoadString()] enqueue failed, callback is dropped!!!");
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::AddEntityAddString(std::string&& ename, std::string&& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddEntityAddString()] can't enqueue, callback is dropped!!!");
		return false;
	}

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_E_ADD_STRING;
	cmd._typename = std::move(ename);
	cmd._request = std::move(entity);
	cmd._tablename = sTableName;
	cmd._key_list = vKey;
	cmd._pbmsg_ptr = nullptr;
	cmd._load_cb = nullptr;
	cmd._dispose_cb = nullptr;

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddEntityAddString()] enqueue failed, callback is dropped!!!");
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::AddEntitySaveString(std::string&& ename, std::string&& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddEntitySaveString()] can't enqueue, callback is dropped!!!");
		return false;
	}

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_E_SAVE_STRING;
	cmd._typename = std::move(ename);
	cmd._request = std::move(entity);
	cmd._tablename = sTableName;
	cmd._key_list = vKey;
	cmd._pbmsg_ptr = nullptr;
	cmd._load_cb = nullptr;
	cmd._dispose_cb = nullptr;

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddEntitySaveString()] enqueue failed, callback is dropped!!!");
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::AddEntityDeleteString(std::string&& ename, std::string&& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddEntityDeleteString()] can't enqueue, callback is dropped!!!");
		return false;
	}

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_E_DELETE_STRING;
	cmd._typename = std::move(ename);
	cmd._request = std::move(entity);
	cmd._tablename = sTableName;
	cmd._key_list = vKey;
	cmd._pbmsg_ptr = nullptr;
	cmd._dispose_cb = nullptr;

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddEntityDeleteString()] enqueue failed, callback is dropped!!!");
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::SynchronizeEntityLoad(const google::protobuf::Message& entity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string>& vKey) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeEntityLoad()] can't enqueue, callback is dropped!!!");
		return false;
	}

	auto workCb = std::bind([&vResult](std::vector<std::string>& result_list) {
		vResult = std::move(result_list);
	}, std::move(std::placeholders::_1));

	auto prms = std::make_shared<std::promise<void>>();
	auto dispose_cb = [&prms]() {
		prms->set_value();
	};

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_E_LOAD;
	cmd._tablename = sTableName;
	cmd._key_list = vKey;
	cmd._pbmsg_ptr = &entity;
	cmd._load_cb = std::move(workCb);
	cmd._dispose_cb = std::move(dispose_cb);

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeEntityLoad()] enqueue failed, callback is dropped!!!");
		return false;
	}
	prms->get_future().get();
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::SynchronizeEntityAdd(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeEntityAdd()] can't enqueue, callback is dropped!!!");
		return false;
	}

	auto prms = std::make_shared<std::promise<void>>();
	auto dispose_cb = [&prms]() {
		prms->set_value();
	};

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_E_ADD;
	cmd._tablename = sTableName;
	cmd._key_list = vKey;
	cmd._pbmsg_ptr = &entity;
	cmd._load_cb = nullptr;
	cmd._dispose_cb = std::move(dispose_cb);

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeEntityAdd()] enqueue failed, callback is dropped!!!");
		return false;
	}
	prms->get_future().get();
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::SynchronizeEntitySave(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeEntitySave()] can't enqueue, callback is dropped!!!");
		return false;
	}

	auto prms = std::make_shared<std::promise<void>>();
	auto dispose_cb = [&prms]() {
		prms->set_value();
	};

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_E_SAVE;
	cmd._tablename = sTableName;
	cmd._key_list = vKey;
	cmd._pbmsg_ptr = &entity;
	cmd._load_cb = nullptr;
	cmd._dispose_cb = std::move(dispose_cb);

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeEntitySave()] enqueue failed, callback is dropped!!!");
		return false;
	}
	prms->get_future().get();
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::SynchronizeEntityDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeEntityDelete()] can't enqueue, callback is dropped!!!");
		return false;
	}

	auto prms = std::make_shared<std::promise<void>>();
	auto dispose_cb = [&prms]() {
		prms->set_value();
	};

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_E_DELETE;
	cmd._tablename = sTableName;
	cmd._key_list = vKey;
	cmd._pbmsg_ptr = &entity;
	cmd._load_cb = nullptr;
	cmd._dispose_cb = std::move(dispose_cb);

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeEntityDelete()] enqueue failed, callback is dropped!!!");
		return false;
	}
	prms->get_future().get();
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::AddStoredProcQueryString(std::string&& spname, std::string&& storedProc, IDbService::query_cb_t&& cb) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddStoredProcQueryString()] can't enqueue, callback is dropped!!!");
		return false;
	}

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_SP_QUERY_STRING;
	cmd._typename = std::move(spname);
	cmd._request = std::move(storedProc);
	cmd._pbmsg_ptr = nullptr;
	cmd._query_cb = std::move(cb);
	cmd._dispose_cb = nullptr;

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddStoredProcQueryString()] enqueue failed, callback is dropped!!!");
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::AddStoredProcUpdateString(std::string&& spname, std::string&& storedProc) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddStoredProcUpdateString()] can't enqueue, callback is dropped!!!");
		return false;
	}

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_SP_UPDATE_STRING;
	cmd._typename = std::move(spname);
	cmd._request = std::move(storedProc);
	cmd._pbmsg_ptr = nullptr;
	cmd._query_cb = nullptr;
	cmd._dispose_cb = nullptr;

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::AddStoredProcUpdateString()] enqueue failed, callback is dropped!!!");
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::SynchronizeStoredProcQuery(const google::protobuf::Message& storedProc, std::string& strResponse) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeStoredProcQuery()] can't enqueue, callback is dropped!!!");
		return false;
	}

	auto workCb = std::bind([&strResponse](std::string& response) {
		strResponse = std::move(response);
	}, std::move(std::placeholders::_1));

	auto prms = std::make_shared<std::promise<void>>();
	auto dispose_cb = [&prms]() {
		prms->set_value();
	};

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_SP_QUERY;
	cmd._pbmsg_ptr = &storedProc;
	cmd._query_cb = std::move(workCb);
	cmd._dispose_cb = std::move(dispose_cb);

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeStoredProcQuery()] enqueue failed, callback is dropped!!!");
		return false;
	}
	prms->get_future().get();
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CPbStubExecQueue::SynchronizeStoredProcUpdate(const google::protobuf::Message& storedProc) {

	if (_done) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeStoredProcUpdate()] can't enqueue, callback is dropped!!!");
		return false;
	}

	auto prms = std::make_shared<std::promise<void>>();
	auto dispose_cb = [&prms]() {
		prms->set_value();
	};

	//
	// Add work item.
	//
	IDbService::cmd_t cmd;
	cmd._type = IDbService::cmd_t::DB_CMD_TYPE_SP_UPDATE;
	cmd._pbmsg_ptr = &storedProc;
	cmd._query_cb = nullptr;
	cmd._dispose_cb = std::move(dispose_cb);

	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CPbStubExecQueue::SynchronizeStoredProcUpdate()] enqueue failed, callback is dropped!!!");
		return false;
	}
	prms->get_future().get();
	return true;
}

//------------------------------------------------------------------------------
/**

*/
void
CPbStubExecQueue::Finish() {
	//
	// Set done flag and notify.
	//
	_done = true;

	// join
	for (auto& t : _threads) {
		if (t.joinable()) {
			t.join();
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CPbStubExecQueue::Start() {
	// start thread
	int i;
	for (i = 0; i < _consumerNum; ++i) {
		std::thread t([this]() {
			this->Run();
		});
		_threads.emplace_back(std::move(t));
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CPbStubExecQueue::Run() {
	// thread init
	std::shared_ptr<CProtobufDbStub> stub = std::make_shared<CProtobufDbStub>(_refParam._refLog);
	stub->Open(_refParam);

	CPbStubExecQueue::CallbackEntry cmd;
	while (!_done) {
		//
		// Get next work item. wait 1.5 seconds
		//
		bool found = _callbacks.wait_dequeue_timed(cmd, std::chrono::milliseconds(1500));
		if (found) {
			switch (cmd._type) {
			/* entity */
			case IDbService::cmd_t::DB_CMD_TYPE_E_LOAD_STRING: {
				// load
				std::vector<std::string> vResponse;
				stub->EntityLoadString(cmd._typename, cmd._request, vResponse, cmd._tablename, cmd._key_list);

				if (cmd._load_cb) {
					cmd._load_cb(vResponse);
				}

				break;
			}

			case IDbService::cmd_t::DB_CMD_TYPE_E_ADD_STRING: {
				// save
				stub->EntityAddString(cmd._typename, cmd._request, cmd._tablename, cmd._key_list);

				break;
			}

			case IDbService::cmd_t::DB_CMD_TYPE_E_SAVE_STRING: {
				// save
				stub->EntitySaveString(cmd._typename, cmd._request, cmd._tablename, cmd._key_list);

				break;
			}

			case IDbService::cmd_t::DB_CMD_TYPE_E_DELETE_STRING: {
				// delete
				stub->EntityDeleteString(cmd._typename, cmd._request, cmd._tablename, cmd._key_list);

				break;
			}

			case IDbService::cmd_t::DB_CMD_TYPE_E_LOAD: {
				// e load
				const std::string& sTypeName = cmd._pbmsg_ptr->GetTypeName();
				std::string sRequest = cmd._pbmsg_ptr->SerializePartialAsString();

				std::vector<std::string> vResponse;
				stub->EntityLoadString(sTypeName, sRequest, vResponse, cmd._tablename, cmd._key_list);

				if (cmd._load_cb) {
					cmd._load_cb(vResponse);
				}

				break;
			}

			case IDbService::cmd_t::DB_CMD_TYPE_E_ADD: {
				// e save
				stub->EntityAdd(*cmd._pbmsg_ptr, cmd._tablename, cmd._key_list);

				break;
			}

			case IDbService::cmd_t::DB_CMD_TYPE_E_SAVE: {
				// e save
				stub->EntitySave(*cmd._pbmsg_ptr, cmd._tablename, cmd._key_list);

				break;
			}

			case IDbService::cmd_t::DB_CMD_TYPE_E_DELETE: {
				// e delete
				stub->EntityDelete(*cmd._pbmsg_ptr, cmd._tablename, cmd._key_list);

				break;
			}
														   
			/* stored proc */
			case IDbService::cmd_t::DB_CMD_TYPE_SP_QUERY_STRING: {
				// query
				std::string strResponse;
				stub->StoredProcQueryString(cmd._typename, cmd._request, strResponse);

				if (cmd._query_cb) {
					cmd._query_cb(strResponse);
				}

				break;
			}

			case IDbService::cmd_t::DB_CMD_TYPE_SP_UPDATE_STRING: {
				// update
				stub->StoredProcUpdateString(cmd._typename, cmd._request);

				break;
			}

			case IDbService::cmd_t::DB_CMD_TYPE_SP_QUERY: {
				// sp query
				const std::string& sTypeName = cmd._pbmsg_ptr->GetTypeName();
				std::string sRequest = cmd._pbmsg_ptr->SerializePartialAsString();

				std::string strResponse;
				stub->StoredProcQueryString(sTypeName, sRequest, strResponse);

				if (cmd._query_cb) {
					cmd._query_cb(strResponse);
				}

				break;
			}

			case IDbService::cmd_t::DB_CMD_TYPE_SP_UPDATE: {
				// sp update
				stub->StoredProcUpdate(*cmd._pbmsg_ptr);

				break;
			}

			default:
				break;
			}

			// dispose
			if (cmd._dispose_cb)
				cmd._dispose_cb();
		}
	}

	// thread dispose
	stub->Close();
}

/* -- EOF -- */