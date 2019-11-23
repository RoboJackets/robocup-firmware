#pragma once

#define configSUPPORT_STATIC_ALLOCATION 1

#include "FreeRTOS.h"
#include "semphr.h"

template<typename T>
struct LockedStruct {
public:
    LockedStruct() {
        xSemaphoreCreateMutexStatic(&mutex);
    }

    // No copy/move
    LockedStruct(const LockedStruct&) = delete;
    LockedStruct& operator=(const LockedStruct& ) = delete;
    LockedStruct(LockedStruct&&) = delete;
    LockedStruct& operator=(LockedStruct&& ) = delete;

    ~LockedStruct() {}

    struct Lock {
    public:
        // Cannot copy or move a lock
        Lock(const Lock&) = delete;
        Lock& operator=(const Lock&) = delete;
        Lock(Lock&& other) = delete;
        Lock& operator=(Lock&& other) = delete;

        T& value() {
            return locked->value;
        }

        T* operator->() {
            return &(locked->value);
        }

        ~Lock() {
            locked->release_mutex();
        }

    private:
        Lock(LockedStruct* locked) : locked(locked) {
            locked->acquire_mutex();
        }
        LockedStruct* locked = nullptr;

        friend struct LockedStruct;
    };

    Lock lock() {
        return Lock(this);
    }

    /**
     * Bypass the locking system to directly get a pointer to the underlying
     * data. This is obviously unsafe while the scheduler is running, so it
     * should only be used during startup, before the scheduler has been
     * started (i.e. initializing the struct with useful data).
     *
     * You should _never_ save the result of this function for longer than the
     * function's scope.
     *
     * @return A pointer to the underlying struct
     */
    T *unsafe_value() {
        return &value;
    }

private:
    void acquire_mutex() {
        xSemaphoreTake(&mutex, portMAX_DELAY);
    }

    void release_mutex() {
        xSemaphoreGive(&mutex);
    }

    friend struct Lock;

    T value;
    StaticSemaphore_t mutex;
};