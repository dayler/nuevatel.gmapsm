/*
 * File:   gmapsm.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2011
 */

#ifndef GMAPSM_HPP
#define	GMAPSM_HPP

#include "../base/appconn/appclient.hpp"
#include "gblocksm.hpp"

MessageType FORWARD_MO_SM_REQUEST(0xb0, 0xb1, &Group::REQUEST);
MessageType FORWARD_MO_SM_RESPONSE(0xb1, 0xb0, &Group::RESPONSE);

MessageType FORWARD_MT_SM_REQUEST(0xb2, 0x10, &Group::REQUEST);
MessageType FORWARD_MT_SM_RESPONSE(0x10, 0xb2, &Group::RESPONSE);
MessageType FORWARD_MT_SM_ADVICE(0xb3, 0xff, &Group::ADVICE);

MessageType SEND_RI_F_SM_REQUEST(0xb4, 0x11, &Group::REQUEST);
MessageType SEND_RI_F_SM_RESPONSE(0x11, 0xb4, &Group::RESPONSE);
MessageType SEND_RI_F_SM_ADVICE(0xb5, 0xff, &Group::ADVICE);

/* messageId */
unsigned char MESSAGE_ID=0xb0;        // IEByteArray
/* referenceId */
unsigned char REFERENCE_ID=0xb1;      // IEByteArray
/* type */
unsigned char TYPE=0x10;
/* action */
unsigned char ACTION=0x11;
/* msisdn */
unsigned char MSISDN=0xb2;            // IEByteArray, IEByte
/* tpdu */
unsigned char TPDU=0xb3;              // IEByteArray, IEByte
/* location */
unsigned char NODE_ID=0xb4;           // IEByteArray

unsigned char FROM_NAME=0xb5;         // IEByteArray, IEByte
unsigned char TO_NAME=0xb6;           // IEByteArray, IEByte

unsigned char SMRPPRI=0x12;
unsigned char REMOTE_GT=0xb7;         // IEByteArray, IEByte
unsigned char IE_IMSI=0xb8;           // IEByteArray
unsigned char IE_LMSI=0xb9;           // IEByteArray

/* type 4 bit 2 to 5 */
unsigned char SERVICE_TYPE=0x3c;
unsigned char SM=0x4;
unsigned char MM=0x8;
/* type 2 bit 0 and 1 */
unsigned char REQUEST_TYPE=0x3;
unsigned char O=0x0;
unsigned char T=0x1;

/* action 4 bit 0 to 3 */
unsigned char MESSAGE_ACTION=0x7;
unsigned char ACCEPT=0x1;
unsigned char END=0x3;

/* constants for tpdu type */
unsigned char SMS_DELIVER=0;
unsigned char SMS_DELIVER_REPORT=1;
unsigned char SMS_STATUS_REPORT=2;
unsigned char SMS_COMMAND=3;
unsigned char SMS_SUBMIT=4;
unsigned char SMS_SUBMIT_REPORT=5;
unsigned char RESERVED=6;

/**
 * <p>The ForwardMOSM class should be used to handle MAP_MO_FORWARD_SHORT_MESSAGE dialog.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 05-30-2010
 */
class ForwardMOSM : public Dialog {

    /** The gMAPConn. */
    GMAPConn *gMAPConn;

    /** The appClient. */
    AppClient *appClient;

    /* private variables */
    unsigned char type;
    std::string sca;
    unsigned char scaType;
    std::string msisdn;
    unsigned char msisdnType;
    std::string nodeId;
    unsigned char *ch;
    int len;
    unsigned char chType;

public:

    /**
     * Creates a new instance of ForwardMOSM.
     * @param *gMAPConn GMAPConn
     * @param *appClient AppClient
     */
    ForwardMOSM(GMAPConn *gMAPConn, AppClient *appClient) : Dialog() {
        this->gMAPConn=gMAPConn;
        this->appClient=appClient;
        type=SM | O;
        sca="";
        msisdn="";
        nodeId="";
        ch=NULL;
        len=0;
        chType=SMS_SUBMIT;
    }

    ~ForwardMOSM() {
        if(ch!=NULL) delete[] ch;
    }

    void init() {}

