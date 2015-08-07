/* 
 * File:   cqueue.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef CQUEUE_HPP
#define	CQUEUE_HPP

#include <queue>

#include <boost/thread.hpp>

template <class V>
class Queue {

    /** The queue. */
    std::queue<V*> queue;

    /** The pushCond. */
    boost::condition_variable pushCond;

    /** The queueMutex. */
    boost::mutex queueMutex;

public:

    /**
     * Creates a new instance of Queue.
     */
    Queue() {}

    virtual ~Queue() {}

    /**
     * Returns true if the queue is empty.
     * @return bool
     */
    bool empty() {
        boost::lock_guard<boost::mutex> lock(queueMutex);
        return queue.empty();
    }

    /**
     * Pushes an element.
     * @param *v V
     */
    void push(V *v) {
        {
            boost::lock_guard<boost::mutex> lock(queueMutex);
            queue.push(v);
        }
        pushCond.notify_one();
    }

    /**
     * Returns the front element.
     * @return *V
     */
    V* waitAndPop() {
        boost::unique_lock<boost::mutex> lock(queueMutex);
        while(queue.empty()) pushCond.wait(lock);
        V *v;
        v=queue.front();
        queue.pop();
        return v;
    }

};

#endif	/* CQUEUE_HPP */
