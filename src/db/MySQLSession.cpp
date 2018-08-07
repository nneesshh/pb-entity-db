//------------------------------------------------------------------------------
//  MySQLSession.cpp
//  (C) 2011 n.lee
//------------------------------------------------------------------------------
#include "MySQLSession.h"

#include "../platform/utilities.h"
#include "../utils/util.h"
#include "../snprintf/snprintf.h"

using namespace std;

#define TEST_ERROR(_mysql, status)	\
	if (status) {					\
		fprintf(stderr, "Error: %s(errno: %d).\n", mysql_error(_mysql), mysql_errno(_mysql));	\
		return false;				\
	}

#define TEST_STMT_ERROR(_stmt, status)	\
	if (status) {						\
		fprintf(stderr, "Error: %s(errno: %d).\n", mysql_stmt_error(_stmt), mysql_stmt_errno(_stmt));	\
		return false;					\
	}

#define TEST_STMT_ERROR_SILENT(_stmt, status)	\
	if (status) {								\
		fprintf(stderr, "Error: %s(errno: %d).\n", mysql_stmt_error(_stmt), mysql_stmt_errno(_stmt));	\
	}

using namespace std;

//------------------------------------------------------------------------------
/**

*/
MySQLDriver::MySQLDriver(const char *sURL, const char *sUser, const char *sPass, const char *sSchema)
	: _strUrl(sURL)
	, _strUser(sUser)
	, _strPass(sPass)
	, _strSchema(sSchema)
	, _opened(false)
	, _version(0) {

}

//------------------------------------------------------------------------------
/**

*/
MySQLDriver::~MySQLDriver() {
	Close();
}

//------------------------------------------------------------------------------
/*
	url:
		TCP/IP				-- tcp://[host[:port]][/schema], such as "tcp://192.168.16.206:3306/mydatastore"
		UNIX Domain Socket	-- unix://path/to/unix_socket_file
		Named Pipes			-- pipe://path/to/the/pipe
*/
bool
MySQLDriver::Open() {
	fprintf(stderr, "Open game DB(%s; %s) ......\n", _strUrl.c_str(), _strSchema.c_str());

	char chURL[MAX_PATH]={0};
	char *chArray[2]={{0}, {0}};

	sprintf(chURL, "%s", _strUrl.c_str());
	split(chURL,strlen(chURL),chArray,2,':');

	mysql_init(&_conn);

	if (nullptr != mysql_real_connect(&_conn, chArray[0], _strUser.c_str(), _strPass.c_str(), nullptr, atoi(chArray[1]), nullptr, CLIENT_MULTI_RESULTS)) {
		char value = 1;
		mysql_options(&_conn, MYSQL_OPT_RECONNECT, &value);
		mysql_set_character_set(&_conn, "utf8mb4");

		int nStatus = mysql_select_db(&_conn, _strSchema.c_str());
		TEST_ERROR(&_conn, nStatus);

		_version = mysql_get_server_version(&_conn);
		if (_version<50503) {
			fprintf(stderr, "Server does not support required CALL capabilities!\n");
			return false;
		}

		_opened = true;
		fprintf(stderr, "Open game DB(%s; %s) success!\n", _strUrl.c_str(), _strSchema.c_str());
		return true;
	} else {
		TEST_ERROR(&_conn, 1);
	}
	return false;
}

//------------------------------------------------------------------------------
/**

*/
void MySQLDriver::Close() {
	if (IsOpened()) {
		mysql_close(&_conn);
		mysql_server_end();
		_opened = false;
	}
}

//------------------------------------------------------------------------------
/**

*/
bool MySQLDriver::IsOpened() {
	return _opened;
}

