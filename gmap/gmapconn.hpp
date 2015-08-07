/* 
 * File:   gmapconn.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2011
 */

#ifndef GMAPCONN_HPP
#define	GMAPCONN_HPP

#include "../base/cqueue.hpp"
#include "../base/exception.hpp"
#include "../base/executor.hpp"
#include "../base/logger.hpp"
#include "../base/properties.hpp"
#include "../base/thread.hpp"
#include "../base/timer.hpp"

#include <apiinc.h>
#include <sccp-addr.h>
#include <gmap.h>

#include <iostream>

/**
 * <p>The Dialog class should be implemented to handle GMAP dialogs.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 07-05-2010
 */
class Dialog {

    /** The dialogId. */
    int dialogId;

    /** The invokeId. */
    int invokeId;

    /** The dialogState. */
    unsigned char dialogState;

    /** The dialogStateMutex. */
    boost::mutex dialogStateMutex;

    /** The cTime. */
    time_t cTime;

public:

    /* constants for state */
    static unsigned char NEW;
    static unsigned char OPEN;
    static unsigned char W_OPEN;
    static unsigned char INVOKE;
    static unsigned char W_INVOKE;
    static unsigned char CLOSE_0;
    static unsigned char CLOSE_1;
    static unsigned char W_CLOSE_0;
    static unsigned char W_CLOSE_1;
    static unsigned char ABORT_0;
    static unsigned char ABORT_1;

    /* constants */
    static int TIME_10S;
    static int TIME_20S;
    static int TIME_60S;
    static int TIME_120S;
    static int DEFAULT_TIME_TO_CHECK;

    /**
     * Creates a new instance of Dialog.
     */
    Dialog() {
        dialogId=0;
        invokeId=0;
        dialogState=0;
        time(&cTime);
        cTime+=DEFAULT_TIME_TO_CHECK;
    }

    /**
     * Creates a new instance of Dialog.
     * @param &timeToLive const int
     */
    Dialog(const int &timeToLive) {
        dialogId=0;
        invokeId=0;
        dialogState=0;
        time(&cTime);
        cTime+=timeToLive;
    }

    virtual ~Dialog() {}

    /**
     * Initializes the dialog.
     */
    virtual void init()=0;

    /**
     * Checks the dialog.
     */
    virtual void check()=0;

    /**
     * Implement this method to handle dialog gblock.
     * @param *gb gblock_t
     */
    virtual void handleGBlock(gblock_t *gb)=0;

    /**
     * Implement this method for dialog processing.
     * @return void
     */
    virtual void run()=0;

    /**
     * Sets the dialogId.
     * @param dialogId
     */
    void setDialogId(const int &dialogId) {
        this->dialogId=dialogId;
    }

    /**
     * Returns the dialogId.
     * @return int
     */
    int getDialogId() {
        return dialogId;
    }

    /**
     * Sets the invokeId.
     * @param invokeId
     */
    void setInvokeId(const int &invokeId) {
        this->invokeId=invokeId;
    }

    /**
     * Returns the invokeId.
     * @return int
     */
    int getInvokeId() {
        return invokeId;
    }

    /**
     * Sets the dialogState.
     * @param &dialogState const unsigned char
     */
    void setDialogState(const unsigned char &dialogState) {
        boost::lock_guard<boost::mutex> lock(dialogStateMutex);
        if(this->dialogState!=CLOSE_0 &&
           this->dialogState!=CLOSE_1 &&
           this->dialogState!=ABORT_0 &&
           this->dialogState!=ABORT_1)
            this->dialogState=dialogState;
        else if(this->dialogState==CLOSE_0 && dialogState==CLOSE_1) this->dialogState=dialogState;
        else if(this->dialogState==ABORT_0 && dialogState==ABORT_1) this->dialogState=dialogState;
    }

    /**
     * Returns the dialogState.
     * @return unsigned char
     */
    unsigned char getDialogState() {
        boost::lock_guard<boost::mutex> lock(dialogStateMutex);
        return dialogState;
    }

    /**
     * Sets the cTime.
     * @param &cTime const time_t
     */
    void setCTime(const time_t &cTime) {
        this->cTime=cTime;
    }

    /**
     * Returns the cTime.
     * @return time_t
     */
    time_t getCTime() {
        return cTime;
    }

};

