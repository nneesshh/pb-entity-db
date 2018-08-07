#pragma once
//------------------------------------------------------------------------------
/**
@class CPbStubTrunkQueue

(C) 2016 n.lee
*/
#include "base/CamelConcurrentQueue.h"
#include "base/IDbService.h"

#include "db/ProtobufImporter.h"
#include "db/IDbStubParam.h"

//------------------------------------------------------------------------------
/**
@brief CPbStubTrunkQueue
*/
class CPbStubTrunkQueue {
public:
	CPbStubTrunkQueue(int nProducerNum);
	~CPbStubTrunkQueue();

	void RunOnce() {
		_workQueue->RunOnce();
	}

	void Close() {
		_workQueue->Close();
	}

	void Add(std::function<void()>&& workCb) {
		_workQueue->Add(std::move(workCb));
	}

	void Add(IDbService::load_cb_t&&, std::vector<std::string>&&);
	void Add(IDbService::query_cb_t&&, std::string&&);

private:
	CCamelConcurrentQueuePtr _workQueue;
};
using CPbStubTrunkQueuePtr = std::shared_ptr<CPbStubTrunkQueue>;

/*EOF*/