#ifndef __PROTOBUF_IMPORTER_H__
#define __PROTOBUF_IMPORTER_H__

//------------------------------------------------------------------------------
/**
    @class CProtobufImporter
    
    (C) 2016 n.lee
*/
#include "../UsingProtobuf.h"

#include <string>

#include "../log/StdLog.h"

//------------------------------------------------------------------------------
/** 
	@brief CProtobufImporter
*/
class CProtobufImporter {
public:
	CProtobufImporter(StdLog *pLog, const std::string& strDynamicPath);
	virtual ~CProtobufImporter();
	
	google::protobuf::Message *	GetEntityProto(const std::string& strEntityName) {
		google::protobuf::Message *pProto = nullptr;
		auto& iter = _mapEntityProto.find(strEntityName);
		if (iter != _mapEntityProto.end()) {
			pProto = iter->second;
			pProto->Clear();
		}
		else {
			const google::protobuf::Message *prototype = LookupPrototype(strEntityName);
			if (prototype) {
				// Allocate a protobuf message in the arena.
				pProto = prototype->New(&_arena);

				// cache
				_mapEntityProto[strEntityName] = pProto;
			}
		}
		return pProto;
	}

private:
	const google::protobuf::Descriptor *	FindDescriptor(const std::string& strTypeName) const {
		return _importer->pool()->FindMessageTypeByName(strTypeName);
	}

	const google::protobuf::Message *	FindPrototype(const google::protobuf::Descriptor& descriptor) const {
		return _dmFactory->GetPrototype(&descriptor);
	}

	const google::protobuf::Message *	LookupPrototype(const std::string& strTypeName) {
		const google::protobuf::Descriptor *descriptor = FindDescriptor(strTypeName);
		if (nullptr == descriptor && _refLog) {
			// error
			_refLog->logprint(LOG_LEVEL_ERROR, "\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!!!!!!![CProtobufImporter::LookupPrototype()] failed, type_name = %s!!!!!!!!\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
				, strTypeName.c_str());
			return nullptr;
		}
		return FindPrototype(*descriptor);
	}

	int							GetMessageTypeFromProtoFile(const std::string& proto_filename, google::protobuf::FileDescriptorProto *out_desc_proto);

	void						ImportDynamicPath(const std::string& strDynamicPath);
	int							ImportMessageTypeFromProtoFile(const std::string& proto_filename);

private:
	StdLog *_refLog;

	/* dynamic -- lookup prototype */
	google::protobuf::compiler::DiskSourceTree	*_sourceTree = new google::protobuf::compiler::DiskSourceTree();
	google::protobuf::compiler::Importer		*_importer = new google::protobuf::compiler::Importer(_sourceTree, nullptr);
	google::protobuf::DynamicMessageFactory		*_dmFactory = new google::protobuf::DynamicMessageFactory();

	google::protobuf::Arena _arena;
	std::unordered_map<std::string, google::protobuf::Message *> _mapEntityProto;
};

#endif
/*EOF*/