//------------------------------------------------------------------------------
//  Protobuf2DbEngine.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "Protobuf2DbEngine.h"

#include <time.h>
#include <locale.h>
#include <array>

#include "ZdbConnectionPoolSession.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

std::shared_timed_mutex Protobuf2DbEngine::s_mutex;

typedef enum enum_field_types {
	MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
	MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG,
	MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
	MYSQL_TYPE_NULL, MYSQL_TYPE_TIMESTAMP,
	MYSQL_TYPE_LONGLONG, MYSQL_TYPE_INT24,
	MYSQL_TYPE_DATE, MYSQL_TYPE_TIME,
	MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
	MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
	MYSQL_TYPE_BIT,
	MYSQL_TYPE_TIMESTAMP2,
	MYSQL_TYPE_DATETIME2,
	MYSQL_TYPE_TIME2,
	MYSQL_TYPE_JSON = 245,
	MYSQL_TYPE_NEWDECIMAL = 246,
	MYSQL_TYPE_ENUM = 247,
	MYSQL_TYPE_SET = 248,
	MYSQL_TYPE_TINY_BLOB = 249,
	MYSQL_TYPE_MEDIUM_BLOB = 250,
	MYSQL_TYPE_LONG_BLOB = 251,
	MYSQL_TYPE_BLOB = 252,
	MYSQL_TYPE_VAR_STRING = 253,
	MYSQL_TYPE_STRING = 254,
	MYSQL_TYPE_GEOMETRY = 255
} enum_field_types;

static std::map<int, std::string> s_map_field_type = {
	{ 0,						"MYSQL_TYPE_UNKNOWN" },
	{ MYSQL_TYPE_DECIMAL,		"MYSQL_TYPE_DECIMAL" },
	{ MYSQL_TYPE_TINY,			"MYSQL_TYPE_TINY" },
	{ MYSQL_TYPE_SHORT,			"MYSQL_TYPE_SHORT" },
	{ MYSQL_TYPE_LONG,			"MYSQL_TYPE_LONG" },
	{ MYSQL_TYPE_FLOAT,			"MYSQL_TYPE_FLOAT" },
	{ MYSQL_TYPE_DOUBLE,		"MYSQL_TYPE_DOUBLE" },
	{ MYSQL_TYPE_NULL,			"MYSQL_TYPE_NULL" },
	{ MYSQL_TYPE_TIMESTAMP,		"MYSQL_TYPE_TIMESTAMP" },
	{ MYSQL_TYPE_LONGLONG,		"MYSQL_TYPE_LONGLONG" },
	{ MYSQL_TYPE_INT24,			"MYSQL_TYPE_INT24" },
	{ MYSQL_TYPE_DATE,			"MYSQL_TYPE_DATE" },
	{ MYSQL_TYPE_TIME,			"MYSQL_TYPE_TIME" },
	{ MYSQL_TYPE_DATETIME,		"MYSQL_TYPE_DATETIME" },
	{ MYSQL_TYPE_YEAR,			"MYSQL_TYPE_YEAR" },
	{ MYSQL_TYPE_NEWDATE,		"MYSQL_TYPE_NEWDATE" },
	{ MYSQL_TYPE_VARCHAR,		"MYSQL_TYPE_VARCHAR" },
	{ MYSQL_TYPE_BIT,			"MYSQL_TYPE_BIT" },
	{ MYSQL_TYPE_TIMESTAMP2,	"MYSQL_TYPE_TIMESTAMP2" },
	{ MYSQL_TYPE_DATETIME2,		"MYSQL_TYPE_DATETIME2" },
	{ MYSQL_TYPE_TIME2,			"MYSQL_TYPE_TIME2" },
	{ MYSQL_TYPE_JSON,			"MYSQL_TYPE_JSON" },
	{ MYSQL_TYPE_NEWDECIMAL,	"MYSQL_TYPE_NEWDECIMAL" },
	{ MYSQL_TYPE_ENUM,			"MYSQL_TYPE_ENUM" },
	{ MYSQL_TYPE_SET,			"MYSQL_TYPE_SET" },
	{ MYSQL_TYPE_TINY_BLOB,		"MYSQL_TYPE_TINY_BLOB" },
	{ MYSQL_TYPE_MEDIUM_BLOB,	"MYSQL_TYPE_MEDIUM_BLOB" },
	{ MYSQL_TYPE_LONG_BLOB,		"MYSQL_TYPE_LONG_BLOB" },
	{ MYSQL_TYPE_BLOB,			"MYSQL_TYPE_BLOB" },
	{ MYSQL_TYPE_VAR_STRING,	"MYSQL_TYPE_VAR_STRING" },
	{ MYSQL_TYPE_STRING,		"MYSQL_TYPE_STRING" },
	{ MYSQL_TYPE_GEOMETRY,		"MYSQL_TYPE_GEOMETRY" },
};

