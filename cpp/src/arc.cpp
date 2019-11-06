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
#include <limits>

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
    rep1.max_bitrate = "10000";
    rep2.threadName = "med";
    rep2.max_bitrate = "15000";
    rep3.threadName = "high";
    rep3.max_bitrate = "20000";
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
    LogInfo("/tmp/arcLog_threadswitches.csv") << "[StartTime]\t" << arcStartTime << std::endl;
    LogInfo("/tmp/arcLog_networkMeasurements.csv") << "[StartTime]\t" << arcStartTime << std::endl;
    LogInfo("/tmp/arcLog_networkSummedMeasurements.csv") << "[StartTime]\t" << arcStartTime << std::endl;
    LogInfo("/tmp/arcLog_networkMeasurementValues.csv") << "[StartTime]\t" << arcStartTime << std::endl;
    LogInfo("/tmp/arcLog_retransmissions.csv") << "[StartTime]\t" << arcStartTime << std::endl;
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
        case AdaptionLogic::ReTrans: threadToFetch = reTrans(); break;
        case AdaptionLogic::Thang: threadToFetch = thang(); break;
    }
}


AdaptionLogic Arc::getSelectedAdaptionLogic() {
    return selectedAdaptionLogic;
}

void Arc::onInterestIssued(const boost::shared_ptr<const ndn::Interest> & interest) {

    // Log arrival
    LogTrace("/tmp/arcLog.csv") << "[outgoingInterest]\t" << interest->getName().toUri() << std::endl;
    LogTrace("/tmp/arcLog_consumerSentInterests.csv") << "[ConsOutInterest]\t" << interest->getName().toUri() << std::endl;

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

    // Get cutrrent timestamp
    uint64_t now = ndn_getNowMilliseconds();

    // Prepare variables
    uint64_t prodTime;
    std::string name = wireSeg->getData()->getName().toUri();
    double generationDelay = wireSeg->header().generationDelayMs_;

    // Todo Delete this after debugging (?)
    LogTrace("/tmp/arcLog_consumerReceivedData.csv") << "[ConsInData]\t" << name << std::endl;

    // Search 01 - Check if data segment was already received; Save arrival timestamp in list if not.
    auto searchResult2 = receivedData.find(name);
    if (searchResult2 == receivedData.end()) {
        receivedData.insert(std::make_pair(name, ndn_getNowMilliseconds()));
    } else {
        LogError("/tmp/arcLog_map.csv") << "[mapStatus]\t" << name << "\talready received" << std::endl;
    }

    //Search 02 - Search corresponding Interest, get its sending timestamp and erase it from list if found
    auto searchResult = sentInterests.find(name);
    if (searchResult != sentInterests.end()) {
        prodTime = searchResult->second;
        sentInterests.erase(searchResult);
        LogTrace("/tmp/arcLog_map.csv") << "[mapStatus]\t" << name << "\terased" << std::endl;
    } else if (searchResult2 == receivedData.end()) {
        LogError("/tmp/arcLog_map.csv") << "[mapStatus]\t" << name << "\tnot found" << std::endl;
    }

    // Calculate RTT 
    // double rtt = std::max(1.0, (now - (prodTime + generationDelay)));
    double rtt = now - (prodTime + generationDelay);

    // Save Size of segment
    long size = wireSeg->getData()->getDefaultWireEncoding().size() * 8;  // convert from Byte to bit

    LogTrace("/tmp/arcLog.csv") << "[incomingData]\t" << name << "\t" << rtt << std::endl;
    if (rtt > 5000) {
        LogTrace("/tmp/arcLog_overtimers.csv") << "[incomingData]\t" << name << "\t" << rtt << std::endl;
    }
    // TODO Delete end (?)

    if(wireSeg->getSegmentClass() == SegmentClass::Parity){
//        std::cout << "Parity packet received" << std::endl;
    }

    if(wireSeg->getSampleClass() == SampleClass::Delta){
//        std::cout << "Delta packet received" << std::endl;
    }

    // Only do once per frame
    if(wireSeg->isPacketHeaderSegment()){

        // TODO Delete this after debugging
        // Save time of arrival
//        uint64_t now = ndn_getNowMilliseconds();

        // New frame started
        gopCounter++;

        /*
         * Switch if the following criteria are met:
         *  - Adaption logic that switches is selected
         *  - We are close to the next key frame (gopCounter) // TODO use GopSize-1
         *  - We would actually switch to a new thread
         *  - We are at the end of a keyframe sequence
         *  - A minimum of time has passed since last switch // TODO delete this (?)
         */
        if(getSelectedAdaptionLogic() != AdaptionLogic::NoAdaption
            && gopCounter == 23 
            // && wireSeg->getSampleClass() == SampleClass::Key 
            && threadToFetch != lastThreadToFetch 
            && keyFrameCounter == keyFrameSequenceLength 
            // && now - lastThreadtoFetchChangeTime >= minimumThreadTime
            ) {

            // TODO Delete this after Debugging
//            std::cout << "[values]" << "\t"
//                      << gopCounter << "\t"
//                      << threadToFetch << "\t"
//                      << lastThreadToFetch << "\t"
//                      << now - lastThreadtoFetchChangeTime << "\t"
//                      << minimumThreadTime << "\t"
//                      << std::endl;
            std::cout << "[switchingThread]\t" << threadToFetch << "\t" << now - arcStartTime << std::endl;
            LogInfo("/tmp/arcLog.csv") << "[switchingThread]\t" << threadToFetch << std::endl;
            LogInfo("/tmp/arcLog_threadswitches.csv") << "[switchingThread]\t" << threadToFetch << std::endl;

            // LogInfo("/tmp/arcLog_networkMeasurements.csv") << "[switchingThread]\t" << threadToFetch << std::endl;

            // Actually change threadPrefix in PipelineControlStateMachine
            pimpl->getPipelineControl()->getMachine().setThreadPrefix(threadToFetch);

            // Inform remote stream implementation of new threadName
            pimpl->setThread(threadToFetch);
            
            // Tidy up and prepare for next round
            lastThreadToFetch = threadToFetch;
            lastThreadtoFetchChangeTime = now;
        }

        // Only do once per key frame
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


/*            // Logging map status
            if (now - arcStartTime > 29000) {
                for (auto item : sentInterests) {
                    LogTrace("/tmp/arcLog_unansweredInterests.csv") << "[MapStatus]\t" << item.first << "\t" << item.second << std::endl;
                }
                std::cout << "[mapStatus]\t" << "Size = " << sentInterests.size() << std::endl;
                LogTrace("/tmp/arcLog_map.csv") << "[MapStatus]\t" << "Size = " << sentInterests.size() << std::endl;
                LogTrace("/tmp/arcLog_unansweredInterests.csv") << "[MapStatus]\t" << "========" << std::endl;
            }*/

            // TODO Find out why rtt keeps increasing over time (maybe cause double was wrong data type for timestamps?)
//            double now = ndn_getNowMilliseconds();
            // long size = wireSeg->getData()->getContent().size() * 8; // convert from Byte to bit
            // long size = wireSeg->getData()->getDefaultWireEncoding().size() * 8;  // convert from Byte to bit
            

            // Calculate the time it took for full key frame to arrive
            double fullKeyframeTime = now - keyframeSendingtime;

            // Calculate throuphut based on total keyframe sending time
            // reTrans_lastSegmentMeasuredThroughput = sizeSum / fullKeyframeTime; // kbit/s

            // TODO [Experimental]
            reTrans_lastSegmentMeasuredThroughput = retransmissions;

            // WORKAROUND to dismiss the negative RTT values that pop up occasionally
            // (It shouldn't be needed anymore, but oddly enough, it prevents values from becoming "-nan", so it stays for now)
            if (rtt > 0 && timeSum > 0) {
                old_reTrans_lastSegmentMeasuredThroughput = sizeSum / timeSum; // kbit/s
            }

            // Log network info to file
            LogInfo("/tmp/arcLog_networkMeasurements.csv") << "[measured]\t"
            // << "satT-sentT = " << fullKeyframeTime << " ms"
            // << ", sentT = " << prodTime
            // << ", cT = " << now
            // << ", sizeSum = " << sizeSum << " Bit"
            // << ", timeSum = " << timeSum << " ms"
            << "retransmissions = " << retransmissions
            << ", oldThroughput = " << old_reTrans_lastSegmentMeasuredThroughput << " kBit/s"
            << ", newThroughput = " << reTrans_lastSegmentMeasuredThroughput << " kBit/s"
            << std::endl;

            // Check for end of keyframe sequence
            if (keyFrameCounter >= keyFrameSequenceLength) { 
                reTrans_lastSegmentMeasuredThroughput = retransmissions;
                calculateThreadToFetch();
                retransmissions = 0;
                keyFrameCounter = 0;
            }

            // Reset values
            timeSum = 0;
            sizeSum = 0;
            gopCounter = 0;

            // Increment counters
            keyFrameCounter++ ;
            
            // WORKAROUND to dismiss invalid sending times
            if (now * 0.9 < prodTime && prodTime < now * 1.1) {
                // Remember sending time of first keyframe segment
                keyframeSendingtime = prodTime;  
            } else {
                // WORKAROUND Remember arriving time of first keyframe segment (next best thing available)
                keyframeSendingtime = now;
/*                LogInfo("/tmp/arcLog_networkMeasurements.csv") << "[warning]\t"
                << "the previous line and the next line contain wrong throuphut values, due to [" 
                << prodTime << "]"
                << std::endl;*/
            }

/*            LogInfo("/tmp/arcLog_threadswitches.csv") 
                << "\tkeyFramecounter = " << keyFrameCounter 
                << std::endl;*/

        }
    }

    // segment type visualisation
    segmentTypeVisualisation_file.open("/tmp/arc_segmentTypeVisualisation.txt", std::ios_base::app);

    if(wireSeg->getSampleClass() == SampleClass::Key) {
        if(wireSeg->isPacketHeaderSegment()){
            segmentTypeVisualisation_file << "\nK";
        } else {
            segmentTypeVisualisation_file << "k";
        }
    }

    if(wireSeg->getSampleClass() == SampleClass::Delta) {
        if(wireSeg->isPacketHeaderSegment()){
            segmentTypeVisualisation_file << "\nD";
        } else {
            segmentTypeVisualisation_file << "d";
        }
    }
    segmentTypeVisualisation_file.close();

    // WORKAROUND to dismiss the wrong RTT values that pop up occasionally
    if (10000 > rtt && rtt > 0) {
        // Sum up values for smoothed calculations
        timeSum += rtt;
        sizeSum += size;
        
/*        LogInfo("/tmp/arcLog_networkMeasurementValues.csv") << "[measured]\t"
            << " rtt = " << rtt
            << ", size = " << size
            << ", timeSum = " << timeSum
            << ", sizeSum = " << sizeSum
            << std::endl; */   
    }
}