unsigned char Dialog::NEW=0;
unsigned char Dialog::OPEN=1;
unsigned char Dialog::W_OPEN=2;
unsigned char Dialog::INVOKE=3;
unsigned char Dialog::W_INVOKE=4;
unsigned char Dialog::CLOSE_0=5;
unsigned char Dialog::CLOSE_1=6;
unsigned char Dialog::W_CLOSE_0=7;
unsigned char Dialog::W_CLOSE_1=8;
unsigned char Dialog::ABORT_0=9;
unsigned char Dialog::ABORT_1=10;

int Dialog::TIME_10S=10;
int Dialog::TIME_20S=20;
int Dialog::TIME_60S=60;
int Dialog::TIME_120S=120;
int Dialog::DEFAULT_TIME_TO_CHECK=Dialog::TIME_20S;

/**
 * <p>The DialogRun class.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 07-07-2010
 */
class DialogRun : public Thread {

    /** The dialog. */
    Dialog *dialog;

public:

    /**
     * Creates a new instance of DialogRun.
     * @param *dialog Dialog
     */
    DialogRun(Dialog *dialog) {
        this->dialog=dialog;
    }

    virtual ~DialogRun() {}

private:

    void run() {
        dialog->run();
    }

};

/**
 * <p>The DialogCache class.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 07-06-2010
 */
class DialogCache {

    static int CHECK_PERIOD;

    /** The dialogMap. */
    std::map<int, Dialog*> dialogMap;
    boost::mutex dialogMapMutex;

    /** The checkTimer. */
    Timer *checkTimer;
    TimerTask *checkTimerTask;

public:

    /**
     * Creates a new instance of DialogCache.
     */
    DialogCache() {
        checkTimer=new Timer();
        checkTimerTask=new CheckTimerTask(this);
        checkTimer->scheduleAtFixedRate(checkTimerTask, CHECK_PERIOD, CHECK_PERIOD);
    }

    ~DialogCache() {
        delete checkTimer;
        delete checkTimerTask;
        std::map<int, Dialog*>::iterator iter;
        for(iter=dialogMap.begin(); iter!=dialogMap.end(); iter++) delete iter->second;
    }

    /**
     * Returns the dialog for the given dialogId.
     * @param &dialogId const int
     * @return *Dialog
     */
    Dialog* get(const int &dialogId) {
        boost::lock_guard<boost::mutex> lock(dialogMapMutex);
        std::map<int, Dialog*>::iterator iter;
        iter=dialogMap.find(dialogId);
        if(iter!=dialogMap.end()) return iter->second;
        else return NULL;
    }

    /**
     * Puts a dialog.
     * @param *dialog Dialog
     */
    void put(Dialog *dialog) {
        boost::lock_guard<boost::mutex> lock(dialogMapMutex);
        std::map<int, Dialog*>::iterator iter;
        iter=dialogMap.find(dialog->getDialogId());
        if(iter!=dialogMap.end()) {
            dialogMap.erase(iter);
            delete iter->second;
        }
        dialogMap.insert(std::pair<int, Dialog*>(dialog->getDialogId(), dialog));
    }

    /**
     * Removes the dialog for the given dialogId.
     * @param &dialogId const int
     */
    void remove(const int &dialogId) {
        boost::lock_guard<boost::mutex> lock(dialogMapMutex);
        std::map<int, Dialog*>::iterator iter;
        iter=dialogMap.find(dialogId);
        if(iter!=dialogMap.end()) {
            Dialog *dialog=iter->second;
            dialogMap.erase(iter);
            delete dialog;
        }
    }

private:

    /**
     * Checks all dialogs.
     */
    void check() {
        std::map<int, Dialog*> tmpDialogMap;
        {
            boost::lock_guard<boost::mutex> lock(dialogMapMutex);
            tmpDialogMap=dialogMap;
        }
        time_t cTime;
        time(&cTime);
        std::map<int, Dialog*>::iterator iter;
        iter=tmpDialogMap.begin();
        while(iter!=tmpDialogMap.end()) {
            Dialog *dialog=iter->second;
            if(dialog->getCTime() < cTime) {
                if(dialog->getDialogState()==Dialog::CLOSE_1 || dialog->getDialogState()==Dialog::ABORT_1) {
                    remove(iter->first);
                }
                else {
                    dialog->check();
                    dialog->setCTime(cTime + Dialog::TIME_10S);
                }
            }
            iter++;
        }
    }

