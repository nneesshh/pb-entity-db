#pragma once
//------------------------------------------------------------------------------
/**
    @class Capnp2DbEngine
    
    (C) 2016 n.lee
*/
#include <vector>
#include "../UsingDependency.h"

#include "IDatabase.h"

//------------------------------------------------------------------------------
/** 
	@brief Capnp2DbEngine
*/
class Capnp2DbEngine {
public:
	Capnp2DbEngine();
	~Capnp2DbEngine();

	virtual void				OnInit(StdLog *pLog, const char * sURL, const char *sUser, const char *sPass, const char *sSchema, int nPoolSize);
	virtual void				OnDelete();

public:
	void						Open();
	void						Close();

	bool						IsOpened() {
		return _opened;
	}

	bool						ExecQuery(::capnp::MessageReader& storedProc, ::capnp::StructSchema& root_schema, ::capnp::MessageBuilder& out);
	bool						ExecUpdate(::capnp::MessageReader& storedProc, ::capnp::StructSchema& root_schema);

private:
	void						ParseProc(::capnp::DynamicStruct::Reader& root, const char *sProcName, IDBStatement *pOutProc);

	int							ParseParamsCount(IDBStatement *pProc, ::capnp::DynamicStruct::Reader& param);
	void						ParseParams(IDBStatement *pProc, ::capnp::DynamicStruct::Reader& param);
	void						ParseResult(IDBResultSet& rs, ::capnp::StructSchema::Field& struct_schema_field, ::capnp::Orphanage& root_orphanage, ::capnp::DynamicStruct::Builder& root_builder);

	void						AddFirstReturnRecord(IDBResultSet& rs, ::capnp::DynamicStruct::Builder& return_record, std::array<int, 256>& arrIndex, int& nIndex);
	void						AddReturnRecord(IDBResultSet& rs, ::capnp::DynamicStruct::Builder& return_record, std::array<int, 256>& arrIndex, int& nIndex);

public:
	IDatabase				*_db = nullptr;
	bool					_opened = false;
};

/*EOF*/