//
//  pipeliner.h
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//

#ifndef __ndnrtc__pipeliner__
#define __ndnrtc__pipeliner__

#include "ndnrtc-common.h"
#include "frame-buffer.h"
#include "ndnrtc-object.h"
#include "consumer.h"

namespace ndnrtc {
    namespace new_api {
        
        class Pipeliner : public NdnRtcObject
        {
        public:
            typedef enum _State {
                StateInactive = -1,
                StateChasing = 0,
                StateBuffering = 1,
                StateFetching = 2
            } State;
            
            // average number of segments for delta and key frames
            static const double SegmentsAvgNumDelta;
            static const double SegmentsAvgNumKey;
            static const double ParitySegmentsAvgNumDelta;
            static const double ParitySegmentsAvgNumKey;
            static const int64_t MaxInterruptionDelay;
            static const int64_t MinInterestLifetime;
            static const int MaxRetryNum; // maximum number of retires when
                                          // streaming suddenly inerrupts
            static const int ChaserRateCoefficient; // how faster than producer
                                                    // rate the chaser should
                                                    // skim through cached content
            static const int FullBufferRecycle; // number of slots to recycle if
                                                // the buffer became full during
                                                // chasing stage
            
            Pipeliner(const boost::shared_ptr<Consumer>& consumer);
            ~Pipeliner();
            
            int
            start();
            
            int
            stop();
            
            void
            triggerRebuffering();
            
//            double
//            getAvgSegNum(bool isKey) const DEPRECATED
//            {
//                return NdnRtcUtils::currentMeanEstimation((isKey)?
//                                                          keySegnumEstimatorId_:
//                                                          deltaSegnumEstimatorId_);
//            }
//            
//            double
//            getRtxFreq() const DEPRECATED
//            { return NdnRtcUtils::currentFrequencyMeterValue(rtxFreqMeterId_); }
//            
//            unsigned int
//            getRtxNum() const DEPRECATED
//            { return rtxNum_; }
//            
//            unsigned int
//            getRebufferingNum() DEPRECATED
//            { return rebufferingNum_; }
            
            State
            getState() const
            { return state_; }
            
            void
            setUseKeyNamespace(bool useKeyNamespace)
            { useKeyNamespace_ = useKeyNamespace; }
            
            void
            registerCallback(IPipelinerCallback* callback)
            { callback_ = callback; }
            
            void
            switchToStream(unsigned int streamId);
            
//            unsigned int
//            getInterestNum() { return nInterestSent_; }
            PipelinerStatistics
            getStatistics();
            
        private:
            // this events masks are used in different moments during pipeliner
            // and used for filtering buffer events
            // startup events mask is used for pipeline initiation
            static const int StartupEventsMask = FrameBuffer::Event::StateChanged;
            // this events mask is used when rightmost interest has been issued
            // and initial data is awaited
            static const int WaitForRightmostEventsMask = FrameBuffer::Event::FirstSegment | FrameBuffer::Event::Timeout;
            // this events mask is used during chasing stage
            static const int ChasingEventsMask = FrameBuffer::Event::FirstSegment |
                                                 FrameBuffer::Event::Timeout |
                                                 FrameBuffer::Event::StateChanged;
            // this events mask is used during buffering stage
            static const int BufferingEventsMask = FrameBuffer::Event::FirstSegment |
                                                FrameBuffer::Event::Ready |
                                                FrameBuffer::Event::Timeout |
                                                FrameBuffer::Event::StateChanged |
                                                FrameBuffer::Event::NeedKey;
            // this events mask is used during fetching stage
            static const int FetchingEventsMask = BufferingEventsMask |
                                                FrameBuffer::Event::Playout;
            
            
            State state_;
            Name streamPrefix_, deltaFramesPrefix_, keyFramesPrefix_;
            
            Consumer* consumer_;
            ParamsStruct params_;
            ndnrtc::new_api::FrameBuffer* frameBuffer_;
            ChaseEstimation* chaseEstimation_;
            BufferEstimator* bufferEstimator_;
            IPacketAssembler* ndnAssembler_;
            IPipelinerCallback* callback_;
            
            webrtc::ThreadWrapper &mainThread_;
            
            bool isPipelinePaused_, isPipelining_;
            int64_t pipelineIntervalMs_, recoveryCheckpointTimestamp_;
            webrtc::ThreadWrapper &pipelineThread_;
            webrtc::EventWrapper &pipelineTimer_;
            webrtc::EventWrapper &pipelinerPauseEvent_;
            webrtc::CriticalSectionWrapper &streamSwitchSync_;
            
