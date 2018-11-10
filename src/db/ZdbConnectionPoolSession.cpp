//------------------------------------------------------------------------------
//  ZdbConnectionPoolSession.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "ZdbConnectionPoolSession.h"
#include "base/URLCode.h"

#ifdef _MSC_VER
	#ifdef _DEBUG
		#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
	#endif
#endif

//------------------------------------------------------------------------------
/**

*/
ZdbDriver::ZdbDriver(StdLog *pLog, const char *sURL, const char *sUser, const char *sPass, const char *sSchema, int nPoolsize)
	: _refLog(pLog)
	, _strUrl(sURL)
	, _strUser(CURLCode().Encode(sUser))
	, _strPass(CURLCode().Encode(sPass))
	, _strSchema(CURLCode().Encode(sSchema))
	, _poolsize(nPoolsize)
	, _opened(false)
	, _version(0) {

}

//------------------------------------------------------------------------------
/**

*/
ZdbDriver::~ZdbDriver() {
	Close();
}

//------------------------------------------------------------------------------
/*
	url: mysql://localhost:3306/test?user=root&password=123123&connect-timeout=5&charset=utf8
*/
bool
ZdbDriver::Open() {
	char chURL[MAX_PATH] = { 0 };
	sprintf(chURL, "mysql://%s/%s?user=%s&password=%s&connect-timeout=5&charset=utf8", _strUrl.c_str(), _strSchema.c_str(), _strUser.c_str(), _strPass.c_str());

	_url = URL_new(chURL);
	if (nullptr == _url) {
		fprintf(stderr, "[tid(%d)] URL parse ERROR!\n", 
			(int)::GetCurrentThreadId());
		return false;
	}

	bool bSucc = true;
	TRY
		_pool = ConnectionPool_new(_url);
		ConnectionPool_setInitialConnections(_pool, _poolsize);
		ConnectionPool_setMaxConnections(_pool, _poolsize);
		ConnectionPool_setConnectionTimeout(_pool, 60 * 60 * 24); // 24 hours
		ConnectionPool_start(_pool);
		_opened = true;
	ELSE
		fprintf(stderr, "[tid(%d)] Connection pool init ERROR -- %s!!!\n", 
			(int)::GetCurrentThreadId(), chURL);
		fprintf(stderr, "%s: %s raised in %s at %s:%d\n",
			Exception_frame.exception->name,
			Exception_frame.message,
			Exception_frame.func,
			Exception_frame.file,
			Exception_frame.line);
		bSucc = false;
	END_TRY;
	return bSucc;
}

//------------------------------------------------------------------------------
/**

*/
void ZdbDriver::Close() {
	if (IsOpened()) {
		ConnectionPool_stop(_pool);
		ConnectionPool_free(&_pool);
		URL_free(&_url);
		_opened = false;
	}
}

//------------------------------------------------------------------------------
/**

*/
bool ZdbDriver::IsOpened() {
	return _opened;
}

//------------------------------------------------------------------------------
/**

*/
bool ZdbDriver::ExecuteSQL(char *sSQL, int *pRecordsAffected, long nOption, IDBResultSet *pResultSet) {
	bool bSucc = false;

	int nFieldIndex = 0;
	int nFieldType = 0;
	Connection_T con = ConnectionPool_getConnection(_pool);
	ResultSet_T result = Connection_executeQuery(con, "select * from AlipayTrans");
	fprintf(stderr, "ALL NUMBE:%d\n", ConnectionPool_size(_pool));
	fprintf(stderr, "ACTIVE NUMBER:%d\n", ConnectionPool_active(_pool));
	while (ResultSet_next(result)) {
		fprintf(stderr, "column: %s\n", ResultSet_getColumnName(result, 2));
		fprintf(stderr, "%s\n ", ResultSet_getStringByName(result, "result_code", &nFieldIndex, &nFieldType));
		fprintf(stderr, "%s\n ", ResultSet_getString(result, 3, &nFieldType));
	}
	Connection_close(con);

	return bSucc;
}