    void check() {
        if(getDialogState()==W_INVOKE) setDialogState(W_CLOSE_0);
        else if(getDialogState()==CLOSE_0) setDialogState(CLOSE_1);
        else if(getDialogState()==W_CLOSE_0) setDialogState(W_CLOSE_1);
        else if(getDialogState()==W_CLOSE_1) gMAPConn->push(new UAbortReqBlock(this));
        else if(getDialogState()==ABORT_0) setDialogState(ABORT_1);
    }

    void handleGBlock(gblock_t *gb) {
        if(gb->serviceType==GMAP_REQ) {
            if(gb->serviceMsg==GMAP_OPEN) {
                // nodeId
                if((gb->parameter.openArg.bit_mask & MAP_OpenArg_originatingAddress_present)==MAP_OpenArg_originatingAddress_present) {
                    if((gb->parameter.openArg.originatingAddress.bit_mask & MAP_SccpAddr_gt_present)==MAP_SccpAddr_gt_present) {
                        if(gb->parameter.openArg.originatingAddress.gt.msisdnLength > 0) nodeId=GBlock::getOctetStr(gb->parameter.openArg.originatingAddress.gt.msisdn, gb->parameter.openArg.originatingAddress.gt.msisdnLength);
                    }
                }
                gMAPConn->push(new OpenResBlock(this, dialogAccepted));
                setDialogState(W_INVOKE);
            }
            else if(gb->serviceMsg==MO_FORWARD_SM) {
                setInvokeId(gb->invokeId);
                // sca
                if(gb->parameter.forwardSM_Arg_v2.sm_RP_DA.choice==SM_RP_DA_t2_serviceCentreAddressDA_chosen)
                    if(gb->parameter.forwardSM_Arg_v2.sm_RP_DA.u.serviceCentreAddressDA.length > 0)
                        GBlock::getAddressStr(gb->parameter.forwardSM_Arg_v2.sm_RP_DA.u.serviceCentreAddressDA.value, gb->parameter.forwardSM_Arg_v2.sm_RP_DA.u.serviceCentreAddressDA.length, sca, scaType);
                // msisdn
                if(gb->parameter.forwardSM_Arg_v2.sm_RP_OA.choice==SM_RP_OA_t2_msisdn_chosen)
                    if(gb->parameter.forwardSM_Arg_v2.sm_RP_OA.u.msisdn.length > 0)
                        GBlock::getAddressStr(gb->parameter.forwardSM_Arg_v2.sm_RP_OA.u.msisdn.value, gb->parameter.forwardSM_Arg_v2.sm_RP_OA.u.msisdn.length, msisdn, msisdnType);
                // ch
                if(gb->parameter.forwardSM_Arg_v2.sm_RP_UI.length > 0) {
                    len=gb->parameter.forwardSM_Arg_v2.sm_RP_UI.length;
                    if(ch!=NULL) delete[] ch;
                    ch=new unsigned char[len];
                    for(int i=0; i < len; i++) ch[i]=gb->parameter.forwardSM_Arg_v2.sm_RP_UI.value[i];
                }
                setDialogState(INVOKE);
                gMAPConn->submit(this);
            }
            else if(gb->serviceMsg==ALERT_SERVICE_CENTRE_WITHOUT_RESULT) {
                setInvokeId(gb->invokeId);
                gMAPConn->push(new CloseReqBlock(this));
            }
            else if(gb->serviceMsg==GMAP_DELIMITER) {
                if(getDialogState()==W_INVOKE) gMAPConn->push(new DelimiterReqBlock(this));
            }
            else if(gb->serviceMsg==GMAP_CLOSE) {
                setDialogState(CLOSE_0);
            }
            else if(gb->serviceMsg==GMAP_U_ABORT || gb->serviceMsg==GMAP_P_ABORT) {
                setDialogState(ABORT_0);
            }
            else {
                setInvokeId(gb->invokeId);
                gMAPConn->push(new CloseReqBlock(this));
            }
        }
        else if(gb->serviceType==GMAP_ERROR) {
            setDialogState(W_CLOSE_0);
        }
    }

