#include "CapnpDbStub.h"

CCapnpDbStub::~CCapnpDbStub() {

}

void
CCapnpDbStub::OnInit(StdLog *pLog, const char * sURL, const char *sUser, const char *sPass, const char *sSchema, int nPoolSize) {
	_capnp2dbEngine = new Capnp2DbEngine();
	_capnp2dbEngine->OnInit(pLog, sURL, sUser, sPass, sSchema, nPoolSize);
}

void
CCapnpDbStub::OnDelete() {
	if (_capnp2dbEngine) {
		_capnp2dbEngine->OnDelete();
		delete _capnp2dbEngine;
		_capnp2dbEngine = nullptr;
	}
}

void
CCapnpDbStub::Open() {
	_capnp2dbEngine->Open();
}

void
CCapnpDbStub::Close() {
	_capnp2dbEngine->Close();
}


//------------------------------------------------------------------------------
/**

*/
int
CCapnpDbStub::ImportMessageTypeFromProtoFile(const std::string& proto_filename) {
	kj::StringPtr importPath[] = {
		""
	};

	// 	::capnp::SchemaParser parser;
	// 	::capnp::ParsedSchema schema = parser.parseDiskFile(proto_filename.c_str(), proto_filename.c_str(), importPath);
	// 	_vSchemaPool.push_back(schema);

	//
	return 0;
}