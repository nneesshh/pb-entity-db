#pragma once
//------------------------------------------------------------------------------
/**
    @class Protobuf2DbEngine
    
    (C) 2016 n.lee
*/
#include "../UsingProtobuf.h"

#include <shared_mutex>
#include <string>

#include "IDatabase.h"

typedef struct _pb_input_param_s {
	const google::protobuf::Message          *params;
	const google::protobuf::Reflection       *reflection;
	const google::protobuf::FieldDescriptor  *fieldDescriptor;
} pb_input_param_t;

//------------------------------------------------------------------------------
/** 
	@brief Protobuf2DbEngine
*/
class Protobuf2DbEngine {
public:
	Protobuf2DbEngine();
	~Protobuf2DbEngine();

	virtual void				OnInit(StdLog *pLog, const char * sURL, const char *sUser, const char *sPass, const char *sSchema, int nPoolSize);
	virtual void				OnDelete();

public:
	void						Open();
	void						Close();

	bool						IsOpened() {
		return _opened;
	}

	bool						ESelect(google::protobuf::Message& entity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string>& vInKey);
	bool						EInsert(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey);
	bool						EUpdate(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey, bool bHackInsert);
	bool						EDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey);

	bool						SpQuery(google::protobuf::Message& storedProc);
	bool						SpUpdate(const google::protobuf::Message& storedProc);

private:
	/// entity
	void						CollectEntityInputParams(
		const google::protobuf::Message *pParams,
		const std::vector<std::string>& vInKey,
		const std::vector<std::string>& vInSet,
		std::vector<pb_input_param_t>& vOutKeyParam,
		std::vector<pb_input_param_t>& vOutSetParam);

	void						CollectEntitySetFields(const google::protobuf::Message *pParams, const std::vector<std::string>& vInKey, std::vector<std::string>& vOut);

	bool						ParseSelect(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey, IDBStatement *pOutS);
	bool						ParseInsert(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey, IDBStatement *pOutS);
	bool						ParseUpdate(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey, bool bHackInsert, IDBStatement *pOutS);
	bool						ParseDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vInKey, IDBStatement *pOutS);

	void						ParseEntityParams(IDBStatement *pS, const google::protobuf::Message *pParams, const std::vector<std::string>& vInKey, const std::vector<std::string>& vInSet);
	void						ParseInsertParams(IDBStatement *pS, const google::protobuf::Message *pParams, const std::vector<std::string>& vInKey, const std::vector<std::string>& vInSet);

	void						ParseEntityResult(IDBResultSet& rs, std::vector<std::string>& vResult, google::protobuf::Message& record);

	/// stored proc
	void						CollectProcInputParams(const google::protobuf::Message *pParams, std::vector<pb_input_param_t>& vOut);

	bool						ParseProc(const google::protobuf::Message* pStoredProc, const std::string& sProcName, IDBStatement *pOutS);
	void						ParseProcOptions(const google::protobuf::Message *pParam);
	int							ParseProcParamsCount(const google::protobuf::Message *pParam);
	void						ParseProcParams(IDBStatement *pS, const google::protobuf::Message *pParam);

	void						ParseProcResult(
		IDBResultSet& rs,
		google::protobuf::Message& msg,
		const google::protobuf::Reflection& msg_reflection,
		const google::protobuf::FieldDescriptor& field_desc);

	//////////////////////////////////////////////////////////////////////////

	bool						AddOneInputParam(IDBStatement *pS, pb_input_param_t *in_param);
	void						AddFirstReturnRecord(IDBResultSet *pRs, google::protobuf::Message *pReturnRecord, std::array<int, 256>& arrIndex, int& nIndex);
	void						AddReturnRecord(IDBResultSet *pRs, google::protobuf::Message *pReturnRecord, std::array<int, 256>& arrIndex, int& nIndex);

public:
	static std::shared_timed_mutex	s_mutex;

public:
	IDatabase				*_db = nullptr;
	bool					_opened = false;
	StdLog					*_refLog = nullptr;
};

/*EOF*/