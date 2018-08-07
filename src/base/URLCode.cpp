//------------------------------------------------------------------------------
//  CURLCode.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "URLCode.h"

//------------------------------------------------------------------------------
/**

*/
std::string
CURLCode::Encode(const char* lpszData) {
	std::string strResult = "";
	unsigned char* pInTmp = (unsigned char*)lpszData;

	// do encoding
	while (*pInTmp) {
		if (isalnum(*pInTmp)
			|| '-' == *pInTmp
			|| '_' == *pInTmp
			|| '.' == *pInTmp
			|| '!' == *pInTmp
			|| '~' == *pInTmp
			|| '*' == *pInTmp
			|| '\'' == *pInTmp
			|| '(' == *pInTmp
			|| ')' == *pInTmp) {

			strResult += *pInTmp;
		}
		else if (isspace(*pInTmp)) {

			strResult += '+';
		}
		else {

			strResult += '%';
			strResult += toHex(*pInTmp>>4);
			strResult += toHex(*pInTmp%16);
		}
		++pInTmp;
	}

	return strResult;
}

//------------------------------------------------------------------------------
/**

*/
std::string
CURLCode::Decode(std::string inTmp) {
	std::string strResult = "";
	size_t inlen = inTmp.length();
	for(size_t i=0;i<inlen; ++i) {

		if(inTmp.at(i)=='%') {

			++i;
			char c = fromHex(inTmp.at(i++));
			c = c << 4;
			c += fromHex(inTmp.at(i));
			strResult += c;
		}
		else if (inTmp.at(i) == '+') {

			strResult += ' ';
		}
		else {

			strResult += inTmp.at(i);
		}
	}
	return strResult;
}

/* -- EOF -- */