    void run() {
        BaseConn *baseConn=appClient->nextOnline();
        if(baseConn!=NULL) {
            std::vector<IE*> ieVector;

            CompositeIE *ieType=NULL;
            CompositeIE *ieMSISDN=NULL;
            CompositeIE *ieNodeId=NULL;
            CompositeIE *ieTPDU=NULL;

            // type
            ieType=new CompositeIE(TYPE, type, NULL);
            ieVector.push_back(ieType);

            // msisdn
            if(msisdn.length() > 0) {
                std::vector<IE*> tmpVector;
                IEByteArray tmpByteArray(msisdn);
                IEByte tmpByte(msisdnType);
                tmpVector.push_back(&tmpByteArray);
                tmpVector.push_back(&tmpByte);
                ieMSISDN=new CompositeIE(MSISDN, 0, &tmpVector);
                ieVector.push_back(ieMSISDN);
            }

            // nodeId
            if(nodeId.length() > 0) {
                std::vector<IE*> tmpVector;
                IEByteArray tmpByteArray(nodeId);
                tmpVector.push_back(&tmpByteArray);
                ieNodeId=new CompositeIE(NODE_ID, 0, &tmpVector);
                ieVector.push_back(ieNodeId);
            }

            // tpdu
            if(len > 0) {
                std::vector<IE*> tmpVector;
                IEByteArray tmpByteArray(ch, len);
                IEByte tmpByte(chType);
                tmpVector.push_back(&tmpByteArray);
                tmpVector.push_back(&tmpByte);
                ieTPDU=new CompositeIE(TPDU, 0, &tmpVector);
                ieVector.push_back(ieTPDU);
            }

            if(msisdn.length() > 0 && len > 0) {
                Message forwardMOSMRequest(BaseConn::VERSION, 0, FORWARD_MO_SM_REQUEST.getType(), 0, &ieVector);

                Future<Message> futureResponse;
                baseConn->dispatch(&forwardMOSMRequest, &futureResponse);
                // response
                Message *response=futureResponse.get();
                if(response!=NULL) {
                    if(response->getType()==FORWARD_MO_SM_REQUEST.getLinkedType()) {
                        // action
                        unsigned char action=0;
                        CompositeIE *ieAction;
                        ieAction=response->getCompositeIE(ACTION);
                        if(ieAction!=NULL) action=ieAction->getValueByte();

                        if((action & MESSAGE_ACTION)==ACCEPT) {
                            //std::cout << "forwardMOSM" << " msisdn=" << msisdn << " ACCEPT" << std::endl;
                        }
                        else {
                            //std::cout << "forwardMOSM" << " msisdn=" << msisdn << " END" << std::endl;
                        }
                        gMAPConn->push(new ForwardMOSMResBlock(this));
                        gMAPConn->push(new CloseReqBlock(this));
                    }
                    else {
                        gMAPConn->push(new ErrorBlock(this, SYSTEM_FAILURE));
                        gMAPConn->push(new CloseReqBlock(this));
                        //std::cout << "forwardMOSM" << " msisdn=" << msisdn << " FAILED" << std::endl;
                    }
                    delete response;
                }
                else {
                    gMAPConn->push(new ErrorBlock(this, SYSTEM_FAILURE));
                    gMAPConn->push(new CloseReqBlock(this));
                    //std::cout << "forwardMOSM" << " msisdn=" << msisdn << " FAILED" << std::endl;
                }
            }

            if(ieType!=NULL) delete ieType;
            if(ieMSISDN!=NULL) delete ieMSISDN;
            if(ieNodeId!=NULL) delete ieNodeId;
            if(ieTPDU!=NULL) delete ieTPDU;
        }
        else {
            gMAPConn->push(new ErrorBlock(this, SYSTEM_FAILURE));
            gMAPConn->push(new CloseReqBlock(this));
        }
    }

};

/**
 * <p>The SendRIFSM class should be used to handle MAP_SEND_ROUTING_INFO_FOR_SM dialog.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 05-30-2010
 */
class SendRIFSM : public Dialog {

    /** The gMAPConn. */
    GMAPConn *gMAPConn;

    /** The appClient. */
    AppClient *appClient;

    /* private variables */
    std::string messageId;
    std::string remoteGT;
    unsigned char remoteGTType;
    std::string msisdn;
    unsigned char msisdnType;
    unsigned char smRPPRI;

    std::string imsi;
    std::string lmsi;
    std::string roamingNumber;
    unsigned char roamingNumberType;
    std::string mscNumber;
    unsigned char mscNumberType;
    int serviceMessage;

public:

