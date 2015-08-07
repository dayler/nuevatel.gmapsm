#include "gmap/gmapsm.hpp"

class LoggerHandler : public Handler {

public:

    void publish(LogRecord *logRecord) {
        std::stringstream sstream;
        time_t seconds;
        seconds=logRecord->getSeconds();
        struct tm *timestamp=localtime(&seconds);
        sstream << (timestamp->tm_year + 1900) << "-" << (timestamp->tm_mon + 1) << "-" << timestamp->tm_mday << " " << timestamp->tm_hour << ":" << timestamp->tm_min << ":" << timestamp->tm_sec;
        sstream << " " << logRecord->getLevel()->getName();
        sstream << " " << logRecord->getSourceClass();
        sstream << " " << logRecord->getSourceMethod();
        sstream << " " << logRecord->getMessage();
        std::cout << sstream.str() << std::endl;
    }

    void flush() {}

};

/* constants for properties */
static std::string CLIENT_ID="clientId";

int main(int argc, char** argv) {

    LoggerHandler handler=LoggerHandler();
    Logger::getLogger()->setHandler(&handler);

    if(argc > 1) {
        Properties properties;
        properties.load(argv[1]);

        // clientId;
        int clientId;
        std::string tmpClientId=properties.getProperty(CLIENT_ID, "1");
        if(tmpClientId.length() > 0) clientId=atoi(tmpClientId.c_str());
        else {
            std::cerr << CLIENT_ID + " not well defined" << std::endl;
            exit(1);
        }

        // gMAPSM
        GMAPSM *gMAPSM;
        gMAPSM=NULL;
        // appClient
        AppClient *appClient;
        appClient=NULL;

        TypeCollection *typeCollection=new TypeCollection();
        ActionCollection *actionCollection=new ActionCollection();

        typeCollection->put(&FORWARD_MO_SM_REQUEST);
        typeCollection->put(&FORWARD_MO_SM_RESPONSE);

        typeCollection->put(&SEND_RI_F_SM_REQUEST);
        typeCollection->put(&SEND_RI_F_SM_RESPONSE);
        typeCollection->put(&SEND_RI_F_SM_ADVICE);

        typeCollection->put(&FORWARD_MT_SM_REQUEST);
        typeCollection->put(&FORWARD_MT_SM_RESPONSE);
        typeCollection->put(&FORWARD_MT_SM_ADVICE);

        try {
            // appClient
            appClient=new AppClient(clientId, &properties, typeCollection, actionCollection);
            // gMAPSM
            gMAPSM=new GMAPSM(argc, argv, &properties, appClient);
            int seconds=0;
            while(GMAPSM::getConnState()!=GMAPSM::ONLINE) {
                sleep(1);
                seconds++;
                if(seconds==10) break;
            }
            if(GMAPSM::getConnState()==GMAPSM::ONLINE) {
                SendRIFSMAction sendRIFSMAction(gMAPSM, appClient);
                ForwardMTSMAction forwardMTSMAction(gMAPSM, appClient);
                actionCollection->put(&sendRIFSMAction);
                actionCollection->put(&forwardMTSMAction);
                while(GMAPSM::getConnState()==GMAPSM::ONLINE) sleep(1);
            }
            else Logger::getLogger()->logp(&Level::SEVERE, "<void>", "main", "gMAPSM OFFLINE");
        }
        catch(Exception e) {
            Logger::getLogger()->logp(&Level::SEVERE, "<void>", "main", e.toString());
        }

        if(gMAPSM!=NULL) delete gMAPSM;
        if(appClient!=NULL) delete appClient;
        delete typeCollection;
        delete actionCollection;
    }
    // exit
    exit(1);
}
