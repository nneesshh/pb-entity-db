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

#include "ResultSet.h"
#include "system/Time.h"


/**
* Implementation of the ResultSet interface
*
* @file
*/


/* ----------------------------------------------------------- Definitions */


#define T ResultSet_T
struct ResultSet_S {
	Rop_T op;
	ResultSetDelegate_T D;
};


/* ------------------------------------------------------- Private methods */


static inline int _getIndex(T R, const char *name) {
	int columns = ResultSet_getColumnCount(R);
	for (int i = 1; i <= columns; i++)
		if (Str_isByteEqual(name, ResultSet_getColumnName(R, i)))
			return i;
	THROW(SQLException, "Invalid column name '%s'", name ? name : "null");
	return -1;
}


/* ----------------------------------------------------- Protected methods */


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

T ResultSet_new(ResultSetDelegate_T D, Rop_T op) {
	T R;
	Assertion_assert(D);
	Assertion_assert(op);
	NEW(R);
	R->D = D;
	R->op = op;
	return R;
}


void ResultSet_free(T *R) {
	Assertion_assert(R && *R);
	(*R)->op->free(&(*R)->D);
	FREE(*R);
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif


/* ------------------------------------------------------------ Properties */


int ResultSet_getColumnCount(T R) {
	Assertion_assert(R);
	return R->op->getColumnCount(R->D);
}


const char *ResultSet_getColumnName(T R, int columnIndex) {
	Assertion_assert(R);
	return R->op->getColumnName(R->D, columnIndex);
}


long ResultSet_getColumnSize(T R, int columnIndex) {
	Assertion_assert(R);
	return R->op->getColumnSize(R->D, columnIndex);
}


/* -------------------------------------------------------- Public methods */


int ResultSet_next(T R) {
	return R ? R->op->next(R->D) : false;
}



int ResultSet_nextResultSet(T R) {
	return R ? R->op->nextResultSet(R->D) : false;
}


int ResultSet_isnull(T R, int columnIndex) {
	Assertion_assert(R);
	return R->op->isnull(R->D, columnIndex);
}


/* --------------------------------------------------------------- Columns */


const char *ResultSet_getString(T R, int columnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	return R->op->getString(R->D, columnIndex, outColumnFieldType);
}


const char *ResultSet_getStringByName(T R, const char *columnName, int *outColumnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	*outColumnIndex = _getIndex(R, columnName);
	return ResultSet_getString(R, *outColumnIndex, outColumnFieldType);
}


int ResultSet_getInt(T R, int columnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	const char *s = R->op->getString(R->D, columnIndex, outColumnFieldType);
	return s ? *(int *)(s) : 0;
}


int ResultSet_getIntByName(T R, const char *columnName, int *outColumnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	*outColumnIndex = _getIndex(R, columnName);
	return ResultSet_getInt(R, *outColumnIndex, outColumnFieldType);
}


long long ResultSet_getLLong(T R, int columnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	const char *s = R->op->getString(R->D, columnIndex, outColumnFieldType);
	return s ? *(long long *)(s) : 0;
}


long long ResultSet_getLLongByName(T R, const char *columnName, int *outColumnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	*outColumnIndex = _getIndex(R, columnName);
	return ResultSet_getLLong(R, *outColumnIndex, outColumnFieldType);
}


float ResultSet_getFloat(T R, int columnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	return R->op->getFloat(R->D, columnIndex, outColumnFieldType);
}


float ResultSet_getFloatByName(T R, const char *columnName, int *outColumnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	*outColumnIndex = _getIndex(R, columnName);
	return ResultSet_getFloat(R, *outColumnIndex, outColumnFieldType);
}


double ResultSet_getDouble(T R, int columnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	return R->op->getDouble(R->D, columnIndex, outColumnFieldType);
}


double ResultSet_getDoubleByName(T R, const char *columnName, int *outColumnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	*outColumnIndex = _getIndex(R, columnName);
	return ResultSet_getDouble(R, *outColumnIndex, outColumnFieldType);
}


const void *ResultSet_getBlob(T R, int columnIndex, int *size, int *outColumnFieldType) {
	Assertion_assert(R);
	const void *b = R->op->getBlob(R->D, columnIndex, size, outColumnFieldType);
	if (!b)
		*size = 0;
	return b;
}


const void *ResultSet_getBlobByName(T R, const char *columnName, int *size, int *outColumnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	*outColumnIndex = _getIndex(R, columnName);
	return ResultSet_getBlob(R, *outColumnIndex, size, outColumnFieldType);
}


/* --------------------------------------------------------- Date and Time */


time_t ResultSet_getTimestamp(T R, int columnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	time_t t = 0;
	if (R->op->getTimestamp) {
		t = R->op->getTimestamp(R->D, columnIndex, outColumnFieldType);
	}
	else {
		const char *s = ResultSet_getString(R, columnIndex, outColumnFieldType);
		if (STR_DEF(s))
			t = Time_toTimestamp(s);
	}
	return t;
}


time_t ResultSet_getTimestampByName(T R, const char *columnName, int *outColumnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	*outColumnIndex = _getIndex(R, columnName);
	return ResultSet_getTimestamp(R, *outColumnIndex, outColumnFieldType);
}


struct tm ResultSet_getDateTime(T R, int columnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	struct tm t = { .tm_year = 0 };
	if (R->op->getDateTime) {
		R->op->getDateTime(R->D, columnIndex, &t, outColumnFieldType);
	}
	else {
		const char *s = ResultSet_getString(R, columnIndex, outColumnFieldType);
		if (STR_DEF(s))
			Time_toDateTime(s, &t);
	}
	return t;
}


struct tm ResultSet_getDateTimeByName(T R, const char *columnName, int *outColumnIndex, int *outColumnFieldType) {
	Assertion_assert(R);
	*outColumnIndex = _getIndex(R, columnName);
	return ResultSet_getDateTime(R, *outColumnIndex, outColumnFieldType);
}