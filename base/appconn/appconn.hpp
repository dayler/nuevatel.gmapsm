/* 
 * File:   appconn.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _APPCONN_HPP
#define	_APPCONN_HPP

#include "../executor.hpp"
#include "../logger.hpp"
#include "../socket.hpp"
#include "baseconn.hpp"

/**
 * <p>The BinHandler class should be used to handle received message bytes.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 05-19-2010
 */
class BinHandler : public Thread {

public:

    /** The baseConn. */
    BaseConn *baseConn;

    /** The message bytes. */
    unsigned char *bytes;
    int len;

    /**
     * Creates a new instance of BinHandler.
     * @param *baseConn BaseConn
     * @param *bytes unsigned char
     * @param &len const int
     */
    BinHandler(BaseConn *baseConn, unsigned char *bytes, const int &len) {
        this->baseConn=baseConn;
        this->bytes=new unsigned char[len];
        for(int i=0; i < len; i++) this->bytes[i]=bytes[i];
        this->len=len;
    }

    ~BinHandler() {
        delete[] bytes;
    }

private:

    void run() {
        if(baseConn!=NULL && bytes!=NULL) {
            unsigned char version=bytes[0];
            int sequenceNumber=(bytes[1] << 8) + bytes[2];
            if(version!=BaseConn::VERSION) {
                Message versionNotSupported(BaseConn::VERSION, sequenceNumber, TypeCollection::VERSION_NOT_SUPPORTED_ADVICE.getType(), BaseConn::VERSION, NULL);
                baseConn->write(&versionNotSupported);
            }
            else {
                unsigned char type=bytes[3];
                Message *message;
                if(type < 128) { // Type-Value Representation
                    message=new Message(version, sequenceNumber, type, bytes + 4, 1);
                }
                else { // Type-Length-Value Representation
                    int len=bytes[4];
                    message=new Message(version, sequenceNumber, type, bytes + 5, len);
                }
                MessageType *messageType=baseConn->getTypeCollection()->get(type);
                if(messageType!=NULL) {
                    if(messageType->isRequest() || messageType->isAdvice()) { // request or advice
                        MessageAction *messageAction=baseConn->getActionCollection()->get(type);
                        try {
                            if(messageAction!=NULL) messageAction->execute(baseConn, message);
                        } catch(Exception e) {
                            Logger::getLogger()->logp(&Level::WARNING, "BinHandler", "run", e.toString());
                        }
                    } // response
                    else baseConn->setResponse(new Message(message));
                }
                delete message;
            }
        }
    }

};

/**
 * <p>The Dispatcher class should be used to dispatch messages.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 05-19-2010
 */
class Dispatcher : public ResponseHandler, public Thread {

    /** The baseConn. */
    BaseConn *baseConn;

    /** The message. */
    Message *message;

public:

    /**
     * Creates a new instance of Dispatcher.
     * @param *baseConn BaseConn
     * @param *message Message
     */
    Dispatcher(BaseConn *baseConn, Message *message, Future<Message> *futureResponse) {
        this->baseConn=baseConn;
        this->message=message;
        this->futureResponse=futureResponse;
    }

    ~Dispatcher() {}

private:

    void run() {
        if(baseConn!=NULL && message!=NULL) {
            int sequenceNumber=baseConn->getSequenceNumber();
            message->setSequenceNumber(sequenceNumber);
            MessageType *messageType=baseConn->getTypeCollection()->get(message->getType());
            if(messageType!=NULL) {
                if(messageType->isRequest()) { // request
                    baseConn->putResponseHandler(sequenceNumber, this);
                    baseConn->write(message);
                    {
                        boost::unique_lock<boost::mutex> lock(responseMutex);
                        boost::system_time timeoutTime=boost::get_system_time() + boost::posix_time::millisec(message->getTimeToLive());
                        while(wait && boost::get_system_time() < timeoutTime) responseCond.timed_wait(lock, timeoutTime);
                        baseConn->removeResponseHandler(sequenceNumber);
                    }
                    if(response==NULL) {
                        this->setResponse(NULL);
                        std::stringstream sstream;
                        sstream << "dispatcher timeout connId " << baseConn->getConnId() << " type " << (int)message->getType();
                        Logger::getLogger()->logp(&Level::WARNING, "Dispatcher", "run", sstream.str());
                    }
                } // advice or response
                else baseConn->write(message);
            }
        }
    }

};

