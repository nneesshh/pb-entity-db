//------------------------------------------------------------------------------
//  DbService.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "DbService.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

#define TRUNK_CONSUMER_NUM 1

static std::string
__convert_entity_to_table(const std::string& sEntityName) {
	char chTableName[1024];
	const char *s;
	char *p;
	size_t i;

	s = sEntityName.c_str();
	p = chTableName;

	for (i = 0; i < sEntityName.length() && i < sizeof(chTableName) - 1; ++i) {
		if (isupper(*s)) {
			if (i > 0) {
				*p++ = '_';
			}
			*p++ = tolower(*s++);
		}
		else {
			*p++ = *s++;
		}
	}
	return std::string(chTableName, p - chTableName);
}

//------------------------------------------------------------------------------
/**

*/
CDbService::CDbService(db_stub_param_t *param)
	: _param(*param) {
	//
	int nProducerNum = TRUNK_CONSUMER_NUM;
	int nConsumerNum = std::min<int>(4, std::max<int>(1, _param._poolSize)); // 1~4 consumer

	_trunkQueue = std::make_shared<CPbStubTrunkQueue>(nProducerNum);
	_execQueue = std::make_shared<CPbStubExecQueue>(_param, nProducerNum, nConsumerNum);
}

//------------------------------------------------------------------------------
/**

*/
CDbService::~CDbService() {
	//
	google::protobuf::ShutdownProtobufLibrary();
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::EntityLoad(const google::protobuf::Message& entity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string>& vKey) {
	// check table name
	std::string strTableName = sTableName;
	if (sTableName.length() <= 0) {
		const google::protobuf::Descriptor	*descriptor = entity.GetDescriptor();
		const google::protobuf::Reflection	*reflection = entity.GetReflection();
		const std::string& e_name = descriptor->name();

		strTableName = __convert_entity_to_table(e_name);
	}

	_execQueue->SynchronizeEntityLoad(entity, vResult, strTableName, vKey);
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::EntityAdd(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {
	// check table name
	std::string strTableName = sTableName;
	if (sTableName.length() <= 0) {
		const google::protobuf::Descriptor	*descriptor = entity.GetDescriptor();
		const google::protobuf::Reflection	*reflection = entity.GetReflection();
		const std::string& e_name = descriptor->name();

		strTableName = __convert_entity_to_table(e_name);
	}

	_execQueue->SynchronizeEntityAdd(entity, strTableName, vKey);
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::EntitySave(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {
	// check table name
	std::string strTableName = sTableName;
	if (sTableName.length() <= 0) {
		const google::protobuf::Descriptor	*descriptor = entity.GetDescriptor();
		const google::protobuf::Reflection	*reflection = entity.GetReflection();
		const std::string& e_name = descriptor->name();

		strTableName = __convert_entity_to_table(e_name);
	}

	_execQueue->SynchronizeEntitySave(entity, strTableName, vKey);
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::EntityDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {
	// check table name
	std::string strTableName = sTableName;
	if (sTableName.length() <= 0) {
		const google::protobuf::Descriptor	*descriptor = entity.GetDescriptor();
		const google::protobuf::Reflection	*reflection = entity.GetReflection();
		const std::string& e_name = descriptor->name();

		strTableName = __convert_entity_to_table(e_name);
	}

	_execQueue->SynchronizeEntityDelete(entity, strTableName, vKey);
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::EntityLoadAsync(const google::protobuf::Message& entity, load_cb_t&& cb, const std::string& sTableName, const std::vector<std::string>& vKey) {
	std::string strEntityName = entity.GetTypeName();
	std::string strEntity = entity.SerializePartialAsString();

	// check table name
	std::string strTableName = sTableName;
	if (sTableName.length() <= 0) {
		const std::string& e_name = entity.GetDescriptor()->name();
		strTableName = __convert_entity_to_table(e_name);
	}

	auto workCb = std::bind([this](load_cb_t& onResponse, std::vector<std::string>& vResponse) {
		if (onResponse)
			_trunkQueue->Add(std::move(onResponse), std::move(vResponse));
	}, std::move(cb), std::move(std::placeholders::_1));

	//
	_execQueue->AddEntityLoadString(std::move(strEntityName), std::move(strEntity), std::move(workCb), strTableName, vKey);
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::EntityAddAsync(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {
	std::string strEntityName = entity.GetTypeName();
	std::string strEntity = entity.SerializePartialAsString();

	// check table name
	std::string strTableName = sTableName;
	if (sTableName.length() <= 0) {
		const std::string& e_name = entity.GetDescriptor()->name();
		strTableName = __convert_entity_to_table(e_name);
	}

	_execQueue->AddEntityAddString(std::move(strEntityName), std::move(strEntity), strTableName, vKey);
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::EntitySaveAsync(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {
	std::string strEntityName = entity.GetTypeName();
	std::string strEntity = entity.SerializePartialAsString();

	// check table name
	std::string strTableName = sTableName;
	if (sTableName.length() <= 0) {
		const std::string& e_name = entity.GetDescriptor()->name();
		strTableName = __convert_entity_to_table(e_name);
	}

	_execQueue->AddEntitySaveString(std::move(strEntityName), std::move(strEntity), strTableName, vKey);
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::EntityDeleteAsync(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) {
	std::string strEntityName = entity.GetTypeName();
	std::string strEntity = entity.SerializePartialAsString();

	// check table name
	std::string strTableName = sTableName;
	if (sTableName.length() <= 0) {
		const std::string& e_name = entity.GetDescriptor()->name();
		strTableName = __convert_entity_to_table(e_name);
	}

	_execQueue->AddEntityDeleteString(std::move(strEntityName), std::move(strEntity), strTableName, vKey);
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::StoredProcQuery(google::protobuf::Message& storedProc) {
	std::string strResponse;
	_execQueue->SynchronizeStoredProcQuery(storedProc, strResponse);
	storedProc.ParsePartialFromString(strResponse);
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::StoredProcUpdate(const google::protobuf::Message& storedProc) {
	_execQueue->SynchronizeStoredProcUpdate(storedProc);
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::StoredProcQueryAsync(const google::protobuf::Message& storedProc, query_cb_t&& cb) {
	std::string strSpName = storedProc.GetTypeName();
	std::string strStoredProc = storedProc.SerializeAsString();

	auto workCb = std::bind([this](query_cb_t& onResponse, std::string& strResponse) {
		if (onResponse)
			_trunkQueue->Add(std::move(onResponse), std::move(strResponse));
	}, std::move(cb), std::move(std::placeholders::_1));

	//
	_execQueue->AddStoredProcQueryString(std::move(strSpName), std::move(strStoredProc), std::move(workCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::StoredProcUpdateAsync(const google::protobuf::Message& storedProc) {
	std::string strSpName = storedProc.GetTypeName();
	std::string strStoredProc = storedProc.SerializeAsString();

	_execQueue->AddStoredProcUpdateString(std::move(strSpName), std::move(strStoredProc));
}

//------------------------------------------------------------------------------
/**

*/
void
CDbService::Shutdown() {
	if (!_bShutdown) {
		_bShutdown = true;

		_execQueue->Finish();
		_trunkQueue->Close();
	}
}

/** -- EOF -- **/