//------------------------------------------------------------------------------
/**

*/
bool MySQLDriver::ExecuteSQL(char *sSQL, int *pRecordsAffected, long nOption, IDBResultSet *pResultSet) {
// 	_variant_t vtRecordsAffected;
 	bool bSucc = false;
// 	
// 	CMySQLRecordset *pMySQLRecordset = static_cast<CMySQLRecordset *>(pResultSet);
// 	if (pMySQLRecordset) {
// 		pMySQLRecordset->m_pResultSetPtr = &m_conn2ection->Execute((_bstr_t)(char *)lpszSQL,&vtRecordsAffected,nOption);
// 		pMySQLRecordset->m_pDatabase = this;
// 	} else
// 		&m_conn2ection->Execute((_bstr_t)(char *)lpszSQL,&vtRecordsAffected,nOption);
// 	
// 	if (pRecordsAffected)
// 		*pRecordsAffected = vtRecordsAffected.lVal;
	
	return bSucc;
}

//------------------------------------------------------------------------------
/**

*/
bool MySQLDriver::PrepareStoredProc(IDBStoredProc *proc, char *sql) {
	// loop until success
	while (!proc->Prepare(sql)) {
		fprintf(stderr, "DB broken when prepare [%s], try reconnect after 1000 ms...\n", sql);

		proc->Close();
		Close();
		util_sleep(1000);
		this->Open();
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool MySQLDriver::ExecStoredProc(IDBStoredProc *proc, IDBResultSet *outRs) {
	// loop until success
	while (0 == proc->Exec(outRs)) {
		fprintf(stderr, "DB broken when exec stored proc, try reconnect after 1000 ms...\n");

		proc->Close();
		Close();
		util_sleep(1000);
		this->Open();
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
int MySQLDriver::BeginTrans(void *conn) {
	if (!IsOpened())
		return -1;

	//return (int)&conn->BeginTrans();
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
int MySQLDriver::CommitTrans(void *conn) {
	if (!IsOpened())
		return -1;

	// 	return (int)&conn->CommitTrans();
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
int MySQLDriver::RollbackTrans(void *conn) {
	if (!IsOpened())
		return -1;
	//return (int)&conn->RollbackTrans();
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
MySQLStoredProc::MySQLStoredProc(IDatabase *pDriver)
	: _refDriver(static_cast<MySQLDriver *>(pDriver))
	, _stmt(nullptr)
	, _paramCount(0)
	, _affectedRows(0) {

	memset(_inParamValues, 0, sizeof(_inParamValues));
	memset(_inParams, 0, sizeof(_inParams));
	_inParamIndex = 0;
}

//------------------------------------------------------------------------------
/**

*/
MySQLStoredProc::~MySQLStoredProc() {
	Close();
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLStoredProc::Open() {
	_stmt = mysql_stmt_init(&_refDriver->_conn);
	return true;
}

//------------------------------------------------------------------------------
/**

*/
void
MySQLStoredProc::Close() {
	if (nullptr != _stmt) {
		mysql_stmt_close(_stmt);
		_stmt = nullptr; // clear
	}
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLStoredProc::Prepare(const char *sSQL) {
	if (nullptr == _stmt) {
		Open();
	}

	int nStatus = mysql_stmt_prepare(_stmt, sSQL, strlen(sSQL));
	TEST_STMT_ERROR_SILENT(_stmt, nStatus);

	_paramCount = mysql_stmt_param_count(_stmt);

	nStatus = mysql_stmt_bind_param(_stmt, _inParams);
	TEST_STMT_ERROR(_stmt, nStatus);

	return true;
}

//------------------------------------------------------------------------------
/**

*/
void
MySQLStoredProc::AddReturnValue() {
	// mysql stored procedure can't use "return", only stored function can use "return"
	// 	sql::MethodNotImplementedException e("Don't call me!!!");
	// 	throw e;
}

//------------------------------------------------------------------------------
/**

*/
int
MySQLStoredProc::GetReturnValue() {
	// 	sql::MethodNotImplementedException e("Don't call me!!!");
	// 	throw e;
	return -1;
}

//------------------------------------------------------------------------------
/**

*/
int64_t
MySQLStoredProc::GetValueAsBigint(char *sParam) {
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
int
MySQLStoredProc::GetValueAsInteger(char *sParam) {
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
std::string
MySQLStoredProc::GetValueAsString(char *sParam) {
	return "";
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLStoredProc::GetValueAsString(char *sParam,char *pBuf,int nBufSize) {
	return false;
}

//------------------------------------------------------------------------------
/**

*/
void
MySQLStoredProc::AddInputInt64Parameter(char *sParam, int64_t nValue) {
	int n = _inParamIndex++;
	assert(n < sizeof(_inParams)/sizeof(_inParams[0]));

	_inParamValues[n].llVal			= nValue;

	_inParams[n].buffer_type	= MYSQL_TYPE_LONGLONG;
	_inParams[n].buffer			= &(_inParamValues[n].llVal);
	_inParams[n].buffer_length	= 0;
	_inParams[n].is_null		= 0;
}

//------------------------------------------------------------------------------
/**

*/
void
MySQLStoredProc::AddInputParameter(char *sParam, int nValue) {
	int n = _inParamIndex++;
	assert(n < sizeof(_inParams)/sizeof(_inParams[0]));

	_inParamValues[n].nVal			= nValue;

	_inParams[n].buffer_type	= MYSQL_TYPE_LONG;
	_inParams[n].buffer			= &(_inParamValues[n].nVal);
	_inParams[n].buffer_length	= 0;
	_inParams[n].is_null		= 0;
}

//------------------------------------------------------------------------------
/**

*/
void
MySQLStoredProc::AddInputParameter(char *sParam, char *sStr) {
	int n = _inParamIndex++;
	assert(n < sizeof(_inParams)/sizeof(_inParams[0]));

	if (nullptr != sStr) {
		_inParams[n].buffer_type	= MYSQL_TYPE_STRING;
		_inParams[n].buffer			= sStr;
		_inParams[n].buffer_length	= strlen(sStr);
		_inParams[n].is_null		= 0;
	} else {
		_inParams[n].buffer_type	= MYSQL_TYPE_NULL;
		_inParams[n].buffer			= 0;
		_inParams[n].buffer_length	= 0;
		_inParams[n].is_null		= 0;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
MySQLStoredProc::AddInputParameter(char *sParam, const std::string& sStr) {
	int n = _inParamIndex++;
	assert(n < sizeof(_inParams) / sizeof(_inParams[0]));

	_inParams[n].buffer_type	= MYSQL_TYPE_STRING;
	_inParams[n].buffer			= (void *)sStr.c_str();
	_inParams[n].buffer_length	= sStr.length();
	_inParams[n].is_null		= 0;
}

//------------------------------------------------------------------------------
/**

*/
void
MySQLStoredProc::AddInputParameter(char *sParam, time_t t) {
	int n = _inParamIndex++;
	assert(n < sizeof(_inParams)/sizeof(_inParams[0]));

	struct tm *tp = localtime(&t);
	if (tp) {
		_inParamValues[n].tmVal.year	= tp->tm_year+1900;
		_inParamValues[n].tmVal.month	= tp->tm_mon+1;
		_inParamValues[n].tmVal.day		= tp->tm_mday;
		_inParamValues[n].tmVal.hour	= tp->tm_hour;
		_inParamValues[n].tmVal.minute	= tp->tm_min;
		_inParamValues[n].tmVal.second	= tp->tm_sec;

		_inParams[n].buffer_type	= MYSQL_TYPE_DATETIME;
		_inParams[n].buffer			= &(_inParamValues[n].tmVal);
		_inParams[n].buffer_length	= 0;
		_inParams[n].is_null		= 0;
	} else {
		_inParamValues[n].tmVal.year	= 1900;
		_inParamValues[n].tmVal.month	= 1;
		_inParamValues[n].tmVal.day		= 1;
		_inParamValues[n].tmVal.hour	= 0;
		_inParamValues[n].tmVal.minute	= 0;
		_inParamValues[n].tmVal.second	= 0;

		_inParams[n].buffer_type	= MYSQL_TYPE_DATETIME;
		_inParams[n].buffer			= &(_inParamValues[n].tmVal);
		_inParams[n].buffer_length	= 0;
		_inParams[n].is_null		= 0;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
MySQLStoredProc::AddInputParameter(char *sParam, double dValue) {
	int n = _inParamIndex++;
	assert(n < sizeof(_inParams)/sizeof(_inParams[0]));

	_inParamValues[n].dVal			= dValue;

	_inParams[n].buffer_type	= MYSQL_TYPE_DOUBLE;
	_inParams[n].buffer			= &(_inParamValues[n].dVal);
	_inParams[n].buffer_length	= 0;
	_inParams[n].is_null		= 0;
}

//------------------------------------------------------------------------------
/**

*/
void
MySQLStoredProc::AddInputParameter( char *sParam, unsigned char *pBlob, long lBlobSize ) {
	int n = _inParamIndex++;
	assert(n < sizeof(_inParams)/sizeof(_inParams[0]));

	if (nullptr != pBlob && 0 < lBlobSize) {
		_inParams[n].buffer_type	= MYSQL_TYPE_BLOB;
		_inParams[n].buffer			= pBlob;
		_inParams[n].buffer_length	= lBlobSize;
		_inParams[n].is_null		= 0;
	} else {
		_inParams[n].buffer_type	= MYSQL_TYPE_NULL;
		_inParams[n].buffer			= 0;
		_inParams[n].buffer_length	= 0;
		_inParams[n].is_null		= 0;
	}
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLStoredProc::Exec( IDBResultSet *pResultSet ) {
	if (nullptr == _refDriver)
		return false;

	int nStatus = mysql_stmt_execute(_stmt);
	TEST_STMT_ERROR(_stmt, nStatus);

	_affectedRows = (int)mysql_stmt_affected_rows(_stmt);

	if (pResultSet) {
		return pResultSet->Open();
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
MySQLResultSet::MySQLResultSet(IDatabase *pDriver, IDBStoredProc *pProc)
	: _refDriver(static_cast<MySQLDriver *>(pDriver))
	, _refProc(static_cast<MySQLStoredProc *>(pProc))
	, _opened(false)
	, _numRows(0)
	, _curRowIdx(0)
	, _numFields(0)
	, _resMetadata(nullptr) {

}

//------------------------------------------------------------------------------
/**

*/
MySQLResultSet::~MySQLResultSet() {
	mysql_free_result(_resMetadata);
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::Open() {
	_numFields = mysql_stmt_field_count(_refProc->_stmt);
	assert(_numFields <= sizeof(_rsBinds)/sizeof(_rsBinds[0]));

	if (_numFields>0) {
		_resMetadata = mysql_stmt_result_metadata(_refProc->_stmt);

		if (nullptr != _resMetadata) {
			MYSQL_FIELD *fields = mysql_fetch_fields(_resMetadata);

			memset(&_rsValues, 0, sizeof(_rsValues));
			memset(&_rsBinds, 0, sizeof(_rsBinds));
			_fieldMap.clear();

			int i;
			for(i=0; i<_numFields; ++i) {
				_rsBinds[i].buffer_type		= fields[i].type;
				_rsBinds[i].buffer			= _rsValues[i].chVal;
				_rsBinds[i].buffer_length	= sizeof(_rsValues[i].chVal);
				_rsBinds[i].length			= &(_rsValues[i].nLen);
				_rsBinds[i].is_null			= &(_rsValues[i].bIsNull);

				_fieldMap[fields[i].name] = i+1;
			}

			int nStatus = mysql_stmt_bind_result(_refProc->_stmt, _rsBinds);
			TEST_STMT_ERROR(_refProc->_stmt, nStatus);

			nStatus = mysql_stmt_store_result(_refProc->_stmt);
			TEST_STMT_ERROR(_refProc->_stmt, nStatus);

			_numRows = (int)mysql_stmt_num_rows(_refProc->_stmt);
			_curRowIdx = 0;
		}
	}
	_opened = true;
	return true;
}

//------------------------------------------------------------------------------
/**

*/
void
MySQLResultSet::Close() {
	while(FetchMoreResults()) {
		Open();
	}
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::IsOpened() {
	return _opened;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::MoveNext() {
	int nStatus = mysql_stmt_fetch(_refProc->_stmt);
	if (0 == nStatus) {
		++_curRowIdx;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldValue(char *sField, std::string& str) {
	map<string, int>::iterator it = _fieldMap.find(sField);
	if (it!=_fieldMap.end()) {
		int nField = it->second;
		return GetFieldValue(nField, str);
	}
	return false;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldValue(int nField, std::string& str) {
	assert(nField <= sizeof(_rsBinds)/sizeof(_rsBinds[0]));

	bool bIsNull = (0!=_rsValues[nField-1].bIsNull);
	if (bIsNull) {
		str = "";
		return true;
	}

	// datetime
	if (MYSQL_TYPE_DATETIME == _rsBinds[nField - 1].buffer_type) {
		char chTime[256];
		struct tm t1;
		MYSQL_TIME *t2 = (MYSQL_TIME *)_rsValues[nField - 1].chVal;
		if (t2) {
			t1.tm_year = t2->year;
			t1.tm_mon = t2->month;
			t1.tm_mday = t2->day;
			t1.tm_hour = t2->hour;
			t1.tm_min = t2->minute;
			t1.tm_sec = t2->second;

			o_snprintf(chTime
				, sizeof(chTime)
				, "%04d-%02d-%02d %02d:%02d:%02d"
				, t1.tm_year
				, t1.tm_mon
				, t1.tm_mday
				, t1.tm_hour
				, t1.tm_min
				, t1.tm_sec);

			str = chTime;
			return true;
		}
	}

	// string
	int nLen = _rsValues[nField-1].nLen;
	if (nLen<sizeof(_rsValues[nField-1].chVal)) {
		_rsValues[nField-1].chVal[nLen] = '\0';
		str = _rsValues[nField - 1].chVal;
		return true;
	}

	//
	str = "";
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldValue(char *sField, unsigned char *pBlob, long *pBlobSize) {
	map<string, int>::iterator it = _fieldMap.find(sField);
	if (it!=_fieldMap.end()) {
		int nField = it->second;
		return GetFieldValue(nField, pBlob, pBlobSize);
	}
	return false;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldValue(int nField, unsigned char *pBlob, long *pBlobSize) {
	assert(nField <= sizeof(_rsBinds)/sizeof(_rsBinds[0]));

	bool bIsNull = (0!=_rsValues[nField-1].bIsNull);
	if (bIsNull) {
		*pBlobSize = 0;
		return true;
	}

	// copy
	*pBlobSize = _rsValues[nField-1].nLen;
	if (*pBlobSize>0) {
		memcpy(pBlob, _rsValues[nField-1].chVal, *pBlobSize);
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldFloatValue(char *sField, float *pVal) {
	map<string, int>::iterator it = _fieldMap.find(sField);
	if (it != _fieldMap.end()) {
		int nField = it->second;
		return GetFieldFloatValue(nField, pVal);
	}
	return false;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldFloatValue(int nField, float *pVal) {
	assert(nField <= sizeof(_rsBinds) / sizeof(_rsBinds[0]));

	bool bIsNull = (0 != _rsValues[nField - 1].bIsNull);
	if (bIsNull) {
		*pVal = 0;
		return true;
	}

	// to float
	assert(MYSQL_TYPE_FLOAT == _rsBinds[nField - 1].buffer_type);
	*pVal = *((float *)_rsValues[nField - 1].chVal);
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldDoubleValue(char *sField, double *pVal) {
	map<string, int>::iterator it = _fieldMap.find(sField);
	if (it!=_fieldMap.end()) {
		int nField = it->second;
		return GetFieldDoubleValue(nField, pVal);
	}
	return false;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldDoubleValue(int nField, double *pVal) {
	assert(nField <= sizeof(_rsBinds)/sizeof(_rsBinds[0]));

	bool bIsNull = (0!=_rsValues[nField-1].bIsNull);
	if (bIsNull) {
		*pVal = 0;
		return true;
	}

	// to double
	assert(MYSQL_TYPE_DOUBLE == _rsBinds[nField - 1].buffer_type);
	*pVal = *((double *)_rsValues[nField-1].chVal);
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldInt64Value(char *sField, int64_t *pVal) {
	map<string, int>::iterator it = _fieldMap.find(sField);
	if (it!=_fieldMap.end()) {
		int nField = it->second;
		return GetFieldValue(nField, pVal);
	}
	return false;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldInt64Value(int nField, int64_t *pVal) {
	assert(nField <= sizeof(_rsBinds)/sizeof(_rsBinds[0]));

	bool bIsNull = (0!=_rsValues[nField-1].bIsNull);
	if (bIsNull) {
		*pVal = 0;
		return true;
	}

	// to int64
	assert(MYSQL_TYPE_LONGLONG == _rsBinds[nField - 1].buffer_type);
	*pVal = *((int64_t *)_rsValues[nField-1].chVal);
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldValue(char *sField, int *pVal) {
	map<string, int>::iterator it = _fieldMap.find(sField);
	if (it!=_fieldMap.end()) {
		int nField = it->second;
		return GetFieldValue(nField, pVal);
	}
	return false;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldValue(int nField, int *pVal) {
	assert(nField <= sizeof(_rsBinds)/sizeof(_rsBinds[0]));

	bool bIsNull = (0!=_rsValues[nField-1].bIsNull);
	if (bIsNull) {
		*pVal = 0;
		return true;
	}

	// to int
	*pVal = *((int *)_rsValues[nField-1].chVal);
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldValue(char *sField, short *pVal) {
	map<string, int>::iterator it = _fieldMap.find(sField);
	if (it!=_fieldMap.end()) {
		int nField = it->second;
		return GetFieldValue(nField, pVal);
	}
	return false;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldValue(int nField, short *pVal) {
	assert(nField <= sizeof(_rsBinds)/sizeof(_rsBinds[0]));

	bool bIsNull = (0!=_rsValues[nField-1].bIsNull);
	if (bIsNull) {
		*pVal = 0;
		return true;
	}

	// to short
	*pVal = *((short *)_rsValues[nField-1].chVal);
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldValue(char *sField, time_t *ptime) {
	map<string, int>::iterator it = _fieldMap.find(sField);
	if (it!=_fieldMap.end()) {
		int nField = it->second;
		return GetFieldValue(nField, ptime);
	}
	return false;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::GetFieldValue(int nField, time_t *ptime) {
	assert(nField <= sizeof(_rsBinds)/sizeof(_rsBinds[0]));

	bool bIsNull = (0!=_rsValues[nField-1].bIsNull);
	if (bIsNull) {
		*ptime = 0;
		return true;
	}

	// to time_t
	MYSQL_TIME tmVal;
	tmVal = *((MYSQL_TIME *)_rsValues[nField-1].chVal);

	struct tm t;
	t.tm_year = tmVal.year-1900;
	t.tm_mon = tmVal.month-1;
	t.tm_mday = tmVal.day;
	t.tm_hour = tmVal.hour;
	t.tm_min = tmVal.minute;
	t.tm_sec = tmVal.second;

	*ptime = mktime(&t);
	return true;
}

//------------------------------------------------------------------------------
/**

*/
bool
MySQLResultSet::FetchMoreResults() {
	/* more results? -1 = no, >0 = error, 0 = yest (keep looking) */
	int nStatus = mysql_stmt_next_result(_refProc->_stmt);
	if ( 0==nStatus )
		return true;
	
	if (0<nStatus)
		TEST_STMT_ERROR(_refProc->_stmt, nStatus)

	return false;
}

/** -- EOF -- **/