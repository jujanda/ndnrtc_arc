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

namespace ndnrtc{

    enum class AdaptionLogic {
        NoAdaption,
        Random,
        Dash_JS,
        Thang
    };

    /**
     * TODO: Write description
     * ARC assumes that representations are given in an ordered list, sorted by quality in ascending order
     */
     // TODO Inherit from ndnrtccomponent?
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
        std::unordered_map<std::string, double> sentInterests;
        RemoteStreamImpl* pimpl;
        boost::shared_ptr<statistics::StatisticsStorage> sstorage_;
        std::map<std::string, boost::shared_ptr<NetworkData>> threadsMeta_;

        bool metaFetched = false;
        int minimumThreadTime = 4000;
        int gopCounter = 0;
        double arcStartTime = 0;
        double lastThreadtoFetchChangeTime = 0;
        double counter = 0; // TODO delete this after debugging
        double counter2 = 0; // TODO delete this after debugging
        double counter3 = 0; // TODO delete this after debugging
        double dashJS_lastSegmentMeasuredThroughput = -1; // TODO move this into DASH-JS class
        double dashJS_lastSegmentCalculatedThroughput = 0; // TODO move this into DASH-JS class

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