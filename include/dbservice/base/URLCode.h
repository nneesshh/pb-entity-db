#pragma once
//------------------------------------------------------------------------------
/**
@class CURLCode

(C) 2016 n.lee
*/
#include <string>
#include <assert.h>

//------------------------------------------------------------------------------
/**
@brief CURLCode
*/
class CURLCode {
public:
	CURLCode() { }
	~CURLCode() { }

	static std::string Encode(const char* lpszData);
	static std::string Decode(std::string inTmp);

private:
	inline static unsigned char toHex(const unsigned char &x){ return x > 9 ? x + 55: x + 48; }
	inline static unsigned char fromHex(const unsigned char &x) {
		unsigned char y;  
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;  
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;  
		else if (x >= '0' && x <= '9') y = x - '0';  
		else assert(0);  
		return y;  
	}
};

/*EOF*/