    /**
     * Creates a new instance of SendRIFSM.
     * @param *gMAPConn GMAPConn
     * @param *appClient AppClient
     * @param &messageId const std::string
     * @param &remoteGT const std::string
     * @param &remoteGTType const unsigned char
     * @param &msisdn const std::string
     * @param &msisdnType const unsigned char
     * @param &smRPPRI const unsigned char
     */
    SendRIFSM(GMAPConn *gMAPConn, AppClient *appClient, const std::string &messageId, const std::string &remoteGT, const unsigned char &remoteGTType, const std::string &msisdn, const unsigned char &msisdnType, const unsigned char &smRPPRI) : Dialog() {
        this->gMAPConn=gMAPConn;
        this->appClient=appClient;
        this->messageId=messageId;
        this->remoteGT=remoteGT;
        this->remoteGTType=remoteGTType;
        this->msisdn=msisdn;
        this->msisdnType=msisdnType;
        this->smRPPRI=smRPPRI;

        imsi="";
        lmsi="";
        roamingNumber="";
        mscNumber="";
        serviceMessage=-1;
    }

    void init() {
        setDialogState(W_INVOKE);
        gMAPConn->push(new OpenReqBlock(this, &shortMsgGatewayContext_v1, gMAPConn->getMAPInit().ssn, gMAPConn->getLocalPC(), gMAPConn->getLocalGT(), gMAPConn->getLocalGTType(), 6, gMAPConn->getRemotePC(), remoteGT, remoteGTType));
        gMAPConn->push(new SendRIFSMReqBlock(this, msisdn, msisdnType, smRPPRI, gMAPConn->getLocalGT(), gMAPConn->getLocalGTType()));
        gMAPConn->push(new DelimiterReqBlock(this));
    }

    void check() {
        if(getDialogState()==W_INVOKE) setDialogState(W_CLOSE_0);
        else if(getDialogState()==CLOSE_0) setDialogState(CLOSE_1);
        else if(getDialogState()==W_CLOSE_0) setDialogState(W_CLOSE_1);
        else if(getDialogState()==W_CLOSE_1) gMAPConn->push(new UAbortReqBlock(this));
        else if(getDialogState()==ABORT_0) setDialogState(ABORT_1);
    }

    void handleGBlock(gblock_t *gb) {
        if(gb->serviceType==GMAP_RSP) {
            if(gb->serviceMsg==GMAP_OPEN)
                setDialogState(W_INVOKE);
            else if(gb->serviceMsg==SEND_ROUTING_INFO_FOR_SM) {
                // imsi
                if(gb->parameter.routingInfoForSM_Res_v1.imsi.length > 0)
                    imsi=GBlock::getSemiOctetStr(gb->parameter.routingInfoForSM_Res_v1.imsi.value, gb->parameter.routingInfoForSM_Res_v1.imsi.length);
                // lmsi
                if(gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.bit_mask==LocationInfoWithLMSI_lmsi_present)
                    if(gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.lmsi.length > 0)
                        lmsi=GBlock::getSemiOctetStr(gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.lmsi.value, gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.lmsi.length);
                // roamingNumber
                if(gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.locationInfo.choice==LocationInfo_roamingNumber_chosen)
                    if(gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.locationInfo.u.roamingNumber.length > 0)
                        GBlock::getAddressStr(gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.locationInfo.u.roamingNumber.value, gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.locationInfo.u.roamingNumber.length, roamingNumber, roamingNumberType);
                // mscNumber
                if(gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.locationInfo.choice==LocationInfo_msc_Number_chosen)
                    if(gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.locationInfo.u.msc_Number.length > 0)
                        GBlock::getAddressStr(gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.locationInfo.u.msc_Number.value, gb->parameter.routingInfoForSM_Res_v1.locationInfoWithLMSI.locationInfo.u.msc_Number.length, mscNumber, mscNumberType);
                serviceMessage=0;
                setDialogState(W_CLOSE_0);
            }
        }
        else if(gb->serviceType==GMAP_REQ) {
            if(gb->serviceMsg==GMAP_CLOSE) {
                gMAPConn->submit(this);
                setDialogState(CLOSE_0);
            }
            else if(gb->serviceMsg==GMAP_U_ABORT || gb->serviceMsg==GMAP_P_ABORT) {
                gMAPConn->submit(this);
                setDialogState(ABORT_0);
            }
        }
        else if(gb->serviceType==GMAP_ERROR) {
            serviceMessage=gb->serviceMsg;
            setDialogState(W_CLOSE_0);
        }
    }

