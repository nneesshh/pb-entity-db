#ifndef FLATBUFFERS2DbENGINE_H
#define FLATBUFFERS2DbENGINE_H
//------------------------------------------------------------------------------
/**
    @class FlatBuffers2DbEngine
    
    (C) 2011 n.lee
*/
#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>	// include Windows.h to avoid "GetMessage" conflict between windows and protobuf
#endif

#include <string>
#include "../flatbuffers/flatbuffers.h"
#include "../flatbuffers/idl.h"
#include "../flatbuffers/util.h"

#include "IDatabase.h"

//------------------------------------------------------------------------------
/** 
	@brief FlatBuffers2DbEngine
*/
class	FlatBuffers2DbEngine {
public:
	FlatBuffers2DbEngine();
	~FlatBuffers2DbEngine();

	virtual void				OnInit(const char * sURL, const char *sUser, const char *sPass, const char *sSchema);
	virtual void				OnDelete();

public:
	void						Open();
	void						Close();

	bool						IsOpened() {
		return _opened;
	}

	int							ExecStoredProc(const char *sTableName, flatbuffers::FlatBufferBuilder *pStoredProc);

private:
	void						ParseOptions(const flatbuffers::FlatBufferBuilder *pParam, bool *pOutNeedResponse, bool *pOutBytesIsBlob);
	int							ParseInParamsCount(IDBStoredProc *pProc, const flatbuffers::FlatBufferBuilder *pParam);
	void						ParseInParams(IDBStoredProc *pProc, const flatbuffers::FlatBufferBuilder *pParam, bool bytesIsBlob);
	void						ParseOutParams(IDBStoredProc *pProc, const flatbuffers::FlatBufferBuilder *pOutParam);

	void						AddInputParameter(IDBStoredProc *pProc, const flatbuffers::FlatBufferBuilder *pParam, bool bytesIsBlob);
	void						AddReturnRecord(IDBResultSet *pRs, flatbuffers::FlatBufferBuilder *pReturnRecord, bool bytesIsBlob);

private:
	IDatabase				*_db;
	bool					_opened;

	std::map<std::string, std::string> _mapRootTable;
};

#endif