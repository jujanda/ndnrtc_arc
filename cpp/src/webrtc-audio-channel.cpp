//
//  webrtc-audio-channel.cpp
//  ndnrtc
//
//  Created by Peter Gusev on 4/14/14.
//  Copyright (c) 2014 Peter Gusev. All rights reserved.
//

#include <boost/thread/mutex.hpp>

#include "webrtc-audio-channel.h"
#include "ndnrtc-utils.h"

using namespace std;
using namespace webrtc;
using namespace ndnrtc;
using namespace ndnrtc::new_api;
using namespace ndnlog;
using namespace ndnlog::new_api;

//******************************************************************************
#pragma mark - construction/destruction
WebrtcAudioChannel::WebrtcAudioChannel():
voeBase_(nullptr),
voeNetwork_(nullptr)
{
}

WebrtcAudioChannel::~WebrtcAudioChannel()
{
    if (voeBase_)
        voeBase_->Release();
    
    if (voeNetwork_)
        voeNetwork_->Release();
}

//******************************************************************************
#pragma mark - public
int WebrtcAudioChannel::init()
{
    int res = RESULT_OK;
    boost::mutex m;
    boost::unique_lock<boost::mutex> lock(m);
    boost::condition_variable initialized;
    
    NdnRtcUtils::dispatchOnVoiceThread(
                                       [this](){
                                           voeBase_ = VoEBase::GetInterface(NdnRtcUtils::sharedVoiceEngine());
                                           voeNetwork_ = VoENetwork::GetInterface(NdnRtcUtils::sharedVoiceEngine());
                                           
                                           webrtcChannelId_ = voeBase_->CreateChannel();
                                       },
                                       [this, &res, &initialized](){
                                           if (webrtcChannelId_ < 0)
                                               res = RESULT_ERR;
                                           initialized.notify_one();
                                       });
    initialized.wait(lock);
    return res;
}
