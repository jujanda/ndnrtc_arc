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
        Arc(int initialValue = 42);

        int getValue() const;
        void setValue(int value);
        void write();

    private:
        int privateValue;
    };
}

#endif //NDNRTC_ARC_ARC_HPP