//------------------------------------------------------------------------------
/**

*/
int ZdbDriver::BeginTrans(void *conn) {
	if (!IsOpened())
		return -1;

	//return (int)&Connection_beginTransaction(conn);
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
int ZdbDriver::CommitTrans(void *conn) {
	if (!IsOpened())
		return -1;

	// 	return (int)&Connection_commit(conn);
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
int ZdbDriver::RollbackTrans(void *conn) {
	if (!IsOpened())
		return -1;
	//return (int)&Connection_rollback(conn);
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
bool
ZdbStatement::PrepareSelect(const char *sTableName, const std::vector<std::string>& vKey) {
	bool bSucc = true;

	char chClause[4096];

	size_t len;
	int j;

	// where clause
	len = 0;
	for (j = 0; j < vKey.size(); ++j) {
		const std::string& sKey = vKey[j];

		if (j > 0) {
			memcpy(chClause + len, " AND ", 5);
			len += 5;
		}
		chClause[len++] = '`';
		memcpy(chClause + len, sKey.c_str(), sKey.length());
		len += sKey.length();
		chClause[len++] = '`';

		chClause[len++] = '=';
		chClause[len++] = '?';
	}
	chClause[len] = '\0';

	// construct sql
	if ('\0' == chClause[0]) {
		o_snprintf(_sql, sizeof(_sql), "SELECT * FROM `%s`", sTableName);
	}
	else {
		o_snprintf(_sql, sizeof(_sql), "SELECT * FROM `%s` WHERE %s", sTableName, chClause);
	}

	TRY
		_stmt = Connection_prepareStatement(_conn, _sql);
		_parameterCount = PreparedStatement_getParameterCount(_stmt);
	ELSE
		_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)] zdb statement prepare failed -- sql(%s)!!!\n",
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

//------------------------------------------------------------------------------
/**

*/
bool
ZdbStatement::PrepareInsert(const char *sTableName, const std::vector<std::string>& vKey, const std::vector<std::string>& vSet) {
	bool bSucc = true;

	char chField[4096];
	char chPlaceholder[4096];
	char chDuplicate[4096];

	size_t len;
	int j;

	// field
	len = 0;
	for (j = 0; j < vKey.size(); ++j) {
		const std::string& sField = vKey[j];

		if (j > 0) {
			chField[len++] = ',';
		}
		chField[len++] = '`';
		memcpy(chField + len, sField.c_str(), sField.length());
		len += sField.length();
		chField[len++] = '`';
	}
	chField[len] = '\0';

	for (j = 0; j < vSet.size(); ++j) {
		const std::string& sField = vSet[j];

		if (vKey.size() > 0 || j > 0) {
			chField[len++] = ',';
		}
		chField[len++] = '`';
		memcpy(chField + len, sField.c_str(), sField.length());
		len += sField.length();
		chField[len++] = '`';
	}
	chField[len] = '\0';
	
	// placeholder
	len = 0;
	for (j = 0; j < vKey.size() + vSet.size(); ++j) {
			
		if (j > 0) {
			chPlaceholder[len++] = ',';
		}
		chPlaceholder[len++] = '?';
	}
	chPlaceholder[len] = '\0';

	// duplicate update
	len = 0;
	if (vSet.size() > 0) {
		for (j = 0; j < vSet.size(); ++j) {
			const std::string& sSet = vSet[j];

			if (j > 0) {
				chDuplicate[len++] = ',';
			}
			chDuplicate[len++] = '`';
			memcpy(chDuplicate + len, sSet.c_str(), sSet.length());
			len += sSet.length();
			chDuplicate[len++] = '`';

			chDuplicate[len++] = '=';
			memcpy(chDuplicate + len, " VALUES(", 8);
			len += 8;

			chDuplicate[len++] = '`';
			memcpy(chDuplicate + len, sSet.c_str(), sSet.length());
			len += sSet.length();
			chDuplicate[len++] = '`';

			chDuplicate[len++] = ')';
		}
	}
	else if (vKey.size() > 0) {
		// it is a hack to use "ON DUPLICATE KEY UPDATE", really update nothing
		const std::string& sSet = vKey[vKey.size() - 1];

		chDuplicate[len++] = '`';
		memcpy(chDuplicate + len, sSet.c_str(), sSet.length());
		len += sSet.length();
		chDuplicate[len++] = '`';

		chDuplicate[len++] = '=';
		memcpy(chDuplicate + len, " VALUES(", 8);
		len += 8;

		chDuplicate[len++] = '`';
		memcpy(chDuplicate + len, sSet.c_str(), sSet.length());
		len += sSet.length();
		chDuplicate[len++] = '`';

		chDuplicate[len++] = ')';
	}
	chDuplicate[len] = '\0';

	// construct sql
	if ('\0' == chDuplicate[0]) {
		o_snprintf(_sql, sizeof(_sql), "INSERT INTO `%s` (%s) VALUES (%s)", sTableName, chField, chPlaceholder);
	}
	else {
		o_snprintf(_sql, sizeof(_sql), "INSERT INTO `%s` (%s) VALUES (%s) ON DUPLICATE KEY UPDATE %s ", sTableName, chField, chPlaceholder, chDuplicate);
	}

	TRY
		_stmt = Connection_prepareStatement(_conn, _sql);
		_parameterCount = PreparedStatement_getParameterCount(_stmt);
	ELSE
		_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)] zdb statement prepare insert failed -- sql(%s)!!!\n",
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

//------------------------------------------------------------------------------
/**

*/
bool
ZdbStatement::PrepareUpdate(const char *sTableName, const std::vector<std::string>& vKey, const std::vector<std::string>& vSet) {
	bool bSucc = true;

	char chField[4096];
	char chClause[4096];

	size_t len;
	int j;

	// field
	assert(vSet.size() > 0);
	len = 0;
	for (j = 0; j < vSet.size(); ++j) {
		const std::string& sSet = vSet[j];

		if (j > 0) {
			chField[len++] = ',';
		}
		chField[len++] = '`';
		memcpy(chField + len, sSet.c_str(), sSet.length());
		len += sSet.length();
		chField[len++] = '`';

		chField[len++] = '=';
		chField[len++] = '?';
	}
	chField[len] = '\0';

	// where clause
	len = 0;
	for (j = 0; j < vKey.size(); ++j) {
		const std::string& sKey = vKey[j];

		if (j > 0) {
			memcpy(chClause + len, " AND ", 5);
			len += 5;
		}
		chClause[len++] = '`';
		memcpy(chClause + len, sKey.c_str(), sKey.length());
		len += sKey.length();
		chClause[len++] = '`';

		chClause[len++] = '=';
		chClause[len++] = '?';
	}
	chClause[len] = '\0';

	// construct sql
	if ('\0' == chClause[0]) {
		o_snprintf(_sql, sizeof(_sql), "UPDATE `%s` SET %s", sTableName, chField);
	}
	else {
		o_snprintf(_sql, sizeof(_sql), "UPDATE `%s` SET %s WHERE %s", sTableName, chField, chClause);
	}

	TRY
		_stmt = Connection_prepareStatement(_conn, _sql);
	_parameterCount = PreparedStatement_getParameterCount(_stmt);
	ELSE
		_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)] zdb statement prepare update failed -- sql(%s)!!!\n",
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

