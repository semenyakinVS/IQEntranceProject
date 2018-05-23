//
// Created by test on 5/19/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef IQ_OPTION_ENTRANCE_TEST_UTILS_H
#define IQ_OPTION_ENTRANCE_TEST_UTILS_H

////////////////////////////////////////////////////////////////////////////////////////////////////
#include <limits>
#include <math.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
enum class IQComparationResult: int {
    Less = -1,
    Equals = 0,
    More = 1,
    Invalid = std::numeric_limits<int>::max()
};

template<typename T>
IQComparationResult compareFloats(
        T inA, T inB, T inEpsilon = std::numeric_limits<float>::epsilon())
{
    const float theDifference = inA - inB;
    return theDifference > inEpsilon ? IQComparationResult::More :
           theDifference < -inEpsilon ? IQComparationResult::Less : IQComparationResult::Equals;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //IQ_OPTION_ENTRANCE_TEST_UTILS_H
