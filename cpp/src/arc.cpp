//
// Created by Julian Janda on 07.05.18.
//

#include "arc.hpp"
#include <iostream>
#include <boost/asio/io_service.hpp>
#include <local-stream.hpp>
#include <utility>
#include <remote-stream.hpp>
#include <frame-data.hpp>
#include <thread>

using namespace ndnrtc;

Arc::Arc(AdaptionLogic adaptionLogic,
         RemoteStreamImpl* pimpl,
         boost::shared_ptr<statistics::StatisticsStorage> &storage)
        : selectedAdaptionLogic(adaptionLogic),
          sstorage_(storage)
{
    // TODO write about observing in description
    // TODO Check out 'rate-adaption-module.hpp' for hints
    // TODO pass video thread data through config file params instead
    // TODO Check if info is delivered in meta data.
    videoThread rep1, rep2, rep3;
    rep1.threadName = "low";
    rep1.max_bitrate = "1000";
    rep2.threadName = "med";
    rep2.max_bitrate = "1500";
    rep3.threadName = "high";
    rep3.max_bitrate = "2000";
    videoThreads.emplace_back(rep1);
    videoThreads.emplace_back(rep2);
    videoThreads.emplace_back(rep3);
    this->pimpl = pimpl;
    lastThreadtoFetchChangeTime = ndn_getNowMilliseconds();

    // TODO set initial threadToFetch dynamically
    threadToFetch = videoThreads.begin()->threadName;

}

Arc::~Arc() = default;

void Arc::calculateThreadToFetch() {
    switch (getSelectedAdaptionLogic()) {
        case AdaptionLogic::NoAdaption: threadToFetch = noAdaption(); break;
        case AdaptionLogic::Random: threadToFetch = randomAdaption(); break;
        case AdaptionLogic::Dash_JS: threadToFetch = dashJS(); break;
        case AdaptionLogic::Thang: threadToFetch = thang(); break;
    }
    // Force a minimum time between changes of representations
    double now = ndn_getNowMilliseconds();
    if (now - lastThreadtoFetchChangeTime >= MINIMUM_THREAD_TIME && threadToFetch != lastThreadToFetch) {
        std::cout << "Setting threadToFetch = " << threadToFetch << std::endl;
        pimpl->getPipelineControl()->getMachine().setThreadPrefix(threadToFetch);
        lastThreadToFetch = threadToFetch;
        lastThreadtoFetchChangeTime = now;
    }
}

AdaptionLogic Arc::getSelectedAdaptionLogic() {
    return selectedAdaptionLogic;
}

void Arc::onInterestIssued(const boost::shared_ptr<const ndn::Interest> & interest) {
    sentInterests.insert(std::make_pair(interest->getName().toUri(), ndn_getNowMilliseconds()));

    // TODO delete this (only used for experiments)
/*    std::string name = interest->getName().getSubName(ndnrtc::prefix_filter::Stream, 1).toUri();
    if (name == "/k") {
        std::cout << "\t" << ndn_getNowMilliseconds() - counter
                  << "\t" << counter2 + 1
                  << "\t" << counter3 + 1
                  << std::endl;
        counter = ndn_getNowMilliseconds();
        counter2++;
    }
    counter3++;*/
}

void Arc::segmentArrived(const boost::shared_ptr<WireSegment> & wireSeg) {

    if(wireSeg->isPacketHeaderSegment() && wireSeg->getSampleClass() == SampleClass::Key) {
        // TODO only use info from video segments (?)
//        std::cout << "SegmentsReceivedNum = " << (*sstorage_)[statistics::Indicator::SegmentsReceivedNum] << std::endl;

        double prodTime;
        std::string name = wireSeg->getData()->getName().toUri();
        auto search = sentInterests.find(name);
        if(search != sentInterests.end()) {
            prodTime = search->second;
        } else {
            std::cout << "Interest not found in map." << std::endl;
        }

        // TODO Find out why rtt keeps increasing over time
        double now = ndn_getNowMilliseconds();
        double rtt = (now - prodTime) / 1000; // Divide by 1000 to convert ms --> s // TODO already measured in drd-change-estimator?
        long size = wireSeg->getData()->getContent().size() * 8; // convert from Byte to bit
        long size2 = wireSeg->getData()->getDefaultWireEncoding().size() * 8;  // convert from Byte to bit // TODO is this better for size?
        dashJS_lastSegmentMeasuredThroughput = size / rtt; // bit/s

        // TODO delete (only used for testing)
//        std::cout << "pT = " << prodTime;
//        std::cout << ", cT = " << now;
//        std::cout << ", size = " << size << " bit";
//        std::cout << ", size = " << size2 << " bit";
//        std::cout << ", rtt = " << rtt;
//        std::cout << ", throughput = " << dashJS_lastSegmentMeasuredThroughput;
//        std::cout << std::endl;

        calculateThreadToFetch();
    }
}

