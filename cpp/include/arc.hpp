//
// Created by Julian Janda on 07.05.18.
//

#ifndef NDNRTC_ARC_ARC_HPP
#define NDNRTC_ARC_ARC_HPP


namespace ndnrtc{
    /*
     * TODO: Write description
     */
    class Arc
    {
    public:
        Arc(short adaptionLogic = 0);
        short noAdaption();
        short dashJS ();
        short thang ();
        short getAdaptionLogic();
        void write();

        static const short NO_ADAPTION = 0;
        static const short DASH_JS = 1;
        static const short THANG = 2;

    private:
        short adaptionLogic = 0;
    };
}

#endif //NDNRTC_ARC_ARC_HPP