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
    // TODO Threadinfo not delivered in meta data (only names) --> Find solution for that
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
    arcStartTime = ndn_getNowMilliseconds();
    lastThreadtoFetchChangeTime = ndn_getNowMilliseconds();

    // TODO set initial threadToFetch dynamically from config file | look how remote-stream-impl.cpp handles that
    threadToFetch = videoThreads.begin()->threadName;
    lastThreadToFetch = threadToFetch;

    // TODO Should store path in variable, ideally from config file
    // Prepare simple-logger
    LogInfo("/tmp/arcLog.csv") << "[StartTime]\t" << arcStartTime << std::endl;
    LogInfo("/tmp/arcLog_overtimers.csv") << "[StartTime]\t" << arcStartTime << std::endl;
    LogInfo("/tmp/arcLog_map.csv") << "[StartTime]\t" << arcStartTime << std::endl;
    LogInfo("/tmp/arcLog_unansweredInterests.csv") << "[StartTime]\t" << arcStartTime << std::endl;
    LogInfo("/tmp/arcLog_consumerSentInterests.csv") << "[StartTime]\t" << arcStartTime << std::endl;
    LogInfo("/tmp/arcLog_producerReceivedInterests.csv") << "[StartTime]\t" << arcStartTime << std::endl;
    LogInfo("/tmp/arcLog_producerSentData.csv") << "[StartTime]\t" << arcStartTime << std::endl;
    LogInfo("/tmp/arcLog_consumerReceivedData.csv") << "[StartTime]\t" << arcStartTime << std::endl;
}

Arc::~Arc() = default;

void Arc::setThreadsMeta(std::map<std::string, boost::shared_ptr<NetworkData>> threadsMeta) {
    // TODO Evaluate if this is still needed
    threadsMeta_ = threadsMeta;
    metaFetched = true;
/*    std::cout << "Meta fetched for arc" << std::endl;
    for (auto &x : threadsMeta_)
    {
        std::cout << x.first
                  << ':'
                  << x.second->getLength()
                  << std::endl;
    }*/
}

void Arc::calculateThreadToFetch() {
    // TODO Evaluate if this is still needed
    if (!metaFetched) {
        return;
    }

    switch (getSelectedAdaptionLogic()) {
        case AdaptionLogic::NoAdaption: threadToFetch = noAdaption(); break;
        case AdaptionLogic::Random: threadToFetch = randomAdaption(); break;
        case AdaptionLogic::Sequential: threadToFetch = sequentialAdaption(); break;
        case AdaptionLogic::Dash_JS: threadToFetch = dashJS(); break;
        case AdaptionLogic::Thang: threadToFetch = thang(); break;
    }
}


AdaptionLogic Arc::getSelectedAdaptionLogic() {
    return selectedAdaptionLogic;
}

