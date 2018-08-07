//------------------------------------------------------------------------------
//  ProtobufImporter.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "ProtobufImporter.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CProtobufImporter::CProtobufImporter(StdLog *pLog, const std::string& strDynamicPath)
	: _refLog(pLog)	{
	//
	ImportDynamicPath(strDynamicPath);
}

//------------------------------------------------------------------------------
/**

*/
CProtobufImporter::~CProtobufImporter() {
	//
	delete _dmFactory;
	delete _importer;
	delete _sourceTree;
}

//------------------------------------------------------------------------------
/**
// Parsing given .proto file for Descriptor of the given message (by
// name).  The returned message descriptor can be used with a
// DynamicMessageFactory in order to create prototype message and
// mutable messages.  For example:

//DynamicMessageFactory factory;
//const Message* prototype_msg = factory.GetPrototype(message_descriptor);
//const Message* mutable_msg = prototype_msg->New();
*/
int
CProtobufImporter::GetMessageTypeFromProtoFile(const std::string& proto_filename, google::protobuf::FileDescriptorProto *out_desc_proto) {
	FILE *proto_file = fopen(proto_filename.c_str(), "r"); {
		if (nullptr == proto_file) {
			// warning
			if (_refLog) {
				_refLog->logprint(LOG_LEVEL_WARNING, "[tid(%d)] cannot open .proto file: %s",
					(int)::GetCurrentThreadId(), proto_filename.c_str());
			}
			return -1;
		}

		google::protobuf::io::FileInputStream proto_input_stream(fileno(proto_file));
		google::protobuf::io::Tokenizer tokenizer(&proto_input_stream, nullptr);
		google::protobuf::compiler::Parser parser;
		if (!parser.Parse(&tokenizer, out_desc_proto)) {
			// warning
			if (_refLog) {
				_refLog->logprint(LOG_LEVEL_WARNING, "[tid(%d)] cannot parse.proto file : %s",
					(int)::GetCurrentThreadId(), proto_filename.c_str());
			}
			return -2;
		}
	}
	fclose(proto_file);

	// Here we walk around a bug in protocol buffers that
	// |Parser::Parse| does not set name (.proto filename) in
	// file_desc_proto.
	if (!out_desc_proto->has_name()) {
		out_desc_proto->set_name(proto_filename);
	}

	//
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
void
CProtobufImporter::ImportDynamicPath(const std::string& strDynamicPath) {
	//look up .proto file in target directory
	std::string strPath = strDynamicPath;
	std::size_t found = strPath.find_last_of('/');
	if (std::string::npos != found) {
		strPath = strPath.substr(0, found);
		_sourceTree->MapPath("", strPath);
	}
	else {
		_sourceTree->MapPath("", "./");
	}

	//
	{
		char chLine[512];

		//
		FILE *fp = fopen(strDynamicPath.c_str(), "r");
		if (fp) {
			// notice
			if (_refLog) {
				_refLog->logprint(LOG_LEVEL_NOTICE, "\n    \t[tid(%d)] import dynamic path -- (%s)...",
					(int)::GetCurrentThreadId(), strDynamicPath.c_str());
			}

			while (nullptr != fgets(chLine, sizeof(chLine), fp)) {
				// remove tail '\r' and '\n'
				int nIndex = strlen(chLine);
				while (--nIndex > 0) {
					if ('\r' != chLine[nIndex] && '\n' != chLine[nIndex]) {
						break;
					}
					chLine[nIndex] = '\0';
				}

				// import prototype
				if (nIndex > 0) {
					ImportMessageTypeFromProtoFile(chLine);
				}
			}
			fprintf(stderr, "\n");

			// notice
			if (_refLog) {
				_refLog->logprint(LOG_LEVEL_NOTICE, "\n    \t[tid(%d)] import dynamic path -- (%s) over\n",
					(int)::GetCurrentThreadId(), strDynamicPath.c_str());
			}
			fclose(fp);
		}
		else {
			char chDesc[256];
#if defined(__CYGWIN__) || defined( _WIN32)
			_snprintf_s(chDesc, sizeof(chDesc), "!!! [CProtobufImporter::ImportDynamicPath()] path(%s) invalid!!!",
				strDynamicPath.c_str());
#else
			snprintf(chDesc, sizeof(chDesc), "!!! [CProtobufImporter::ImportDynamicPath()] path(%s) invalid!!!",
				strDynamicPath.c_str());
#endif
			fprintf(stderr, chDesc);
			system("pause");
			throw std::exception(chDesc);
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
int
CProtobufImporter::ImportMessageTypeFromProtoFile(const std::string& proto_filename) {
	//
	if (_refLog) {
		_refLog->logprint(LOG_LEVEL_NOTICE, "\n    \t[tid(%d)] import protobuf file(%s)...",
			(int)::GetCurrentThreadId(), proto_filename.c_str());
	}

	//
	const google::protobuf::FileDescriptor *file_desc = _importer->Import(proto_filename);
	if (nullptr == file_desc) {
		// warning
		if (_refLog) {
			_refLog->logprint(LOG_LEVEL_WARNING, "\n[tid(%d)] cannot import .proto file: %s\n",
				(int)::GetCurrentThreadId(), proto_filename.c_str());
		}
		return -1;
	}
	return 0;
}

/* -- EOF -- */