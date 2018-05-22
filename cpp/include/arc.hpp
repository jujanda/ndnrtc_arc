//
// Created by Julian Janda on 07.05.18.
//

#ifndef NDNRTC_ARC_ARC_HPP
#define NDNRTC_ARC_ARC_HPP


#include <string>
#include <vector>

namespace ndnrtc{
    /*
     * TODO: Write description
     */
    class Arc
    {
    public:
        Arc(int adaptionLogic = 0);
        int getSelectedAdaptionLogic();
        void write();

        std::vector<std::string> threadNames;

        static const int AL_NO_ADAPTION = 0;
        static const int AL_DASH_JS = 1;
        static const int AL_THANG = 2;

    private:
        int selectedAdaptionLogic = 0;
        std::string noAdaption();
        std::string dashJS ();
        std::string thang ();
    };
}

#endif //NDNRTC_ARC_ARC_HPP