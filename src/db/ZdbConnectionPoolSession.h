#pragma once
//------------------------------------------------------------------------------
/**
    @class CZdbConnectionPoolSession
    
    (C) 2016 n.lee
*/
#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
#endif

#include <string>

#ifdef __cplusplus 
extern "C" {
#endif 
#	include "zdb.h"
#ifdef __cplusplus 
}
#endif 

#include "base/platform_types.h"
#include "base/mysnprintf.h"
#include "base/utilities.h"

#include "IDatabase.h"

//------------------------------------------------------------------------------
/**
	@brief ZdbDriver
	Zdb connection pool database driver
*/
class ZdbDriver : public IDatabase {
public:
	StdLog				*_refLog;

	std::string			_strUrl;
	std::string			_strUser;
	std::string			_strPass;
	std::string			_strSchema;
	int					_poolsize; // pool size

	bool				_opened;
	int					_version;

	URL_T				_url;
	ConnectionPool_T	_pool; // pool

public:
	ZdbDriver(StdLog *pLog, const char *sURL, const char *sUser, const char *sPass, const char *sSchema, int nPoolsize);
	virtual ~ZdbDriver();

	virtual bool				Open();
	virtual void				Close();
	virtual bool				IsOpened();

 	virtual StdLog *			GetLogHandler() {
 		return _refLog;
 	}

	virtual bool				ExecuteSQL(char *sSQL, int *pRecordsAffected, long nOption, IDBResultSet *pResultSet = nullptr);

	virtual int					BeginTrans(void *conn);
	virtual int					RollbackTrans(void *conn);
	virtual int					CommitTrans(void *conn);
};

//------------------------------------------------------------------------------
/**
	@brief ZdbStatement
	Zdb prepared statement for stored procedure
*/
class ZdbStatement : public IDBStatement {
public:
	ZdbDriver			*_refDriver;
	const char			*_refName;

	Connection_T		_conn;
	PreparedStatement_T	_stmt;
	int					_inParamIndex;
	ResultSet_T			_resultSet;
	int					_parameterCount;
	long long			_rowChanged;

	char				_sql[4096];

public:
	ZdbStatement(IDatabase *pDriver, const char *sProcName)
		: _refDriver(static_cast<ZdbDriver *>(pDriver))
		, _refName(sProcName)
		, _conn(nullptr)
		, _stmt(nullptr)
		, _inParamIndex(0)
		, _resultSet(nullptr)
		, _parameterCount(0)
		, _rowChanged(0) {

		_sql[0] = '\0';

		Open();
	}

	virtual ~ZdbStatement() {
		Close();
	}
	
	virtual const char *		GetName() override {
		return _refName;
	}

	virtual void				Open() override {
		bool bSucc = true;
		TRY
			do {
				_conn = ConnectionPool_getConnection(_refDriver->_pool);
				if (_conn)
					break;

				_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)] zdb statement(%s) open will wait 15 ms -- pool is busy...\n",
					(int)::GetCurrentThreadId(), _refName);
				util_sleep(15);
			} while (1);
		ELSE
			bSucc = false;
		END_TRY;
	}

	virtual void				Close() override {
		Connection_close(_conn);
	}

	virtual bool				PrepareSelect(const char *sTableName, const std::vector<std::string>& vKey) override;
	virtual bool				PrepareInsert(const char *sTableName, const std::vector<std::string>& vKey, const std::vector<std::string>& vSet) override;
	virtual bool				PrepareUpdate(const char *sTableName, const std::vector<std::string>& vKey, const std::vector<std::string>& vSet) override;
	virtual bool				PrepareDelete(const char *sTableName, const std::vector<std::string>& vKey) override;

	virtual bool				PrepareStoredProcCall(const char *sStoredProcName, int nParamsCount) override;

	virtual void				AddReturnValue() override;
	virtual int					GetReturnValue() override;

	virtual void				AddInputInt64Parameter(char *sParam, int64_t nValue) override {
		PreparedStatement_setLLong(_stmt, ++_inParamIndex, nValue);
	}

	virtual void				AddInputParameter(char *sParam, int nValue) override {
		PreparedStatement_setInt(_stmt, ++_inParamIndex, nValue);
	}

	virtual void				AddInputParameter(char *sParam, char *sStr, size_t size) override {
		PreparedStatement_setString(_stmt, ++_inParamIndex, sStr, size);
	}

	virtual void				AddInputParameter(char *sParam, const std::string& sStr) override {
		PreparedStatement_setString(_stmt, ++_inParamIndex, sStr.c_str(), sStr.length());
	}

	virtual void				AddInputParameter(char *sParam, time_t t) override {
		PreparedStatement_setTimestamp(_stmt, ++_inParamIndex, t);
	}

	virtual void				AddInputParameter(char *sParam, double dValue) override {
		PreparedStatement_setDouble(_stmt, ++_inParamIndex, dValue);
	}

	virtual void				AddInputParameter(char *sParam, unsigned char *pBlob, unsigned long lBlobSize) override {
		PreparedStatement_setBlob(_stmt, ++_inParamIndex, (const void *)pBlob, (size_t)lBlobSize);
	}

	virtual bool				Exec(IDBResultSet *pResultSet = nullptr) override {
		if (nullptr == _refDriver)
			return false;

		bool bSucc = true;
		TRY
			if (pResultSet) {
				_resultSet = PreparedStatement_executeQuery(_stmt);
				_rowChanged = PreparedStatement_rowsChanged(_stmt);
				bSucc = pResultSet->Open();
			}
			else {
				PreparedStatement_execute(_stmt);
			}
		ELSE
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)] zdb statement call failed -- sql(%s)!!!\n",
				(int)::GetCurrentThreadId(), GetName());
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "%s: %s raised in %s at %s:%d\n",
				Exception_frame.exception->name,
				Exception_frame.message,
				Exception_frame.func,
				Exception_frame.file,
				Exception_frame.line);
			bSucc = false;
		END_TRY;
		return bSucc;
	}
};

//------------------------------------------------------------------------------
/**
	@brief ZdbResultSet
	Zdb result set
*/
class ZdbResultSet : public IDBResultSet {
public:
	ZdbDriver			*_refDriver;
	ZdbStatement		*_refStatement;

	bool				_opened;
	int					_numFields;

public:
	ZdbResultSet(IDatabase *pDriver, IDBStatement *pProc)
		: _refDriver(static_cast<ZdbDriver *>(pDriver))
		, _refStatement(static_cast<ZdbStatement *>(pProc))
		, _opened(false)
		, _numFields(0) {

	}

	virtual ~ZdbResultSet() {

	}

	virtual bool				Open() override {
		_numFields = ResultSet_getColumnCount(_refStatement->_resultSet);
		_opened = true;
		return true;
	}

	virtual void				Close() override {
		_opened = false;
	}

	virtual bool				IsOpened() override {
		return _opened;
	}

	virtual bool				MoveNext() override {
		return ResultSet_next(_refStatement->_resultSet);
	}

	virtual bool				HasMoreResultSets() override {
		return ResultSet_nextResultSet(_refStatement->_resultSet);
	}

	virtual bool				GetFieldValue(char *sField, const char **ppStr, int *outFieldIndex) override {
		bool bSucc = true;
		TRY
			(*ppStr) = ResultSet_getStringByName(_refStatement->_resultSet, sField, outFieldIndex);
		ELSE
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)]Get string by name failed -- sql(%s)field(%s)!!!\n",
				(int)::GetCurrentThreadId(), _refStatement->GetName(), sField);
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "%s: %s raised in %s at %s:%d\n",
				Exception_frame.exception->name,
				Exception_frame.message,
				Exception_frame.func,
				Exception_frame.file,
				Exception_frame.line);
			bSucc = false;
		END_TRY;
		return bSucc;
	}

	virtual bool				GetFieldValue(int nField, const char **ppStr) override {
		(*ppStr) = ResultSet_getString(_refStatement->_resultSet, nField);
		return true;
	}

	virtual bool				GetFieldValue(char *sField, unsigned char **ppBlob, unsigned long *pBlobSize, int *outFieldIndex) override {
		bool bSucc = true;
		TRY
			(*ppBlob) = (unsigned char *)ResultSet_getBlobByName(_refStatement->_resultSet, sField, (int *)pBlobSize, outFieldIndex);
		ELSE
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)]Get blob by name failed -- sql(%s)field(%s)!!!\n",
				(int)::GetCurrentThreadId(), _refStatement->GetName(), sField);
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "%s: %s raised in %s at %s:%d\n",
				Exception_frame.exception->name,
				Exception_frame.message,
				Exception_frame.func,
				Exception_frame.file,
				Exception_frame.line);
			bSucc = false;
		END_TRY;
		return bSucc;
	}

	virtual bool				GetFieldValue(int nField, unsigned char **ppBlob, unsigned long *pBlobSize) override {
		(*ppBlob) = (unsigned char *)ResultSet_getBlob(_refStatement->_resultSet, nField, (int *)pBlobSize);
		return true;
	}

	virtual bool				GetFieldFloatValue(char *sField, float *pVal, int *outFieldIndex) override {
		bool bSucc = true;
		TRY
			*pVal = ResultSet_getFloatByName(_refStatement->_resultSet, sField, outFieldIndex);
		ELSE
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)]Get float by name failed -- sql(%s)field(%s)!!!\n",
				(int)::GetCurrentThreadId(), _refStatement->GetName(), sField);
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "%s: %s raised in %s at %s:%d\n",
				Exception_frame.exception->name,
				Exception_frame.message,
				Exception_frame.func,
				Exception_frame.file,
				Exception_frame.line);
			bSucc = false;
		END_TRY;
		return bSucc;
	}

	virtual bool				GetFieldFloatValue(int nField, float *pVal) override {
		*pVal = ResultSet_getFloat(_refStatement->_resultSet, nField);
		return true;
	}

	virtual bool				GetFieldDoubleValue(char *sField, double *pVal, int *outFieldIndex) override {
		bool bSucc = true;
		TRY
			*pVal = ResultSet_getDoubleByName(_refStatement->_resultSet, sField, outFieldIndex);
		ELSE
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)]Get double by name failed -- sql(%s)field(%s)!!!\n",
				(int)::GetCurrentThreadId(), _refStatement->GetName(), sField);
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "%s: %s raised in %s at %s:%d\n",
				Exception_frame.exception->name,
				Exception_frame.message,
				Exception_frame.func,
				Exception_frame.file,
				Exception_frame.line);
			bSucc = false;
		END_TRY;
		return bSucc;
	}

	virtual bool				GetFieldDoubleValue(int nField, double *pVal) override {
		*pVal = ResultSet_getDouble(_refStatement->_resultSet, nField);
		return true;
	}

	virtual bool				GetFieldInt64Value(char *sField, int64_t *pVal, int *outFieldIndex) override {
		bool bSucc = true;
		TRY
			*pVal = ResultSet_getLLongByName(_refStatement->_resultSet, sField, outFieldIndex);
		ELSE
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)]Get llong by name failed -- sql(%s)field(%s)!!!\n",
				(int)::GetCurrentThreadId(), _refStatement->GetName(), sField);
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "%s: %s raised in %s at %s:%d\n",
				Exception_frame.exception->name,
				Exception_frame.message,
				Exception_frame.func,
				Exception_frame.file,
				Exception_frame.line);
			bSucc = false;
		END_TRY;
		return bSucc;
	}

	virtual bool				GetFieldInt64Value(int nField, int64_t *pVal) override {
		*pVal = ResultSet_getLLong(_refStatement->_resultSet, nField);
		return true;
	}

	virtual bool				GetFieldValue(char *sField, int *pVal, int *outFieldIndex) override {
		bool bSucc = true;
		TRY
			*pVal = ResultSet_getIntByName(_refStatement->_resultSet, sField, outFieldIndex);
		ELSE
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)] Get int by name failed -- sql(%s)field(%s)!!!\n",
				(int)::GetCurrentThreadId(), _refStatement->GetName(), sField);
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "%s: %s raised in %s at %s:%d\n",
				Exception_frame.exception->name,
				Exception_frame.message,
				Exception_frame.func,
				Exception_frame.file,
				Exception_frame.line);
			bSucc = false;
		END_TRY;
		return bSucc;
	}

	virtual bool				GetFieldValue(int nField, int *pVal) override {
		*pVal = ResultSet_getInt(_refStatement->_resultSet, nField);
		return true;
	}

	virtual bool				GetFieldValue(char *sField, short *pVal, int *outFieldIndex) override {
		bool bSucc = true;
		TRY
			*pVal = (short)ResultSet_getIntByName(_refStatement->_resultSet, sField, outFieldIndex);
		ELSE
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)]Get short by name failed -- sql(%s)field(%s)!!!\n", 
				(int)::GetCurrentThreadId(), _refStatement->GetName(), sField);
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "%s: %s raised in %s at %s:%d\n",
				Exception_frame.exception->name,
				Exception_frame.message,
				Exception_frame.func,
				Exception_frame.file,
				Exception_frame.line);
			bSucc = false;
		END_TRY;
		return bSucc;
	}

	virtual bool				GetFieldValue(int nField, short *pVal) override {
		*pVal = (short)ResultSet_getInt(_refStatement->_resultSet, nField);
		return true;
	}

	virtual bool				GetFieldValue(char *sField, time_t *ptime, int *outFieldIndex) override {
		bool bSucc = true;
		TRY
			*ptime = ResultSet_getTimestampByName(_refStatement->_resultSet, sField, outFieldIndex);
		ELSE
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)]Get timestamp by name failed -- sql(%s)field(%s)!!!\n",
				(int)::GetCurrentThreadId(), _refStatement->GetName(), sField);
			_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "%s: %s raised in %s at %s:%d\n",
				Exception_frame.exception->name,
				Exception_frame.message,
				Exception_frame.func,
				Exception_frame.file,
				Exception_frame.line);
			bSucc = false;
		END_TRY;
		return bSucc;
	}

	virtual bool				GetFieldValue(int nField, time_t *ptime) override {
		*ptime = ResultSet_getTimestamp(_refStatement->_resultSet, nField);
		return true;
	}

};

/*EOF*/
