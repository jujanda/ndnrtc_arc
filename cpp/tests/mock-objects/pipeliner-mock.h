// 
// pipeliner-mock.h
//
//  Created by Peter Gusev on 14 June 2016.
//  Copyright 2013-2016 Regents of the University of California
//

#ifndef __pipeliner_mock_h__
#define __pipeliner_mock_h__

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "pipeliner.h"

class MockPipeliner : public ndnrtc::IPipeliner {
public:
	MOCK_METHOD1(express, void(const ndn::Name&));
	MOCK_METHOD1(express, void(const std::vector<boost::shared_ptr<const ndn::Interest>>&));
	MOCK_METHOD1(segmentArrived, void(const ndn::Name&));
	MOCK_METHOD0(reset, void());
	MOCK_METHOD1(setNeedSample, void(ndnrtc::SampleClass));
	MOCK_METHOD0(setNeedRightmost, void());
	MOCK_METHOD2(setSequenceNumber, void(PacketNumber seqNo, ndnrtc::SampleClass cls));
};

#endif