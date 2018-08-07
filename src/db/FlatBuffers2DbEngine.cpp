//------------------------------------------------------------------------------
//  FlatBuffers2DbEngine.cpp
//  (C) 2011 n.lee
//------------------------------------------------------------------------------
#include "FlatBuffers2DbEngine.h"

#include <time.h>
#include <locale.h>

#include "../utils/util.h"
#include "../platform/utilities.h"
#include "../snprintf/snprintf.h"


//#include "MySQLSession.h" // it changed _WIN32_WINNT to 0x0500
#include "ZdbConnectionPoolSession.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
FlatBuffers2DbEngine::FlatBuffers2DbEngine()
	: _db(nullptr) {

}

//------------------------------------------------------------------------------
/**

*/
FlatBuffers2DbEngine::~FlatBuffers2DbEngine() {
	if (_db) {
		_db->Close();
		delete _db;
	}

}

//------------------------------------------------------------------------------
/**

*/
void
FlatBuffers2DbEngine::OnInit(const char * sURL, const char *sUser, const char *sPass, const char *sSchema) {
	char chLine[512];
	char chFullPath[1024];

	//
	FILE *fp = fopen("dynamic_fbs/fbs.def", "r");
	if (fp) {
		while (nullptr != fgets(chLine, sizeof(chLine), fp)) {
			// remove tail '\r' and '\n'
			int nIndex = strlen(chLine);
			while (--nIndex > 0) {
				if ('\r' != chLine[nIndex] && '\n' != chLine[nIndex]) {
					break;
				}
				chLine[nIndex] = '\0';
			}

			//
			if (nIndex > 0) {
				std::string sContent;

				fprintf(stderr, "\n    \t[FlatBuffers2DbEngine] import schema file(%s)...", chLine);

				// Load a schema file
				o_snprintf(chFullPath, sizeof(chFullPath), "dynamic_fbs/%s", chLine);
				bool bSucc = flatbuffers::LoadFile(chFullPath, true, &sContent);
				if (bSucc) {
					// Verify it, just in case:
					flatbuffers::Verifier verifier(
						reinterpret_cast<const uint8_t *>(sContent.c_str()), sContent.length());
					bSucc = reflection::VerifySchemaBuffer(verifier);
					if (bSucc) {
						char chKey[512];

						char *chArray[5] = { { 0 },{ 0 },{ 0 },{ 0 },{ 0 } };
						split(chLine, strlen(chLine), chArray, 3, '.');
						o_snprintf(chKey, sizeof(chKey), "%s.%s", chArray[0], chArray[1]);

						_mapRootTable[chKey] = sContent;
					}
				}
			}
		}
		fclose(fp);
		fprintf(stderr, "\n");
	}

	// init db
	_db = new ZdbDriver(sURL, sUser, sPass, sSchema);
}

//------------------------------------------------------------------------------
/**

*/
void
FlatBuffers2DbEngine::OnDelete() {

}

