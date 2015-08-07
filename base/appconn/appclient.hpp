/*
 * File:   appclient.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _APPCLIENT_HPP
#define	_APPCLIENT_HPP

#include "../properties.hpp"
#include "appconn.hpp"

#include <iostream>

/**
 * <p>The AppClient class should be used to handle client connections.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 06-21-2010
 */
class AppClient {

public:

    /* constants for client state */
    static int OFFLINE;
    static int ONLINE;

    /* constants for properties */
    static std::string ADDRESS;
    static std::string PORT;
    static std::string CONN_INDEX;
    static std::string SIZE;
    static std::string REGISTRABLE;
    static std::string FAILSAFE;

    /* constants */
    static int FAILSAFE_PERIOD;

private:

    /** The clientId. */
    int clientId;

    /** The clientState. */
    unsigned char clientState;
    boost::mutex clientStateMutex;

    /** The baseConnCache. */
    BaseConnCache baseConnCache;

    /* properties */
    std::string address;
    int port;
    int connIndex;
    int size;
    bool registrable;
    bool failsafe;

    Timer *failsafeTimer;
    TimerTask *failsafeTimerTask;

public:

    /**
     * Creates a new instance of AppClient.
     * @param clientId int
     * @param *properties Properties
     * @param *typeCollection TypeCollection
     * @param *actionCollection ActionCollection
     * @throws Exception
     */
    AppClient(int clientId, Properties *properties, TypeCollection *typeCollection, ActionCollection *actionCollection) throw(Exception) {
        this->clientId=clientId;
        setClientState(OFFLINE);
        // properties
        if(setProperties(properties)) {
            for(int tmpConnIndex=connIndex; tmpConnIndex < (connIndex + size); tmpConnIndex++) {
                BaseConn *baseConn=new AppConn(typeCollection, actionCollection, address, port, clientId, tmpConnIndex, registrable);
                if(baseConn->getConnState()==BaseConn::ONLINE) setClientState(ONLINE);
                baseConnCache.add(baseConn);
            }
            if(failsafe) {
                failsafeTimerTask=new FailsafeTimerTask(this);
                failsafeTimer=new Timer();
                failsafeTimer->scheduleAtFixedRate(failsafeTimerTask, FAILSAFE_PERIOD, FAILSAFE_PERIOD);
            }
            else {
                failsafeTimer=NULL;
                failsafeTimerTask=NULL;
            }
        }
        else throw Exception("properties not well defined", __FILE__, __LINE__);
    }

    ~AppClient() {
        if(failsafeTimer!=NULL) delete failsafeTimer;
        if(failsafeTimerTask!=NULL) delete failsafeTimerTask;
        clear();
    }

    /**
     * Clears the appClient.
     */
    void clear() {
        setClientState(OFFLINE);
        std::vector<BaseConn*> baseConnVector;
        baseConnCache.getVector(&baseConnVector);
        for(unsigned int i=0; i < baseConnVector.size(); i++) delete baseConnVector[i];
    }

private:

    /**
     * Sets the properties, returns true if all properties were properly defined.
     * @param *properties Properties
     * @return bool
     */
    bool setProperties(Properties *properties) {
        if(properties!=NULL) {

            // address
            address=properties->getProperty(ADDRESS);
            if(address.length()==0) {
                std::cerr << ADDRESS + " not well defined" << std::endl;
                return false;
            }

            // port
            std::stringstream sstream;
            sstream << BaseConn::DEFAULT_PORT;
            std::string tmpPort=properties->getProperty(PORT, sstream.str());
            if(tmpPort.length() > 0) port=atoi(tmpPort.c_str());
            else {
                std::cerr << PORT + " not well defined" << std::endl;
                return false;
            }

            // connIndex
            std::string tmpConnIndex=properties->getProperty(CONN_INDEX, "0");
            if(tmpConnIndex.length() > 0) connIndex=atoi(tmpConnIndex.c_str());
            else {
                std::cerr << CONN_INDEX + " not well defined" << std::endl;
                return false;
            }

            // size
            std::string tmpSize=properties->getProperty(SIZE, "8");
            if(tmpSize.length() > 0) size=atoi(tmpSize.c_str());
            else {
                std::cerr << SIZE + " not well defined" << std::endl;
                return false;
            }

            // registrable
            std::string tmpRegistrable=properties->getProperty(REGISTRABLE, "true");
            if(tmpRegistrable.compare("true")==0) registrable=true;
            else registrable=false;

            // failsafe
            std::string tmpFailsafe=properties->getProperty(FAILSAFE, "true");
            if(tmpFailsafe.compare("true")==0) failsafe=true;
            else failsafe=false;

            return true;
        }
        else {
            std::cerr << "NULL properties" << std::endl;
            return false;
        }
    }

public:

    /**
     * Returns the next online baseConn, otherwise NULL.
     * @return *BaseConn
     */
    BaseConn* nextOnline() {
        return baseConnCache.nextOnline();
    }

    /**
     * Returns the clientId.
     * @return int
     */
    int getClientId() {
        return clientId;
    }

    /**
     * Sets the clientState.
     * @param clientState int
     */
    void setClientState(int clientState) {
        boost::lock_guard<boost::mutex> lock(clientStateMutex);
        this->clientState=clientState;
    }

    /**
     * Returns the clientState.
     * @return int
     */
    int getClientState() {
        boost::lock_guard<boost::mutex> lock(clientStateMutex);
        return clientState;
    }

private:

    class FailsafeTimerTask : public TimerTask {

        /** The appClient. */
        AppClient *appClient;

    public:

        /**
         * Creates a new instance of FailsafeTimerTask.
         * @param *appClient AppClient
         */
        FailsafeTimerTask(AppClient *appClient) {
            this->appClient=appClient;
        }

    private:

        void run() {
            bool online=false;
            std::vector<BaseConn*> baseConnVector;
            appClient->baseConnCache.getVector(&baseConnVector);
            for(unsigned int i=0; i < baseConnVector.size(); i++) {
                BaseConn *baseConn=baseConnVector[i];
                if(baseConn->getConnState()!=BaseConn::ONLINE) {
                    BaseConn *tmpBaseConn=new AppConn(baseConn->getTypeCollection(), baseConn->getActionCollection(), appClient->address, appClient->port, appClient->clientId, baseConn->getConnIndex(), appClient->registrable);
                    if(tmpBaseConn->getConnState()==BaseConn::ONLINE) {
                        appClient->baseConnCache.add(tmpBaseConn);
                        online=true;
                    }
                    else delete tmpBaseConn;
                }
                else online=true;
            }
            if(online) appClient->setClientState(AppClient::ONLINE);
            else appClient->setClientState(AppClient::OFFLINE);
        }

    };

};

int AppClient::OFFLINE=0;
int AppClient::ONLINE=1;
std::string AppClient::ADDRESS="address";
std::string AppClient::PORT="port";
std::string AppClient::CONN_INDEX="connIndex";
std::string AppClient::SIZE="size";
std::string AppClient::REGISTRABLE="registrable";
std::string AppClient::FAILSAFE="failsafe";
int AppClient::FAILSAFE_PERIOD=8000;

#endif	/* _APPCLIENT_HPP */

