/*
 * Copyright (C) Tildeslash Ltd. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 *
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.
 */


#include "Config.h"

#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <errmsg.h>

#include "db/ResultSetDelegate.h"
#include "MysqlResultSet.h"


/**
 * Implementation of the ResultSet/Delegate interface for mysql. 
 * Accessing columns with index outside range throws SQLException
 *
 * @file
 */


/* ----------------------------------------------------------- Definitions */


#define MYSQL_OK 0

const struct Rop_S mysqlrops = {
	.name           = "mysql",
        .free           = MysqlResultSet_free,
        .getColumnCount = MysqlResultSet_getColumnCount,
        .getColumnName  = MysqlResultSet_getColumnName,
        .getColumnSize  = MysqlResultSet_getColumnSize,
        .next           = MysqlResultSet_next,
		.nextResultSet	= MysqlResultSet_nextResultSet,
        .isnull         = MysqlResultSet_isnull,
        .getString      = MysqlResultSet_getString,
        .getFloat       = MysqlResultSet_getFloat,
        .getDouble      = MysqlResultSet_getDouble,
        .getBlob        = MysqlResultSet_getBlob,
        .getTimestamp   = MysqlResultSet_getTimestamp,
        .getDateTime    = MysqlResultSet_getDatetime
};

typedef struct column_t {
        my_bool is_null;
        MYSQL_FIELD *field;
        unsigned long real_length;
        char *buffer;
} *column_t;

#define T ResultSetDelegate_T
struct ResultSetDelegate_S {
        int stop;
        int keep;
        int maxRows;
        int lastError;
        int needRebind;
	int currentRow;
	int columnCount;
        MYSQL_RES *meta;
		MYSQL_FIELD *meta_fields;
        MYSQL_BIND *bind;
	MYSQL_STMT *stmt;
        column_t columns;
};


/* ------------------------------------------------------- Private methods */


static inline void _ensureCapacity(T R, int i) {
        if ((R->columns[i].real_length > R->bind[i].buffer_length)) {
                /* Column was truncated, resize and fetch column directly. */
                RESIZE(R->columns[i].buffer, R->columns[i].real_length + 1);
                R->bind[i].buffer = R->columns[i].buffer;
                R->bind[i].buffer_length = R->columns[i].real_length;
                if ((R->lastError = mysql_stmt_fetch_column(R->stmt, &R->bind[i], i, 0)))
                        THROW(SQLException, "mysql_stmt_fetch_column -- %s", mysql_stmt_error(R->stmt));
                R->needRebind = true;
        }
}


/* ----------------------------------------------------- Protected methods */


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

T MysqlResultSet_new(void *stmt, int maxRows, int keep) {
	T R;
	Assertion_assert(stmt);
	NEW(R);
	R->stmt = stmt;
        R->keep = keep;
        R->maxRows = maxRows;
		/* the column count is > 0 if there is a result set */
		/* 0 if the result is only the final status packet */
		R->columnCount = mysql_stmt_field_count(R->stmt);
		if (R->columnCount <= 0
			|| NULL == R->stmt->mysql
			|| !(R->meta = mysql_stmt_result_metadata(R->stmt))) {
			DEBUG("Warning: column error - %s\n", mysql_stmt_error(R->stmt));
			R->stop = true;
			/* column buffer is not allocated yet, so we force columCount to be zero. */
			R->columnCount = 0;
		}
		else {
			/* there is a result set to fetch */
			R->meta_fields = mysql_fetch_fields(R->meta);
			R->bind = CALLOC(R->columnCount, sizeof(MYSQL_BIND));
			R->columns = CALLOC(R->columnCount, sizeof(struct column_t));
			/* set up and bind result set output buffers */
			for (int i = 0; i < R->columnCount; i++) {
				R->columns[i].buffer = ALLOC(STRLEN + 1);
				R->bind[i].buffer_type = R->meta_fields[i].type;
				R->bind[i].buffer = R->columns[i].buffer;
				R->bind[i].buffer_length = STRLEN;
				R->bind[i].is_null = &R->columns[i].is_null;
				R->bind[i].length = &R->columns[i].real_length;
				R->columns[i].field = mysql_fetch_field_direct(R->meta, i);
			}
			if ((R->lastError = mysql_stmt_bind_result(R->stmt, R->bind))) {
				DEBUG("Warning: bind error - %s\n", mysql_stmt_error(R->stmt));
				R->stop = true;
			}
			else {
				/* Now buffer all results to client (optional step), CURSOR_TYPE_READ_ONLY already do mysql_stmt_store_result */
				if (!(R->stmt->flags & CURSOR_TYPE_READ_ONLY)
					&& (R->lastError = mysql_stmt_store_result(R->stmt))) {
					DEBUG("Warning: mysql_stmt_store_result() failed - %s\n", mysql_stmt_error(R->stmt));
					R->stop = true;
				}
			}
		}
	return R;
}


