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

Arc::Arc(int adaptionLogic) :
        selectedAdaptionLogic(adaptionLogic)
{
    // TODO write about observing in description
    // TODO pass video thread data through config file params instead
    // TODO Check if info is delivered in meta data.
    videoThread rep1, rep2, rep3;
    rep1.threadName = "representation01";
    rep1.max_bitrate = "1000";
    rep2.threadName = "representation02";
    rep2.max_bitrate = "1500";
    rep3.threadName = "representation03";
    rep3.max_bitrate = "2000";
    videoThreads.emplace_back(rep1);
    videoThreads.emplace_back(rep2);
    videoThreads.emplace_back(rep3);

    // TODO set threadToFetch dynamically
    threadToFetch = videoThreads.begin()->threadName;
}

std::string Arc::calculateThreadToFetch() {
    switch (getSelectedAdaptionLogic()) {
        case AL_NO_ADAPTION: threadToFetch = noAdaption(); break;
        case AL_DASH_JS: threadToFetch = dashJS(); break;
        case AL_THANG: threadToFetch = thang(); break;
        default: threadToFetch = noAdaption();
    }
//    std::cout << "New threadToFetch = " << threadToFetch << std::endl;

    return threadToFetch;
}

int Arc::getSelectedAdaptionLogic() {
    return selectedAdaptionLogic;
}

void Arc::segmentArrived(const boost::shared_ptr<WireSegment> & wireSeg) {

    if(wireSeg->isPacketHeaderSegment() && wireSeg->getSampleClass() == SampleClass::Key) {
        // TODO only use info from video segments
        double prodTime;
        std::string name = wireSeg->getData()->getName().toUri();
        auto search = sentInterests.find(name);
        if(search != sentInterests.end()) {
            prodTime = search->second;
        } else {
            std::cout << "Interest not found in map." << std::endl;
        }

        double now = ndn_getNowMilliseconds();

        double rtt = (now - prodTime) / 1000; // Divide by 1000 to convert ms --> s
        long size = wireSeg->getData()->getContent().size() * 8;
        long size2 = wireSeg->getData()->getDefaultWireEncoding().size() * 8; // TODO is this better for size?
        dashJS_lastSegmentMeasuredThroughput = size / rtt;

//        std::cout << "pT = " << prodTime;
//        std::cout << ", pT2 = " << prodTime2;
//        std::cout << ", pT3 = " << prodTime3;
//        std::cout << ", cT = " << now;
//        std::cout << ", cT2 = " << now2;
//        std::cout << ", size = " << size << " bit";
//        std::cout << ", size = " << size2 << " bit";
//        std::cout << ", rtt = " << rtt;
//        std::cout << ", throughput = " << dashJS_lastSegmentMeasuredThroughput;
//        std::cout << std::endl;
    }
}

/*void segmentRequestTimeout(const NamespaceInfo&) {
    std::cout << "segmentRequestTimeout" << std::endl;

}

void segmentNack(const NamespaceInfo&, int reason) {
    std::cout << "segmentNack" << std::endl;

}

void segmentStarvation() {
    std::cout << "segmentStarvation" << std::endl;

};*/

void Arc::addSentInterest(std::string name) {
    sentInterests.insert(std::make_pair(name, ndn_getNowMilliseconds()));
}

void Arc::write()
{
    // TODO Implement additional info to be written
    // TODO Integrate messages into logging
    std::cout << "Offered Adaption Logics: " << std::endl;
    std::cout << "NO_ADAPTION = " << AL_NO_ADAPTION << std::endl;
    std::cout << "DASH_JS = " << AL_DASH_JS << std::endl;
    std::cout << "THANG = " << AL_THANG << std::endl;

    std::cout << "Chosen Adaption Logic =  " << getSelectedAdaptionLogic() << std::endl;

    std::cout << "Registered representations:" << std::endl;
    if (!videoThreads.empty()) {
        for(std::vector<std::string>::size_type i = 0; i != videoThreads.size(); i++) {
            std::cout << "[" << i << "]: " << videoThreads[i].threadName << std::endl;
        }
    } else {
        std::cout << "No representations found." << std::endl;
    }

    std::cout << "Current thread to fetch: " << threadToFetch << std::endl;

}

std::string Arc::noAdaption() {
    // TODO delete randomizer (only used for testing)
    int min = 0;
    int max = videoThreads.size() -1;
    int randNum = min + (rand() % static_cast<int>(max - min + 1));
    threadToFetch = videoThreads[randNum].threadName;
//    std::cout << "No adaption performed." << std::endl;
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

//    std::cout << "nextbn = " << nextBn; // TODO delete (only used for testing)
//    std::cout << "   bn = " << bn; // TODO delete (only used for testing)
//    std::cout << "   bm = " << bm; TODO delete (only used for testing)
//    std::cout << std::endl; // TODO delete (only used for testing)

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