    void run() {
        BaseConn *baseConn=appClient->nextOnline();
        if(baseConn!=NULL) {
            std::vector<IE*> ieVector;

            CompositeIE *ieMessageId;
            ieMessageId=NULL;
            IEByte *ieValue;
            ieValue=NULL;
            IEInteger *ieServiceMessage;
            ieServiceMessage=NULL;
            CompositeIE *ieIMSI;
            ieIMSI=NULL;
            CompositeIE *ieLMSI;
            ieLMSI=NULL;
            CompositeIE *ieRemoteGT;
            ieRemoteGT=NULL;

            if(messageId.length() > 0) {
                std::vector<IE*> tmpVector;
                IEByteArray tmpByteArray(messageId);
                tmpVector.push_back(&tmpByteArray);
                ieMessageId=new CompositeIE(MESSAGE_ID, 0, &tmpVector);
                ieVector.push_back(ieMessageId);
            }

            if(imsi.length() > 0) {
                std::vector<IE*> tmpVector;
                IEByteArray tmpByteArray(imsi);
                tmpVector.push_back(&tmpByteArray);
                ieIMSI=new CompositeIE(IE_IMSI, 0, &tmpVector);
                ieVector.push_back(ieIMSI);
            }

            if(lmsi.length() > 0) {
                std::vector<IE*> tmpVector;
                IEByteArray tmpByteArray(lmsi);
                tmpVector.push_back(&tmpByteArray);
                ieLMSI=new CompositeIE(IE_LMSI, 0, &tmpVector);
                ieVector.push_back(ieLMSI);
            }

            if(roamingNumber.length() > 0) {
                std::vector<IE*> tmpVector;
                IEByteArray tmpByteArray(roamingNumber);
                IEByte tmpByte(roamingNumberType);
                tmpVector.push_back(&tmpByteArray);
                tmpVector.push_back(&tmpByte);
                ieRemoteGT=new CompositeIE(REMOTE_GT, 0, &tmpVector);
                ieVector.push_back(ieRemoteGT);
            }
            else if(mscNumber.length() > 0) {
                std::vector<IE*> tmpVector;
                IEByteArray tmpByteArray(mscNumber);
                IEByte tmpByte(mscNumberType);
                tmpVector.push_back(&tmpByteArray);
                tmpVector.push_back(&tmpByte);
                ieRemoteGT=new CompositeIE(REMOTE_GT, 0, &tmpVector);
                ieVector.push_back(ieRemoteGT);
            }

            if(serviceMessage==0) {
                ieValue=new IEByte(TypeCollection::REQUEST_ACCEPTED);
                ieVector.push_back(ieValue);
            }
            else {
                ieValue=new IEByte(TypeCollection::REQUEST_FAILED);
                ieVector.push_back(ieValue);
                ieServiceMessage=new IEInteger(serviceMessage);
                ieVector.push_back(ieServiceMessage);
            }

            Message sendRIFSMAdvice(BaseConn::VERSION, 0, SEND_RI_F_SM_ADVICE.getType(), 0, &ieVector);
            baseConn->write(&sendRIFSMAdvice);

            if(ieMessageId!=NULL) delete ieMessageId;
            if(ieValue!=NULL) delete ieValue;
            if(ieServiceMessage!=NULL) delete ieServiceMessage;
            if(ieIMSI!=NULL) delete ieIMSI;
            if(ieLMSI!=NULL) delete ieLMSI;
            if(ieRemoteGT!=NULL) delete ieRemoteGT;

            //std::cout << "sendRIFSMAdvice" << " id=" << id << " msisdn=" << msisdn << std::endl;
        }
    }

};

/**
 * <p>The ForwardMTSM class should be used to handle MAP_MT_FORWARD_SHORT_MESSAGE dialog.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 05-30-2010
 */
class ForwardMTSM : public Dialog {

    /** The gMAPConn. */
    GMAPConn *gMAPConn;

    /** The appClient. */
    AppClient *appClient;

    /* private variables */
    std::string messageId;
    unsigned char *ch;
    int len;
    std::string imsi;
    std::string lmsi;
    std::string remoteGT;
    unsigned char remoteGTType;

    int serviceMessage;

public:

