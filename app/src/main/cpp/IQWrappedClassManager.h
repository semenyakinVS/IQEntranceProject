//
// Created by test on 5/23/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef IQ_OPTION_ENTRANCE_TEST_IQWRAPPEDCLASSMANAGER_H
#define IQ_OPTION_ENTRANCE_TEST_IQWRAPPEDCLASSMANAGER_H

////////////////////////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <cstdlib>

////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T_Type>
class IQWrappedClassManager {
public:
    template<typename ... T_Args>
    int createInstance(T_Args ... inArgs) {
        //TODO: Use here object pool
        T_Type *theObjectMemory = (T_Type *)malloc(sizeof(T_Type));
        new (theObjectMemory) T_Type(inArgs ...);
        _instances.push_back(theObjectMemory);

        return (_instances.size() - 1);
    }

    T_Type *getInstance(int inInstanceID) { return _instances[inInstanceID]; }

    void deleteInstance(int inInstanceID) {
        //TODO: Put here error tracking
        T_Type *theInstance = _instances[inInstanceID];
        delete theInstance;
        _instances.erase(_instances.begin() + inInstanceID);
    }

private:
    std::vector<T_Type *> _instances;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //IQ_OPTION_ENTRANCE_TEST_IQWRAPPEDCLASSMANAGER_H
