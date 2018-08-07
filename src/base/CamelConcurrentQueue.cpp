//------------------------------------------------------------------------------
//  CamelConcurrentQueue.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "CamelConcurrentQueue.h"

//------------------------------------------------------------------------------
/**

*/
CCamelConcurrentQueue::CCamelConcurrentQueue(int nProducerNum)
	: _producerNum(nProducerNum)
	, _callbacks(nProducerNum * MyTraits::BLOCK_SIZE) {

}

//------------------------------------------------------------------------------
/**

*/
CCamelConcurrentQueue::~CCamelConcurrentQueue() {

}

//------------------------------------------------------------------------------
/**

*/
void
CCamelConcurrentQueue::RunOnce() {
	// work queue
	int nCount = 0;
	CallbackEntry workCb;
	while (!_close && _callbacks.try_dequeue(workCb)) {
		workCb();

		//
		++nCount;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CCamelConcurrentQueue::Close() {
	//
	// Set done flag and notify.
	//
	_close = true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CCamelConcurrentQueue::Add(std::function<void()>&& workCb) {
	if (_close) {
		// error
		fprintf(stderr, "[CCamelConcurrentQueue::Add()] can't enqueue, callback is dropped!!!");
		return false;
	}

	//
	// Add work item.
	//
	if (!_callbacks.enqueue(std::move(workCb))) {
		// error
		fprintf(stderr, "[CCamelConcurrentQueue::Add()] enqueue failed, callback is dropped!!!");
		return false;
	}
	return true;
}

/** -- EOF -- **/