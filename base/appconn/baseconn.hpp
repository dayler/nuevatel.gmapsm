/*
 * File:   baseconn.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _BASECONN_HPP
#define	_BASECONN_HPP

#include "message.hpp"

#include <boost/thread/mutex.hpp>

class BaseConn;

/**
 * <p>The Future template class.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 05-19-2010
 */
template <class V>
class Future {

    /** The v object. */
    V *v;

    /** The vCond. */
    boost::condition_variable vCond;

    /** The vMutex. */
    boost::mutex vMutex;

    /* private variables */
    bool wait;

public:

    /**
     * Creates a new instance of Future.
     */
    Future() {
        v=NULL;
        wait=true;
    }

    virtual ~Future() {}

    /**
     * Sets the result.
     * @param *v V
     */
    void set(V *v) {
        {
            boost::lock_guard<boost::mutex> lock(vMutex);
            if(this->v==NULL) this->v=v;
            wait=false;
        }
        vCond.notify_one();
    }

    /**
     * Waits if necessary for the computation to complete, and then 
     * retrieves its result.
     * @return the computed result
     */
    V* get() {
        boost::unique_lock<boost::mutex> lock(vMutex);
        while(wait) vCond.wait(lock);
        return v;
    }

};

/**
 * <p>The MessageAction abstract class should be extended to define message actions.</p>
 *
 * @author  Eduardo Marin
 * @version 1.0, 05-18-2010
 */
class MessageAction {

    /** The type. */
    unsigned char type;

public:

    /**
     * Creates a new instance of MessageAction.
     * @param *messageType MessageType
     */
    MessageAction(MessageType *messageType) {
        type=messageType->getType();
    }

    virtual ~MessageAction() {}

    /**
     * Implement this method to define message action, it should throw an 
     * exception when fails to execute.
     * @param *baseConn BaseConn
     * @param *message Message
     */
    virtual void execute(BaseConn *baseConn, Message *message)=0;

    /**
     * Returns the type.
     * @return unsigned char
     */
    unsigned char getType() {
        return type;
    }

};

/**
 * <p>The ActionCollection class should be used define supported
 * message actions collection.</p>
 *
 * @author  Eduardo Marin
 * @version 1.0, 05-18-2010
 */
class ActionCollection {

    /** The messageActionMap. */
    std::map<unsigned char, MessageAction*> messageActionMap;

public:

    /**
     * Creates a new empty instance of TypeCollection.
     */
    ActionCollection() {}

    virtual ~ActionCollection() {}

    /**
     * Puts a messageAction.
     * @param *messageAction MessageAction
     */
    void put(MessageAction *messageAction) {
        messageActionMap.erase(messageAction->getType());
        messageActionMap.insert(std::pair<unsigned char, MessageAction*>(messageAction->getType(), messageAction));
    }

    /**
     * Returns the messageAction for the given message type, or null if the given
     * type is not in the collection.
     * @param &type const unsigned char
     * @return *MessageAction
     */
    MessageAction* get(const unsigned char &type) {
        std::map<unsigned char, MessageAction*>::iterator iter;
        iter=messageActionMap.find(type);
        if(iter!=messageActionMap.end()) return iter->second;
        else return NULL;
    }

};

/**
 * <p>The ResponseHandler abstract class should be extended to handle response messages.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 05-18-2010
 */
class ResponseHandler {

protected:

    /** The futureResponse. */
    Future<Message> *futureResponse;

    /** The responseCondition. */
    boost::condition_variable responseCond;

    /** The responseHandlerMutex. */
    boost::mutex responseMutex;

    /** The response. */
    Message *response;

    /* private variables */
    bool wait;

public:

    /**
     * Creates a new instance of ResponseHandler.
     */
    ResponseHandler() {
        response=NULL;
        wait=true;
    }

    virtual ~ResponseHandler() {}

    /**
     * Sets the response.
     * @param *response Message
     */
    void setResponse(Message *response) {
        {
            boost::lock_guard<boost::mutex> lock(responseMutex);
            this->response=response;
            if(futureResponse!=NULL) futureResponse->set(response);
            wait=false;
        }
        if(response!=NULL) responseCond.notify_one();
    }

};

/**
 * <p>The BaseConn abstract class should be extended to handle client connections.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 05-18-2010
 */