    class CheckTimerTask : public TimerTask {

        /** The dialogCache. */
        DialogCache *dialogCache;

    public:

        /**
         * Creates a new instance of CheckTimerTask.
         * @param *dialogCache DialogCache
         */
        CheckTimerTask(DialogCache *dialogCache) {
            this->dialogCache=dialogCache;
        }

    private:

        void run() {
            dialogCache->check();
        }

    };

};

int DialogCache::CHECK_PERIOD=16000;

static char hexCh[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

/**
 * <p>The GBlock class should be used to handle gblocks.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 07-05-2010
 */
class GBlock {

public:

    /* constants for address */
    static unsigned char INTERNATIONAL_ISDN;
    static unsigned char NATIONAL_ISDN;

private:

    /* private variables */
    Dialog *dialog;
    unsigned short bitmask;
    ServiceType serviceType;
    int serviceMsg;
    int applicationId;
    int linkedId;

protected:

    /**
     * Creates a new instance of GBlock.
     * @param *dialog Dialog
     * @param &bitmask const unsigned short
     * @param &serviceType ServiceType
     * @param &serviceMsg const int
     * @param &applicationId const int
     * @param &linkedId const int
     */
    GBlock(Dialog *dialog,
           const unsigned short &bitmask,
           const ServiceType &serviceType,
           const int &serviceMsg,
           const int &applicationId,
           const int &linkedId) {
        this->dialog=dialog;
        this->bitmask=bitmask;
        this->serviceType=serviceType;
        this->serviceMsg=serviceMsg;
        this->applicationId=applicationId;
        this->linkedId=linkedId;
    }

public:

    virtual ~GBlock() {}

    /**
     * Returns the gblock.
     * @param *gb gblock_t
     */
    virtual void getGBlock(gblock_t *gb) {
        gb->bit_mask=bitmask;
        gb->serviceType=serviceType;
        gb->serviceMsg=serviceMsg;
        gb->dialogId=dialog->getDialogId();
        gb->applicationId=applicationId;
        gb->invokeId=dialog->getInvokeId();
        gb->linkedId=linkedId;
    }

    /**
     * Returns the dialog.
     * @return *Dialog
     */
    Dialog* getDialog() {
        return dialog;
    }

    /**
     * Return the string for the unsigned char array.
     * @param *ch unsigned char
     * @param &len const int
     * @return std::string
     */
    static std::string getHexString(unsigned char *ch, const int &len) {
        std::string str="";
        for(int chIndex=0; chIndex < len; chIndex++) {
            str+=hexCh[(ch[chIndex] >> 4) & 0x0f];
            str+=hexCh[ch[chIndex] & 0x0f];
        }
        return str;
    }

    /**
     * Returns the octet for the given char.
     * @param ch const char
     * @return char
     */
    static char getHexO(const char ch) {
        if(isxdigit(ch)) {
            if(ch > 0x2f && ch < 0x3a) return ch - 0x30;
            else {
                if((ch & 0xf)==1) return 0xa;
                else if((ch & 0xf)==2) return 0xb;
                else if((ch & 0xf)==3) return 0xc;
                else if((ch & 0xf)==4) return 0xd;
                else if((ch & 0xf)==5) return 0xe;
                else if((ch & 0xf)==6) return 0xf;
            }
        }
        return -1;
    }

    /**
     * Returns the octet.
     * @param &str const std::string
     * @param *o char
     * @param &len int
     */
    static void getOctet(const std::string &str, unsigned char *o, int &len) {
        len=str.length();
        for(unsigned int strIndex=0; strIndex < str.length(); strIndex++) {
            char tmpO=getHexO(str.at(strIndex));
            if(tmpO!=-1) o[strIndex]=tmpO;
        }
    }

    /**
     * Returns the string for a octet.
     * @param *o unsigned char
     * @param &len const int
     * @return std::string
     */
    static std::string getOctetStr(unsigned char *o, const int &len) {
        std::string str="";
        for(int chIndex=0; chIndex < len; chIndex++) str+=hexCh[o[chIndex] & 0x0f];
        return str;
    }

    /**
     * Returns the semi octet.
     * @param &str const std::string
     * @param *so unsigned char
     * @param &len int
     */
    static void getSemiOctet(const std::string &str, unsigned char *so, int &len) {
        len=str.length() >> 1;
        if((str.length() & 1)==1) len++;
        for(unsigned char soIndex=0; soIndex < len; soIndex++) {
            char tmpO=getHexO(str.at(soIndex << 1));
            if(tmpO!=-1) so[soIndex]=tmpO;
            if(((soIndex << 1) + 1) < (int)str.length()) {
                tmpO=getHexO(str.at((soIndex << 1) + 1));
                if(tmpO!=-1) so[soIndex]|=tmpO << 4;
            }
            else so[soIndex]|=0xf0;
        }
    }

    /**
     * Return the string for a semi octet.
     * @param *so unsigned char
     * @param &len const int
     * @return std::string
     */
    static std::string getSemiOctetStr(unsigned char *so, const int &len) {
        std::string str="";
        for(int soIndex=0; soIndex < len; soIndex++) {
            int ls=so[soIndex] & 0x0f;
            int ms=(so[soIndex] >> 4) & 0x0f;
            if(ls < 0x0f) str=str + hexCh[ls];
            if(ms < 0x0f) str=str + hexCh[ms];
        }
        return str;
    }

    /**
     * Returns the address.
     * @param &str const std::string
     * @param *addr unsigned char
     * @param &len int
     */
    static void getAddress(const std::string &str, const unsigned char &toa, unsigned char *addr, int &len) {
        addr[0]=toa;
        getSemiOctet(str, &addr[1], len);
        ++len;
    }

    /**
     * Returns the string for an address.
     * @param *addr unsigned char
     * @param &len const int
     */
    static void getAddressStr(unsigned char *addr, const int &len, std::string &addrStr, unsigned char &toa) {
        addrStr=getSemiOctetStr(addr + 1, len - 1);
        toa=addr[0];
    }

    /**
     * Returns the natureOfAddres for a given type.
     * @param &type const unsigned char
     * @return NatureOfAddress
     */
    static NatureOfAddress getNatureOfAddress(const unsigned char &type) {
        unsigned char ton=type & 0x70;
        if(ton==16) return msisdnInternationalNumber;
        else if(ton==32) return msisdnNationalSignificantNumber;
        else if(ton==64) return msisdnSubscriberNumber;
        else return natureOfAddressNotPresent;
    }

    /**
     * Returns the numberingPlan for a given type.
     * @param &type const unsigned char
     * @return NumberingPlan
     */
    static NumberingPlan getNumberingPlan(const unsigned char &type) {
        return e164;
    }

};

unsigned char GBlock::INTERNATIONAL_ISDN=0x91;
unsigned char GBlock::NATIONAL_ISDN=0xa1;

/**
 * <p>The PutGBlockQueue class should be used to put GMAP gblocks.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 07-05-2010
 */
class PutGBlockQueue : public Thread {

    /** The gBlockQueue. */
    Queue<GBlock> gBlockQueue;

    /** The dialogCache. */
    DialogCache *dialogCache;

    /* private variables */
    gblock_t gb;

public:

    /**
     * Creates a new instance of PutGBlockQueue.
     * @param *dialogCache DialogCache
     */
    PutGBlockQueue(DialogCache *dialogCache) {
        this->dialogCache=dialogCache;
        start();
    }

    /**
     * Pushes a gBlock.
     * @param *gBlock GBlock
     */
    void push(GBlock *gBlock) {
        gBlockQueue.push(gBlock);
    }

private:

    void run();

};

boost::mutex gMAPMutex;

/**
 * <p>The GMAPConn class should be extended to implement a GMAP application.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 07-03-2010
 */
class GMAPConn : public Thread {

public:

    /* constants for connState */
    static int OFFLINE;
    static int ONLINE;

    /* constants for properties */
    static std::string LOGICAL_NAME;
    static std::string LOCAL_PC;
    static std::string REMOTE_PC;
    static std::string LOCAL_GT;
    static std::string LOCAL_GT_TYPE;
    static std::string LOCAL_SSN;
    static std::string N_DIALOGS;
    static std::string N_INVOKES;
    static std::string NODE_NAME;
    static std::string STAND_ALONE;

private:

    /** appId. */
    int appId;

protected:

    /** The gMAPInit. */
    MAP_Init gMAPInit;

    /* properties */
    std::string logicalName;
    unsigned int localPC;
    unsigned int remotePC;
    std::string localGT;
    unsigned char localGTType;
    bool standAlone;

    /** The dialogCache. */
    DialogCache *dialogCache;

private:

    /** The putGBlockQueue. */
    PutGBlockQueue *putGBlockQueue;

    /** The dialogRunService. */
    Executor *dialogRunService;

    /** The connState. */
    static unsigned char connState;

    /* private variables */
    gblock_t gb;

public:

    /**
     * Creates a new instance of GMAPConn.
     * @param argc int
     * @param **argv char
     * @param *properties Properties
     * @throws Exception
     */
    GMAPConn(int argc, char** argv, Properties *properties) throw(Exception) {
        dialogCache=NULL;
        putGBlockQueue=NULL;
        dialogRunService=NULL;
        // properties
        if(setProperties(properties)) {
            try {
                gMAPInit.protocol=itu7;
                gMAPInit.debugFile=stdout;

                if(standAlone) {
                    // FtAttach
                    if(FtAttach(logicalName.c_str(),    // process logical name
                                argv[0],                // process executable name
                                " ",                    // execution parameters
                                0,                      // execution priority
                                0,                      // RT time quantum
                                0,                      // RT time quantum
                                0,                      // process class identifier
                                10)==-1) {              // max. wait for CPT entry
                        std::stringstream sstream;
                        sstream << "FTAttach() failed, errno=" << errno + "(" << LastErrorReport << ")";
                        throw Exception(sstream.str(), __FILE__, __LINE__);
                    }
                }

                // FtRegister
                if(FtRegister(argc,             // command Line Argument count
                              argv,             // command Line Arguments
                              FALSE,            // Debug Printouts Required ?
                              FALSE,            // msg Activity Monitor Required ?
                              TRUE,             // Ipc Queue Required ?
                              TRUE,             // flush Ipc Queue Before Start ?
                              FALSE,            // allow Ipc Msg Queueing Always
                              TRUE,             // process Has SIGINT Handler
                              (U16)AUTOrestart, // automatic Restart allowed ?
                              0,                // process Class Designation
                              0,                // initial Process State Declaration
                              0,                // event Distribution Filter Value
                              10)==-1) {        // retry
                    std::stringstream sstream;
                    sstream << "FtRegister() failed, errno=" << errno + "(" << LastErrorReport << ")";
                    throw Exception(sstream.str(), __FILE__, __LINE__);
                }

                // FtAssignHandler
                if(FtAssignHandler(SIGINT, GMAPConn::sigintHandler)==RETURNerror) {
                    std::stringstream sstream;
                    sstream << "cannot assign SIGINT handler, errno=" << errno;
                    throw Exception(sstream.str(), __FILE__, __LINE__);
                }

                // SYSattach
                int gAliasNameIndex=SYSattach(gMAPInit.nodeName, FALSE);
                if(gAliasNameIndex==RETURNerror) {
                    std::stringstream sstream;
                    sstream << "SYSattach() failed, errno=" << errno + "(" << LastErrorReport << ")";
                    throw Exception(sstream.str(), __FILE__, __LINE__);
                }

                // SYSbind
                if(SYSbind(gAliasNameIndex,
                           FALSE,               // non-designatable
                           MTP_SCCP_TCAP_USER,
                           gMAPInit.ssn,
                           SCCP_TCAP_CLASS)!=RETURNok) {
                    std::stringstream sstream;
                    sstream << "SYSbind() failed, errno=" << errno + "(" << LastErrorReport << ")";
                    throw Exception(sstream.str(), __FILE__, __LINE__);
                }

                // gMAPInitialize
                appId=gMAPInitialize(&gMAPInit, argc, argv);
                if(appId==-1) {
                    std::stringstream sstream;
                    sstream << "gMAPInitialize() failed, errno=" << gMAPInit.error + "(" << gMAPInit.errorReport << ")";
                    throw Exception(sstream.str(), __FILE__, __LINE__);
                }
                else {
                    std::stringstream sstream;
                    sstream << "gMAPInitialize(), appId=" << appId;
                    Logger::getLogger()->logp(&Level::INFO, "GMAPConn", "<init>", sstream.str());
                }

                // CscUIS
                CscUIS(gAliasNameIndex, gMAPInit.ssn);

                // dialogCache
                dialogCache=new DialogCache();
                // dialogRunService
                dialogRunService=new Executor();

                start();
                setConnState(ONLINE);

                // putGBlockQueue
                putGBlockQueue=new PutGBlockQueue(dialogCache);
            }
            catch(Exception e) {
                Logger::getLogger()->logp(&Level::SEVERE, "GMAPConn", "<init>", e.toString());
                clear();
            }
        }
        else throw Exception("properties not well defined", __FILE__, __LINE__);
    }

    virtual ~GMAPConn() {
        clear();
        if(dialogRunService!=NULL) delete dialogRunService;
        if(putGBlockQueue!=NULL) delete putGBlockQueue;
        if(dialogCache!=NULL) delete dialogCache;
    }

    /**
     * Clears the GMAPConn.
     */
    void clear() {
        setConnState(OFFLINE);
        FtTerminate(NOrestart, 1);
    }

private:

    void run() {
        try {
            // FtThreadRegister
            if(FtThreadRegister()==RETURNok) {
                union {
                    Header_t hdr;
                    cblock_t cblock;
                    char ipc[MAXipcBUFFERsize];
                } ipc;
                while(getConnState()==ONLINE) {
                    if(FtGetIpcEx(&ipc.hdr,
                                  0,                    // any message type
                                  sizeof(ipc),          // max. size to rcv
			          TRUE,                 // truncate if large
			          TRUE,                 // blocking read
			          TRUE)==RETURNerror) { // interruptible
                        if(errno!=EINTR) {              // not interrupt
                            std::stringstream sstream;
                            sstream << "FtGetIpcEx() failed, errno=" << errno;
                            Logger::getLogger()->logp(&Level::SEVERE, "GMAPConn", "run", sstream.str());
                        }
                    }
                    else {
                        switch(ipc.hdr.messageType) {
                            case N_NOTICE_IND:
                            case N_UNITDATA_IND: {
                                boost::lock_guard<boost::mutex> lock(gMAPMutex);
                                gMAPTakeMsg(&ipc.cblock);
                            }
                                break;
                            case N_STATE_IND: {
                                std::stringstream sstream;
                                scmg_nstate_t *nstate;
                                nstate=&((iblock_t *)&ipc.hdr)->primitives.nstate;
                                sstream << "N_STATE_IND PC=" << nstate->NS_affect_pc << " SSN=" << (int)nstate->NS_affect_ssn << " ";
                                if(nstate->NS_user_status==SCMG_UIS) sstream << "UIS";
                                else if(nstate->NS_user_status==SCMG_UOS) sstream << "UOS";
                                Logger::getLogger()->logp(&Level::WARNING, "GMAPConn", "run", sstream.str());
                            }
                                break;
                            case N_PCSTATE_IND: {
                                std::stringstream sstream;
                                scmg_pcstate_t *pcstate;
                                pcstate=&((iblock_t *)&ipc.hdr)->primitives.pcstate;
                                sstream << "N_PCSTATE_IND PC=" << pcstate->pc_pc << " ";
                                if(pcstate->pc_status==SCMG_INACCESSABLE) sstream << "INACCESSABLE";
                                else if(pcstate->pc_status==SCMG_ACCESSABLE) sstream << "ACCESSABLE";
                                Logger::getLogger()->logp(&Level::WARNING, "GMAPConn", "run", sstream.str());
                            }
                                break;
                            case TAP_STATE_CHANGE:
                                Logger::getLogger()->logp(&Level::INFO, "GMAPConn", "run", "TAP_STATE_CHANGE received");
                                break;
                            default: {
                                std::stringstream sstream;
                                sstream << "unknown ipc messageType received " << ipc.hdr.messageType;
                                Logger::getLogger()->logp(&Level::WARNING, "GMAPConn", "run", sstream.str());
                            }
                                break;
                        }
                    }
                    {
                        boost::lock_guard<boost::mutex> lock(gMAPMutex);
                        while(gMAPGetGBlock(&gb)==0) {
                            handleGBlock(&gb);
                        }
                    }
                }
            }
            else {
                std::stringstream sstream;
                sstream << "FtThreadRegister() failed, errno=" << errno;
                throw Exception(sstream.str(), __FILE__, __LINE__);
            }
        }
        catch(Exception e) {
            Logger::getLogger()->logp(&Level::SEVERE, "GMAPConn", "run", e.toString());
        }
        clear();
        // FtThreadUnregister
        FtThreadUnregister();
    }

    /**
     * Sets the properties, returns true if all properties were properly defined.
     * @param *properties Properties
     * @return bool
     */
    bool setProperties(Properties *properties) {
        if(properties!=NULL) {

            // logicalName
            logicalName=properties->getProperty(LOGICAL_NAME);
            if(logicalName.length()==0) {
                std::cerr << LOGICAL_NAME + " not well defined" << std::endl;
                return false;
            }

            // localPC
            std::string tmpLocalPC=properties->getProperty(LOCAL_PC);
            if(tmpLocalPC.length() > 0) localPC=atoi(tmpLocalPC.c_str());
            else {
                std::cerr << LOCAL_PC + " not well defined" << std::endl;
                return false;
            }

            // remotePC
            std::string tmpRemotePC=properties->getProperty(REMOTE_PC);
            if(tmpRemotePC.length() > 0) remotePC=atoi(tmpRemotePC.c_str());
            else {
                std::cerr << REMOTE_PC + " not well defined" << std::endl;
                return false;
            }

            // localGT
            localGT=properties->getProperty(LOCAL_GT);
            if(localGT.length()==0) {
                std::cerr << LOCAL_GT + " not well defined" << std::endl;
                return false;
            }

            // localGTType
            std::string tmpLocalGTType=properties->getProperty(LOCAL_GT_TYPE, "145");
            if(tmpLocalGTType.length() > 0) localGTType=atoi(tmpLocalGTType.c_str());
            else {
                std::cerr << LOCAL_GT_TYPE + " not well defined" << std::endl;
                return false;
            }

            // localSSN
            std::string tmpLocalSSN=properties->getProperty(LOCAL_SSN, "8");
            if(tmpLocalSSN.length() > 0) gMAPInit.ssn=(U8)atoi(tmpLocalSSN.c_str());
            else {
                std::cerr << LOCAL_SSN + " not well defined" << std::endl;
                return false;
            }

            // nDialogs
            std::string tmpNDialogs=properties->getProperty(N_DIALOGS, "16384");
            if(tmpNDialogs.length() > 0) gMAPInit.nDialogs=atoi(tmpNDialogs.c_str());
            else {
                std::cerr << N_DIALOGS + " not well defined" << std::endl;
                return false;
            }

            // nInvokes
            std::string tmpNInvokes=properties->getProperty(N_INVOKES, "16384");
            if(tmpNInvokes.length() > 0) gMAPInit.nInvokes=atoi(tmpNInvokes.c_str());
            else {
                std::cerr << N_INVOKES + " not well defined" << std::endl;
                return false;
            }

            // nodeName
            std::string nodeName=properties->getProperty(NODE_NAME);
            if(nodeName.length() > 0) strcpy(gMAPInit.nodeName, nodeName.c_str());
            else {
                std::cerr << NODE_NAME + " not well defined" << std::endl;
                return false;
            }

            // standAlone
            std::string tmpStandAlone=properties->getProperty(STAND_ALONE, "false");
            if(tmpStandAlone.compare("true")==0) standAlone=true;
            else standAlone=false;

            return true;
        }
        else {
            std::cerr << "NULL properties" << std::endl;
            return false;
        }
    }

protected:

    /**
     * Handles the gblock.
     * @param *gb gblock_t
     */
    virtual void handleGBlock(gblock_t *gb)=0;

public:

    /**
     * Submits a dialog for run.
     * @param *dialog Dialog
     */
    void submit(Dialog *dialog) {
        if(dialogRunService!=NULL) dialogRunService->submit(new DialogRun(dialog));
    }

    /**
     * Pushes a gBlock.
     * @param *gBlock GBlock
     */
    void push(GBlock *gBlock) {
        if(putGBlockQueue!=NULL) putGBlockQueue->push(gBlock);
    }

    /**
     * Returns the gMAPInit.
     * @return MAP_Init
     */
    MAP_Init getMAPInit() {
        return gMAPInit;
    }

    /**
     * Returns the logicalName.
     * @return std::string
     */
    std::string getLogicalName() {
        return logicalName;
    }

    /**
     * Returns the localPC.
     * @return unsigned int
     */
    unsigned int getLocalPC() {
        return localPC;
    }

    /**
     * Returns the remotePC.
     * @return unsigned int
     */
    unsigned int getRemotePC() {
        return remotePC;
    }

    /**
     * Returns the localGT.
     * @return std::string
     */
    std::string getLocalGT() {
        return localGT;
    }

    /**
     * Returns the localGTType.
     * @return unsigned char
     */
    unsigned char getLocalGTType() {
        return localGTType;
    }

private:

    /**
     * Sets the connState.
     * @param &connState const unsigned char
     */
    static void setConnState(const unsigned char &connState) {
        GMAPConn::connState=connState;
    }

public:

    /**
     * Returns the connState.
     * @return unsigned char
     */
    static unsigned char getConnState() {
        return connState;
    }

    /**
     * The sigintHandler.
     */
    static void sigintHandler() {
        if(getConnState()==ONLINE) setConnState(OFFLINE);
        else exit(0);
    }

};

int GMAPConn::OFFLINE=0;
int GMAPConn::ONLINE=1;
unsigned char GMAPConn::connState=OFFLINE;

std::string GMAPConn::LOGICAL_NAME="logicalName";
std::string GMAPConn::LOCAL_PC="localPC";
std::string GMAPConn::REMOTE_PC="remotePC";
std::string GMAPConn::LOCAL_GT="localGT";
std::string GMAPConn::LOCAL_GT_TYPE="localGTType";
std::string GMAPConn::LOCAL_SSN="localSSN";
std::string GMAPConn::N_DIALOGS="nDialogs";
std::string GMAPConn::N_INVOKES="nInvokes";
std::string GMAPConn::NODE_NAME="nodeName";
std::string GMAPConn::STAND_ALONE="standAlone";

void PutGBlockQueue::run() {
    try {
        // FtThreadRegister
        if(FtThreadRegister()==RETURNok) {
            while(GMAPConn::getConnState()==GMAPConn::ONLINE) {
                GBlock *gBlock=gBlockQueue.waitAndPop();
                gBlock->getGBlock(&gb);
                //gMAPPrintGBlock(gb);
                Dialog *dialog;
                dialog=gBlock->getDialog();
                if(dialog->getDialogState()!=Dialog::CLOSE_0 && dialog->getDialogState()!=Dialog::ABORT_0) {
                    int putGBlockRes=-1;
                    {
                        boost::lock_guard<boost::mutex> lock(gMAPMutex);
                        putGBlockRes=gMAPPutGBlock(&gb);
                    }
                    if(putGBlockRes==0) {
                        if(gb.serviceType==GMAP_REQ) {
                            if(gb.serviceMsg==GMAP_OPEN) {
                                dialog->setDialogId(gb.dialogId);
                                dialogCache->put(dialog);
                            }
                            else if(gb.serviceMsg==GMAP_CLOSE)
                                dialog->setDialogState(Dialog::CLOSE_0);
                        }
                    }
                    else {
                        std::stringstream sstream;
                        sstream << "gMAPPutGBlock failed " << gb.serviceType << " " << gb.serviceMsg << " ";
                        sstream << std::hex << dialog->getDialogId();
                        Logger::getLogger()->logp(&Level::SEVERE, "PutGBlockQueue", "run", sstream.str());
                        dialog->setDialogState(Dialog::ABORT_0);
                    }
                }
                delete gBlock;
            }
        }
        else {
            std::stringstream sstream;
            sstream << "FtThreadRegister() failed, errno=" << errno;
            throw Exception(sstream.str(), __FILE__, __LINE__);
        }
    }
    catch(Exception e) {
        Logger::getLogger()->logp(&Level::SEVERE, "PutGBlockQueue", "run", e.toString());
    }
    // FtThreadUnregister
    FtThreadUnregister();
}

#endif	/* GMAPCONN_HPP */