void MysqlResultSet_free(T *R) {
	Assertion_assert(R && *R);
    for (int i = 0; i < (*R)->columnCount; i++)
            FREE((*R)->columns[i].buffer);
    mysql_stmt_free_result((*R)->stmt);
    if ((*R)->keep == false)
            mysql_stmt_close((*R)->stmt);
    if ((*R)->meta)
            mysql_free_result((*R)->meta);
	(*R)->columnCount = 0;
    FREE((*R)->columns);
    FREE((*R)->bind);
	FREE(*R);
}


int MysqlResultSet_getColumnCount(T R) {
	Assertion_assert(R);
	return R->columnCount;
}


const char *MysqlResultSet_getColumnName(T R, int columnIndex) {
	Assertion_assert(R);
	columnIndex--;
	if (R->columnCount <= 0 || columnIndex < 0 || columnIndex > R->columnCount)
		return NULL;
	return R->columns[columnIndex].field->name;
}


long MysqlResultSet_getColumnSize(T R, int columnIndex) {
        int i = checkAndSetColumnIndex(columnIndex, R->columnCount);
        if (R->columns[i].is_null)
                return 0;
        return R->columns[i].real_length;
}


int MysqlResultSet_next(T R) {
	Assertion_assert(R);
        if (R->stop)
                return false;
        if (R->maxRows && (R->currentRow++ >= R->maxRows)) {
                R->stop = true;
#if MYSQL_VERSION_ID >= 50002
                /* Seems to need a cursor to work */
                mysql_stmt_reset(R->stmt); 
#else
                /* Bhaa! Where's my cancel method? 
                   Pencil neck mysql developers! */
                while (mysql_stmt_fetch(R->stmt) == 0); 
#endif
                return false;
        }
        if (R->needRebind) {
                if ((R->lastError = mysql_stmt_bind_result(R->stmt, R->bind)))
                        THROW(SQLException, "mysql_stmt_bind_result -- %s", mysql_stmt_error(R->stmt));
                R->needRebind = false;
        }
        R->lastError = mysql_stmt_fetch(R->stmt);
        if (R->lastError == 1)
                THROW(SQLException, "mysql_stmt_fetch -- %s", mysql_stmt_error(R->stmt));
        return ((R->lastError == MYSQL_OK) || (R->lastError == MYSQL_DATA_TRUNCATED));
}


int MysqlResultSet_nextResultSet(T R) {
	int status;

	/* free last result set */
	for (int i = 0; i < R->columnCount; i++)
		FREE(R->columns[i].buffer);
	mysql_stmt_free_result(R->stmt);
	if (R->meta) {
		mysql_free_result(R->meta);
		R->meta_fields = NULL;
		R->meta = NULL;
	}
	FREE(R->columns);
	FREE(R->bind);
	R->columnCount = 0;

	/* more results? -1 = no, >0 = error, 0 = yes (keep looking) */
	status = mysql_stmt_next_result(R->stmt);
	if (0 == status) {
		/* the column count is > 0 if there is a result set */
		/* 0 if the result is only the final status packet */
		R->columnCount = mysql_stmt_field_count(R->stmt);
		if (R->columnCount <= 0
			|| NULL == R->stmt->mysql
			|| !(R->meta = mysql_stmt_result_metadata(R->stmt))) {
			DEBUG("Warning: column error - %s\n", mysql_stmt_error(R->stmt));
			R->stop = true;
			/* column buffer is not allocated yet, so we force columCount to be zero. */
			R->columnCount = 0; 
			return false;
		}
		else {
			/* there is a result set to fetch */
			R->meta_fields = mysql_fetch_fields(R->meta);
			R->bind = CALLOC(R->columnCount, sizeof(MYSQL_BIND));
			R->columns = CALLOC(R->columnCount, sizeof(struct column_t));
			/* set up and bind result set output buffers */
			for (int i = 0; i < R->columnCount; i++) {
				R->columns[i].buffer = ALLOC(STRLEN + 1);
				R->bind[i].buffer_type = R->meta_fields[i].type;
				R->bind[i].buffer = R->columns[i].buffer;
				R->bind[i].buffer_length = STRLEN;
				R->bind[i].is_null = &R->columns[i].is_null;
				R->bind[i].length = &R->columns[i].real_length;
				R->columns[i].field = mysql_fetch_field_direct(R->meta, i);
			}
			if ((R->lastError = mysql_stmt_bind_result(R->stmt, R->bind))) {
				DEBUG("Warning: bind error - %s\n", mysql_stmt_error(R->stmt));
				R->stop = true;
				return false;
			}
			else {
				/* Now buffer all results to client (optional step), CURSOR_TYPE_READ_ONLY already do mysql_stmt_store_result */
				if (!(R->stmt->flags & CURSOR_TYPE_READ_ONLY)
					&& (R->lastError = mysql_stmt_store_result(R->stmt))) {
					DEBUG("Warning: mysql_stmt_store_result() failed - %s\n", mysql_stmt_error(R->stmt));
					R->stop = true;
				}
			}
		}
		return true;
	}
	return false;
}


