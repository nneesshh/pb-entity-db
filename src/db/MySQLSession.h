#ifndef _MYSQL_SESSION_H_
#define _MYSQL_SESSION_H_

//------------------------------------------------------------------------------
/**
    @class MySQLSession
    
    (C) 2011 n.lee
*/
#include "IDatabase.h"

// int64
#ifndef uint64_t
	typedef unsigned __int64 uint64_t;
#endif
#ifndef int64_t
	typedef __int64 int64_t;
#endif

#include <iostream>
#include <vector>
#include <deque>
#include <map>
#include <string>

#include "../platform/types.h"

// strtok_r may be defined in pthread.h, it conflicts with my_global.h
#if !defined(strtok_r)
	INLINE char *strtok_r(char *str, const char *delim, char **saveptr) {
		return strtok(str,delim);
	}
#endif

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 
# include <winsock2.h>
#endif

//////////////////////////////////////////////////////////////////////////
#define MAX_COLUMN_NUM	64

#include "my_global.h"
#include "mysql.h"

///
struct mysql_param_value_t {
	int64_t				llVal;
	int					nVal;
	float				fVal;
	double				dVal;
	MYSQL_TIME			tmVal;
};

///
struct mysql_rs_value_t {
	char				chVal[1024 *4];
	unsigned long		nLen;

	my_bool				bIsNull;
};

//------------------------------------------------------------------------------
/** 
	@brief MySQLDriver
	MySQL database driver
*/
class MySQLDriver : public IDatabase {
public:
	MySQLDriver(const char *sURL, const char *sUser, const char *sPass, const char *sSchema);
	virtual ~MySQLDriver();

public:
	virtual bool				Open();
	virtual void				Close();
	virtual bool				IsOpened();

	virtual bool				ExecuteSQL(char *sSQL, int *pRecordsAffected, long nOption, IDBResultSet *pResultSet = nullptr);
	virtual bool				PrepareStoredProc(IDBStoredProc *proc, char *sql);
	virtual bool				ExecStoredProc(IDBStoredProc *proc, IDBResultSet *outRs = nullptr);

	virtual int					BeginTrans(void *conn);
	virtual int					RollbackTrans(void *conn);
	virtual int					CommitTrans(void *conn);

public:
	std::string			_strUrl;
	std::string			_strUser;
	std::string			_strPass;
	std::string			_strSchema;

	MYSQL				_conn;
	bool				_opened;
	int					_version;
};

//------------------------------------------------------------------------------
/** 
	@brief MySQLStoredProc
	MySQL prepared statement for stored procedure
*/
class MySQLStoredProc : public IDBStoredProc {
public:
	MySQLStoredProc(IDatabase *pDriver);
	virtual ~MySQLStoredProc();

public:
	virtual bool				Open();
	virtual void				Close();
	virtual bool				Prepare(const char *sSQL);

	virtual void				AddReturnValue();
	virtual int					GetReturnValue();

	virtual int64_t				GetValueAsBigint(char *sParam);
	virtual int					GetValueAsInteger(char *sParam);
	virtual std::string			GetValueAsString(char *sParam);
	virtual bool				GetValueAsString(char *sParam,char *pBuf,int nBufSize);

	virtual void				AddInputInt64Parameter(char *sParam, int64_t nValue);
	virtual void				AddInputParameter(char *sParam, int nValue);
	virtual void				AddInputParameter(char *sParam, char *sStr);
	virtual void				AddInputParameter(char *sParam, const std::string& sStr);
	virtual void				AddInputParameter(char *sParam, time_t t);
	virtual void				AddInputParameter(char *sParam, double dValue);
	virtual void				AddInputParameter( char *sParam, unsigned char *pBlob, long lBlobSize );

	virtual bool				Exec(IDBResultSet *pResultSet = nullptr);

public:
	MySQLDriver		*_refDriver;
	MYSQL_STMT			*_stmt;

	/* Param bind in */
	mysql_param_value_t	_inParamValues[MAX_COLUMN_NUM];
	MYSQL_BIND			_inParams[MAX_COLUMN_NUM];
	int					_inParamIndex;

	int					_paramCount;
	int					_affectedRows;
};

//------------------------------------------------------------------------------
/** 
	@brief MySQLResultSet
	MySQL result set
*/
class MySQLResultSet : public IDBResultSet {
public:
	MySQLResultSet(IDatabase *pDriver, IDBStoredProc *pProc);
	virtual ~MySQLResultSet();

public:
	virtual bool				Open();
	virtual void				Close();
	virtual bool				IsOpened();

	virtual bool				MoveNext();

	virtual bool				GetFieldValue(char *sField, std::string& str);
	virtual bool				GetFieldValue(int nField, std::string& str);
	virtual bool				GetFieldValue(char *sField, unsigned char *pBlob, long *pBlobSize);
	virtual bool				GetFieldValue(int nField, unsigned char *pBlob, long *pBlobSize);
	virtual bool				GetFieldFloatValue(char *sField, float *pVal);
	virtual bool				GetFieldFloatValue(int nField, float *pVal);
	virtual bool				GetFieldDoubleValue(char *sField, double *pVal);
	virtual bool				GetFieldDoubleValue(int nField, double *pVal);
	virtual bool				GetFieldInt64Value(char *sField, int64_t *pVal);
	virtual bool				GetFieldInt64Value(int nField, int64_t *pVal);
	virtual bool				GetFieldValue(char *sField, int *pVal);
	virtual bool				GetFieldValue(int nField, int *pVal);
	virtual bool				GetFieldValue(char *sField, short *pVal);
	virtual bool				GetFieldValue(int nField, short *pVal);
	virtual bool				GetFieldValue(char *sField, time_t *ptime);
	virtual bool				GetFieldValue(int nField, time_t *ptime);

private:
	bool						FetchMoreResults();

public:
	MySQLDriver		*_refDriver;
	MySQLStoredProc		*_refProc;

	bool				_opened;
	int					_numRows;
	int					_curRowIdx;
	int					_numFields;
	MYSQL_RES			*_resMetadata;

	/* ResultSet bind */
	mysql_rs_value_t	_rsValues[MAX_COLUMN_NUM];
	MYSQL_BIND			_rsBinds[MAX_COLUMN_NUM];

	std::map<std::string, int>	_fieldMap;
};

#endif // _MYSQL_SESSION_H_