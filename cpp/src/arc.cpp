//
// Created by Julian Janda on 07.05.18.
//

#include <iostream>
#include <boost/asio/io_service.hpp>
#include <local-stream.hpp>
#include <arc.hpp>

using namespace ndnrtc;

Arc::Arc(int adaptionLogic) :
        selectedAdaptionLogic(adaptionLogic)
{
    std::string tmp = 0;
    switch (getSelectedAdaptionLogic()) {
        case AL_NO_ADAPTION: tmp = noAdaption(); break;
        case AL_DASH_JS: tmp = dashJS(); break;
        case AL_THANG: tmp = thang(); break;
        default: tmp = noAdaption();
    }

    //Todo Find proper way to pass chosen representation
    std::cout << "Chosen Representation = " << tmp << std::endl;

}

std::string Arc::noAdaption() {
    // TODO Implement
    std::cout << "No adaption performed." << std::endl;
    return "";
}

std::string Arc::dashJS() {
    // TODO Implement from pseudocode
    std::cout << "DASH-JS adaption performed." << std::endl;
/*    1 Prerequisites:
    2 Last segment downloaded bitrate as bm
    3 MPD bitrate as bn if bn is not defined
    4
    5 Estimated bitrate calculation:
    6 var num1= bn * 0.7;
    7 var num2 = bm * 1.3;
    8 var den= w1 + w2;
    9 var bn +1= Math.floor (( num1 + num2 ) / den ) ;
    10
    11
    12 Rate selection:
    13 for ( var selRep = maxRepresentation; selRep >= 0; selRep-- )
    {
        14 var representationBitrate= getRepresentationBitrate (
                Period , selRep ) ;
        15 if( representationBitrate <= bn +1) {
            16 return representationBitrate;
            17 }
        18 }
    19 return 0;*/
    return "";
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
    return "";
}

int Arc::getSelectedAdaptionLogic() {
    return selectedAdaptionLogic;
}

void Arc::write()
{
    std::cout << "Offered Adaption Logics: " << std::endl;
    std::cout << "NO_ADAPTION = " << AL_NO_ADAPTION << std::endl;
    std::cout << "DASH_JS = " << AL_DASH_JS << std::endl;
    std::cout << "THANG = " << AL_THANG << std::endl;

    std::cout << "Chosen Adaption Logic =  " << getSelectedAdaptionLogic() << std::endl;

    std::cout << "Registered representations:" << getSelectedAdaptionLogic() << std::endl;
    for(std::vector<std::string>::size_type i = 0; i != threadNames.size(); i++) {
        std::cout << "[" << i << "]: " << threadNames[i] << std::endl;
    }

}