#define PROTOBUF_2_DB_ENGINE_RAISE_PROTO_TYPE_EXCEPTION(__PROTO_TYPE__)																							\
	{																																		\
		char chDesc[1024];																													\
		o_snprintf(chDesc, sizeof(chDesc), "[tid(%d)][Protobuf2DbEngine::AddFirstReturnRecord()] FAILED -- proto(%s)->name(%s)type(%s) mismatched with resultset fieldtype(%d)(%s)",	\
		(int)::GetCurrentThreadId(), strtypename.c_str(), name.c_str(), __PROTO_TYPE__, nFieldType, s_map_field_type[nFieldType].c_str());	\
		fprintf(stderr, "\n!!! !!! %s !!! !!!\n", chDesc);																					\
		if (_refLog) _refLog->logprint(LOG_LEVEL_WARNING, "\n!!! !!! %s !!! !!!\n", chDesc);												\
		THROW(SQLException, chDesc);																										\
	}

//------------------------------------------------------------------------------
/**

*/
Protobuf2DbEngine::Protobuf2DbEngine() {

}

//------------------------------------------------------------------------------
/**

*/
Protobuf2DbEngine::~Protobuf2DbEngine() {
	if (_db) {
		_db->Close();
		delete _db;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::OnInit(StdLog *pLog, const char * sURL, const char *sUser, const char *sPass, const char *sSchema, int nPoolSize) {
	// init db
	_refLog = pLog;
	_db = new ZdbDriver(_refLog, sURL, sUser, sPass, sSchema, nPoolSize);
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::OnDelete() {

}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::Open() {
	// mysql_init is not thread safe, so we must synchronize "Open()" method.
	std::unique_lock<std::shared_timed_mutex> _(s_mutex);
	while (!_db->Open()) {
		util_sleep(15 * 1000);
	}
	_opened = true;
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::Close() {
	_db->Close();
	_opened = false;
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::ESelect(google::protobuf::Message& entity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string>& vInKey) {
	// prepare
	ZdbStatement s(_db, sTableName.c_str());
	if (!ParseSelect(entity, sTableName, vInKey, &s))
		return false;

	// exec query
	ZdbResultSet rs(_db, &s);
	if (!s.Exec(&rs))
		return false;

	// return recordset
	// no need to clear return recordset, because it will be always success from here
	if (rs.IsOpened()) {
		vResult.reserve(256);

		ParseEntityResult(rs, vResult, entity);
	}

	//
	rs.Close();
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::EInsert(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey) {
	// prepare
	ZdbStatement s(_db, sTableName.c_str());
	if (!ParseInsert(entity, sTableName, vInKey, &s))
		return false;

	// exec update
	if (!s.Exec())
		return false;

	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::EUpdate(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey, bool bHackInsert) {
	// prepare
	ZdbStatement s(_db, sTableName.c_str());
	if (!ParseUpdate(entity, sTableName, vInKey, bHackInsert, &s))
		return false;

	// exec update
	if (!s.Exec())
		return false;

	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::EDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey) {
	// prepare
	ZdbStatement s(_db, sTableName.c_str());
	if (!ParseDelete(entity, sTableName, vInKey, &s))
		return false;

	// exec update
	if (!s.Exec())
		return false;

	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::SpQuery(google::protobuf::Message& storedProc) {
	const google::protobuf::Descriptor	*descriptor = storedProc.GetDescriptor();
	const google::protobuf::Reflection	*reflection = storedProc.GetReflection();
	const std::string& proc_name = descriptor->name();

	// prepare
	ZdbStatement s(_db, proc_name.c_str());
	if (!ParseProc(&storedProc, proc_name.c_str(), &s))
		return false;

	// exec query
	ZdbResultSet rs(_db, &s);
	if (!s.Exec(&rs))
		return false;

	// return recordset
	// no need to clear return recordset, because it will be always success from here
	if (1) {
		// return records
		if (rs.IsOpened()) {
			const google::protobuf::FieldDescriptor *fld_return_record_list = fld_return_record_list = descriptor->FindFieldByName("return_record_list");
			if (fld_return_record_list) {
				//
				ParseProcResult(rs, storedProc, *reflection, *fld_return_record_list);
			}
			else {
				// multiple record set
				const google::protobuf::FieldDescriptor *fld_return_recordsets = descriptor->FindFieldByName("return_recordsets");
				if (fld_return_recordsets) {
					google::protobuf::Message *return_recordsets = reflection->MutableMessage(&storedProc, fld_return_recordsets);
					const google::protobuf::Descriptor	*descriptor2 = return_recordsets->GetDescriptor();
					const google::protobuf::Reflection	*reflection2 = return_recordsets->GetReflection();

					// parse
					int nFieldCount = descriptor2->field_count();
					int i;
					for (i = 0; i < nFieldCount; ++i) {
						fld_return_record_list = descriptor2->field(i);

						//
						ParseProcResult(rs, *return_recordsets, *reflection2, *fld_return_record_list);

						//
						if (!rs.HasMoreResultSets()) {
							break;
						}
					}
				}
			}
		}

		//
		rs.Close();
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::SpUpdate(const google::protobuf::Message& storedProc) {
	const google::protobuf::Descriptor	*descriptor = storedProc.GetDescriptor();
	const google::protobuf::Reflection	*reflection = storedProc.GetReflection();
	const std::string& proc_name = descriptor->name();

	// prepare
	ZdbStatement s(_db, proc_name.c_str());
	if (!ParseProc(&storedProc, proc_name.c_str(), &s))
		return false;

	// exec update
	if (!s.Exec())
		return false;

	return true;
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::CollectEntityInputParams(
	const google::protobuf::Message *pParams,
	const std::vector<std::string>& vInKey,
	const std::vector<std::string>& vInSet,
	std::vector<pb_input_param_t>& vOutKeyParam,
	std::vector<pb_input_param_t>& vOutSetParam) {

	const google::protobuf::Descriptor *descriptor = pParams->GetDescriptor();
	const google::protobuf::Reflection *reflection = pParams->GetReflection();

	int nFieldCount, i;

	//
	nFieldCount = descriptor->field_count();

	//
	for (i = 0; i < nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *fieldDescriptor = descriptor->field(i);
		const std::string& sName = fieldDescriptor->name();

		bool bSkip = (fieldDescriptor->is_repeated() || !reflection->HasField(*pParams, fieldDescriptor));
		if (bSkip)
			continue;

		const google::protobuf::FieldDescriptor::Type fieldDescriptorType = fieldDescriptor->type();
		if (google::protobuf::FieldDescriptor::TYPE_MESSAGE == fieldDescriptorType) {
			const google::protobuf::Message& rSubParams = reflection->GetMessage(*pParams, fieldDescriptor);
			CollectEntityInputParams(&rSubParams, vInKey, vInSet, vOutKeyParam, vOutSetParam);
		}
		else {
			// label is not repeated
			assert(fieldDescriptor->label() == google::protobuf::FieldDescriptor::LABEL_OPTIONAL ||
				fieldDescriptor->label() == google::protobuf::FieldDescriptor::LABEL_REQUIRED);

			for (auto& it : vInKey) {
				const std::string& sKey = it;
				if (sKey == sName) {
					pb_input_param_t in_param;
					in_param.params = pParams;
					in_param.reflection = reflection;
					in_param.fieldDescriptor = fieldDescriptor;

					vOutKeyParam.emplace_back(in_param);
					break;
				}
			}

			for (auto& it : vInSet) {
				const std::string& sSet = it;
				if (sSet == sName) {
					pb_input_param_t in_param;
					in_param.params = pParams;
					in_param.reflection = reflection;
					in_param.fieldDescriptor = fieldDescriptor;

					vOutSetParam.emplace_back(in_param);
					break;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::CollectEntitySetFields(const google::protobuf::Message *pParams, const std::vector<std::string>& vInKey, std::vector<std::string>& vOut) {

	const google::protobuf::Descriptor *descriptor = pParams->GetDescriptor();
	const google::protobuf::Reflection *reflection = pParams->GetReflection();

	int nFieldCount, i;

	//
	nFieldCount = descriptor->field_count();
	vOut.reserve(std::max<int>(256, nFieldCount * 2));

	//
	for (i = 0; i < nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *fieldDescriptor = descriptor->field(i);
		const std::string& sName = fieldDescriptor->name();

		bool bSkip = (fieldDescriptor->is_repeated() || !reflection->HasField(*pParams, fieldDescriptor));
		if (bSkip)
			continue;

		const google::protobuf::FieldDescriptor::Type fieldDescriptorType = fieldDescriptor->type();
		if (google::protobuf::FieldDescriptor::TYPE_MESSAGE == fieldDescriptorType) {
			const google::protobuf::Message& rSubParams = reflection->GetMessage(*pParams, fieldDescriptor);
			CollectEntitySetFields(&rSubParams, vInKey, vOut);
		}
		else {
			// label is not repeated
			assert(fieldDescriptor->label() == google::protobuf::FieldDescriptor::LABEL_OPTIONAL ||
				fieldDescriptor->label() == google::protobuf::FieldDescriptor::LABEL_REQUIRED);

			bool bIsKey = false;
			for (auto& it : vInKey) {
				const std::string& sKey = it;
				if (sKey == sName) {
					bIsKey = true;
					break;
				}
			}

			// only set non-keys field
			if (!bIsKey) {
				vOut.emplace_back(sName);
			}
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::ParseSelect(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey, IDBStatement *pOutS) {
	const google::protobuf::Descriptor	*descriptor = entity.GetDescriptor();
	const google::protobuf::Reflection	*reflection = entity.GetReflection();

	std::vector<std::string> vValidKey;
	int nFieldCount, i;

	// validate key
	nFieldCount = descriptor->field_count();
	vValidKey.reserve(vInKey.size());

	for (i = 0; i < nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *field = descriptor->field(i);
		const std::string& sName = field->name();

		bool bSkip = (field->is_repeated() || !reflection->HasField(entity, field));
		if (bSkip)
			continue;

		// label is not repeated
		assert(field->label() == google::protobuf::FieldDescriptor::LABEL_OPTIONAL ||
			field->label() == google::protobuf::FieldDescriptor::LABEL_REQUIRED);

		for (auto& it : vInKey) {
			const std::string& sKey = it;
			if (sKey == sName) {
				vValidKey.push_back(sKey);
			}
		}
	}

	// prepare
	if (!pOutS->PrepareSelect(sTableName.c_str(), vValidKey))
		return false;

	// parse in params
	ParseEntityParams(pOutS, &entity, vValidKey, {});
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::ParseInsert(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey, IDBStatement *pOutS) {
	const google::protobuf::Descriptor	*descriptor = entity.GetDescriptor();
	const google::protobuf::Reflection	*reflection = entity.GetReflection();

	std::vector<std::string> vSet;
	CollectEntitySetFields(&entity, vInKey, vSet);

	// prepare
	if (!pOutS->PrepareInsert(sTableName.c_str(), vInKey, vSet))
		return false;

	// parse in params
	ParseInsertParams(pOutS, &entity, vInKey, vSet);
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::ParseUpdate(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey, bool bHackInsert, IDBStatement *pOutS) {
	const google::protobuf::Descriptor	*descriptor = entity.GetDescriptor();
	const google::protobuf::Reflection	*reflection = entity.GetReflection();

	std::vector<std::string> vSet;
	CollectEntitySetFields(&entity, vInKey, vSet);

	// prepare
	// use insert as update if enable hack
	if (bHackInsert) {
		// prepare
		if (!pOutS->PrepareInsert(sTableName.c_str(), vInKey, vSet))
			return false;

		// parse in params
		ParseInsertParams(pOutS, &entity, vInKey, vSet);
		return true;
	}
	else {
		if (vSet.size() > 0) {
			// prepare
			if (!pOutS->PrepareUpdate(sTableName.c_str(), vInKey, vSet))
				return false;

			// parse in params
			ParseEntityParams(pOutS, &entity, vInKey, vSet);
		}
		else {
			fprintf(stderr, "[tid(%d)][Protobuf2DbEngine::ParseUpdate()] table(%s) set is empty, nothing to update.\n",
				(int)::GetCurrentThreadId(), sTableName.c_str());
		}
		return true;
	}
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::ParseDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey, IDBStatement *pOutS) {
	// prepare
	if (!pOutS->PrepareDelete(sTableName.c_str(), vInKey))
		return false;

	// parse in params
	ParseEntityParams(pOutS, &entity, vInKey, {});
	return true;
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::ParseEntityParams(IDBStatement *pS, const google::protobuf::Message *pParams, const std::vector<std::string>& vInKey, const std::vector<std::string>& vSet) {
	std::vector<pb_input_param_t> vKeyParam, vSetParam;
	CollectEntityInputParams(pParams, vInKey, vSet, vKeyParam, vSetParam);

	// 1.set, 2.key
	std::vector<pb_input_param_t> vInParam;
	vInParam.reserve(vKeyParam.size() + vSetParam.size());
	vInParam.insert(vInParam.end(), vSetParam.begin(), vSetParam.end());
	vInParam.insert(vInParam.end(), vKeyParam.begin(), vKeyParam.end());

	for (auto& it : vInParam) {
		AddOneInputParam(pS, &it);
	}

	//pS->AddReturnValue();
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::ParseInsertParams(IDBStatement *pS, const google::protobuf::Message *pParams, const std::vector<std::string>& vInKey, const std::vector<std::string>& vSet) {
	std::vector<pb_input_param_t> vKeyParam, vSetParam;
	CollectEntityInputParams(pParams, vInKey, vSet, vKeyParam, vSetParam);

	// 1.key, 2.set
	std::vector<pb_input_param_t> vInParam;
	vInParam.reserve(vKeyParam.size() + vSetParam.size());
	vInParam.insert(vInParam.end(), vKeyParam.begin(), vKeyParam.end());
	vInParam.insert(vInParam.end(), vSetParam.begin(), vSetParam.end());

	for (auto& it : vInParam) {
		AddOneInputParam(pS, &it);
	}

	//pS->AddReturnValue();
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::ParseEntityResult(IDBResultSet& rs, std::vector<std::string>& vResult, google::protobuf::Message& record) {

	std::array<int, 256> arrIndex = { 0 };

	// first get by name
	if (rs.MoveNext()) {
		int nIndex = 0;
		AddFirstReturnRecord(&rs, &record, arrIndex, nIndex);

		//
		std::string sResult = record.SerializeAsString();
		vResult.push_back(std::move(sResult));
	}

	// more get by index
	while (rs.MoveNext()) {
		int nIndex = 0;
		AddReturnRecord(&rs, &record, arrIndex, nIndex);

		//
		std::string sResult = record.SerializeAsString();
		vResult.push_back(std::move(sResult));
	}
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::CollectProcInputParams(const google::protobuf::Message *pParams, std::vector<pb_input_param_t>& vOut) {

	const google::protobuf::Descriptor *descriptor = pParams->GetDescriptor();
	const google::protobuf::Reflection *reflection = pParams->GetReflection();

	int nFieldCount, i;

	//
	nFieldCount = descriptor->field_count();
	vOut.reserve(std::max<int>(256, nFieldCount * 2));

	//
	for (i = 0; i < nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *fieldDescriptor = descriptor->field(i);
		const std::string& sName = fieldDescriptor->name();

		bool bSkip = (fieldDescriptor->is_repeated() || !reflection->HasField(*pParams, fieldDescriptor));
		if (bSkip)
			continue;

		const google::protobuf::FieldDescriptor::Type fieldDescriptorType = fieldDescriptor->type();
		if (google::protobuf::FieldDescriptor::TYPE_MESSAGE == fieldDescriptorType) {
			const google::protobuf::Message& rSubParams = reflection->GetMessage(*pParams, fieldDescriptor);
			CollectProcInputParams(&rSubParams, vOut);
		}
		else {
			// label is not repeated
			assert(fieldDescriptor->label() == google::protobuf::FieldDescriptor::LABEL_OPTIONAL ||
				fieldDescriptor->label() == google::protobuf::FieldDescriptor::LABEL_REQUIRED);

			pb_input_param_t in_param;
			in_param.params = pParams;
			in_param.reflection = reflection;
			in_param.fieldDescriptor = fieldDescriptor;

			vOut.emplace_back(in_param);
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::ParseProc(const google::protobuf::Message *pStoredProc, const std::string& sProcName, IDBStatement *pOutS) {
	const google::protobuf::Descriptor	*descriptor = pStoredProc->GetDescriptor();
	const google::protobuf::Reflection	*reflection = pStoredProc->GetReflection();

	int nFieldCount, i;

	// parse
	nFieldCount = descriptor->field_count();

	for (i = 0; i < nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *field = descriptor->field(i);
		bool bSkip = (field->is_repeated() || !reflection->HasField(*pStoredProc, field));
		if (bSkip)
			continue;

		// label is not repeated
		assert(field->label() == google::protobuf::FieldDescriptor::LABEL_OPTIONAL ||
			field->label() == google::protobuf::FieldDescriptor::LABEL_REQUIRED);

		// parse field name
		const std::string &field_name = field->name();
		if (field_name == "param") {
			const google::protobuf::Message& param = reflection->GetMessage(*pStoredProc, field);

			int nParamsCount = ParseProcParamsCount(&param);
			if (!pOutS->PrepareStoredProcCall(sProcName.c_str(), nParamsCount))
				return false;

			//
			ParseProcParams(pOutS, &param);
		}
		// 		else if (field_name == "opt") {
		// 			const google::protobuf::Message& options = reflection->GetMessage(*pStoredProc, field);
		// 			ParseProcOptions(&options);
		// 		}

	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::ParseProcOptions(const google::protobuf::Message *pParam) {
	const google::protobuf::Descriptor	*descriptor = pParam->GetDescriptor();
	const google::protobuf::Reflection	*reflection = pParam->GetReflection();
	const std::string&					table_name = descriptor->name();

	int nFieldCount = descriptor->field_count();
	int i;
	for (i = 0; i < nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *field = descriptor->field(i);
		bool bSkip = (field->is_repeated() || !reflection->HasField(*pParam, field));
		if (bSkip)
			continue;

		assert(field->label() == google::protobuf::FieldDescriptor::LABEL_OPTIONAL ||
			field->label() == google::protobuf::FieldDescriptor::LABEL_REQUIRED);

		// parse bytes_is_blob, deprecated
		//		const std::string &name = field->name();
		// 		if (name == "bytes_is_blob") {
		// 			*pOutBytesIsBlob = reflection->GetBool(*pParam, field);
		// 		}
	}
}

//------------------------------------------------------------------------------
/**

*/
int
Protobuf2DbEngine::ParseProcParamsCount(const google::protobuf::Message *pParam) {
	int nCount = 0;
	const google::protobuf::Descriptor	*descriptor = pParam->GetDescriptor();
	const google::protobuf::Reflection	*reflection = pParam->GetReflection();
	std::string							strtypename = pParam->GetTypeName();

	int nFieldCount = descriptor->field_count();
	int i;
	for (i = 0; i < nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *field = descriptor->field(i);
		const google::protobuf::FieldDescriptor::Type field_type = field->type();
		const std::string &name = field->name();
		bool bSkip = (field->is_repeated() || !reflection->HasField(*pParam, field));
		if (bSkip)
			continue;

		//
		switch (field_type) {
		case google::protobuf::FieldDescriptor::TYPE_MESSAGE: {
			const google::protobuf::Message& rSub = reflection->GetMessage(*pParam, field);
			nCount += ParseProcParamsCount(&rSub);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_ENUM:
		case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
		case google::protobuf::FieldDescriptor::TYPE_FIXED64:
		case google::protobuf::FieldDescriptor::TYPE_INT64:
		case google::protobuf::FieldDescriptor::TYPE_UINT64:
		case google::protobuf::FieldDescriptor::TYPE_FIXED32:
		case google::protobuf::FieldDescriptor::TYPE_INT32:
		case google::protobuf::FieldDescriptor::TYPE_UINT32:
		case google::protobuf::FieldDescriptor::TYPE_STRING:
		case google::protobuf::FieldDescriptor::TYPE_BYTES:
		case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
		case google::protobuf::FieldDescriptor::TYPE_FLOAT:
		case google::protobuf::FieldDescriptor::TYPE_BOOL: {
			++nCount;
			break;
		}

		default:
			assert(false);
			break;
		}
	}
	return nCount;
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::ParseProcParams(IDBStatement *pS, const google::protobuf::Message *pParams) {
	std::vector<pb_input_param_t> vInParam;
	CollectProcInputParams(pParams, vInParam);

	for (auto& it : vInParam) {
		AddOneInputParam(pS, &it);
	}

	//pS->AddReturnValue();
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::ParseProcResult(IDBResultSet& rs, google::protobuf::Message& msg, const google::protobuf::Reflection& msg_reflection, const google::protobuf::FieldDescriptor& field_desc) {
	google::protobuf::Message *one_record;
	std::array<int, 256> arrIndex = { 0 };

	// first get by name
	if (rs.MoveNext()) {
		int nIndex = 0;
		one_record = msg_reflection.AddMessage(&msg, &field_desc);
		AddFirstReturnRecord(&rs, one_record, arrIndex, nIndex);
	}

	// more get by index
	while (rs.MoveNext()) {
		int nIndex = 0;
		one_record = msg_reflection.AddMessage(&msg, &field_desc);
		AddReturnRecord(&rs, one_record, arrIndex, nIndex);
	}
}

//------------------------------------------------------------------------------
/**

*/
bool
Protobuf2DbEngine::AddOneInputParam(IDBStatement *pS, pb_input_param_t *in_param) {

	const google::protobuf::Message *pParams = in_param->params;
	const google::protobuf::Reflection *reflection = in_param->reflection;
	const google::protobuf::FieldDescriptor *fieldDescriptor = in_param->fieldDescriptor;

	const std::string &name = fieldDescriptor->name();
	const google::protobuf::FieldDescriptor::Type fieldDescriptorType = fieldDescriptor->type();

	//
	switch (fieldDescriptorType) {
	case google::protobuf::FieldDescriptor::TYPE_ENUM: {
		const google::protobuf::EnumValueDescriptor* evd = reflection->GetEnum(*pParams, fieldDescriptor);
		int nVal = evd->number();
		pS->AddInputParameter((char *)name.c_str(), nVal);
		break;
	}

	case google::protobuf::FieldDescriptor::TYPE_SFIXED64: {
		// we use TYPE_SFIXED64 as time_t
		time_t tmVal = (time_t)reflection->GetInt64(*pParams, fieldDescriptor);
		pS->AddInputParameter((char *)name.c_str(), tmVal);
		break;
	}

	case google::protobuf::FieldDescriptor::TYPE_FIXED64:
	case google::protobuf::FieldDescriptor::TYPE_INT64: {
		int64_t nVal = reflection->GetInt64(*pParams, fieldDescriptor);
		pS->AddInputInt64Parameter((char *)name.c_str(), nVal);
		break;
	}

	case google::protobuf::FieldDescriptor::TYPE_UINT64: {
		int64_t nVal = (int64_t)reflection->GetUInt64(*pParams, fieldDescriptor);
		pS->AddInputInt64Parameter((char *)name.c_str(), nVal);
		break;
	}

	case google::protobuf::FieldDescriptor::TYPE_FIXED32:
	case google::protobuf::FieldDescriptor::TYPE_INT32: {
		int32_t nVal = reflection->GetInt32(*pParams, fieldDescriptor);
		pS->AddInputParameter((char *)name.c_str(), nVal);
		break;
	}

	case google::protobuf::FieldDescriptor::TYPE_UINT32: {
		int32_t nVal = reflection->GetUInt32(*pParams, fieldDescriptor);
		pS->AddInputParameter((char *)name.c_str(), nVal);
		break;
	}

	case google::protobuf::FieldDescriptor::TYPE_STRING: {
		std::string scratch;
		const std::string& str = reflection->GetStringReference(*pParams, fieldDescriptor, &scratch);
		// Must be a NUL terminated string, otherwise YOU SHOULD USE TYPE_BYTES
		pS->AddInputParameter((char *)name.c_str(), (char *)str.c_str());
		break;
	}

	case google::protobuf::FieldDescriptor::TYPE_BYTES: {
		std::string scratch;
		const std::string& str = reflection->GetStringReference(*pParams, fieldDescriptor, &scratch);
		pS->AddInputParameter((char *)name.c_str(), (unsigned char *)str.c_str(), (long)str.length());
		break;
	}

	case google::protobuf::FieldDescriptor::TYPE_DOUBLE: {
		double dVal = reflection->GetDouble(*pParams, fieldDescriptor);
		pS->AddInputParameter((char *)name.c_str(), dVal);
		break;
	}

	case google::protobuf::FieldDescriptor::TYPE_FLOAT: {
		double dVal = (double)reflection->GetFloat(*pParams, fieldDescriptor);
		pS->AddInputParameter((char *)name.c_str(), dVal);
		break;
	}

	case google::protobuf::FieldDescriptor::TYPE_BOOL: {
		bool b = reflection->GetBool(*pParams, fieldDescriptor);
		pS->AddInputParameter((char *)name.c_str(), (int)b);
		break;
	}

	default:
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::AddFirstReturnRecord(IDBResultSet *pRs, google::protobuf::Message *one_record, std::array<int, 256>& arrIndex, int& nIndex) {
	const google::protobuf::Descriptor	*descriptor = one_record->GetDescriptor();
	const google::protobuf::Reflection	*reflection = one_record->GetReflection();
	std::string							strtypename = one_record->GetTypeName();

	int nFieldIndex = 0;
	int nFieldType = 0;
	int nFieldCount = descriptor->field_count();
	int i;
	for (i = 0; i < nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *field = descriptor->field(i);
		const google::protobuf::FieldDescriptor::Type field_type = field->type();
		const std::string &name = field->name();
		bool repeated = field->is_repeated();
		if (repeated)
			continue;

		//
		switch (field_type) {
		case google::protobuf::FieldDescriptor::TYPE_MESSAGE: {
			google::protobuf::Message *pSub = reflection->MutableMessage(one_record, field);
			AddFirstReturnRecord(pRs, pSub, arrIndex, nIndex);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_SFIXED64: {
			// we use TYPE_SFIXED64 as time_t
			time_t tmVal = INT64_MIN;
			pRs->GetFieldValue((char *)name.c_str(), &tmVal, &nFieldIndex, &nFieldType);
#ifdef _DEBUG
			if (MYSQL_TYPE_DATETIME != nFieldType
				&& MYSQL_TYPE_DATE != nFieldType) {
				//
				PROTOBUF_2_DB_ENGINE_RAISE_PROTO_TYPE_EXCEPTION("TYPE_SFIXED64")
			}
#endif
			reflection->SetInt64(one_record, field, (int64_t)tmVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FIXED64:
		case google::protobuf::FieldDescriptor::TYPE_INT64: {
			int64_t nVal = INT64_MIN;
			pRs->GetFieldInt64Value((char *)name.c_str(), &nVal, &nFieldIndex, &nFieldType);
#ifdef _DEBUG
			if (MYSQL_TYPE_LONGLONG != nFieldType) {
				//
				PROTOBUF_2_DB_ENGINE_RAISE_PROTO_TYPE_EXCEPTION("TYPE_FIXED64, TYPE_INT64")
			}
#endif
			reflection->SetInt64(one_record, field, nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_UINT64: {
			int64_t nVal = INT64_MIN;
			pRs->GetFieldInt64Value((char *)name.c_str(), &nVal, &nFieldIndex, &nFieldType);
#ifdef _DEBUG
			if (MYSQL_TYPE_LONGLONG != nFieldType) {
				//
				PROTOBUF_2_DB_ENGINE_RAISE_PROTO_TYPE_EXCEPTION("TYPE_UINT64")
			}
#endif
			reflection->SetUInt64(one_record, field, (uint64_t)nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FIXED32:
		case google::protobuf::FieldDescriptor::TYPE_INT32: {
			int32_t nVal = INT32_MIN;
			pRs->GetFieldValue((char *)name.c_str(), &nVal, &nFieldIndex, &nFieldType);
#ifdef _DEBUG
			if (MYSQL_TYPE_LONG != nFieldType
				&& MYSQL_TYPE_SHORT != nFieldType
				&& MYSQL_TYPE_TINY != nFieldType) {
				//
				PROTOBUF_2_DB_ENGINE_RAISE_PROTO_TYPE_EXCEPTION("TYPE_FIXED32, TYPE_INT32")
			}
#endif
			reflection->SetInt32(one_record, field, nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_UINT32: {
			int32_t nVal = INT32_MIN;
			pRs->GetFieldValue((char *)name.c_str(), &nVal, &nFieldIndex, &nFieldType);
#ifdef _DEBUG
			if (MYSQL_TYPE_LONG != nFieldType
				&& MYSQL_TYPE_SHORT != nFieldType
				&& MYSQL_TYPE_TINY != nFieldType) {
				//
				PROTOBUF_2_DB_ENGINE_RAISE_PROTO_TYPE_EXCEPTION("TYPE_UINT32")
			}
#endif
			reflection->SetUInt32(one_record, field, (uint32_t)nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_STRING: {
			const char *scratch = nullptr;
			pRs->GetFieldValue((char *)name.c_str(), &scratch, &nFieldIndex, &nFieldType);
#ifdef _DEBUG
			if (MYSQL_TYPE_VARCHAR != nFieldType
				&& MYSQL_TYPE_STRING != nFieldType) {
				//
				PROTOBUF_2_DB_ENGINE_RAISE_PROTO_TYPE_EXCEPTION("TYPE_STRING")
			}
#endif
			if (scratch) {
				reflection->SetString(one_record, field, scratch);
			}
			else {
				reflection->SetString(one_record, field, "");
			}
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_BYTES: {
			unsigned char *blob = nullptr;
			unsigned long blobSize;
			pRs->GetFieldValue((char *)name.c_str(), &blob, &blobSize, &nFieldIndex, &nFieldType);
#ifdef _DEBUG
			if (MYSQL_TYPE_BLOB != nFieldType
				&& MYSQL_TYPE_VAR_STRING != nFieldType) {
				//
				PROTOBUF_2_DB_ENGINE_RAISE_PROTO_TYPE_EXCEPTION("TYPE_BYTES")
			}
#endif
			if (blob) {
				std::string scratch((char *)blob, blobSize);
				reflection->SetString(one_record, field, scratch);
			}
			else {
				reflection->SetString(one_record, field, "");
			}
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_DOUBLE: {
			double dVal = DBL_MIN;
			pRs->GetFieldDoubleValue((char *)name.c_str(), &dVal, &nFieldIndex, &nFieldType);
#ifdef _DEBUG
			if (MYSQL_TYPE_DOUBLE != nFieldType) {
				//
				PROTOBUF_2_DB_ENGINE_RAISE_PROTO_TYPE_EXCEPTION("TYPE_DOUBLE")
			}
#endif
			reflection->SetDouble(one_record, field, dVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FLOAT: {
			float fVal = FLT_MIN;
			pRs->GetFieldFloatValue((char *)name.c_str(), &fVal, &nFieldIndex, &nFieldType);
#ifdef _DEBUG
			if (MYSQL_TYPE_FLOAT != nFieldType) {
				//
				PROTOBUF_2_DB_ENGINE_RAISE_PROTO_TYPE_EXCEPTION("TYPE_FLOAT")
			}
#endif
			reflection->SetFloat(one_record, field, fVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_BOOL: {
			int nVal = INT_MIN;
			pRs->GetFieldValue((char *)name.c_str(), &nVal, &nFieldIndex, &nFieldType);
#ifdef _DEBUG
			if (MYSQL_TYPE_LONG != nFieldType
				&& MYSQL_TYPE_SHORT != nFieldType
				&& MYSQL_TYPE_TINY != nFieldType) {
				//
				PROTOBUF_2_DB_ENGINE_RAISE_PROTO_TYPE_EXCEPTION("TYPE_BOOL")
			}
#endif
			reflection->SetBool(one_record, field, (0 != nVal));
			break;
		}

		default:
			break;
		}

		//
		arrIndex[nIndex++] = nFieldIndex;
		nFieldType = 0;
	}

}

//------------------------------------------------------------------------------
/**

*/
void
Protobuf2DbEngine::AddReturnRecord(IDBResultSet *pRs, google::protobuf::Message *one_record, std::array<int, 256>& arrIndex, int& nIndex) {
	const google::protobuf::Descriptor	*descriptor = one_record->GetDescriptor();
	const google::protobuf::Reflection	*reflection = one_record->GetReflection();
	std::string							strtypename = one_record->GetTypeName();

	int nFieldCount = descriptor->field_count();
	int i;
	for (i = 0; i < nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *field = descriptor->field(i);
		const google::protobuf::FieldDescriptor::Type field_type = field->type();
		const std::string &name = field->name();
		bool repeated = field->is_repeated();
		if (repeated)
			continue;

		//
		switch (field_type) {
		case google::protobuf::FieldDescriptor::TYPE_MESSAGE: {
			google::protobuf::Message *pSub = reflection->MutableMessage(one_record, field);
			AddReturnRecord(pRs, pSub, arrIndex, nIndex);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_SFIXED64: {
			// we use TYPE_SFIXED64 as time_t
			time_t tmVal = INT64_MIN;
			pRs->GetFieldValue(arrIndex[nIndex], &tmVal);
			reflection->SetInt64(one_record, field, (int64_t)tmVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FIXED64:
		case google::protobuf::FieldDescriptor::TYPE_INT64: {
			int64_t nVal = INT64_MIN;
			pRs->GetFieldInt64Value(arrIndex[nIndex], &nVal);
			reflection->SetInt64(one_record, field, nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_UINT64: {
			int64_t nVal = INT64_MIN;
			pRs->GetFieldInt64Value(arrIndex[nIndex], &nVal);
			reflection->SetUInt64(one_record, field, (uint64_t)nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FIXED32:
		case google::protobuf::FieldDescriptor::TYPE_INT32: {
			int32_t nVal = INT32_MIN;
			pRs->GetFieldValue(arrIndex[nIndex], &nVal);
			reflection->SetInt32(one_record, field, nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_UINT32: {
			int32_t nVal = INT32_MIN;
			pRs->GetFieldValue(arrIndex[nIndex], &nVal);
			reflection->SetUInt32(one_record, field, (uint32_t)nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_STRING: {
			const char *scratch = nullptr;
			pRs->GetFieldValue(arrIndex[nIndex], &scratch);
			if (scratch) {
				reflection->SetString(one_record, field, scratch);
			}
			else {
				reflection->SetString(one_record, field, "");
			}
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_BYTES: {
			unsigned char *blob = nullptr;
			unsigned long blobSize;
			pRs->GetFieldValue(arrIndex[nIndex], &blob, &blobSize);
			if (blob) {
				std::string scratch((char *)blob, blobSize);
				reflection->SetString(one_record, field, scratch);
			}
			else {
				reflection->SetString(one_record, field, "");
			}
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_DOUBLE: {
			double dVal = DBL_MIN;
			pRs->GetFieldDoubleValue(arrIndex[nIndex], &dVal);
			reflection->SetDouble(one_record, field, dVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FLOAT: {
			float fVal = FLT_MIN;
			pRs->GetFieldFloatValue(arrIndex[nIndex], &fVal);
			reflection->SetFloat(one_record, field, fVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_BOOL: {
			int nVal = INT_MIN;
			pRs->GetFieldValue(arrIndex[nIndex], &nVal);
			reflection->SetBool(one_record, field, (0 != nVal));
			break;
		}

		default:
			break;
		}

		//
		++nIndex;
	}

}

/* -- EOF -- */