int MysqlResultSet_isnull(T R, int columnIndex) {
        Assertion_assert(R);
        int i = checkAndSetColumnIndex(columnIndex, R->columnCount);
        return R->columns[i].is_null;
}


const char *MysqlResultSet_getString(T R, int columnIndex, int *outColumnFieldType) {
        Assertion_assert(R);
        int i = checkAndSetColumnIndex(columnIndex, R->columnCount);
		*outColumnFieldType = R->columns[i].field->type;
        if (R->columns[i].is_null)
                return NULL;
        _ensureCapacity(R, i);
        R->columns[i].buffer[R->columns[i].real_length] = 0;
        return R->columns[i].buffer;
}


float MysqlResultSet_getFloat(T R, int columnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	int i = checkAndSetColumnIndex(columnIndex, R->columnCount);
	*outColumnFieldType = R->columns[i].field->type;
	if (R->columns[i].is_null)
		return 0.0f;
	_ensureCapacity(R, i);
	return *(float *)R->columns[i].buffer;
}


double MysqlResultSet_getDouble(T R, int columnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	int i = checkAndSetColumnIndex(columnIndex, R->columnCount);
	*outColumnFieldType = R->columns[i].field->type;
	if (R->columns[i].is_null)
		return 0.0f;
	_ensureCapacity(R, i);
	return *(double *)R->columns[i].buffer;
}


const void *MysqlResultSet_getBlob(T R, int columnIndex, int *size, int *outColumnFieldType) {
        Assertion_assert(R);
        int i = checkAndSetColumnIndex(columnIndex, R->columnCount);
		*outColumnFieldType = R->columns[i].field->type;
		if (R->columns[i].is_null)
                return NULL;
        _ensureCapacity(R, i);
        *size = (int)R->columns[i].real_length;
        return R->columns[i].buffer;
}


time_t MysqlResultSet_getTimestamp(T R, int columnIndex, int *outColumnFieldType) {
	Assertion_assert(R);

	MYSQL_TIME *tmVal;
	struct tm tm_;

	int i = checkAndSetColumnIndex(columnIndex, R->columnCount);
	*outColumnFieldType = R->columns[i].field->type;
	if (R->columns[i].is_null)
		return 0;
	_ensureCapacity(R, i);
	R->columns[i].buffer[R->columns[i].real_length] = 0;
	tmVal = (MYSQL_TIME *)(R->columns[i].buffer);
	tm_.tm_year = tmVal->year - 1900;
	tm_.tm_mon = tmVal->month - 1;
	tm_.tm_mday = tmVal->day;
	tm_.tm_hour = tmVal->hour;
	tm_.tm_min = tmVal->minute;
	tm_.tm_sec = tmVal->second;
	/*return timegm(&tm_); */
#define CHINA_TZ 28800
	return timegm(&tm_) - CHINA_TZ;

	/* timegm no timezone, mktime use timezone */
	/*return mktime(&tm_);*/
}


struct tm * MysqlResultSet_getDatetime(T R, int columnIndex, struct tm *tm, int *outColumnFieldType) {
	Assertion_assert(R);
	
	MYSQL_TIME *tmVal;

	int i = checkAndSetColumnIndex(columnIndex, R->columnCount);
	*outColumnFieldType = R->columns[i].field->type;
	if (R->columns[i].is_null)
		return NULL;
	_ensureCapacity(R, i);
	R->columns[i].buffer[R->columns[i].real_length] = 0;
	tmVal = (MYSQL_TIME *)(R->columns[i].buffer);
	tm->tm_year = tmVal->year - 1900;
	tm->tm_mon = tmVal->month - 1;
	tm->tm_mday = tmVal->day;
	tm->tm_hour = tmVal->hour;
	tm->tm_min = tmVal->minute;
	tm->tm_sec = tmVal->second;
	return tm;
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif

