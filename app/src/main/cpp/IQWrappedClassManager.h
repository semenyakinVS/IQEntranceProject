//
// Created by test on 5/23/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef IQ_OPTION_ENTRANCE_TEST_IQWRAPPEDCLASSMANAGER_H
#define IQ_OPTION_ENTRANCE_TEST_IQWRAPPEDCLASSMANAGER_H

////////////////////////////////////////////////////////////////////////////////////////////////////
#include <jni.h>

//Pool expanding
#include <cstdlib> //for 'malloc(...)' and 'free(...)'
#include <string.h> //for 'memcpy(...)'

//Pool thread-safe
#include <mutex>

//Pool objects initializing
#include <new> //for using 'placement new' syntax

////////////////////////////////////////////////////////////////////////////////////////////////////
//======================================== Utilities ===============================================
typedef int PullObjectID;
extern const PullObjectID kPullObjectInvalidIndex;
typedef char ByteType;

template<typename T_Type, int T_ElementsCount = 100>
class USVectorBasedPullAllocator {
private:
    //------------------------------ Types & Constants ---------------------------------------------
    typedef USVectorBasedPullAllocator<T_Type, T_ElementsCount> OwnType;

    //NB: As we use elements data to store pointers to elements - make it's enough data for pointer.
    static const int kSizeOfElementInBytes = (0 == sizeof(PullObjectID)/sizeof(T_Type)) ?
                                             sizeof(T_Type) : sizeof(PullObjectID);
    static const int kDataSizeInBytes = T_ElementsCount * kSizeOfElementInBytes;

public:
    //----------------------------------- Methods --------------------------------------------------
    //- - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - - - -
    USVectorBasedPullAllocator()
            : _data(nullptr), _topElementByteIndex(0),
              _methodCallLock()
    {
        _data = reinterpret_cast<ByteType *>(malloc(kDataSizeInBytes));

        //Build list of free block elements using elements data to store pointers. List structure:
        // Index (in 'size_of_elements')    |0    |1    |2    |3    |
        // Value                            |1    |2    |3    |-1   |
        // Top element                                         ^
        PullObjectID *theElementAsIndexPointer = nullptr;

        const int kLastElementIndex = kDataSizeInBytes - kSizeOfElementInBytes;
        for (int theByteIndex = 0; theByteIndex < kLastElementIndex;
             theByteIndex += kSizeOfElementInBytes)
        {
            theElementAsIndexPointer = reinterpret_cast<PullObjectID  *>(_data + theByteIndex);
            *theElementAsIndexPointer = (theByteIndex + kSizeOfElementInBytes);
        }

        theElementAsIndexPointer = reinterpret_cast<PullObjectID  *>(_data + kLastElementIndex);
        *theElementAsIndexPointer = kPullObjectInvalidIndex;

        _topElementByteIndex = 0;
    }

    USVectorBasedPullAllocator(const OwnType &) = delete;
    USVectorBasedPullAllocator(OwnType &&) = delete;

    //NB: Destruction of pool invalidate all objects, that was stored there
    ~USVectorBasedPullAllocator() { free(_data); }

    //- - - - - - - - - - - - - - - - Objects lifecycle - - - - - - - - - - - - - - - - - - - - - -
    template<typename ... T_Args>
    PullObjectID createObject(T_Args ... inArgs) {
        //TODO: Perform throughing an exception on creation more then T_ElementsCount
        if (kPullObjectInvalidIndex == _topElementByteIndex) return kPullObjectInvalidIndex;

        std::lock_guard<std::mutex> theWaitObjectCreationLock(_methodCallLock);

        const PullObjectID theCreatedObjectIndex = _topElementByteIndex;
        void *theElementPointer = (_data + _topElementByteIndex);

        PullObjectID *theElementAsIndexPointer =
                reinterpret_cast<PullObjectID  *>(theElementPointer);

        _topElementByteIndex = *theElementAsIndexPointer;
        T_Type *theElementAsObjectPointer = reinterpret_cast<T_Type *>(theElementPointer);
        new (theElementAsObjectPointer) T_Type(inArgs ...);

        return theCreatedObjectIndex;
    }

    T_Type *getObject(PullObjectID inObjectID) {
        if (kPullObjectInvalidIndex == inObjectID) return nullptr;

        std::lock_guard<std::mutex> theWaitObjectAccessLock(_methodCallLock);

        void *theElementPointer = (_data + inObjectID);

        T_Type *theElementAsObjectPointer = reinterpret_cast<T_Type *>(theElementPointer);
        return theElementAsObjectPointer;
    }

    //TODO: If it will
    //NB:
    //

    void removeObject(PullObjectID inObjectID) {
        //TODO: Notify here an error - removing null object
        if (kPullObjectInvalidIndex == inObjectID) return;

        std::lock_guard<std::mutex> theWaitObjectRemovingLock(_methodCallLock);

        void *theElementPointer = _data + _topElementByteIndex;

        T_Type *theElementAsObjectPointer = reinterpret_cast<T_Type *>(theElementPointer);
        theElementAsObjectPointer->~T_Type();

        PullObjectID *theElementAsIndexPointer =
                reinterpret_cast<PullObjectID *>(theElementPointer);
        *theElementAsIndexPointer = _topElementByteIndex;
        _topElementByteIndex = inObjectID;
    }

private:
    //------------------------------------ State ---------------------------------------------------
    ByteType *_data;
    PullObjectID _topElementByteIndex;

    std::mutex _methodCallLock;
};

//================================== Wrapped class manager =========================================
template<typename T_Type, int T_ObjectPoolBlockSize = 100>
class IQWrappedClassManager {
private:
    //------------------------------------ Types ---------------------------------------------------
    typedef USVectorBasedPullAllocator<T_Type, T_ObjectPoolBlockSize> StorageType;

public:
    //----------------------------------- Methods --------------------------------------------------
    //- - - - - - - - - - - - - - - Instances lifecycle - - - - - - - - - - - - - - - - - - - - - -
    //NB: Here are implicit "PullObjectID -> jint" conversions used

    template<typename ... T_Args>
    jint createInstance(T_Args ... inArgs) { return _storage.createObject(inArgs ...); }
    T_Type *getInstance(jint inInstanceID) { return _storage.getObject(inInstanceID); }
    void deleteInstance(jint inInstanceID) { _storage.removeObject(inInstanceID); }

private:
    //------------------------------------ State ---------------------------------------------------
    USVectorBasedPullAllocator<T_Type, T_ObjectPoolBlockSize> _storage;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //IQ_OPTION_ENTRANCE_TEST_IQWRAPPEDCLASSMANAGER_H