std::string Arc::noAdaption() {
    // Do nothing
    return threadToFetch;
}

std::string Arc::randomAdaption() {
    int min = 0;
    int max = videoThreads.size() -1;
    // Make sure the chosen representation differs from the last one
    do {
        int randNum = min + (rand() % static_cast<int>(max - min + 1));
        threadToFetch = videoThreads[randNum].threadName;
    } while (threadToFetch == lastThreadToFetch);

    return threadToFetch;
}

std::string Arc::dashJS() {
    double bn = dashJS_lastSegmentCalculatedThroughput;
    double bm = dashJS_lastSegmentMeasuredThroughput;
    double w1 = 0.7; // weight 1
    double w2 = 1.3; // weight 2
    int representationBitrate = 0; // bit rate of currently selected representation

    // Estimated bitrate calculation:
    double num1 = bn * w1;
    double num2 = bm * w2;
    double den = w1 + w2;
    double nextBn = std::floor((num1 + num2) / den); // nextSegmentCalculatedThroughput
    dashJS_lastSegmentCalculatedThroughput = nextBn;

    // TODO delete (only used for testing)
/*    std::cout << "nextbn = " << nextBn
              << "\tbn = " << bn
              << "\tbm = " << bm
              << std::endl*/;

    // Rate selection
    for (auto selRep = videoThreads.rbegin(); selRep != videoThreads.rend(); ++selRep) {
        representationBitrate = std::stoi(selRep->max_bitrate);
        if (representationBitrate <= nextBn) {
            return selRep->threadName;
        }
    }
//    std::cout << "dashJS couldn't find a threadToFetch!" << std::endl;
    return threadToFetch;
}

std::string Arc::thang() {
    // TODO Implement from pseudocode
    std::cout << "THANG adaption performed." << std::endl;
/*    1 Prerequisites:
    2 bitrate observed in bpsS
    3 bitrate estimated in bpsE
    4
    5 function computeP () {
        6 lastBpsC= bpsS [ bpsS.length - 1];
        7 lastBpsE= bpsE [ bpsE.length - 1];
        8 return ( Math.abs ( lastBpsC - lastBpsE ) ) / lastBpsE;
        9 }
    10
    11 function computeDelta () {
        12 p=computeP () ;
        13 delta= 1 / (1 + Math.exp (( -k ) *( p-p0 ) ) ) ;
        14 }
    15
    16
    17 function calculateBitrate () {
        18 if( bpsE.length < 2) {
            19 lastBps= bpsS [ bpsS.length - 1];
            20 bpsE.push ( lastBps ) ;
            21 calcBps= lastBps;
            22 } else {
            23 pos= bpsE.length - 2;
            24 bpsE.shift () ;
            25 te= (1 -delta ) * bpsE [ pos ] + delta * bpsS [ bpsS.length - 1];
            26 bpsE.push ( Math.floor ( te ) ) ;
            27 calcBps = Math.floor ( te ) ;
            28 }
        29 calcBps= Math.floor ((1 -mu ) * calcBps );
        30 }
    31
    32
    33 Rate selection:
    34 bitrateSupposed= getBitrate () ;
    35 nres= getVideoRepsNum ( period , selRepV ) ;
    36 for ( var i=nres-1; i> =0; i-- ) {
        37 videoBandwidth= getVideoBandwidth ( period , i ) ;
        38 if( videoBandwidth < bitrateSupposed ) {
            39 if( i != selRepV ) {
                40 selRepV=i;
                41 }
            42 return;
            43 }
        44 }
    45 selRepV=0;*/
    return threadToFetch;
}