//------------------------------------------------------------------------------
/**

*/
void
FlatBuffers2DbEngine::Open() {
	while (!_db->Open()) {
		util_sleep(15 * 1000);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
FlatBuffers2DbEngine::Close() {
	_db->Close();
	_opened = false;
}

//------------------------------------------------------------------------------
/**

*/
int
FlatBuffers2DbEngine::ExecStoredProc(const char *sTableName, flatbuffers::FlatBufferBuilder *pStoredProc) {
	int32_t returnValue = 0;

	//
	int reqUUID = 0;
	int pageNo = 0, pageRange = 0;

	// 	MySQLStoredProc proc(_db);
	// 	MySQLResultSet rs(_db, &proc);
// 	ZdbStoredProc proc(_db);
// 	ZdbResultSet rs(_db, &proc);

	char callSql[512];
	bool needResponse = false;
	bool bytesIsBlob = false;

	std::map<std::string, std::string>::iterator it = _mapRootTable.find(sTableName);
	if (it != _mapRootTable.end()) {
		std::string& sContent = it->second;

		const reflection::Schema *schema = reflection::GetSchema(sContent.c_str());
// 		std::string ext = schema->file_ext()->str();
// 		std::string ident = schema->file_ident()->str();
		const reflection::Object *root_table = schema->root_table();
		const char *root_table_name = root_table->name()->c_str();
		const flatbuffers::Vector<flatbuffers::Offset<reflection::Field>> *fields = root_table->fields();
		const reflection::Field *field;
		std::string field_name;
		flatbuffers::Vector<flatbuffers::Offset<reflection::Field>>::const_iterator it = fields->begin(), itEnd = fields->end();

		while (it != itEnd) {
			field = (*it);

			// parse name
			const std::string& field_name = field->name()->str();
			if (field_name == "req_uuid") {
// 				auto hp_field_ptr = fields->LookupByKey("req_uuid");
// 				flatbuffers::GetAnyFieldI(root, hp_field);
// 
// 				reflection::
// 				reqUUID = reflection->GetInt32(*pStoredProc, field);
			}
			else if (field_name == "opt") {
// 				const flatbuffers::FlatBufferBuilder& options = reflection->GetMessage(*pStoredProc, field);
// 				ParseOptions(&options, &needResponse, &bytesIsBlob);
			}
			else if (field_name == "param") {
// 				const flatbuffers::FlatBufferBuilder& param = reflection->GetMessage(*pStoredProc, field);
// 
// 				int nParamsCount = ParseInParamsCount(&proc, &param);
// 
// 				//
// 				o_snprintf(callSql, sizeof(callSql), "call %s (", table_name.c_str());
// 
// 				// construct sql
// 				size_t len = strlen(callSql);
// 				for (j = 0; j < nParamsCount; ++j) {
// 					if (j > 0) { callSql[len++] = ','; }
// 					callSql[len++] = '?';
// 				}
// 				callSql[len++] = ')';
// 				callSql[len++] = '\0';
// 
// 				_db->PrepareStoredProcedure(&proc, callSql);
// 
// 				//
// 				ParseInParams(&proc, &param, bytesIsBlob);
			}

			++it;
		}
	}
	
	/*
	const google::protobuf::Descriptor	*descriptor = pStoredProc->GetDescriptor();
	const google::protobuf::Reflection	*reflection = pStoredProc->GetReflection();
	std::string							table_name = descriptor->name();



	// parse
	int nFieldCount = descriptor->field_count();
	int i, j;
	for (i = 0; i != nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *field = descriptor->field(i);
		bool repeated = field->is_repeated();

		if (repeated || !reflection->HasField(*pStoredProc, field))
			continue;

		assert(field->label() == google::protobuf::FieldDescriptor::LABEL_OPTIONAL ||
			field->label() == google::protobuf::FieldDescriptor::LABEL_REQUIRED);

		// parse name
		const std::string &name = field->name();
		if (name == "req_uuid") {
			reqUUID = reflection->GetInt32(*pStoredProc, field);
		}
		else if (name == "opt") {
			const flatbuffers::FlatBufferBuilder& options = reflection->GetMessage(*pStoredProc, field);
			ParseOptions(&options, &needResponse, &bytesIsBlob);
		}
		else if (name == "param") {
			const flatbuffers::FlatBufferBuilder& param = reflection->GetMessage(*pStoredProc, field);

			int nParamsCount = ParseInParamsCount(&proc, &param);

			//
			o_snprintf(callSql, sizeof(callSql), "call %s (", table_name.c_str());

			// construct sql
			size_t len = strlen(callSql);
			for (j = 0; j < nParamsCount; ++j) {
				if (j > 0) { callSql[len++] = ','; }
				callSql[len++] = '?';
			}
			callSql[len++] = ')';
			callSql[len++] = '\0';

			_db->PrepareStoredProcedure(&proc, callSql);

			//
			ParseInParams(&proc, &param, bytesIsBlob);
		}
	}

	//log
	//  	_refLog->logprint(LOG_LEVEL_INFO, "\n================================================================");
	//  	_refLog->logprint(LOG_LEVEL_INFO, "SP[%s] -- sql(%s), response=%d, blob=%d.\n", _call_sql, sSql.c_str(), _needResponse, _bytesIsBlob);

	// exec
	if (needResponse) {
		_db->ExecuteStoredProcedure(&proc, &rs);

		// return records
		const google::protobuf::FieldDescriptor *return_record_list = descriptor->FindFieldByName("return_record_list");
		if (return_record_list && rs.IsOpened()) {
			flatbuffers::FlatBufferBuilder *return_record = nullptr;
 			while (rs.MoveNext()) {
 				return_record = reflection->AddMessage(pStoredProc, return_record_list);
 				AddReturnRecord(&rs, return_record, bytesIsBlob);
 			}
		}

		//
		rs.Close();

		//
		returnValue = 1;
	}
	else {
		_db->ExecuteStoredProcedure(&proc);

		//
		returnValue = -1;
	}
	*/
	//
	return returnValue;
}

//------------------------------------------------------------------------------
/**

*/
void
FlatBuffers2DbEngine::ParseOptions(const flatbuffers::FlatBufferBuilder *pParam, bool *pOutNeedResponse, bool *pOutBytesIsBlob) {
	/*
	const google::protobuf::Descriptor	*descriptor = pParam->GetDescriptor();
	const google::protobuf::Reflection	*reflection = pParam->GetReflection();
	std::string							table_name = descriptor->name();

	int nFieldCount = descriptor->field_count();
	int i;
	for (i = 0; i != nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *field = descriptor->field(i);
		bool repeated = field->is_repeated();

		if (repeated || !reflection->HasField(*pParam, field))
			continue;

		assert(field->label() == google::protobuf::FieldDescriptor::LABEL_OPTIONAL ||
			field->label() == google::protobuf::FieldDescriptor::LABEL_REQUIRED);

		// parse name
		const std::string &name = field->name();
		if (name == "need_response") {
			*pOutNeedResponse = reflection->GetBool(*pParam, field);
		}
		else if (name == "bytes_is_blob") {
			*pOutBytesIsBlob = reflection->GetBool(*pParam, field);
		}
	}
	*/
}

//------------------------------------------------------------------------------
/**

*/
int
FlatBuffers2DbEngine::ParseInParamsCount(IDBStoredProc *pProc, const flatbuffers::FlatBufferBuilder *pParam) {
	int nCount = 0;

	/*
	const google::protobuf::Descriptor	*descriptor = pParam->GetDescriptor();
	const google::protobuf::Reflection	*reflection = pParam->GetReflection();
	std::string							strtypename = pParam->GetTypeName();

	int nFieldCount = descriptor->field_count();
	int i;
	for (i = 0; i != nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *field = descriptor->field(i);
		const google::protobuf::FieldDescriptor::Type field_type = field->type();
		const std::string &name = field->name();
		bool repeated = field->is_repeated();
		bool bFieldIsSet = reflection->HasField(*pParam, field);

		//
		if (repeated || !bFieldIsSet)
			continue;

		//
		switch (field_type) {
		case google::protobuf::FieldDescriptor::TYPE_MESSAGE: {
			const flatbuffers::FlatBufferBuilder& rSub = reflection->GetMessage(*pParam, field);
			nCount += ParseInParamsCount(pProc, &rSub);
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
			break;
		}
	}
	*/
	return nCount;
}

//------------------------------------------------------------------------------
/**

*/
void
FlatBuffers2DbEngine::ParseInParams(IDBStoredProc *pProc, const flatbuffers::FlatBufferBuilder *pParam, bool bytesIsBlob) {
	//pProc->AddReturnValue();
	AddInputParameter(pProc, pParam, bytesIsBlob);

}

//------------------------------------------------------------------------------
/**

*/
void
FlatBuffers2DbEngine::AddInputParameter(IDBStoredProc *pProc, const flatbuffers::FlatBufferBuilder *pParam, bool bytesIsBlob) {
	/*
	const google::protobuf::Descriptor	*descriptor = pParam->GetDescriptor();
	const google::protobuf::Reflection	*reflection = pParam->GetReflection();
	std::string							strtypename = pParam->GetTypeName();

	int nFieldCount = descriptor->field_count();
	int i;
	for (i = 0; i != nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *field = descriptor->field(i);
		const google::protobuf::FieldDescriptor::Type field_type = field->type();
		const std::string &name = field->name();
		bool repeated = field->is_repeated();
		bool bFieldIsSet = reflection->HasField(*pParam, field);

		//
		if (repeated || !bFieldIsSet)
			continue;

		//
		switch (field_type) {
		case google::protobuf::FieldDescriptor::TYPE_MESSAGE: {
			const flatbuffers::FlatBufferBuilder& rSub = reflection->GetMessage(*pParam, field);
			AddInputParameter(pProc, &rSub, bytesIsBlob);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_ENUM: {
			const google::protobuf::EnumValueDescriptor* evd = reflection->GetEnum(*pParam, field);
			int nVal = evd->number();
			pProc->AddInputParameter((char *)name.c_str(), nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_SFIXED64: {
			// we use TYPE_SFIXED64 as time_t
			time_t tmVal = (time_t)reflection->GetInt64(*pParam, field);
			pProc->AddInputParameter((char *)name.c_str(), tmVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FIXED64:
		case google::protobuf::FieldDescriptor::TYPE_INT64: {
			int64_t nVal = reflection->GetInt64(*pParam, field);
			pProc->AddInputInt64Parameter((char *)name.c_str(), nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_UINT64: {
			int64_t nVal = (int64_t)reflection->GetUInt64(*pParam, field);
			pProc->AddInputInt64Parameter((char *)name.c_str(), nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FIXED32:
		case google::protobuf::FieldDescriptor::TYPE_INT32: {
			int32_t nVal = reflection->GetInt32(*pParam, field);
			pProc->AddInputParameter((char *)name.c_str(), nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_UINT32: {
			int32_t nVal = reflection->GetUInt32(*pParam, field);
			pProc->AddInputParameter((char *)name.c_str(), nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_STRING: {
			std::string scratch;
			const std::string& str = reflection->GetStringReference(*pParam, field, &scratch);
			pProc->AddInputParameter((char *)name.c_str(), (char *)str.c_str());
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_BYTES: {
			std::string scratch;

			if (bytesIsBlob) {
				const std::string& str = reflection->GetStringReference(*pParam, field, &scratch);
				pProc->AddInputParameter((char *)name.c_str(), (unsigned char *)str.c_str(), str.length());
			}
			else {
				const std::string& str = reflection->GetStringReference(*pParam, field, &scratch);
				pProc->AddInputParameter((char *)name.c_str(), (char *)str.c_str());
			}
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_DOUBLE: {
			double dVal = reflection->GetDouble(*pParam, field);
			pProc->AddInputParameter((char *)name.c_str(), dVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FLOAT: {
			double dVal = (double)reflection->GetFloat(*pParam, field);
			pProc->AddInputParameter((char *)name.c_str(), dVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_BOOL: {
			bool b = reflection->GetBool(*pParam, field);
			pProc->AddInputParameter((char *)name.c_str(), (int)b);
			break;
		}

		default:
			break;
		}
	}
	*/
}

//------------------------------------------------------------------------------
/**

*/
void
FlatBuffers2DbEngine::AddReturnRecord(IDBResultSet *pRs, flatbuffers::FlatBufferBuilder *pReturnRecord, bool bytesIsBlob) {
	/*
	const google::protobuf::Descriptor	*descriptor = pReturnRecord->GetDescriptor();
	const google::protobuf::Reflection	*reflection = pReturnRecord->GetReflection();
	std::string							strtypename = pReturnRecord->GetTypeName();

	int nFieldCount = descriptor->field_count();
	int i;
	for (i = 0; i != nFieldCount; ++i) {
		const google::protobuf::FieldDescriptor *field = descriptor->field(i);
		const google::protobuf::FieldDescriptor::Type field_type = field->type();
		const std::string &name = field->name();
		bool repeated = field->is_repeated();

		//
		if (repeated)
			continue;

		//
		switch (field_type) {
		case google::protobuf::FieldDescriptor::TYPE_MESSAGE: {
			flatbuffers::FlatBufferBuilder *pSub = reflection->MutableMessage(pReturnRecord, field);
			AddReturnRecord(pRs, pSub, bytesIsBlob);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_SFIXED64: {
			// we use TYPE_SFIXED64 as time_t
			time_t tmVal;
			pRs->GetFieldValue((char *)name.c_str(), &tmVal);
			reflection->SetInt64(pReturnRecord, field, (int64_t)tmVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FIXED64:
		case google::protobuf::FieldDescriptor::TYPE_INT64: {
			int64_t nVal;
			pRs->GetFieldInt64Value((char *)name.c_str(), &nVal);
			reflection->SetInt64(pReturnRecord, field, (int64_t)nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_UINT64: {
			int64_t nVal;
			pRs->GetFieldInt64Value((char *)name.c_str(), &nVal);
			reflection->SetUInt64(pReturnRecord, field, (uint64_t)nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FIXED32:
		case google::protobuf::FieldDescriptor::TYPE_INT32: {
			int32_t nVal;
			pRs->GetFieldValue((char *)name.c_str(), &nVal);
			reflection->SetInt32(pReturnRecord, field, (int32_t)nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_UINT32: {
			int32_t nVal;
			pRs->GetFieldValue((char *)name.c_str(), &nVal);
			reflection->SetUInt32(pReturnRecord, field, (uint32_t)nVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_STRING: {
			std::string scratch;
			pRs->GetFieldValue((char *)name.c_str(), scratch);
			reflection->SetString(pReturnRecord, field, scratch);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_BYTES: {
			std::string scratch;

			if (bytesIsBlob) {
				unsigned char blob[65536];
				long blobSize;

				pRs->GetFieldValue((char *)name.c_str(), blob, &blobSize);
				std::string scratch((char *)blob, blobSize);
				reflection->SetString(pReturnRecord, field, scratch);
			}
			else {
				pRs->GetFieldValue((char *)name.c_str(), scratch);
				reflection->SetString(pReturnRecord, field, scratch);
			}
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_DOUBLE: {
			double dVal;
			pRs->GetFieldDoubleValue((char *)name.c_str(), &dVal);
			reflection->SetDouble(pReturnRecord, field, dVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_FLOAT: {
			float fVal;
			pRs->GetFieldFloatValue((char *)name.c_str(), &fVal);
			reflection->SetFloat(pReturnRecord, field, fVal);
			break;
		}

		case google::protobuf::FieldDescriptor::TYPE_BOOL: {
			int nVal;
			pRs->GetFieldValue((char *)name.c_str(), &nVal);
			reflection->SetBool(pReturnRecord, field, (0 != nVal));
			break;
		}

		default:
			break;
		}
	}
	*/
}

/* -- EOF -- */