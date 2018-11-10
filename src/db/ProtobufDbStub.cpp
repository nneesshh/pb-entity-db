//------------------------------------------------------------------------------
//  ProtobufDbStub.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "ProtobufDbStub.h"

#include "base/utilities.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CProtobufDbStub::CProtobufDbStub(StdLog *pLog)
	: _refLog(pLog)
	, _importer(new CProtobufImporter(nullptr, "dynamic_pb/protos.def"))
	, _pb2dbEngine(new Protobuf2DbEngine()) {
	
}

//------------------------------------------------------------------------------
/**

*/
CProtobufDbStub::~CProtobufDbStub() {

	delete _pb2dbEngine;
	delete _importer;
}

//------------------------------------------------------------------------------
/**

*/
void
CProtobufDbStub::Open(db_stub_param_t& param) {
	//
	_pb2dbEngine->OnInit(
		_refLog,
		param._dbServerId.c_str(),
		param._dbUid.c_str(),
		param._dbPwd.c_str(),
		param._dbName.c_str(),
		1);
	
	_pb2dbEngine->Open();
}

//------------------------------------------------------------------------------
/**

*/
void
CProtobufDbStub::Close() {
	//
	if (_pb2dbEngine) {
		_pb2dbEngine->Close();
		_pb2dbEngine->OnDelete();
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CProtobufDbStub::EntityLoadString(const std::string& strEntityName, const std::string& strEntity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string >& vKey) {
	google::protobuf::Message *pProto = _importer->GetEntityProto(strEntityName);
	if (pProto && pProto->ParsePartialFromString(strEntity)) {
		EntityLoad(*pProto, vResult, sTableName, vKey);
	}

	// Must not reset arena here. 
	// It will destruct pre-allocated message cache
}

//------------------------------------------------------------------------------
/**

*/
void
CProtobufDbStub::EntityAddString(const std::string& strEntityName, const std::string& strEntity, const std::string& sTableName, const std::vector<std::string >& vKey) {
	google::protobuf::Message *pProto = _importer->GetEntityProto(strEntityName);
	if (pProto && pProto->ParsePartialFromString(strEntity)) {
		EntityAdd(*pProto, sTableName, vKey);
	}

	// Must not reset arena here. 
	// It will destruct pre-allocated message cache
}

//------------------------------------------------------------------------------
/**

*/
void
CProtobufDbStub::EntitySaveString(const std::string& strEntityName, const std::string& strEntity, const std::string& sTableName, const std::vector<std::string >& vKey) {
	google::protobuf::Message *pProto = _importer->GetEntityProto(strEntityName);
	if (pProto && pProto->ParsePartialFromString(strEntity)) {
		EntitySave(*pProto, sTableName, vKey);
	}

	// Must not reset arena here. 
	// It will destruct pre-allocated message cache
}

//------------------------------------------------------------------------------
/**

*/
void
CProtobufDbStub::EntityDeleteString(const std::string& strEntityName, const std::string& strEntity, const std::string& sTableName, const std::vector<std::string >& vKey) {
	google::protobuf::Message *pProto = _importer->GetEntityProto(strEntityName);
	if (pProto && pProto->ParsePartialFromString(strEntity)) {
		EntityDelete(*pProto, sTableName, vKey);
	}

	// Must not reset arena here. 
	// It will destruct pre-allocated message cache
}

//------------------------------------------------------------------------------
/**

*/
void
CProtobufDbStub::StoredProcQueryString(const std::string& strProcName, const std::string& strStoredProc, std::string& strResult) {
	google::protobuf::Message *pProto = _importer->GetEntityProto(strProcName);
	if (pProto) {
#ifdef _DEBUG
		if (pProto->ParseFromString(strStoredProc)) {
#else
		if (pProto->ParsePartialFromString(strStoredProc)) {
#endif
			StoredProcQuery(*pProto);
			pProto->SerializeToString(&strResult);
		}
	}

	// Must not reset arena here. 
	// It will destruct pre-allocated message cache
}

//------------------------------------------------------------------------------
/**

*/
void
CProtobufDbStub::StoredProcUpdateString(const std::string& strProcName, const std::string& strStoredProc) {
	google::protobuf::Message *pProto = _importer->GetEntityProto(strProcName);
	if (pProto) {
#ifdef _DEBUG
		if (pProto->ParseFromString(strStoredProc)) {
#else
		if (pProto->ParsePartialFromString(strStoredProc)) {
#endif
			StoredProcUpdate(*pProto);
		}
	}

	// Must not reset arena here. 
	// It will destruct pre-allocated message cache
}

//------------------------------------------------------------------------------
/**

*/
bool
CProtobufDbStub::EntityAdd(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string >& vKey) {
	const google::protobuf::Descriptor *descriptor = entity.GetDescriptor();
	const std::string& entity_name = descriptor->name();

	while (_pb2dbEngine->IsOpened()
		&& !_pb2dbEngine->EInsert(entity, sTableName, vKey)) {
		fprintf(stderr, "[tid(%d)][EntityAdd()] DB broken when add entity [%s], try reconnect after 6 seconds ...\n",
			(int)::GetCurrentThreadId(), sTableName.c_str());

		util_sleep(6000);
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CProtobufDbStub::EntitySave(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string >& vKey) {
	const google::protobuf::Descriptor *descriptor = entity.GetDescriptor();
	const std::string& entity_name = descriptor->name();

	while (_pb2dbEngine->IsOpened()
		&& !_pb2dbEngine->EUpdate(entity, sTableName, vKey, true)) {
		fprintf(stderr, "[tid(%d)][EntitySave()] DB broken when save entity [%s], try reconnect after 6 seconds ...\n",
			(int)::GetCurrentThreadId(), sTableName.c_str());

		util_sleep(6000);
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CProtobufDbStub::EntityDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string >& vKey) {
	const google::protobuf::Descriptor *descriptor = entity.GetDescriptor();
	const std::string& entity_name = descriptor->name();

	while (_pb2dbEngine->IsOpened()
		&& !_pb2dbEngine->EDelete(entity, sTableName, vKey)) {
		fprintf(stderr, "[tid(%d)][EntityDelete()] DB broken when delete entity [%s], try reconnect after 6 seconds ...\n",
			(int)::GetCurrentThreadId(), sTableName.c_str());

		util_sleep(6000);
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CProtobufDbStub::StoredProcUpdate(const google::protobuf::Message& storedProc) {
	const google::protobuf::Descriptor *descriptor = storedProc.GetDescriptor();
	const std::string& proc_name = descriptor->name();

	while (_pb2dbEngine->IsOpened()
		&& !_pb2dbEngine->SpUpdate(storedProc)) {
		fprintf(stderr, "[tid(%d)][StoredProcUpdate()] DB broken when exec update [%s], try reconnect after 6 seconds ...\n",
			(int)::GetCurrentThreadId(), proc_name.c_str());

		util_sleep(6000);
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CProtobufDbStub::EntityLoad(google::protobuf::Message& entity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string >& vKey) {
	const google::protobuf::Descriptor *descriptor = entity.GetDescriptor();
	const std::string& entity_name = descriptor->name();

	while (_pb2dbEngine->IsOpened()
		&& !_pb2dbEngine->ESelect(entity, vResult, sTableName, vKey)) {
		fprintf(stderr, "[tid(%d)][EntityLoad()] DB broken when load entity [%s], try reconnect after 6 seconds ...\n",
			(int)::GetCurrentThreadId(), sTableName.c_str());

		util_sleep(6000);
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CProtobufDbStub::StoredProcQuery(google::protobuf::Message& storedProc) {
	const google::protobuf::Descriptor *descriptor = storedProc.GetDescriptor();
	const std::string& proc_name = descriptor->name();

	while (_pb2dbEngine->IsOpened()
		&& !_pb2dbEngine->SpQuery(storedProc)) {
		fprintf(stderr, "[tid(%d)][StoredProcQuery()] DB broken when exec query [%s], try reconnect after 6 seconds ...\n",
			(int)::GetCurrentThreadId(), proc_name.c_str());

		util_sleep(6000);
	}
	return true;
}

/* -- EOF -- */