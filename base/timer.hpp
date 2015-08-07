/*
 * File:   timer.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _TIMER_HPP
#define	_TIMER_HPP

#include "thread.hpp"

#include <unistd.h>

/**
 * <p>The TimerTask class.</p>
 *
 * @author  Eduardo Marin
 * @version 1.0, 04-16-2010
 */
class TimerTask {

public:

    TimerTask() {}

    virtual ~TimerTask() {}

    virtual void run()=0;

};

/**
 * <p>The Timer class.</p>
 *
 * @author  Eduardo Marin
 * @version 1.0, 04-16-2010
 */
class Timer : public Thread {

    /** The task. */
    TimerTask *task;

    /* private variables */
    long delay;
    long period;
    bool cancelled;

    /** The cancelCond. */
    boost::condition_variable cancelCond;

    /** The cancelMutex. */
    boost::mutex cancelMutex;

public:

    /**
     * Creates an unscheduled timer.
     */
    Timer() : Thread() {
        cancelled=false;
    }

    virtual ~Timer() {
        cancel();
    }

    /**
     * Cancels the timer.
     */
    void cancel() {
        {
            boost::lock_guard<boost::mutex> lock(cancelMutex);
            cancelled=true;
        }
        cancelCond.notify_one();
        join();
    }

    /**
     * Schedules the specified task for execution after the specified delay.
     * @param *task TimerTask
     * @param &delay const long
     */
    void schedule(TimerTask *task, const long &delay) {
        cancelled=false;
        this->task=task;
        this->delay=delay;
        period=0;
        start();
    }

    /**
     * Schedules the specified task for repeated fixed-rate execution, beginning after the specified delay.
     * @param *task TimerTask
     * @param &delay const long
     * @param &period const long
     */
    void scheduleAtFixedRate(TimerTask *task, const long &delay, const long &period) {
        cancelled=false;
        this->task=task;
        this->delay=delay;
        this->period=period;
        start();
    }

private:

    void run() {
        {
            boost::unique_lock<boost::mutex> lock(cancelMutex);
            boost::system_time delayTime=boost::get_system_time() + boost::posix_time::millisec(delay);
            while(!cancelled && boost::get_system_time() < delayTime) cancelCond.timed_wait(lock, delayTime);
        }
        if(!cancelled) {
            task->run();
            if(period > 0) {
                while(true) {
                    boost::unique_lock<boost::mutex> lock(cancelMutex);
                    boost::system_time periodTime=boost::get_system_time() + boost::posix_time::millisec(period);
                    while(!cancelled && boost::get_system_time() < periodTime) cancelCond.timed_wait(lock, periodTime);
                    if(!cancelled) task->run();
                    else break;
                }
            }
        }
    }

};

#endif	/* _TIMER_HPP */
