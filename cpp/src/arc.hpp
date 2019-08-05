//
// Created by Julian Janda on 07.05.18.
//

#ifndef NDNRTC_ARC_ARC_HPP
#define NDNRTC_ARC_ARC_HPP


#include <string>
#include <vector>
#include <unordered_map>
#include <boost/thread.hpp>
#include "segment-controller.hpp"
#include "remote-stream-impl.hpp"
#include "pipeline-control-state-machine.hpp"
#include "pipeline-control.hpp"
#include "interest-queue.hpp"
#include "simple-log.hpp"

namespace ndnrtc{

    enum class AdaptionLogic {
        NoAdaption,
        Random,
        Sequential,
        Dash_JS,
        Thang
    };

    /**
     * TODO Write description
     * TODO Write about observing in description
     * ARC assumes that representations are given in an ordered list, sorted by quality in ascending order
     */
     // TODO Inherit from NdnRtcComponent
class Arc : public ndnrtc::ISegmentControllerObserver, public ndnrtc::IInterestQueueObserver
    {
    public:
        Arc(AdaptionLogic adaptionLogic,
            RemoteStreamImpl* pimpl,
            boost::shared_ptr<statistics::StatisticsStorage> &storage);
        ~Arc();

        void calculateThreadToFetch();
        void switchThread();
        void setThreadsMeta (std::map<std::string, boost::shared_ptr<NetworkData>> threadsMeta);
        AdaptionLogic getSelectedAdaptionLogic(); // TODO delete this?

        // TODO Should videoThread vector & structs be private?
        struct videoThread {
            std::string threadName = "";
            std::string gop = "";
            std::string start_bitrate = "";
            std::string max_bitrate = "";
            std::string encode_height = "";
            std::string encode_width = "";
            std::string drop_frames = "";
        };
        std::vector<videoThread> videoThreads;


    private:
        AdaptionLogic selectedAdaptionLogic = AdaptionLogic::NoAdaption;
        std::string threadToFetch;
        std::string lastThreadToFetch = "";
        int sequentialAdaptionThreadCounter = 0;
        std::unordered_map<std::string, uint64_t> sentInterests;
        std::unordered_map<std::string, uint64_t> receivedData;
        RemoteStreamImpl* pimpl;
        boost::shared_ptr<statistics::StatisticsStorage> sstorage_;
        std::map<std::string, boost::shared_ptr<NetworkData>> threadsMeta_;

        bool metaFetched = false;
        int minimumThreadTime = 4000;
        int gopCounter = 0;
        uint64_t arcStartTime = 0;
        uint64_t lastThreadtoFetchChangeTime = 0;
        double counter = 0; // TODO delete this after debugging
        double counter2 = 0; // TODO delete this after debugging
        double counter3 = 0; // TODO delete this after debugging
        double dashJS_lastSegmentMeasuredThroughput = -1;
        double dashJS_lastSegmentCalculatedThroughput = 0;
        int videoThreadsOrder [6] = {1,2,0,1,2,0}; // TODO Delete this after debugging
        std::ofstream segmentTypeVisualisation_file;
        double timeSum = 0;
        double sizeSum = 0;

        // IInterestQueueObserver method
        void onInterestIssued(const boost::shared_ptr<const ndn::Interest>&) override;

        // ISegmentControllerObserver methods:
        void segmentArrived(const boost::shared_ptr<WireSegment>&) override;
        void segmentRequestTimeout(const NamespaceInfo&) override { /*ignored*/ }
        void segmentNack(const NamespaceInfo&, int) override { /*ignored*/ }
        void segmentStarvation() override { /*ignored*/ }

        /**
         * This logic doesn't change the current representation at all. Used when ARC is disabled.
         */
        std::string noAdaption();

        /**
         * This logic changes to a random representation out of all available ones
         */
        std::string randomAdaption();

        /**
         * This logic changes to the next highest representation available until it reaches
         * the highest, after which ist loops back to the lowest and starts from anew.
         */
        std::string sequentialAdaption();

        /**
         * This adaption logic calculates a suggested bitrate for the next segment,
         * by using the calculation result from the last segment (low weight) and the
         * measured actual throughput of the last segment (high weight).
         *
         * It then goes through the list of available representations from highest to lowest
         * until it finds one that is lower than the calculated bitrate for the next segment.
         */
        std::string dashJS ();

        /**
         *
         */
        std::string thang ();
    };
}

#endif //NDNRTC_ARC_ARC_HPP