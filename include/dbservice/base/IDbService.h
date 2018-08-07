#pragma once
//------------------------------------------------------------------------------
/**
@class IDbService

(C) 2016 n.lee
*/
#include "../UsingProtobuf.h"

#include <functional>
#include <string>

#include "platform_types.h"

//------------------------------------------------------------------------------
/**
@brief IDbService
*/
class MY_DB_EXTERN	IDbService {
public:
	virtual ~IDbService() { }

	using load_cb_t = std::function<void(std::vector<std::string>& vResult)>;
	using query_cb_t = std::function<void(std::string& strResonse)>;
	using dispose_cb_t = std::function<void()>;

	struct cmd_t {
		enum DB_CMD_TYPE {
			// entity
			DB_CMD_TYPE_E_LOAD_STRING = 1,
			DB_CMD_TYPE_E_ADD_STRING = 2,
			DB_CMD_TYPE_E_SAVE_STRING = 3,
			DB_CMD_TYPE_E_DELETE_STRING = 4,
			DB_CMD_TYPE_E_LOAD = 5,
			DB_CMD_TYPE_E_ADD = 6,
			DB_CMD_TYPE_E_SAVE = 7,
			DB_CMD_TYPE_E_DELETE = 8,

			// stored proc
			DB_CMD_TYPE_SP_QUERY_STRING = 11,
			DB_CMD_TYPE_SP_UPDATE_STRING = 12,
			DB_CMD_TYPE_SP_QUERY = 13,
			DB_CMD_TYPE_SP_UPDATE = 14,
		};

		DB_CMD_TYPE _type;
		std::string _typename;
		std::string _request;
		std::string _tablename;
		std::vector<std::string> _key_list;
		const google::protobuf::Message *_pbmsg_ptr;
		load_cb_t _load_cb;
		query_cb_t _query_cb;
		dispose_cb_t _dispose_cb;
	};

	virtual void				RunOnce() = 0;

	//////////////////////////////////////////////////////////////////////////
	/// entity
	//////////////////////////////////////////////////////////////////////////
	virtual void				EntityLoad(const google::protobuf::Message& entity, std::vector<std::string>& vResult, const std::string& sTableName, const std::vector<std::string>& vKey) = 0;
	virtual void				EntityAdd(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) = 0;
	virtual void				EntitySave(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) = 0;
	virtual void				EntityDelete(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) = 0;

	// async version
	virtual void				EntityLoadAsync(const google::protobuf::Message& entity, load_cb_t&& cb, const std::string& sTableName, const std::vector<std::string>& vKey) = 0;
	virtual void				EntityAddAsync(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) = 0;
	virtual void				EntitySaveAsync(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) = 0;
	virtual void				EntityDeleteAsync(const google::protobuf::Message& entity, const std::string& sTableName, const std::vector<std::string>& vKey) = 0;

	//////////////////////////////////////////////////////////////////////////
	/// stored proc
	//////////////////////////////////////////////////////////////////////////
	virtual void				StoredProcQuery(google::protobuf::Message& storedProc) = 0;
	virtual void				StoredProcUpdate(const google::protobuf::Message& storedProc) = 0;

	// async version
	virtual void				StoredProcQueryAsync(const google::protobuf::Message& storedProc, query_cb_t&& cb) = 0;
	virtual void				StoredProcUpdateAsync(const google::protobuf::Message& storedProc) = 0;

	virtual void				Shutdown() = 0;
};

/*EOF*/