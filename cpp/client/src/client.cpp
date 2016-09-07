// 
// client.cpp
//
//  Created by Peter Gusev on 07 March 2016.
//  Copyright 2013-2016 Regents of the University of California
//

#include <boost/make_shared.hpp>
#include <ndn-cpp/security/key-chain.hpp>
#include <ndn-cpp/security/identity/memory-private-key-storage.hpp>
#include <ndn-cpp/security/identity/memory-identity-storage.hpp>
#include <ndn-cpp/security/policy/no-verify-policy-manager.hpp>

#include "client.h"
#include "config.h"
#include "renderer.h"

using namespace std;
using namespace ndn;

//******************************************************************************
void Client::run(unsigned int runTimeSec, 
	unsigned int statSamplePeriodMs, const ClientParams& params)
{
	runTimeSec_ = runTimeSec;
	statSampleIntervalMs_ = statSamplePeriodMs;
	params_ = params;

	setupConsumer();
	setupProducer();
	setupStatGathering();

	runProcessLoop();

	tearDownStatGathering();
	tearDownProducer();
	tearDownConsumer();
}

//******************************************************************************

void Client::setupConsumer()
{
	if (!params_.isConsuming())
		return;

	ConsumerClientParams ccp = params_.getConsumerParams();

	for (auto p:ccp.fetchedStreams_)
	{
		ndnrtc::GeneralConsumerParams gp = (p.type_ == ClientMediaStreamParams::MediaStreamType::MediaStreamTypeAudio ? ccp.generalAudioParams_ : ccp.generalVideoParams_);
		RemoteStream rs = initRemoteStream(p, gp);
#warning check move semantics
		remoteStreams_.push_back(boost::move(rs));

		LogInfo("") << "Set up fetching from " << p.sessionPrefix_ << ":" 
			<< p.streamName_ << endl;
	}

	LogInfo("") << "Fetching " << remoteStreams_.size() << " remote stream(s) total" << endl;
}

void Client::setupProducer()
{
	if (!params_.isProducing())
		return;

	ProducerClientParams pcp = params_.getProducerParams();

	for (auto p:pcp.publishedStreams_)
	{
		try
		{
			LocalStream ls = initLocalStream(p);
			localStreams_.push_back(boost::move(ls));

			LogInfo("") << "Set up publishing stream " << p.streamName_ << endl;
		}
		catch (const runtime_error& e)
		{
			LogError("") << "error while trying to publish stream " << p.streamName_ << ": "
				<< e.what() << endl;
		}
	}

	LogInfo("") << "Publishing " << localStreams_.size() << " streams total" << endl;
}

void Client::setupStatGathering()
{
	if (!params_.isGatheringStats())
		return;

	LogInfo("") << "new stat colllector" << endl;
	statCollector_.reset(new StatCollector(io_));
	
	for (auto& rs:remoteStreams_)
		statCollector_->addStream(rs.getStream());

	statCollector_->startCollecting(statSampleIntervalMs_, 
		params_.getGeneralParameters().logPath_, 
		params_.getConsumerParams().statGatheringParams_);

	LogInfo("") << "Gathering statistics into " 
		<< statCollector_->getWritersNumber() << " files" << std::endl;
}

void Client::runProcessLoop()
{
    boost::asio::deadline_timer runTimer(io_);
    runTimer.expires_from_now(boost::posix_time::seconds(runTimeSec_));
    runTimer.wait();
}

void Client::tearDownStatGathering(){
	if (!params_.isGatheringStats())
		return;

	statCollector_->stop();
	statCollector_.reset();
	LogInfo("") << "Stopped gathering statistics" << std::endl;
}

void Client::tearDownProducer(){
	if (!params_.isProducing())
		return;

	LogInfo("Tearing down producing...");

	for (auto& ls:localStreams_)
	{
		if (!ls.getVideoSource().get())
			boost::dynamic_pointer_cast<ndnrtc::LocalAudioStream>(ls.getStream())->stop();
		else
			ls.stopSource();

		LogInfo("") << "...stopped publishing " << ls.getStream()->getPrefix() << std::endl;
	}
	localStreams_.clear();
}