void Arc::onInterestIssued(const boost::shared_ptr<const ndn::Interest> & interest) {

    // Log arrival
    LogTrace("/tmp/arcLog.csv") << "[outgoingInterest]\t" << interest->getName().toUri() << std::endl;
    LogTrace("/tmp/arcLog_consumerSentInterests.csv") << "[outgoingInterest]\t" << interest->getName().toUri() << std::endl;

    // TODO delete this after debugging
    sentInterests.insert(std::make_pair(interest->getName().toUri(), ndn_getNowMilliseconds()));

/*    // TODO Only do for segment headers of key frames
    // Only do for key frame interests
    std::string name = interest->getName().getSubName(ndnrtc::prefix_filter::Stream, 1).toUri();
    if (name == "/k") {
        sentInterests.insert(std::make_pair(interest->getName().toUri(), ndn_getNowMilliseconds()));
    }*/


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

    // Todo Delete this after debugging
    uint64_t now = ndn_getNowMilliseconds();

    // Get the timestamp for when the corresponding Interest was sent
    uint64_t prodTime;
    std::string name = wireSeg->getData()->getName().toUri();

    // Todo Delete this after debugging
    LogTrace("/tmp/arcLog_consumerReceivedData.csv") << "[incomingData]\t" << name << std::endl;

    // search 01
    auto searchResult2 = receivedData.find(name);
    if (searchResult2 == receivedData.end()) {
        receivedData.insert(std::make_pair(name, ndn_getNowMilliseconds()));
    } else {
        LogError("/tmp/arcLog_map.csv") << "[mapStatus]\t" << name << "\talready received" << std::endl;
    }

    //search 02
    auto searchResult = sentInterests.find(name);
    if (searchResult != sentInterests.end()) {
        prodTime = searchResult->second;
        sentInterests.erase(searchResult);
        LogTrace("/tmp/arcLog_map.csv") << "[mapStatus]\t" << name << "\terased" << std::endl;
    } else if (searchResult2 == receivedData.end()) {
        LogError("/tmp/arcLog_map.csv") << "[mapStatus]\t" << name << "\tnot found" << std::endl;
    }

    double rtt = (now - prodTime);
    LogTrace("/tmp/arcLog.csv") << "[incomingData]\t" << name << "\t" << rtt << std::endl;
    if (rtt > 5000) {
        LogTrace("/tmp/arcLog_overtimers.csv") << "[incomingData]\t" << name << "\t" << rtt << std::endl;
    }
    // TODO Delete end

    if(wireSeg->getSegmentClass() == SegmentClass::Parity){
//        std::cout << "Parity packet received" << std::endl;
    }

    if(wireSeg->getSampleClass() == SampleClass::Delta){
//        std::cout << "Delta packet received" << std::endl;
    }

    // Only do once per frame
    if(wireSeg->isPacketHeaderSegment()){

        // TODO Enable this after debugging
        // Save time of arrival
//        uint64_t now = ndn_getNowMilliseconds();

        // New frame started
        gopCounter++;

        /*
         * Switch if the following criteria are met:
         *  - Adaption logic that switches is selected
         *  - We are close to the next key frame (gopCounter) // TODO use GopSize-1
         *  - We would actually switch to a new thread
         *  - A minimum of time has passed since last switch
         */
        if(getSelectedAdaptionLogic() != AdaptionLogic::NoAdaption &&
           gopCounter == 29 &&
//           wireSeg->getSampleClass() == SampleClass::Key &&
           threadToFetch != lastThreadToFetch &&
           now - lastThreadtoFetchChangeTime >= minimumThreadTime) {

            // TODO Delete this after Debugging
            std::cout << "[switchingThread]\t" << threadToFetch << "\t" << now - arcStartTime << std::endl;
            LogInfo("/tmp/arcLog.csv") << "[switchingThread]\t" << threadToFetch << std::endl;

            // TODO find out if this can be omitted (seems beneficial)
            pimpl->setThread(threadToFetch);

            // Actually change threadPrefix in PipelineControlStateMachine
            pimpl->getPipelineControl()->getMachine().setThreadPrefix(threadToFetch);

            // Tidy up and prepare for next round
            lastThreadToFetch = threadToFetch;
            lastThreadtoFetchChangeTime = now;
        }

        // Only do for key frames
        if(wireSeg->getSampleClass() == SampleClass::Key) {

            // TODO only use info from video segments (is this still necessary?)
//            std::cout << "SegmentsReceivedNum = " << (*sstorage_)[statistics::Indicator::SegmentsReceivedNum] << std::endl;
//            LogTrace("/tmp/arcLog.csv") << "[incomingKeyFrame]" << gopCounter << std::endl;

/*            // Get the timestamp for when the corresponding Interest was sent
            double prodTime;
            std::string name = wireSeg->getData()->getName().toUri();
            auto searchResult = sentInterests.find(name);
            if(searchResult != sentInterests.end()) {
                prodTime = searchResult->second;
                // TODO find error free way to delete items from the list after reading them (don't use key to delete)
//                sentInterests.erase(searchResult->first);
            } else {
                LogError("/tmp/arcLog.csv") << "[unknownData]\t" << "Interest not found in map." << std::endl;
                std::cout << "[unknownData]\t" << "Interest not found in map." << std::endl;
            }*/


            // Logging map status
            if (now - arcStartTime > 29000) {
                for (auto item : sentInterests) {
                    LogTrace("/tmp/arcLog_unansweredInterests.csv") << "[MapStatus]\t" << item.first << "\t" << item.second << std::endl;
                }
                std::cout << "[mapStatus]\t" << "Size = " << sentInterests.size() << std::endl;
                LogTrace("/tmp/arcLog_map.csv") << "[MapStatus]\t" << "Size = " << sentInterests.size() << std::endl;
                LogTrace("/tmp/arcLog_unansweredInterests.csv") << "[MapStatus]\t" << "========" << std::endl;
            }

            // TODO Find out why rtt keeps increasing over time (maybe cause double was wrong data type for timestamps?)
//            double now = ndn_getNowMilliseconds();
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
            gopCounter = 0;
        }
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

std::string Arc::sequentialAdaption() {
    // Skip adaption if there already is a new threadToFetch determined
    if(lastThreadToFetch != threadToFetch) {
        return threadToFetch;
    }

    // Reset counter to avoid overflow
/*    if(sequentialAdaptionThreadCounter >= videoThreads.size()) {
        sequentialAdaptionThreadCounter = 0;
    }*/
    // Set next thread to be the next one in the list of representations
//    threadToFetch = videoThreads[sequentialAdaptionThreadCounter].threadName;
    threadToFetch = videoThreads[videoThreadsOrder[sequentialAdaptionThreadCounter]].threadName;
    sequentialAdaptionThreadCounter++;
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