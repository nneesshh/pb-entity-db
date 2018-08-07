#pragma once
//------------------------------------------------------------------------------
/**
@class CCapnpDbStub

(C) 2013 n.lee
*/
#include <unordered_map>

#include "Capnp2DbEngine.h"

//------------------------------------------------------------------------------
/**
@brief CCapnpDbStub
*/
class CCapnpDbStub {
public:
	CCapnpDbStub();
	~CCapnpDbStub();

public:
	void						OnInit(StdLog *pLog, const char * sURL, const char *sUser, const char *sPass, const char *sSchema, int nPoolSize);
	void						OnDelete();
	void						Open();
	void						Close();

	bool						IsOpened() {
		return _capnp2dbEngine->IsOpened();
	}

	bool						ExecQuery(::capnp::MessageReader& storedProc, ::capnp::StructSchema& root_schema, ::capnp::MessageBuilder& out) {
		return _capnp2dbEngine->ExecQuery(storedProc, root_schema, out);
	}

	bool						ExecUpdate(::capnp::MessageReader& storedProc, ::capnp::StructSchema& root_schema) {
		return _capnp2dbEngine->ExecUpdate(storedProc, root_schema);
	}

	Capnp2DbEngine *			GetDbEngine() {
		return _capnp2dbEngine;
	}

	int							ImportMessageTypeFromProtoFile(const std::string& proto_filename);

	void						AddSchema(::capnp::StructSchema schema) {
		uint64_t uSchemaProtoId = schema.getProto().getId();
		_mapSchema[uSchemaProtoId] = schema;
	}

	capnp::StructSchema *		LookupPrototype(uint64_t uSchemaProtoId, const char *sTypeName) {
		std::unordered_map<uint64_t, ::capnp::StructSchema>::iterator it = _mapSchema.find(uSchemaProtoId);
		if (it != _mapSchema.end()) {
			::capnp::StructSchema& schema = it->second;
			return &schema;
		}

		// error
		_capnp2dbEngine->_db->GetLogHandler()->logprint(LOG_LEVEL_ERROR, "\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!!!!!!![Capnp2DbEngine::LookupPrototype()] failed, type_name = %s!!!!!!!!\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
			, sTypeName);
		return nullptr;
	}

public:
	Capnp2DbEngine *_capnp2dbEngine = nullptr;

	//
	std::unordered_map<uint64_t, ::capnp::StructSchema> _mapSchema;
};

/*EOF*/