class BaseConn {

public:

    /* constants */
    static int VERSION;
    static int DEFAULT_PORT;

    static int OFFLINE;
    static int ONLINE;

private:

    /** The typeCollection. */
    TypeCollection *typeCollection;

    /** The actionCollection. */
    ActionCollection *actionCollection;

    /** The responseHandlerMap. */
    std::map<int, ResponseHandler*> responseHandlerMap;
    boost::mutex responseHandlerMapMutex;

    /** The sequenceNumber. */
    int sequenceNumber;
    boost::mutex sequenceNumberMutex;

    /** The conn id. */
    int connId;

    /** The conn index. */
    int connIndex;

    /** The connState. */
    unsigned char connState;
    boost::mutex connStateMutex;

protected:

    /**
     * Constructor for use by subclasses.
     * @param *typeCollection TypeCollection
     * @param *actionCollection ActionCollection
     */
    BaseConn(TypeCollection *typeCollection, ActionCollection *actionCollection) {
        this->typeCollection=typeCollection;
        this->actionCollection=actionCollection;
        sequenceNumber=0;
        connId=-1;
        connIndex=-1;
        setConnState(OFFLINE);
    }

public:

    virtual ~BaseConn() {}

    /**
     * Clears the baseConn.
     */
    virtual void clear()=0;

    /**
     * Writes a message through the interface.
     * @param *message Message
     */
    virtual void write(Message *message)=0;

    /**
     * Dispatches the given message.
     * @param *message Message
     * @param *futureResponse Future<Message>
     */
    virtual void dispatch(Message *message, Future<Message> *futureResponse)=0;

    /**
     * Schedules the echo request.
     */
    virtual void scheduleEchoRequest()=0;

    /**
     * Returns the sequenceNumber.
     * @return int
     */
    int getSequenceNumber() {
        boost::lock_guard<boost::mutex> lock(sequenceNumberMutex);
        if(sequenceNumber < 0xffff) sequenceNumber++;
        else sequenceNumber=0;
        return sequenceNumber;
    }

    /**
     * Returns the typeCollection.
     * @return *TypeCollection
     */
    TypeCollection* getTypeCollection() {
        return typeCollection;
    }

    /**
     * Returns the actionCollection.
     * @return *ActionCollection
     */
    ActionCollection* getActionCollection() {
        return actionCollection;
    }

    /**
     * Puts a responseHandler for the given sequenceNumber.
     * @param &sequenceNumber const int
     * @param *handler ResponseHandler
     */
    void putResponseHandler(const int &sequenceNumber, ResponseHandler *handler) {
        boost::lock_guard<boost::mutex> lock(responseHandlerMapMutex);
        responseHandlerMap.erase(sequenceNumber);
        responseHandlerMap.insert(std::pair<int, ResponseHandler*>(sequenceNumber, handler));
    }

    /**
     * Removes the responseHandler for the given sequenceNumber.
     * @param &sequenceNumber const int
     */
    void removeResponseHandler(const int &sequenceNumber) {
        boost::lock_guard<boost::mutex> lock(responseHandlerMapMutex);
        responseHandlerMap.erase(sequenceNumber);
    }

    /**
     * Sets a response for a responseHandler.
     * @param *response Message
     */
    void setResponse(Message *response) {
        boost::lock_guard<boost::mutex> lock(responseHandlerMapMutex);
        std::map<int, ResponseHandler*>::iterator iter;
        iter=responseHandlerMap.find(response->getSequenceNumber());
        if(iter!=responseHandlerMap.end()) iter->second->setResponse(response);
        else delete response;
    }

protected:

    /**
     * Sets the connId.
     * @param &connId const int
     */
    void setConnId(const int &connId) {
        this->connId=connId;
    }

    /**
     * Sets the connIndex.
     * @param &connIndex const int
     */
    void setConnIndex(const int &connIndex) {
        this->connIndex=connIndex;
    }

    /**
     * Sets the connState.
     * @param &connState const unsigned char
     */
    void setConnState(const unsigned char &connState) {
        boost::lock_guard<boost::mutex> lock(connStateMutex);
        this->connState=connState;
    }

public:

    /**
     * Returns the connId.
     * @return int
     */
    int getConnId() {
        return connId;
    }