void Client::tearDownConsumer(){
	if (!params_.isConsuming())
		return ;

	LogInfo("") << "Tearing down consuming..." << std::endl;

	for (auto& rs:remoteStreams_)
	{
		boost::dynamic_pointer_cast<ndnrtc::RemoteStream>(rs.getStream())->stop();
		LogInfo("") << "...stopped fetching from " << rs.getStream()->getPrefix() << std::endl;
	}
	remoteStreams_.clear();
}

RemoteStream Client::initRemoteStream(const ConsumerStreamParams& p,
	const ndnrtc::GeneralConsumerParams& gcp)
{
	RendererInternal *renderer = (p.type_ == ConsumerStreamParams::MediaStreamTypeVideo ? new RendererInternal(p.streamSink_, true) : nullptr);
	
	if (p.type_ == ConsumerStreamParams::MediaStreamTypeVideo)
	{
		boost::shared_ptr<ndnrtc::RemoteVideoStream> 
			remoteStream(boost::make_shared<ndnrtc::RemoteVideoStream>(io_, face_, keyChain_,
				p.sessionPrefix_, p.streamName_));
		remoteStream->start(p.threadToFetch_, renderer);
		return RemoteStream(remoteStream, boost::shared_ptr<RendererInternal>(renderer));
	}
	else
	{
		boost::shared_ptr<ndnrtc::RemoteAudioStream>
			remoteStream(boost::make_shared<ndnrtc::RemoteAudioStream>(io_, face_, keyChain_,
				p.sessionPrefix_, p.streamName_));
		remoteStream->start(p.threadToFetch_);
		return RemoteStream(remoteStream, boost::shared_ptr<RendererInternal>(renderer));
	}
}

LocalStream Client::initLocalStream(const ProducerStreamParams& p)
{
	boost::shared_ptr<ndnrtc::IStream> localStream;
	boost::shared_ptr<VideoSource> videoSource;
	ndnrtc::MediaStreamSettings settings(io_, p);

	settings.keyChain_ = keyChain_.get();
	settings.face_ = face_.get();

	if (p.type_ == ndnrtc::MediaStreamParams::MediaStreamTypeVideo)
	{
		LogDebug("") << "initializing video source at " << p.source_ << endl;

		boost::shared_ptr<RawFrame> sampleFrame = sampleFrameForStream(p);
		
		LogDebug("") << "source should support frames of size " 
			<< sampleFrame->getWidth() << "x" << sampleFrame->getHeight() << endl;
		videoSource.reset(new VideoSource(io_, p.source_, sampleFrame));
		LogDebug("") << "video source initialized" << endl;

		boost::shared_ptr<ndnrtc::LocalVideoStream> s =
			boost::make_shared<ndnrtc::LocalVideoStream>(p.sessionPrefix_, settings);
		videoSource->addCapturer(s.get());
		localStream = s;

#warning double-check whether FPS should be 30
		LogDebug("") << "starting video source..." << endl;
		videoSource->start(30);
		LogDebug("") << "...video source started" << endl;
	}
	else
	{
		boost::shared_ptr<ndnrtc::LocalAudioStream> s = 
			boost::make_shared<ndnrtc::LocalAudioStream>(p.sessionPrefix_, settings);
		s->start();
		localStream = s;
	}

	return LocalStream(localStream, videoSource);
}

boost::shared_ptr<RawFrame> Client::sampleFrameForStream(const ProducerStreamParams& p)
{
	unsigned int width = 0, height = 0;
	p.getMaxResolution(width, height);

	if (width == 0 || height == 0)
	{
		stringstream ss;
		ss << "incorrect max resolution for stream " << p.streamName_;
		throw runtime_error(ss.str());
	}

	return boost::shared_ptr<RawFrame>(new ArgbFrame(width, height));
}
