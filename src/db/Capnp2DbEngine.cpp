//------------------------------------------------------------------------------
//  Capnp2DbEngine.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "Capnp2DbEngine.h"

#include <capnp/schema-parser.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <capnp/schema.h>
#include <capnp/dynamic.h>

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <kj/windows-sanity.h>

#include <time.h>
#include <locale.h>
#include <array>

#include "ZdbConnectionPoolSession.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
Capnp2DbEngine::Capnp2DbEngine() {
	
}

//------------------------------------------------------------------------------
/**

*/
Capnp2DbEngine::~Capnp2DbEngine() {
	if (_db) {
		_db->Close();
		delete _db;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
Capnp2DbEngine::OnInit(StdLog *pLog, const char * sURL, const char *sUser, const char *sPass, const char *sSchema, int nPoolSize) {
	// init db
	_db = new ZdbDriver(pLog, sURL, sUser, sPass, sSchema, nPoolSize);
}

//------------------------------------------------------------------------------
/**

*/
void
Capnp2DbEngine::OnDelete() {
	
}

//------------------------------------------------------------------------------
/**

*/
void
Capnp2DbEngine::Open() {
	while (!_db->Open()) {
		util_sleep(15 * 1000);
	}
	_opened = true;
}

//------------------------------------------------------------------------------
/**

*/
void
Capnp2DbEngine::Close() {
	_db->Close();
	_opened = false;
}

//------------------------------------------------------------------------------
/**

*/
bool
Capnp2DbEngine::ExecQuery(::capnp::MessageReader& storedProc, ::capnp::StructSchema& root_schema, ::capnp::MessageBuilder& out) {
	::capnp::DynamicStruct::Reader root = storedProc.getRoot<::capnp::DynamicStruct, ::capnp::StructSchema>(root_schema);
	auto& root_proto = root_schema.getProto();
	const std::string& full_proc_name = root_proto.getDisplayName();
	auto proc_name_offset = root_proto.getDisplayNamePrefixLength();
	const char *sProcName = full_proc_name.c_str() + proc_name_offset;

	ZdbStoredProc proc(_db, sProcName);
	ParseProc(root, sProcName, &proc);

	// exec query
	if (1) {	
		ZdbResultSet rs(_db, &proc);
		proc.Call(&rs);

		// return records
		if (rs.IsOpened()) {
			//
			::capnp::Orphanage root_orphanage = out.getOrphanage();
			::capnp::DynamicStruct::Builder root_struct_builder = out.initRoot<::capnp::DynamicStruct, ::capnp::StructSchema>(root_schema);

			//
			KJ_IF_MAYBE(fld_return_record_list, root_schema.findFieldByName("return_record_list")) {
				//
				ParseResult(rs, *fld_return_record_list, root_orphanage, root_struct_builder);
			}
			else {
				// multiple record set
				KJ_IF_MAYBE(fld_return_recordsets, root_schema.findFieldByName("return_recordsets")) {
					if (fld_return_recordsets->getType().isStruct()) {
						//
						::capnp::DynamicStruct::Builder return_recordsets_struct_builder = root_struct_builder.init(*fld_return_recordsets).as<::capnp::DynamicStruct>();
						::capnp::StructSchema structType = fld_return_recordsets->getType().asStruct();
						for (auto field : structType.getFields()) {
							//
							ParseResult(rs, field, root_orphanage, return_recordsets_struct_builder);

							//
							if (!rs.HasMoreResultSets()) {
								break;
							}
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
Capnp2DbEngine::ExecUpdate(::capnp::MessageReader& storedProc, ::capnp::StructSchema& root_schema) {
	::capnp::DynamicStruct::Reader root = storedProc.getRoot<::capnp::DynamicStruct, ::capnp::StructSchema>(root_schema);
	auto& root_proto = root_schema.getProto();
	const std::string& full_proc_name = root_proto.getDisplayName();
	auto proc_name_offset = root_proto.getDisplayNamePrefixLength();
	const char *sProcName = full_proc_name.c_str() + proc_name_offset;

	ZdbStoredProc proc(_db, sProcName);
	ParseProc(root, sProcName, &proc);

	// exec update
	proc.Call();
	return true;
}

//------------------------------------------------------------------------------
/**

*/
void
Capnp2DbEngine::ParseProc(::capnp::DynamicStruct::Reader& root, const char *sProcName, IDBStoredProc *pOutProc) {
	auto structValue = root.as<::capnp::DynamicStruct>();
	for (auto field : structValue.getSchema().getFields()) {
		if (!structValue.has(field)) continue;

		const std::string &field_name = field.getProto().getName();
		// parse name
		if (field_name == "param") {
			::capnp::DynamicValue::Reader field_reader = structValue.get(field);
			switch (field_reader.getType()) {
			case ::capnp::DynamicValue::STRUCT: {
				::capnp::DynamicStruct::Reader& param = field_reader.as<::capnp::DynamicStruct>();
				int nParamsCount = ParseParamsCount(pOutProc, param);
				pOutProc->Prepare(sProcName, nParamsCount);

				//
				ParseParams(pOutProc, param);
				break;
			}

			default:
				break;
			}
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
int
Capnp2DbEngine::ParseParamsCount(IDBStoredProc *pProc, ::capnp::DynamicStruct::Reader& param) {
	int nCount = 0;
	
	for (auto field : param.getSchema().getFields()) {
		if (!param.has(field)) continue;

		::capnp::schema::Type::Which which = field.getType().which();
		switch (which) {
		case ::capnp::schema::Type::BOOL:
		case ::capnp::schema::Type::INT8:
		case ::capnp::schema::Type::INT16:
		case ::capnp::schema::Type::INT32:
		case ::capnp::schema::Type::INT64:
		case ::capnp::schema::Type::UINT8:
		case ::capnp::schema::Type::UINT16:
		case ::capnp::schema::Type::UINT32:
		case ::capnp::schema::Type::UINT64:
		case ::capnp::schema::Type::FLOAT32:
		case ::capnp::schema::Type::FLOAT64:
		case ::capnp::schema::Type::TEXT:
		case ::capnp::schema::Type::DATA:
		case ::capnp::schema::Type::ENUM: {
			++nCount;
			break;
		}

		case ::capnp::schema::Type::STRUCT: {
			::capnp::DynamicValue::Reader field_reader = param.get(field);
			::capnp::DynamicStruct::Reader& subparam = field_reader.as<::capnp::DynamicStruct>();
			nCount += ParseParamsCount(pProc, subparam);
			break;
		}

		default:
			break;
		}
	}
	return nCount;
}

//------------------------------------------------------------------------------
/**

*/
void
Capnp2DbEngine::ParseParams(IDBStoredProc *pProc, ::capnp::DynamicStruct::Reader& param) {
	for (auto field : param.getSchema().getFields()) {
		if (!param.has(field)) continue;

		::capnp::DynamicValue::Reader field_reader = param.get(field);
		const std::string &field_name = field.getProto().getName();
		::capnp::schema::Type::Which which = field.getType().which();
		switch (which) {
		case ::capnp::schema::Type::BOOL:
		case ::capnp::schema::Type::INT8:
		case ::capnp::schema::Type::INT16:
		case ::capnp::schema::Type::INT32:
		case ::capnp::schema::Type::UINT8:
		case ::capnp::schema::Type::UINT16:
		case ::capnp::schema::Type::UINT32: {
			int32_t nVal = field_reader.as<int32_t>();
			pProc->AddInputParameter((char *)field_name.c_str(), nVal);
			break;
		}

		case ::capnp::schema::Type::INT64: {
			int64_t nVal = field_reader.as<int64_t>();
			pProc->AddInputInt64Parameter((char *)field_name.c_str(), nVal);
			break;
		}

		case ::capnp::schema::Type::UINT64: {
			// we use UINT64 as time_t
			time_t tmVal = (time_t)field_reader.as<uint64_t>();
			pProc->AddInputParameter((char *)field_name.c_str(), tmVal);
			break;
		}

		case ::capnp::schema::Type::FLOAT32:
		case ::capnp::schema::Type::FLOAT64: {
			double dVal = field_reader.as<double>();
			pProc->AddInputParameter((char *)field_name.c_str(), dVal);
			break;
		}

		case ::capnp::schema::Type::TEXT: {
			::kj::StringPtr chars = field_reader.as<::capnp::Text>();
			pProc->AddInputParameter((char *)field_name.c_str(), chars.cStr());
			break;
		}
		case ::capnp::schema::Type::DATA: {
			::kj::ArrayPtr<const kj::byte> bytes = field_reader.as<::capnp::Data>();
			pProc->AddInputParameter((char *)field_name.c_str(), (unsigned char *)bytes.begin(), bytes.size());
			break;
		}

		case ::capnp::schema::Type::ENUM: {
			auto enumValue = field_reader.as<::capnp::DynamicEnum>();
			int nVal = enumValue.getRaw();
			pProc->AddInputParameter((char *)field_name.c_str(), nVal);
			break;
		}

		case ::capnp::schema::Type::STRUCT: {
			::capnp::DynamicStruct::Reader& subparam = field_reader.as<::capnp::DynamicStruct>();
			ParseParams(pProc, subparam);
			break;
		}

		default:
			break;
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
Capnp2DbEngine::ParseResult(IDBResultSet& rs, ::capnp::StructSchema::Field& struct_schema_field, ::capnp::Orphanage& root_orphanage, ::capnp::DynamicStruct::Builder& parent_struct_builder) {
	::capnp::Type& filed_type = struct_schema_field.getType();
	if (filed_type.isList()) {
		::capnp::ListSchema listType = filed_type.asList();
		if (listType.getElementType().isStruct()) {
			::capnp::StructSchema structElementType = listType.getStructElementType();

			//
			std::array<int, 256> arrIndex = { 0 };
			std::vector<::capnp::Orphan<::capnp::DynamicStruct> > vTmp;
			vTmp.reserve(256);

			// first get by name
			if (rs.MoveNext()) {
				int nIndex = 0;
				::capnp::Orphan<::capnp::DynamicStruct> return_record_orphan = root_orphanage.newOrphan(structElementType);
				::capnp::DynamicStruct::Builder& one_record = return_record_orphan.get();
				AddFirstReturnRecord(rs, one_record, arrIndex, nIndex);
				vTmp.push_back(kj::mv(return_record_orphan));
			}

			// more get by index
			while (rs.MoveNext()) {
				int nIndex = 0;
				::capnp::Orphan<::capnp::DynamicStruct> return_record_orphan = root_orphanage.newOrphan(structElementType);
				::capnp::DynamicStruct::Builder& one_record = return_record_orphan.get();
				AddReturnRecord(rs, one_record, arrIndex, nIndex);
				vTmp.push_back(kj::mv(return_record_orphan));
			}

			int nRows = vTmp.size();
			if (nRows > 0) {
				::capnp::DynamicList::Builder list_builder = parent_struct_builder.init(struct_schema_field, nRows).as<::capnp::DynamicList>();

				//
				int i;
				for (i = 0; i < nRows; ++i) {
					list_builder.adopt(i, kj::mv(vTmp[i]));
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
Capnp2DbEngine::AddFirstReturnRecord(IDBResultSet& rs, ::capnp::DynamicStruct::Builder& return_record, std::array<int, 256>& arrIndex, int& nIndex) {
	int nFieldIndex = 0;
	for (auto field : return_record.getSchema().getFields()) {
		if (!return_record.has(field)) continue;

		const std::string &field_name = field.getProto().getName();
		::capnp::schema::Type::Which which = field.getType().which();	
		switch(which) {
		case ::capnp::schema::Type::BOOL:
		case ::capnp::schema::Type::INT8:
		case ::capnp::schema::Type::INT16:
		case ::capnp::schema::Type::INT32:
		case ::capnp::schema::Type::UINT8:
		case ::capnp::schema::Type::UINT16:
		case ::capnp::schema::Type::UINT32: {
			int32_t nVal = INT32_MIN;
			rs.GetFieldValue((char *)field_name.c_str(), &nVal, &nFieldIndex);
			return_record.set(field, nVal);
			break;
		}

		case ::capnp::schema::Type::INT64: {
			int64_t nVal = INT64_MIN;
			rs.GetFieldInt64Value((char *)field_name.c_str(), &nVal, &nFieldIndex);
			return_record.set(field, nVal);
			break;
		}

		case ::capnp::schema::Type::UINT64: {
			// we use UINT64 as time_t
			time_t tmVal = INT64_MIN;
			rs.GetFieldValue((char *)field_name.c_str(), &tmVal, &nFieldIndex);
			return_record.set(field, (uint64_t)tmVal);
			break;
		}

		case ::capnp::schema::Type::FLOAT32: {
			float fVal = FLT_MIN;
			rs.GetFieldFloatValue((char *)field_name.c_str(), &fVal, &nFieldIndex);
			return_record.set(field, fVal);
			break;
		}

		case ::capnp::schema::Type::FLOAT64: {
			double dVal = DBL_MIN;
			rs.GetFieldDoubleValue((char *)field_name.c_str(), &dVal, &nFieldIndex);
			return_record.set(field, dVal);
			break;
		}

		case ::capnp::schema::Type::TEXT: {
			const char *scratch;
			rs.GetFieldValue((char *)field_name.c_str(), &scratch, &nFieldIndex);
			return_record.set(field, scratch);
			break;
		}

		case ::capnp::schema::Type::DATA: {
			unsigned char *blob;
			long blobSize;
			rs.GetFieldValue((char *)field_name.c_str(), &blob, &blobSize, &nFieldIndex);
			return_record.set(field, ::capnp::Data::Reader(reinterpret_cast<const ::kj::byte*>(blob), (size_t)blobSize));
			break;
		}

		case ::capnp::schema::Type::ENUM: {
			int nVal = INT_MIN;
			rs.GetFieldValue((char *)field_name.c_str(), &nVal, &nFieldIndex);
			return_record.set(field, nVal);
			break;
		}

		case ::capnp::schema::Type::STRUCT: {
			::capnp::DynamicValue::Builder field_builder = return_record.get(field);
			AddFirstReturnRecord(rs, field_builder.as<::capnp::DynamicStruct>(), arrIndex, nIndex);
			break;
		}

		default:
			break;
		}

		//
		arrIndex[nIndex++] = nFieldIndex;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
Capnp2DbEngine::AddReturnRecord(IDBResultSet& rs, ::capnp::DynamicStruct::Builder& return_record, std::array<int, 256>& arrIndex, int& nIndex) {
	for (auto field : return_record.getSchema().getFields()) {
		if (!return_record.has(field)) continue;

		const std::string &field_name = field.getProto().getName();
		::capnp::schema::Type::Which which = field.getType().which();
		switch (which) {
		case ::capnp::schema::Type::BOOL:
		case ::capnp::schema::Type::INT8:
		case ::capnp::schema::Type::INT16:
		case ::capnp::schema::Type::INT32:
		case ::capnp::schema::Type::UINT8:
		case ::capnp::schema::Type::UINT16:
		case ::capnp::schema::Type::UINT32: {
			int32_t nVal = INT32_MIN;
			rs.GetFieldValue(arrIndex[nIndex], &nVal);
			return_record.set(field, nVal);
			break;
		}

		case ::capnp::schema::Type::INT64: {
			int64_t nVal = INT64_MIN;
			rs.GetFieldInt64Value(arrIndex[nIndex], &nVal);
			return_record.set(field, nVal);
			break;
		}

		case ::capnp::schema::Type::UINT64: {
			// we use UINT64 as time_t
			time_t tmVal = INT64_MIN;
			rs.GetFieldValue(arrIndex[nIndex], &tmVal);
			return_record.set(field, (uint64_t)tmVal);
			break;
		}

		case ::capnp::schema::Type::FLOAT32: {
			float fVal = FLT_MIN;
			rs.GetFieldFloatValue(arrIndex[nIndex], &fVal);
			return_record.set(field, fVal);
			break;
		}

		case ::capnp::schema::Type::FLOAT64: {
			double dVal = DBL_MIN;
			rs.GetFieldDoubleValue(arrIndex[nIndex], &dVal);
			return_record.set(field, dVal);
			break;
		}

		case ::capnp::schema::Type::TEXT: {
			const char *scratch;
			rs.GetFieldValue(arrIndex[nIndex], &scratch);
			return_record.set(field, scratch);
			break;
		}

		case ::capnp::schema::Type::DATA: {
			unsigned char *blob;
			long blobSize;
			rs.GetFieldValue(arrIndex[nIndex], &blob, &blobSize);
			return_record.set(field, ::capnp::Data::Reader(reinterpret_cast<const ::kj::byte*>(blob), (size_t)blobSize));
			break;
		}

		case ::capnp::schema::Type::ENUM: {
			int nVal = INT_MIN;
			rs.GetFieldValue(arrIndex[nIndex], &nVal);
			return_record.set(field, nVal);
			break;
		}

		case ::capnp::schema::Type::STRUCT: {
			::capnp::DynamicValue::Builder field_builder = return_record.get(field);
			AddReturnRecord(rs, field_builder.as<::capnp::DynamicStruct>(), arrIndex, nIndex);
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