    /**
     * Creates a new instance of ForwardMTSM.
     * @param *gMAPConn GMAPConn
     * @param *appClient AppClient
     * @param &messageId const std::string
     * @param *ch unsingned char
     * @param &len const int
     * @param &imsi const std::string
     * @param &lmsi const std::string
     * @param &remoteGT const std::string
     * @param &remoteGTType const unsigned char
     */
    ForwardMTSM(GMAPConn *gMAPConn, AppClient *appClient, const std::string &messageId, unsigned char *ch, const int &len, const std::string &imsi, const std::string &lmsi, const std::string &remoteGT, const unsigned char &remoteGTType) : Dialog(Dialog::TIME_60S) {
        this->gMAPConn=gMAPConn;
        this->appClient=appClient;
        this->messageId=messageId;
        if(len > 0) {
            this->ch=new unsigned char[len];
            for(int i=0; i < len; i++) this->ch[i]=ch[i];
        }
        this->len=len;
        this->imsi=imsi;
        this->lmsi=lmsi;
        this->remoteGT=remoteGT;
        this->remoteGTType=remoteGTType;
        serviceMessage=-1;
    }

    ~ForwardMTSM() {
        if(len > 0) delete[] ch;
    }

    void init() {
        setDialogState(W_INVOKE);
        gMAPConn->push(new OpenReqBlock(this, &shortMsgMT_RelayContext_v2, gMAPConn->getMAPInit().ssn, gMAPConn->getLocalPC(), gMAPConn->getLocalGT(), gMAPConn->getLocalGTType(), 8, gMAPConn->getRemotePC(), remoteGT, remoteGTType));
        gMAPConn->push(new ForwardMTSMReqBlock(this, ch, len, imsi, lmsi, gMAPConn->getLocalGT(), gMAPConn->getLocalGTType()));
        gMAPConn->push(new DelimiterReqBlock(this));
    }

    void check() {
        if(getDialogState()==W_INVOKE) setDialogState(W_CLOSE_0);
        else if(getDialogState()==CLOSE_0) setDialogState(CLOSE_1);
        else if(getDialogState()==W_CLOSE_0) setDialogState(W_CLOSE_1);
        else if(getDialogState()==W_CLOSE_1) gMAPConn->push(new UAbortReqBlock(this));
        else if(getDialogState()==ABORT_0) setDialogState(ABORT_1);
    }

    void handleGBlock(gblock_t *gb) {
        if(gb->serviceType==GMAP_RSP) {
            if(gb->serviceMsg==GMAP_OPEN)
                setDialogState(W_INVOKE);
            else if(gb->serviceMsg==FORWARD_SM) {
                serviceMessage=0;
                setDialogState(W_CLOSE_0);
            }
        }
        else if(gb->serviceType==GMAP_REQ) {
            if(gb->serviceMsg==GMAP_CLOSE) {
                gMAPConn->submit(this);
                setDialogState(CLOSE_0);
            }
            else if(gb->serviceMsg==GMAP_U_ABORT || gb->serviceMsg==GMAP_P_ABORT) {
                gMAPConn->submit(this);
                setDialogState(ABORT_0);
            }
        }
        else if(gb->serviceType==GMAP_ERROR) {
            serviceMessage=gb->serviceMsg;
            setDialogState(W_CLOSE_0);
        }
    }

    void run() {
        BaseConn *baseConn=appClient->nextOnline();
        if(baseConn!=NULL) {
            std::vector<IE*> ieVector;

            CompositeIE *ieMessageId;
            ieMessageId=NULL;
            IEByte *ieValue;
            ieValue=NULL;
            IEInteger *ieServiceMessage;
            ieServiceMessage=NULL;

            if(messageId.length() > 0) {
                std::vector<IE*> tmpVector;
                IEByteArray tmpByteArray(messageId);
                tmpVector.push_back(&tmpByteArray);
                ieMessageId=new CompositeIE(MESSAGE_ID, 0, &tmpVector);
                ieVector.push_back(ieMessageId);
            }

            if(serviceMessage==0) {
                ieValue=new IEByte(TypeCollection::REQUEST_ACCEPTED);
                ieVector.push_back(ieValue);
            }
            else {
                ieValue=new IEByte(TypeCollection::REQUEST_FAILED);
                ieVector.push_back(ieValue);
                ieServiceMessage=new IEInteger(serviceMessage);
                ieVector.push_back(ieServiceMessage);
            }

            Message forwardMTSMAdvice(BaseConn::VERSION, 0, FORWARD_MT_SM_ADVICE.getType(), 0, &ieVector);
            baseConn->write(&forwardMTSMAdvice);

            if(ieMessageId!=NULL) delete ieMessageId;
            if(ieValue!=NULL) delete ieValue;
            if(ieServiceMessage!=NULL) delete ieServiceMessage;

            //std::cout << "forwardMTSMAdvice" << " id=" << id << " serviceMsg=" << serviceMsg << std::endl;
        }
    }

};