void Arc::segmentRequestTimeout(const NamespaceInfo&) {
    //Do something
    LogInfo("tmp/arcLog_unansweredInterests.csv") << "[warning]\t"
    << "Request timeout happened!" << std::endl;
}

void Arc::segmentNack(const NamespaceInfo&, int) {
    //Do something
    LogInfo("tmp/arcLog_unansweredInterests.csv") << "[warning]\t"
    << "Nack happened!" << std::endl;
}

void Arc::segmentStarvation() {
    //Do something
    LogInfo("tmp/arcLog_unansweredInterests.csv") << "[warning]\t"
    << "Starvation happened!" << std::endl;
}

void Arc::onRetransmissionRequired(const std::vector<boost::shared_ptr<const ndn::Interest>>& interest) {

    for(int i = 0; i < interest.size(); i++) {
        LogTrace("/tmp/arcLog_retransmissions.csv") << interest[i]->getName().toUri() << std::endl;
    } 

    retransmissions += interest.size();
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

    // Prevent index getting out of bounds
    if (sequentialAdaptionThreadCounter >= sizeof(videoThreadsOrder) / sizeof(videoThreadsOrder[0])) {
        sequentialAdaptionThreadCounter = 0;
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

std::string Arc::reTrans() {
    double bn = reTrans_lastSegmentCalculatedThroughput;
    double bm = reTrans_lastSegmentMeasuredThroughput;
    int representationBitrate = 0; // bit rate of currently selected representation

    // Estimated bitrate calculation:
    double num1 = bn * reTrans_w1;
    double num2 = bm * reTrans_w2;
    double den = reTrans_w1 + reTrans_w2;
    double tmp = (num1 + num2) / den;
    double nextBn = std::floor(tmp);
    // double nextBn = std::floor((num1 + num2) / den); // nextSegmentCalculatedThroughput
    reTrans_lastSegmentCalculatedThroughput = nextBn;

    // TODO delete (only used for testing)
    LogInfo("/tmp/arcLog_threadswitches.csv")
            << "bn = " << bn
            << " bm = " << bm
            // << "\tnum1 = " << num1
            // << "\tnum2 = " << num2
            // << "\tden = " << den
            // << "\ttmp = " << tmp
            << " nextBn = " << nextBn
            // << "\tretransmissions = " << retransmissions
            << std::endl;

    // Rate selection
/*    for (auto selRep = videoThreads.rbegin(); selRep != videoThreads.rend(); ++selRep) {
        representationBitrate = std::stoi(selRep->max_bitrate);
        if (representationBitrate <= nextBn) {
            return selRep->threadName;
        }
    }*/

/*    // Rate selection (alternative DRAFT)
    if (lastThreadToFetch == videoThreads[0].threadName) { // current: low
        if (nextBn > 2500) {
            return videoThreads[1].threadName; // med
        } else {
            return lastThreadToFetch; // low
        }

    } else if (lastThreadToFetch == videoThreads[1].threadName) { // current: med
        if (nextBn < 1000) {
            return videoThreads[0].threadName; // low
        } else if (nextBn > 1500) {
            return videoThreads[2].threadName; // high
        } else {
            return lastThreadToFetch; // med
        }

    }else if (lastThreadToFetch == videoThreads[2].threadName) { // current: high
        if (nextBn < 1000) {
            return videoThreads[1].threadName; // med
        } else {
            return lastThreadToFetch; // high
        }
    } */

/*        // Rate selection (another alternative DRAFT)
    if (lastThreadToFetch == videoThreads[0].threadName) { // current: low
        if (nextBn < 20) {
            return videoThreads[1].threadName; // med
        } else {
            return lastThreadToFetch; // low
        }

    } else if (lastThreadToFetch == videoThreads[1].threadName) { // current: med
        if (nextBn > 40) {
            return videoThreads[0].threadName; // low
        } else if (nextBn < 20) {
            return videoThreads[2].threadName; // high
        } else {
            return lastThreadToFetch; // med
        }

    } else if (lastThreadToFetch == videoThreads[2].threadName) { // current: high
        if (nextBn > 4) {
            return videoThreads[1].threadName; // med
        } else {
            return lastThreadToFetch; // high
        }
    } */


    // Rate selection (yet another DRAFT)
    if (nextBn < lowerThreshold) {
        // switch up
        if (lastThreadToFetch == videoThreads[0].threadName) { return videoThreads[1].threadName; } // low --> med
        else if (lastThreadToFetch == videoThreads[1].threadName) { return videoThreads[2].threadName; } // med --> high

    } else if (nextBn > upperThreshold) {
        //switch down
        if (lastThreadToFetch == videoThreads[2].threadName) { return videoThreads[1].threadName; } // high --> med
        else if (lastThreadToFetch == videoThreads[1].threadName) { return videoThreads[0].threadName; } // med --> low
    } // else: stay the same and dont switch

    

//    std::cout << "reTrans couldn't find a threadToFetch!" << std::endl;
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