    /**
     * Returns the connIndex.
     * @return int
     */
    int getConnIndex() {
        return connIndex;
    }

    /**
     * Returns the connState.
     * @return unsigned char
     */
    unsigned char getConnState() {
        boost::lock_guard<boost::mutex> lock(connStateMutex);
        return connState;
    }

};

int BaseConn::VERSION=0x02;
int BaseConn::DEFAULT_PORT=8482;
int BaseConn::OFFLINE=0;
int BaseConn::ONLINE=1;

/**
 * <p>The BaseConnCache class.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 06-21-2010
 */
class BaseConnCache {

private:

    /** The baseConnSet. */
    std::map<int, BaseConn*> baseConnMap;
    boost::mutex baseConnMapMutex;

    /* private variables */
    std::map<int, BaseConn*>::iterator baseConnIterator;

public:

    /**
     * Creates a new instance of BaseConnCache.
     */
    BaseConnCache() {
        baseConnIterator=baseConnMap.begin();
    }

    /**
     * Adds the baseConn.
     * @param *baseConn BaseConn
     */
    void add(BaseConn *baseConn) {
        boost::lock_guard<boost::mutex> lock(baseConnMapMutex);
        if(baseConn!=NULL && baseConn->getConnIndex() > -1) {
            std::map<int, BaseConn*>::iterator iter;
            iter=baseConnMap.find(baseConn->getConnIndex());
            if(iter!=baseConnMap.end()) {
                baseConnMap.erase(iter);
                delete iter->second;
            }
            baseConnMap.insert(std::pair<int, BaseConn*>(baseConn->getConnIndex(), baseConn));
            baseConnIterator=baseConnMap.begin();
        }
    }

    /**
     * Returns the next online baseConn from the cache, otherwise null.
     * @return *BaseConn
     */
    BaseConn* nextOnline() {
        BaseConn *baseConn;
        baseConn=NULL;
        int connIndex=0;
        while(connIndex < size()) {
            {
                boost::lock_guard<boost::mutex> lock(baseConnMapMutex);
                if(baseConnIterator!=baseConnMap.end()) {
                    BaseConn *tmpBaseConn=baseConnIterator->second;
                    if(tmpBaseConn->getConnState()==BaseConn::ONLINE) baseConn=tmpBaseConn;
                    baseConnIterator++;
                    connIndex++;
                }
                else baseConnIterator=baseConnMap.begin();
            }
            if(baseConn!=NULL) break;
        }
        return baseConn;
    }

    /**
     * Removes the baseConn.
     * @param *baseConn BaseConn
     */
    void remove(BaseConn *baseConn) {
        boost::lock_guard<boost::mutex> lock(baseConnMapMutex);
        if(baseConn!=NULL && baseConn->getConnIndex() > -1) {
            std::map<int, BaseConn*>::iterator iter;
            iter=baseConnMap.find(baseConn->getConnIndex());
            if(iter!=baseConnMap.end()) {
                baseConnMap.erase(iter);
                delete iter->second;
            }
            baseConnIterator=baseConnMap.begin();
        }
    }

    /**
     * Returns the size.
     * @return int
     */
    int size() {
        boost::lock_guard<boost::mutex> lock(baseConnMapMutex);
        return baseConnMap.size();
    }

    /**
     * Returns the vector of baseConn.
     * @param *baseConnVector std::vector<BaseConn*>
     */
    void getVector(std::vector<BaseConn*> *baseConnVector) {
        boost::lock_guard<boost::mutex> lock(baseConnMapMutex);
        std::map<int, BaseConn*>::iterator iter;
        for(iter=baseConnMap.begin(); iter!=baseConnMap.end(); iter++)
            baseConnVector->push_back(iter->second);
    }

};

/**
 * The EchoRequestAction class.
 */
class EchoRequestAction : public MessageAction {

public:

    /**
     * Creates a new instance of EchoRequestAction.
     */
    EchoRequestAction() : MessageAction(&TypeCollection::ECHO_REQUEST) {}

    void execute(BaseConn *baseConn, Message *message) throw(Exception) {
        Message echoResponse(BaseConn::VERSION, message->getSequenceNumber(), TypeCollection::ECHO_REQUEST.getLinkedType(), 0xaa, NULL);
        baseConn->write(&echoResponse);
    }

};

#endif	/* _BASECONN_HPP */
