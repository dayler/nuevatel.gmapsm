/*
 * File:    thread.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _THREAD_HPP
#define	_THREAD_HPP

#include <boost/thread.hpp>

/**
 * <p>The Thread class.</p>
 *
 * @author  Eduardo Marin
 * @version 1.0, 04-16-2010
 */
class Thread {

    /** The thread. */
    boost::thread *thread;

    /** The state. */
    unsigned char state;

public:

    /* constants for state */
    static unsigned char NEW;
    static unsigned char RUNNABLE;
    static unsigned char TERMINATED;

    /**
     * Creates a thread.
     */
    Thread() {
        thread=NULL;
        state=NEW;
    }

    virtual ~Thread() {
        join();
        if(thread!=NULL) delete thread;
    }

    /**
     * Starts the thread.
     */
    void start() {
        state=RUNNABLE;
        thread=new boost::thread(&Thread::func, this);
    }

    /**
     * Returns the state.
     * @return unsigned char
     */
    unsigned char getState() {
        return state;
    }

    void join() {
        if(state!=NEW && thread!=NULL) thread->join();
    }

protected:

    /**
     * Implement this method for functionality.
     */
    virtual void run()=0;

private:

    /**
     * Thread method.
     */
    void func() {
        run();
        state=TERMINATED;
    }

};

unsigned char Thread::NEW=0;
unsigned char Thread::RUNNABLE=1;
unsigned char Thread::TERMINATED=2;

#endif	/* _THREAD_HPP */
