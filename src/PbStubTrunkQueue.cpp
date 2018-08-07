//------------------------------------------------------------------------------
//  PbStubTrunkQueue.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "PbStubTrunkQueue.h"

//------------------------------------------------------------------------------
/**

*/
CPbStubTrunkQueue::CPbStubTrunkQueue(int nProducerNum)
	: _workQueue(std::make_shared<CCamelConcurrentQueue>(nProducerNum)) {

}

//------------------------------------------------------------------------------
/**

*/
CPbStubTrunkQueue::~CPbStubTrunkQueue() {

}

//------------------------------------------------------------------------------
/**

*/
void
CPbStubTrunkQueue::Add(IDbService::load_cb_t&& onResponse, std::vector<std::string>&& vResponse) {

	auto workCb = std::bind([](IDbService::load_cb_t& onResponse, std::vector<std::string>& vResponse) {
		onResponse(std::move(vResponse));
	}, std::move(onResponse), std::move(vResponse));

	Add(std::move(workCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CPbStubTrunkQueue::Add(IDbService::query_cb_t&& onResponse, std::string&& strResponse) {

	auto workCb = std::bind([](IDbService::query_cb_t& onResponse, std::string& strResponse) {
		onResponse(std::move(strResponse));
	}, std::move(onResponse), std::move(strResponse));

	Add(std::move(workCb));
}

/* -- EOF -- */