/**
 * <p>The GMAPSM class.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 07-03-2010
 */
class GMAPSM : public GMAPConn {

    /** The appClient. */
    AppClient *appClient;

public:

    /**
     * Creates a new instance of GMAPSM.
     * @param argc int
     * @param **argv char
     * @param *properties Properties
     * @param *appClient AppClient
     * @throws Exception
     */
    GMAPSM(int argc, char** argv, Properties *properties, AppClient *appClient) throw(Exception) : GMAPConn(argc, argv, properties) {
        this->appClient=appClient;
    }

private:

    void handleGBlock(gblock_t *gb) {
        //gMAPPrintGBlock(gb);
        if(gb->serviceType==GMAP_RSP || gb->serviceType==GMAP_ERROR || gb->serviceType==GMAP_PROVIDER_ERROR) {
            Dialog *dialog=dialogCache->get(gb->dialogId);
            if(dialog!=NULL) dialog->handleGBlock(gb);
        }
        else if(gb->serviceType==GMAP_REQ) {
            if(gb->serviceMsg==GMAP_OPEN) {
                ForwardMOSM *dialog=new ForwardMOSM(this, appClient);
                dialog->setDialogId(gb->dialogId);
                dialog->init();
                dialogCache->put(dialog);
                dialog->handleGBlock(gb);
            }
            else if(gb->serviceMsg==MO_FORWARD_SM || gb->serviceMsg==ALERT_SERVICE_CENTRE_WITHOUT_RESULT || gb->serviceMsg==GMAP_DELIMITER || gb->serviceMsg==GMAP_CLOSE || gb->serviceMsg==GMAP_P_ABORT || gb->serviceMsg==GMAP_U_ABORT) {
                Dialog *dialog=dialogCache->get(gb->dialogId);
                if(dialog!=NULL) dialog->handleGBlock(gb);
            }
        }
    }

};

/**
 * <p>The SendRIFSMAction class.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 07-06-2010
 */
class SendRIFSMAction : public MessageAction {

    /** The gMAPSM. */
    GMAPSM *gMAPSM;

    /** The appClient. */
    AppClient *appClient;

public:

    /**
     * Creates a new instance of SendRIFSMAction.
     * @param *gMAPSM GMAPSM
     * @param *appClient AppClient
     */
    SendRIFSMAction(GMAPSM *gMAPSM, AppClient *appClient) : MessageAction(&SEND_RI_F_SM_REQUEST) {
        this->appClient=appClient;
        this->gMAPSM=gMAPSM;
    }

private:

    void execute(BaseConn *baseConn, Message *message) throw(Exception) {
        CompositeIE *compositeIE;

        // messageId
        std::string messageId="";
        compositeIE=message->getCompositeIE(MESSAGE_ID);
        if(compositeIE!=NULL) messageId=compositeIE->getIEString();

        // remoteGT
        std::string remoteGT="";
        unsigned char remoteGTType;
        compositeIE=message->getCompositeIE(REMOTE_GT);
        if(compositeIE!=NULL) {
            remoteGT=compositeIE->getIEString();
            remoteGTType=compositeIE->getIEByte();
        }

        // msisdn
        std::string msisdn="";
        unsigned char msisdnType;
        compositeIE=message->getCompositeIE(MSISDN);
        if(compositeIE!=NULL) {
            msisdn=compositeIE->getIEString();
            msisdnType=compositeIE->getIEByte();
        }

        // smRPPRI
        unsigned char smRPPRI=0;
        compositeIE=message->getCompositeIE(SMRPPRI);
        if(compositeIE!=NULL) smRPPRI=compositeIE->getValueByte();

        if(messageId.length() > 0 && remoteGT.length() > 0 && msisdn.length() > 0) {
            SendRIFSM *dialog=new SendRIFSM(gMAPSM, appClient, messageId, remoteGT, remoteGTType, msisdn, msisdnType, smRPPRI);
            dialog->init();
            Message sendRIFSMResponse(BaseConn::VERSION, message->getSequenceNumber(), SEND_RI_F_SM_REQUEST.getLinkedType(), TypeCollection::REQUEST_ACCEPTED, NULL);
            baseConn->write(&sendRIFSMResponse);
            //std::cout << "sendRIFSM" << " id=" << id << " remoteGT=" << remoteGT << " msisdn=" << msisdn << " ACCEPTED" << std::endl;
        }
        else {
            Message sendRIFSMResponse(BaseConn::VERSION, message->getSequenceNumber(), SEND_RI_F_SM_REQUEST.getLinkedType(), TypeCollection::REQUEST_FAILED, NULL);
            baseConn->write(&sendRIFSMResponse);
            //std::cout << "sendRIFSM" << " id=" << id << " remoteGT=" << remoteGT << " msisdn=" << msisdn << " FAILED" << std::endl;
        }
    }

};