/**
 * <p>The AppConn class should be used to handle client connections.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 05-19-2010
 */
class AppConn : public BaseConn, public Thread {

public:

    /* constants */
    static int MAX_FAILED_ECHO;
    static int ECHO_REQUEST_PERIOD;

private:

    static unsigned char T_LENGTH;
    static int BUFFER_LENGTH;

    /** The socket. */
    Socket *socket;
    boost::mutex writeMutex;

    unsigned char *readBuffer;
    unsigned char *writeBuffer;

    /** The handleService. */
    Executor *handleService;

    /** The dispatchService. */
    Executor *dispatchService;

    /** The echoRequestAction. */
    EchoRequestAction echoRequestAction;

    Timer *echoRequestTimer;
    TimerTask *echoRequestTimerTask;
    int failedEchoRegister;

public:

    /**
     * Constructor for use by client subclasses.
     * @param *typeCollection TypeCollection
     * @param *actionCollection ActionCollection
     * @param &host const std:string
     * @param &port const int
     * @param &connId const int
     * @param &connIndex const int
     * @param &registrable const bool
     */
    AppConn(TypeCollection *typeCollection, ActionCollection *actionCollection, const std::string &host, const int &port, const int &connId, const int &connIndex, const bool &registrable) : BaseConn(typeCollection, actionCollection) {
        try {
            readBuffer=new unsigned char[BUFFER_LENGTH];
            writeBuffer=new unsigned char[BUFFER_LENGTH];

            actionCollection->put(&echoRequestAction);

            echoRequestTimer=NULL;
            echoRequestTimerTask=NULL;

            setConnId(connId);
            setConnIndex(connIndex);

            handleService=new Executor();
            dispatchService=new Executor();

            socket=NULL;
            socket=new Socket(host, port);

            start();
            if(registrable) {
                if(registerAppConn()) {
                    scheduleEchoRequest();
                    setConnState(ONLINE);
                }
                else clear();
            }
            else setConnState(ONLINE);
        } catch(Exception e) {
            Logger::getLogger()->logp(&Level::SEVERE, "AppConn", "<init>", e.toString());
            clear();
        }
    }

    ~AppConn() {
        if(echoRequestTimer!=NULL) delete echoRequestTimer;
        if(echoRequestTimerTask!=NULL) delete echoRequestTimerTask;
        clear();
        delete handleService;
        delete dispatchService;
        delete[] readBuffer;
        delete[] writeBuffer;
    }

    void clear() {
        setConnState(OFFLINE);
        if(socket!=NULL) {
            socket->close();
            delete socket;
            socket=NULL;
        }
    }

    void write(Message *message) {
        boost::lock_guard<boost::mutex> lock(writeMutex);
        if(socket!=NULL) {
            try {
                int len;
                message->getMessage(writeBuffer, len);
                socket->write(writeBuffer, len);
            } catch(SocketException se) {
                Logger::getLogger()->logp(&Level::SEVERE, "AppConn", "write", se.toString());
                clear();
            }
        }
    }

    void dispatch(Message *message, Future<Message> *futureResponse) {
        dispatchService->submit(new Dispatcher(this, message, futureResponse));
    }

    void scheduleEchoRequest() {
        failedEchoRegister=0;
        if(echoRequestTimer!=NULL) delete echoRequestTimer;
        if(echoRequestTimerTask!=NULL) delete echoRequestTimerTask;
        echoRequestTimerTask=new EchoRequestTimerTask(this);
        echoRequestTimer=new Timer();
        echoRequestTimer->scheduleAtFixedRate(echoRequestTimerTask, ECHO_REQUEST_PERIOD, ECHO_REQUEST_PERIOD);
    }

private:

