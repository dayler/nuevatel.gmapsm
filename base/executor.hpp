/* 
 * File:   executor.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _EXECUTOR_HPP
#define	_EXECUTOR_HPP

#include "thread.hpp"
#include "timer.hpp"

#include <vector>

/**
 * <p>The Executor class.</p>
 *
 * @author  Eduardo Marin
 * @version 1.0, 05-19-2010
 */
class Executor {

    static int DELETE_TERMINATED_PERIOD;

    /** The threadVector. */
    std::vector<Thread*> threadVector;
    unsigned int threadIdx;

    /** The deleteTimer. */
    Timer *deleteTimer;
    TimerTask *deleteTimerTask;

    /** The executorMutex. */
    boost::mutex executorMutex;

public:

    /**
     * Creates a new instance of Executor.
     */
    Executor() {
        deleteTimer=new Timer();
        deleteTimerTask=new DeleteTimerTask(this);
        deleteTimer->scheduleAtFixedRate(deleteTimerTask, DELETE_TERMINATED_PERIOD, DELETE_TERMINATED_PERIOD);
        threadIdx=0;
    }

    virtual ~Executor() {
        delete deleteTimer;
        delete deleteTimerTask;
        for(unsigned int i=0; i < threadVector.size(); i++) delete threadVector[i];
    }

    /**
     * Submits a new thread for execution. The thread pointer will be deleted 
     * after execution.
     * @param *thread Thread
     */
    void submit(Thread *thread) {
        boost::lock_guard<boost::mutex> lock(executorMutex);
        while(threadIdx < threadVector.size()) {
            Thread *tmpThread=threadVector[threadIdx];
            if(tmpThread->getState()==Thread::TERMINATED) {
                delete tmpThread;
                threadVector[threadIdx]=thread;
                if(threadIdx < threadVector.size() - 1) threadIdx++;
                else threadIdx=0;
                thread->start();
                return;
            }
            else {
                if(threadIdx < threadVector.size() - 1) threadIdx++;
                else break;
            }
        }
        threadVector.push_back(thread);
        threadIdx=0;
        thread->start();
    }

private:

    /**
     * Deletes all terminated threads.
     */
    void deleteTerminated() {
        boost::lock_guard<boost::mutex> lock(executorMutex);
        std::vector<Thread*>::iterator iter;
        iter=threadVector.begin();
        while(iter!=threadVector.end()) {
            Thread *thread=*iter;
            if(thread->getState()==Thread::TERMINATED) {
                delete thread;
                iter=threadVector.erase(iter);
            }
            else iter++;
        }
        threadIdx=0;
    }

    class DeleteTimerTask : public TimerTask {

        /** The executor. */
        Executor *executor;

    public:

        /**
         * Creates a new instance of DeleteTimerTask.
         * @param *executor Executor
         */
        DeleteTimerTask(Executor *executor) {
            this->executor=executor;
        }

    private:

        void run() {
            executor->deleteTerminated();
        }

    };

};

int Executor::DELETE_TERMINATED_PERIOD=60000;

#endif	/* _EXECUTOR_HPP */
