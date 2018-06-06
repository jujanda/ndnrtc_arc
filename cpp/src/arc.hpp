//
// Created by Julian Janda on 07.05.18.
//

#ifndef NDNRTC_ARC_ARC_HPP
#define NDNRTC_ARC_ARC_HPP


#include <string>
#include <vector>
#include "segment-controller.hpp"

namespace ndnrtc{
    /**
     * TODO: Write description
     * ARC assumes that representations are given in an ordered list, sorted by quality in ascending order
     */
class Arc : public ndnrtc::ISegmentControllerObserver
    {
    public:
        Arc(int adaptionLogic = 0);
        std::string calculateThreadToFetch();
        int getSelectedAdaptionLogic();
        void write();

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

        // TODO solve this with 'typdef enum'?
        static const int AL_NO_ADAPTION = 0;
        static const int AL_DASH_JS = 1;
        static const int AL_THANG = 2;

    private:
        int selectedAdaptionLogic = 0;
        std::string threadToFetch;
        double dashJS_lastSegmentCalculatedThroughput = 0; // TODO move this into DASH-JS class

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