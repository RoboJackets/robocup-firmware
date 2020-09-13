#pragma once

#include "mJackets.hpp"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/**
 * A locked-struct abstraction. To access the inner value (either read or write)
 * you must first acquire a Lock, an RAII structure that will release the lock
 * upon destruction.
 *
 * The structure uses a recursive mutex, so it is safe to acquire locks
 * repeatedly from the same thread.
 *
 * @tparam T
 */
template<typename T>
struct LockedStruct {
public:
    template<typename... Args>
    LockedStruct(Args... args) : value(std::forward<Args>(args)...) {
        mutex = xSemaphoreCreateRecursiveMutex();
    }

    // No copy/move
    LockedStruct(const LockedStruct&) = delete;
    LockedStruct& operator=(const LockedStruct& ) = delete;
    LockedStruct(LockedStruct&&) = delete;
    LockedStruct& operator=(LockedStruct&& ) = delete;

    ~LockedStruct() {}

    struct Lock {
    public:
        // Cannot copy a lock
        Lock(const Lock&) = delete;
        Lock& operator=(const Lock&) = delete;

        Lock(Lock&& other) = default;
        Lock& operator=(Lock&& other) = default;

        T& value() {
            return locked->value;
        }

        T* operator->() {
            return &(locked->value);
        }

        ~Lock() {
            locked->mutex_depth--;
            locked->release_mutex();
        }

    private:
        Lock(LockedStruct* locked, bool *first_lock) : locked(locked) {
            locked->acquire_mutex();
            if (first_lock) {
                *first_lock = (locked->mutex_depth == 0);
            }
            locked->mutex_depth++;
        }
        LockedStruct* locked = nullptr;

        friend struct LockedStruct;
    };

    /**
     * Lock this struct.
     * @param first_lock [optional] output parameter to determine whether or
     *      not this is the only current lock on this struct.
     * @return a lock on this struct.
     */
    Lock lock(bool *first_lock = nullptr) {
        return Lock(this, first_lock);
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
        xSemaphoreTakeRecursive(mutex, 100);
    }

    void release_mutex() {
        xSemaphoreGiveRecursive(mutex);
    }

    friend struct Lock;

    T value;

    int mutex_depth = 0;

//    StaticSemaphore_t mutex;
    SemaphoreHandle_t mutex = nullptr;
};
