#pragma once
//------------------------------------------------------------------------------
/**
    @class IGameDB
    
    (C) 2016 n.lee
*/
#include <stdint.h>
#include <string>
#include <vector>

#include "../log/StdLog.h"

class IDatabase;
class IDBResultSet;
class IDBStatement;

//------------------------------------------------------------------------------
/** 
	@brief
 */
class IDatabase {
public:
	virtual ~IDatabase() { }

	virtual bool				Open() = 0;
	virtual void				Close() = 0;
	virtual bool				IsOpened() = 0;
	
	virtual StdLog *			GetLogHandler() = 0;

	virtual bool				ExecuteSQL(char *sSQL, int *pRecordsAffected, long nOption, IDBResultSet *pResultSet = nullptr) = 0;

	virtual int					BeginTrans(void *conn) = 0;
	virtual int					RollbackTrans(void *conn) = 0;
	virtual int					CommitTrans(void *conn) = 0;

};

//------------------------------------------------------------------------------
/** 
	@brief
 */
class IDBStatement {
public:
	virtual ~IDBStatement() { }

public:
	virtual const char *		GetName() = 0;
	virtual void				Open() = 0;
	virtual void				Close() = 0;
	
	virtual bool				PrepareSelect(const char *sTableName, const std::vector<std::string>& vKey) = 0;
	virtual bool				PrepareInsert(const char *sTableName, const std::vector<std::string>& vKey, const std::vector<std::string>& vSet) = 0;
	virtual bool				PrepareUpdate(const char *sTableName, const std::vector<std::string>& vKey, const std::vector<std::string>& vSet) = 0;
	virtual bool				PrepareDelete(const char *sTableName, const std::vector<std::string>& vKey) = 0;

	virtual bool				PrepareStoredProcCall(const char *sStoredProcName, int nParamsCount) = 0;

	virtual void				AddReturnValue() = 0;
	virtual int					GetReturnValue() = 0;

	virtual void				AddInputInt64Parameter(char *sParam, int64_t nValue) = 0;
	virtual void				AddInputParameter(char *sParam, int nValue) = 0;
	virtual void				AddInputParameter(char *sParam, char *sStr, size_t nSize) = 0;
	virtual void				AddInputParameter(char *sParam, const std::string& sStr) = 0;
	virtual void				AddInputParameter(char *sParam, time_t t) = 0;
	virtual void				AddInputParameter(char *sParam, double dValue) = 0;
	virtual void				AddInputParameter(char *sParam, unsigned char *pBlob, unsigned long nBlobSize ) = 0;

	virtual bool				Exec(IDBResultSet *pResultSet = nullptr) = 0;
};

//------------------------------------------------------------------------------
/** 
	@brief
 */
class IDBResultSet {
public:
	virtual ~IDBResultSet() { }

	virtual bool				Open() = 0;
	virtual void				Close() = 0;
	virtual bool				IsOpened() = 0;

	virtual bool				MoveNext() = 0;
	virtual bool				HasMoreResultSets() = 0;

	virtual bool				GetFieldValue(char *sField, const char **ppStr, int *outFieldIndex) = 0;
	virtual bool				GetFieldValue(int nField, const char **ppStr) = 0;
	virtual bool				GetFieldValue(char *sField, unsigned char **ppBlob, unsigned long *pBlobSize, int *outFieldIndex) = 0;
	virtual bool				GetFieldValue(int nField, unsigned char **ppBlob, unsigned long *pBlobSize) = 0;
	virtual bool				GetFieldFloatValue(char *sField, float *pVal, int *outFieldIndex) = 0;
	virtual bool				GetFieldFloatValue(int nField, float *pVal) = 0;
	virtual bool				GetFieldDoubleValue(char *sField, double *pVal, int *outFieldIndex) = 0;
	virtual bool				GetFieldDoubleValue(int nField, double *pVal) = 0;
	virtual bool				GetFieldInt64Value(char *sField, int64_t *pVal, int *outFieldIndex) = 0;
	virtual bool				GetFieldInt64Value(int nField, int64_t *pVal) = 0;
	virtual bool				GetFieldValue(char *sField, int *pVal, int *outFieldIndex) = 0;
	virtual bool				GetFieldValue(int nField, int *pVal) = 0;
	virtual bool				GetFieldValue(char *sField, short *pVal, int *outFieldIndex) = 0;
	virtual bool				GetFieldValue(int nField, short *pVal) = 0;
	virtual bool				GetFieldValue(char *sField, time_t *ptime, int *outFieldIndex) = 0;
	virtual bool				GetFieldValue(int nField, time_t *ptime) = 0;
};

/*EOF*/