//------------------------------------------------------------------------------
/**

*/
bool
ZdbStatement::PrepareDelete(const char *sTableName, const std::vector<std::string>& vKey) {
	bool bSucc = true;

	char chClause[4096];

	size_t len;
	int j;

	// where clause
	len = 0;
	for (j = 0; j < vKey.size(); ++j) {
		const std::string& sKey = vKey[j];

		if (j > 0) {
			memcpy(chClause + len, " AND ", 5);
			len += 5;
		}
		chClause[len++] = '`';
		memcpy(chClause + len, sKey.c_str(), sKey.length());
		len += sKey.length();
		chClause[len++] = '`';

		chClause[len++] = '=';
		chClause[len++] = '?';
	}
	chClause[len] = '\0';

	// construct sql
	if ('\0' == chClause[0]) {
		o_snprintf(_sql, sizeof(_sql), "DELETE FROM `%s`", sTableName);
	}
	else {
		o_snprintf(_sql, sizeof(_sql), "DELETE FROM `%s` WHERE %s", sTableName, chClause);
	}

	TRY
		_stmt = Connection_prepareStatement(_conn, _sql);
		_parameterCount = PreparedStatement_getParameterCount(_stmt);
	ELSE
		_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)] zdb statement prepare delete failed -- sql(%s)!!!\n",
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

//------------------------------------------------------------------------------
/**

*/
bool
ZdbStatement::PrepareStoredProcCall(const char *sStoredProcName, int nParamsCount) {
	bool bSucc = true;

	char *p;
	int j;

	p = _sql;

	// construct call sql
	o_snprintf(p, sizeof(_sql), "CALL %s (", sStoredProcName);
	p += strlen(_sql);
	
	for (j = 0; j < nParamsCount; ++j) {
		if (j > 0) {
			*p++ = ',';
		}
		*p++ = '?';
	}
	*p++ = ')';
	*p++ = '\0';

	TRY
		_stmt = Connection_prepareStatement(_conn, _sql);
		_parameterCount = PreparedStatement_getParameterCount(_stmt);
	ELSE
		_refDriver->GetLogHandler()->logprint(LOG_LEVEL_WARNING, "[tid(%d)] zdb statement prepare stored proc failed -- sql(%s)!!!\n",
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

//------------------------------------------------------------------------------
/**

*/
void
ZdbStatement::AddReturnValue() {
	// mysql stored procedure can't use "return", only stored function can use "return"
	// 	sql::MethodNotImplementedException e("Don't call me!!!");
	// 	throw e;
}

//------------------------------------------------------------------------------
/**

*/
int
ZdbStatement::GetReturnValue() {
	// 	sql::MethodNotImplementedException e("Don't call me!!!");
	// 	throw e;
	return -1;
}

/** -- EOF -- **/