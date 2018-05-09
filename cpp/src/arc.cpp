//
// Created by Julian Janda on 07.05.18.
//

#include <iostream>

#include "arc.hpp"

using namespace ndnrtc;

Arc::Arc(int initialValue):
        privateValue(initialValue)
{}

int Arc::getValue() const
{
    return privateValue;
}

void Arc::setValue(int value)
{
    privateValue = value;
}

void Arc::write()
{
    std::cout << "hello world!" << std::endl;
}