#ifndef __STD_LOG_H__
#define __STD_LOG_H__

//------------------------------------------------------------------------------
/**
    @class CStdLog
    
    (C) 2016 n.lee
*/
#include "StdLogLevelDef.h"

#ifdef MYTOOLKIT_NAMESPACE
namespace MyToolkit {
#endif

//------------------------------------------------------------------------------
/** 
	@brief StdLog
*/
class StdLog {
public:
	virtual ~StdLog() {};

	virtual void logprint(int lvl, const char *format, ...) = 0;
};

#ifdef MYTOOLKIT_NAMESPACE
}
#endif

#endif
/*EOF*/