    void run() {
        try {
            while(socket!=NULL && socket->isConnected()) {
                if(socket->read(readBuffer, T_LENGTH) > -1) {
                    unsigned char type=readBuffer[3];
                    int len;
                    if(type < 128) { // Type-Value Representation
                        socket->read(readBuffer, T_LENGTH, 1);
                        len=T_LENGTH + 1;
                    }
                    else { // Type-Length-Value Representation
                        socket->read(readBuffer, T_LENGTH, 1);
                        int length=readBuffer[4];
                        socket->read(readBuffer, T_LENGTH + 1, length);
                        len=T_LENGTH + 1 + length;
                    }
                    handleService->submit(new BinHandler(this, readBuffer, len));
                }
                else break;
            }
        }
        catch(Exception e) {
            Logger::getLogger()->logp(&Level::WARNING, "AppConn", "run", e.toString());
        }
        clear();
    }

public:

    /**
     * Registers a failed echo.
     */
    void registerFailedEcho() {
        failedEchoRegister+=1;
        if(failedEchoRegister > MAX_FAILED_ECHO - 1) {
            std::stringstream sstream;
            sstream << "max failed echo connId " << getConnId() << " connIndex " << getConnIndex();
            Logger::getLogger()->logp(&Level::WARNING, "AppConn", "registerFailedEcho", sstream.str());
            clear();
        }
    }

private:

    /**
     * Registers the appConn.
     * @return bool
     */
    bool registerAppConn() {
        if(getConnId() > -1 && getConnIndex() > -1) {
            IELong ieLong(getConnId());
            IEByte ieByte(getConnIndex());

            std::vector<IE*> ieVector;
            ieVector.push_back(&ieLong);
            ieVector.push_back(&ieByte);

            CompositeIE compositeIE(TypeCollection::NODE, 0, &ieVector);
            ieVector.clear();
            ieVector.push_back(&compositeIE);

            Message registerRequest(VERSION, 0, TypeCollection::REGISTER_REQUEST.getType(), 0, &ieVector);
            Future<Message> futureResponse;
            dispatch(&registerRequest, &futureResponse);

            Message *response=futureResponse.get();
            if(response!=NULL && response->getType()==TypeCollection::REGISTER_REQUEST.getLinkedType() && response->getValueByte()==TypeCollection::REQUEST_ACCEPTED) {
                std::stringstream sstream;
                sstream << "register request connId " << getConnId() << " connIndex " << getConnIndex() << " accepted";
                Logger::getLogger()->logp(&Level::FINE, "AppConn", "registerAppConn", sstream.str());
                delete response;
                return true;
            }
            if(response!=NULL) delete response;
        }
        std::stringstream sstream;
        sstream << "register request connId " << getConnId() << " connIndex " << getConnIndex() << " failed";
        Logger::getLogger()->logp(&Level::WARNING, "AppConn", "registerAppConn", sstream.str());
        return false;
    }

    class EchoRequestTimerTask : public TimerTask {

        /** The appConn. */
        AppConn *appConn;

    public:

        /**
         * Creates a new instance of EchoRequestTimerTask.
         * @param *appConn AppConn
         */
        EchoRequestTimerTask(AppConn *appConn) {
            this->appConn=appConn;
        }

    private:

        void run() {
            if(appConn->getConnState()==BaseConn::ONLINE) {
                Message echoRequest(VERSION, 0, TypeCollection::ECHO_REQUEST.getType(), 0xaa, NULL);
                Future<Message> futureResponse;
                appConn->dispatch(&echoRequest, &futureResponse);
                Message *response=futureResponse.get();
                if(response==NULL || response->getType()!=TypeCollection::ECHO_REQUEST.getLinkedType() || response->getValueByte()!=0xaa) appConn->registerFailedEcho();
                if(response!=NULL) delete response;
            }
        }

    };

};

int AppConn::MAX_FAILED_ECHO=2;
int AppConn::ECHO_REQUEST_PERIOD=32000;
unsigned char AppConn::T_LENGTH=4;
int AppConn::BUFFER_LENGTH=512;

#endif	/* _APPCONN_HPP */