/**
 * <p>The ForwardMTSMAction class.</p>
 *
 * @author Eduardo Marin
 * @version 1.0, 07-06-2010
 */
class ForwardMTSMAction : public MessageAction {

    /** The gMAPSM. */
    GMAPSM *gMAPSM;

    /** The appClient. */
    AppClient *appClient;

public:

    /**
     * Creates a new instance of ForwardMTSMAction.
     * @param *gMAPSM GMAPSM
     * @param *appClient AppClient
     */
    ForwardMTSMAction(GMAPSM *gMAPSM, AppClient *appClient) : MessageAction(&FORWARD_MT_SM_REQUEST) {
        this->appClient=appClient;
        this->gMAPSM=gMAPSM;
    }

private:

    void execute(BaseConn *baseConn, Message *message) throw(Exception) {
        CompositeIE *compositeIE;

        // messageId
        std::string messageId="";
        compositeIE=message->getCompositeIE(MESSAGE_ID);
        if(compositeIE!=NULL) messageId=compositeIE->getIEString();

        // ch
        unsigned char ch[256];
        int len=0;
        compositeIE=message->getCompositeIE(TPDU);
        if(compositeIE!=NULL) compositeIE->getIEByteArray(ch, len);

        // imsi
        std::string imsi="";
        compositeIE=message->getCompositeIE(IE_IMSI);
        if(compositeIE!=NULL) imsi=compositeIE->getIEString();

        // lmsi
        std::string lmsi="";
        compositeIE=message->getCompositeIE(IE_LMSI);
        if(compositeIE!=NULL) lmsi=compositeIE->getIEString();

        // remoteGT
        std::string remoteGT="";
        unsigned char remoteGTType=0;
        compositeIE=message->getCompositeIE(REMOTE_GT);
        if(compositeIE!=NULL) {
            remoteGT=compositeIE->getIEString();
            remoteGTType=compositeIE->getIEByte();
        }

        if(messageId.length() > 0 && len > 0 && (imsi.length() > 0 || lmsi.length() > 0) && remoteGT.length() > 0) {
            ForwardMTSM *dialog=new ForwardMTSM(gMAPSM, appClient, messageId, ch, len, imsi, lmsi, remoteGT, remoteGTType);
            dialog->init();
            Message forwardMTSMResponse(BaseConn::VERSION, message->getSequenceNumber(), FORWARD_MT_SM_REQUEST.getLinkedType(), TypeCollection::REQUEST_ACCEPTED, NULL);
            baseConn->write(&forwardMTSMResponse);
            //std::cout << "forwardMTSM" << " id=" << id << " remoteGT=" << remoteGT << " imsi=" << imsi << " ACCEPTED" << std::endl;
        }
        else {
            Message forwardMTSMResponse(BaseConn::VERSION, message->getSequenceNumber(), FORWARD_MT_SM_REQUEST.getLinkedType(), TypeCollection::REQUEST_FAILED, NULL);
            baseConn->write(&forwardMTSMResponse);
            //std::cout << "forwardMTSM" << " id=" << id << " remoteGT=" << remoteGT << " imsi=" << imsi << " FAILED" << std::endl;
        }
    }

};

#endif	/* GMAPSM_HPP */
