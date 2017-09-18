#pragma once

#include "Rtos.hpp"

#include <functional>

/**
 * This class adds some nifty features to RtosTimer to make it more convenient
 * to use.
 *
 * RtosTimer allows you to pass a function pointer to the constructor that will
 * be called whenever the timer fires.  This class adds the ability to instead
 * pass an instance method and an object or a lambda.
 */
class RtosTimerHelper : public RtosTimer {
public:
    /// Call an instance method on a given object when the timer fires
    template <class T>
    RtosTimerHelper(T* instance, void (T::*method)(), os_timer_type type)
        : RtosTimer(&timerFired, type, this),
          m_callback(std::bind(method, instance)) {}

    /// Call a function/lambda when the timer fires
    RtosTimerHelper(std::function<void()> callback, os_timer_type type)
        : RtosTimer(&timerFired, type, this), m_callback(callback) {}

    virtual ~RtosTimerHelper() { stop(); }

    RtosTimerHelper(const RtosTimerHelper& other) = delete;

private:
    static void timerFired(const void* instance) {
        auto mutableThis = const_cast<RtosTimerHelper*>(
            reinterpret_cast<const RtosTimerHelper*>(instance));
        mutableThis->m_callback();
    }

    std::function<void()> m_callback;
};