            int deltaSegnumEstimatorId_, keySegnumEstimatorId_;
            int deltaParitySegnumEstimatorId_, keyParitySegnumEstimatorId_;
            
            PacketNumber keyFrameSeqNo_, deltaFrameSeqNo_;
            PacketNumber playbackStartFrameNo_;
            
            // --
//            unsigned rebufferingNum_,
            unsigned int reconnectNum_;
            PacketNumber exclusionFilter_;
            unsigned int rtxFreqMeterId_;//, rtxNum_;
            int bufferEventsMask_;
            bool useKeyNamespace_;
            unsigned int streamId_; // currently fetched stream id
            // statistics
            PipelinerStatistics stat_;
//            unsigned int nInterestSent_;
            
            static bool
            mainThreadRoutine(void *pipeliner){
                return ((Pipeliner*)pipeliner)->processEvents();
            }
            bool
            processEvents();
            
            static bool
            pipelineThreadRoutin(void *pipeliner){
                return ((Pipeliner*)pipeliner)->processPipeline();
            }
            bool
            processPipeline();
            
            int
            handleInvalidState(const ndnrtc::new_api::FrameBuffer::Event &event);
            
            int
            handleChase(const FrameBuffer::Event& event);
            
            void
            initialDataArrived(const boost::shared_ptr<FrameBuffer::Slot>& slot);
            
            void
            handleChasing(const FrameBuffer::Event& event);
            
            void
            handleBuffering(const FrameBuffer::Event& event);
            
            int
            handleValidState(const FrameBuffer::Event& event);
            
            void
            handleTimeout(const FrameBuffer::Event& event);
            
            int
            initialize();
            
            boost::shared_ptr<Interest>
            getDefaultInterest(const Name& prefix, int64_t timeoutMs = 0);
            
            boost::shared_ptr<Interest>
            getInterestForRightMost(int64_t timeoutMs, bool isKeyNamespace = false,
                                    PacketNumber exclude = -1);
            
            void
            updateSegnumEstimation(FrameBuffer::Slot::Namespace frameNs,
                                   int nSegments, bool isParity);
            
            void
            requestNextKey(PacketNumber& keyFrameNo);
            
            void
            requestNextDelta(PacketNumber& deltaFrameNo);
            
            void
            expressRange(Interest& interest, SegmentNumber startNo,
                         SegmentNumber endNo, int64_t priority, bool isParity);
            
            void
            express(Interest& interest, int64_t priority);
            
            void
            startChasePipeliner(PacketNumber startPacketNo,
                                 int64_t intervalMs);
            
            void
            setChasePipelinerPaused(bool isPaused);
            
            void
            stopChasePipeliner();
            
            void
            requestMissing(const boost::shared_ptr<FrameBuffer::Slot>& slot,
                           int64_t lifetime, int64_t priority,
                           bool wasTimedOut = false);
            
            int64_t
            getInterestLifetime(int64_t playbackDeadline,
                                FrameBuffer::Slot::Namespace nspc = FrameBuffer::Slot::Delta,
                                bool rtx = false);
            
            void
            prefetchFrame(const Name& basePrefix, PacketNumber packetNo,
                          int prefetchSize, int parityPrefetchSize,
                          FrameBuffer::Slot::Namespace nspc = FrameBuffer::Slot::Delta);
            
            /**
             * Requests additional frames to keep buffer meet its target size
             * @param useEstimatedSize Indicates whether estimated buffer size 
             * or playable buffer size should be used for checking
             * @return Number of frames requested
             */
            int
            keepBuffer(bool useEstimatedSize = true);
            
            void
            resetData();
            
            void
            recoveryCheck(const ndnrtc::new_api::FrameBuffer::Event& event);
            
            void
            rebuffer();
            
            void
            switchToState(State newState)
            {
                State oldState = state_;
                state_ = newState;
                
                LogDebugC << "new state " << toString(state_) << std::endl;
                
                if (callback_)
                    callback_->onStateChanged(oldState, state_);
            }
            
            std::string
            toString(State state)
            {
                switch (state) {
                    case StateInactive:
                        return "Inactive";
                    case StateChasing:
                        return "Chasing";
                    case StateBuffering:
                        return "Buffering";
                    case StateFetching:
                        return "Fetching";
                    default:
                        return "Unknown";
                }
            }
        };
    }
}

#endif /* defined(__ndnrtc__pipeliner__) */