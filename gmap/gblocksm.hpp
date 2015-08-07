/*
 * File:   gblocksm.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef GBLOCKSM_HPP
#define	GBLOCKSM_HPP

#include "gmapconn.hpp"

#include <string>

#include <gmap.h>

/**
 * <p>The OpenReqBlock class.</p>
 */
class OpenReqBlock : public GBlock {

    /* private variables */
    ObjectID *objectId;
    unsigned char localSSN;
    unsigned int localPC;
    std::string localMSISDN;
    unsigned char localMSISDNType;
    unsigned char remoteSSN;
    unsigned int remotePC;
    std::string remoteMSISDN;
    unsigned char remoteMSISDNType;

public:

    /**
     * Creates a new instance of OpenReqBlock.
     * @param *dialog Dialog
     * @param *objectId ObjectID
     * @param &localSSN const unsigned char
     * @param &localPC const unsigned int
     * @param &localMSISDN const std::string
     * @param &localMSISDNType const unsigned char
     * @param &remoteSSN const unsigned char
     * @param &remotePC const unsigned int
     * @param &remoteMSISDN const std::string
     * @param &remoteMSISDNType const unsigned char
     */
    OpenReqBlock(Dialog *dialog,
                 ObjectID *objectId,
                 const unsigned char &localSSN,
                 const unsigned int &localPC,
                 const std::string &localMSISDN,
                 const unsigned char &localMSISDNType,
                 const unsigned char &remoteSSN,
                 const unsigned int &remotePC,
                 const std::string &remoteMSISDN,
                 const unsigned char &remoteMSISDNType) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, GMAP_OPEN, 0, -1) {
        this->objectId=objectId;
        this->localSSN=localSSN;
        this->localPC=localPC;
        this->localMSISDN=localMSISDN;
        this->localMSISDNType=localMSISDNType;
        this->remoteSSN=remoteSSN;
        this->remotePC=remotePC;
        this->remoteMSISDN=remoteMSISDN;
        this->remoteMSISDNType=remoteMSISDNType;
    }

    void getGBlock(gblock_t *gb) {
        GBlock::getGBlock(gb);
        gb->parameter.openArg.bit_mask=MAP_OpenArg_originatingAddress_present;
        memcpy(&gb->parameter.openArg.applicationContext, objectId, sizeof(ObjectID));
        // originatingAddress
        gb->parameter.openArg.originatingAddress.bit_mask=MAP_SccpAddr_gt_present;
        gb->parameter.openArg.originatingAddress.routingIndicator=routeOnGt;
        gb->parameter.openArg.originatingAddress.ssn=localSSN;
        gb->parameter.openArg.originatingAddress.pointCode=localPC;
        gb->parameter.openArg.originatingAddress.gt.bit_mask=MAP_SccpAddr_gt_numberingPlan_present;
        gb->parameter.openArg.originatingAddress.gt.natureOfAddress=getNatureOfAddress(localMSISDNType);
        gb->parameter.openArg.originatingAddress.gt.numberingPlan=getNumberingPlan(localMSISDNType);
        int tmpLocalLen=32;
        unsigned char tmpLocalMSISDN[tmpLocalLen];
        getOctet(localMSISDN, tmpLocalMSISDN, tmpLocalLen);
        gb->parameter.openArg.originatingAddress.gt.msisdnLength=tmpLocalLen;
        memcpy(gb->parameter.openArg.originatingAddress.gt.msisdn, tmpLocalMSISDN, tmpLocalLen);
        gb->parameter.openArg.originatingAddress.gt.gtIndicator=4;
        gb->parameter.openArg.originatingAddress.gt.translationType=0;

        // destinationAddress
        gb->parameter.openArg.destinationAddress.bit_mask=MAP_SccpAddr_gt_present;
        gb->parameter.openArg.destinationAddress.routingIndicator=routeOnGt;
        gb->parameter.openArg.destinationAddress.ssn=remoteSSN;
        gb->parameter.openArg.destinationAddress.pointCode=remotePC;
        gb->parameter.openArg.destinationAddress.gt.bit_mask=MAP_SccpAddr_gt_numberingPlan_present;
        gb->parameter.openArg.destinationAddress.gt.natureOfAddress=getNatureOfAddress(remoteMSISDNType);
        gb->parameter.openArg.destinationAddress.gt.numberingPlan=getNumberingPlan(remoteMSISDNType);
        int tmpRemoteLen=32;
        unsigned char tmpRemoteMSISDN[tmpRemoteLen];
        getOctet(remoteMSISDN, tmpRemoteMSISDN, tmpRemoteLen);
        gb->parameter.openArg.destinationAddress.gt.msisdnLength=tmpRemoteLen;
        memcpy(gb->parameter.openArg.destinationAddress.gt.msisdn, tmpRemoteMSISDN, tmpRemoteLen);
        gb->parameter.openArg.destinationAddress.gt.gtIndicator=4;
        gb->parameter.openArg.destinationAddress.gt.translationType=0;
    }

};

/**
 * <p>The OpenResBlock class.</p>
 */
class OpenResBlock : public GBlock {

    /* private variables */
    DialogResult result;

public:

    /**
     * Creates a new instance of OpenResBlock.
     * @param *dialog Dialog
     * @param &result DialogResult
     */
    OpenResBlock(Dialog *dialog,
                 const DialogResult &result) : GBlock(dialog, gblock_t_parameter_present, GMAP_RSP, GMAP_OPEN, 0, -1) {
        this->result=result;
    }

    void getGBlock(gblock_t *gb) {
        GBlock::getGBlock(gb);
        gb->parameter.openRes.bit_mask=0;
        gb->parameter.openRes.result=result;
    }

};

/**
 * <p>The ForwardMOSMResBlock class.</p>
 */
class ForwardMOSMResBlock : public GBlock {

public:

    /**
     * Creates a new instance of ForwardMOSMResBlock.
     * @param *dialog Dialog
     */
    ForwardMOSMResBlock(Dialog *dialog) : GBlock(dialog, 0, GMAP_RSP, MO_FORWARD_SM, 0, -1) {}

    void getGBlock(gblock_t *gb) {
        GBlock::getGBlock(gb);
    }

};

/**
 * <p>The SendRIFSMReqBlock class.</p>
 */
class SendRIFSMReqBlock : public GBlock {

    /* private variables */
    std::string msisdn;
    unsigned char msisdnType;
    unsigned char smRPPRI;
    std::string serviceCentreAddress;
    unsigned char serviceCentreAddressType;

public:

    /**
     * Creates a new instance of SendRIFSMReqBlock.
     * @param *dialog Dialog
     * @param &msisdn const std::string
     * @param &msisdnType const unsigned char
     * @param &smRPPRI const unsigned char
     * @param &serviceCentreAddress const std::string
     * @param &serviceCentreAddressType const unsigned char
     */
    SendRIFSMReqBlock(Dialog *dialog,
                      const std::string &msisdn,
                      const unsigned char &msisdnType,
                      const unsigned char &smRPPRI,
                      const std::string &serviceCentreAddress,
                      const unsigned char &serviceCentreAddressType) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, SEND_ROUTING_INFO_FOR_SM, 0, -1) {
        this->msisdn=msisdn;
        this->msisdnType=msisdnType;
        this->smRPPRI=smRPPRI;
        this->serviceCentreAddress=serviceCentreAddress;
        this->serviceCentreAddressType=serviceCentreAddressType;
    }

    void getGBlock(gblock_t *gb) {
        GBlock::getGBlock(gb);
        int tmpMSISDNLen=32;
        unsigned char tmpMSISDN[tmpMSISDNLen];
        getAddress(msisdn, msisdnType, tmpMSISDN, tmpMSISDNLen);
        gb->parameter.routingInfoForSM_Arg_v1.msisdn.length=tmpMSISDNLen;
        memcpy(gb->parameter.routingInfoForSM_Arg_v1.msisdn.value, tmpMSISDN, tmpMSISDNLen);
        gb->parameter.routingInfoForSM_Arg_v1.sm_RP_PRI=smRPPRI;
        int tmpSCALen=32;
        unsigned char tmpSCA[tmpSCALen];
        getAddress(serviceCentreAddress, serviceCentreAddressType, tmpSCA, tmpSCALen);
        gb->parameter.routingInfoForSM_Arg_v1.serviceCentreAddress.length=tmpSCALen;
        memcpy(gb->parameter.routingInfoForSM_Arg_v1.serviceCentreAddress.value, tmpSCA, tmpSCALen);
    }

};

/**
 * <p>The ForwardMTSMReqBlock class.</p>
 */
class ForwardMTSMReqBlock : public GBlock {

    /* private variables */
    std::string imsi;
    std::string lmsi;
    std::string serviceCentreAddress;
    unsigned char serviceCentreAddressType;
    unsigned char *ch;
    int len;

public:

    /**
     * Creates a new instance of ForwardMTSMReqBlock.
     * @param *dialog Dialog
     * @param *ch unsigned char
     * @param &len const int
     * @param &imsi const std::string
     * @param &lmsi const std::string
     * @param &serviceCentreAddress const std::string
     * @param &serviceCentreAddressType const unsigned char
     */
    ForwardMTSMReqBlock(Dialog *dialog,
                        unsigned char *ch,
                        const int &len,
                        const std::string &imsi,
                        const std::string &lmsi,
                        const std::string &serviceCentreAddress,
                        const unsigned char &serviceCentreAddressType) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, FORWARD_SM, 0, -1) {
        this->ch=ch;
        this->len=len;
        this->imsi=imsi;
        this->lmsi=lmsi;
        this->serviceCentreAddress=serviceCentreAddress;
        this->serviceCentreAddressType=serviceCentreAddressType;
    }

    void getGBlock(gblock_t *gb) {
        GBlock::getGBlock(gb);
        gb->parameter.forwardSM_Arg_v2.sm_RP_UI.length=len;
        memcpy(gb->parameter.forwardSM_Arg_v2.sm_RP_UI.value, ch, len);
        if(imsi.length() > 0) {
            gb->parameter.forwardSM_Arg_v2.sm_RP_DA.choice=SM_RP_DA_t2_imsi_chosen;
            int tmpIMSILen=32;
            unsigned char tmpIMSI[tmpIMSILen];
            getSemiOctet(imsi, tmpIMSI, tmpIMSILen);
            gb->parameter.forwardSM_Arg_v2.sm_RP_DA.u.imsi.length=tmpIMSILen;
            memcpy(gb->parameter.forwardSM_Arg_v2.sm_RP_DA.u.imsi.value, tmpIMSI, tmpIMSILen);
        }
        else if(lmsi.length() > 0) {
            gb->parameter.forwardSM_Arg_v2.sm_RP_DA.choice=SM_RP_DA_t2_lmsi_chosen;
            int tmpLMSILen=32;
            unsigned char tmpLMSI[tmpLMSILen];
            getSemiOctet(lmsi, tmpLMSI, tmpLMSILen);
            gb->parameter.forwardSM_Arg_v2.sm_RP_DA.u.lmsi.length=tmpLMSILen;
            memcpy(gb->parameter.forwardSM_Arg_v2.sm_RP_DA.u.lmsi.value, tmpLMSI, tmpLMSILen);
        }

        gb->parameter.forwardSM_Arg_v2.sm_RP_OA.choice=SM_RP_OA_t2_serviceCentreAddressOA_chosen;
        int tmpSCALen=32;
        unsigned char tmpSCA[tmpSCALen];
        getAddress(serviceCentreAddress, serviceCentreAddressType, tmpSCA, tmpSCALen);
        gb->parameter.forwardSM_Arg_v2.sm_RP_OA.u.serviceCentreAddressOA.length=tmpSCALen;
        memcpy(gb->parameter.forwardSM_Arg_v2.sm_RP_OA.u.serviceCentreAddressOA.value, tmpSCA, tmpSCALen);        
    }

};

/**
 * <p>The DelimiterReqBlock class.</p>
 */
class DelimiterReqBlock : public GBlock {

public:

    /**
     * Creates a new instance of DelimiterReqBlock.
     * @param *dialog Dialog
     */
    DelimiterReqBlock(Dialog *dialog) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, GMAP_DELIMITER, 0, -1) {}

    void getGBlock(gblock_t *gb) {
        GBlock::getGBlock(gb);
        gb->parameter.delimiter.qualityOfService=CL_SVC_CLASS_0;
    }

};

/**
 * <p>The CloseReqBlock class.</p>
 */
class CloseReqBlock : public GBlock {

public:

    /**
     * Creates a new instance of CloseReqBlock.
     * @param *dialog Dialog
     */
    CloseReqBlock(Dialog *dialog) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, GMAP_CLOSE, 0, -1) {}

    void getGBlock(gblock_t *gb) {
        GBlock::getGBlock(gb);
        gb->parameter.closeArg.releaseMethod=normalRelease;
        gb->parameter.closeArg.qualityOfService=CL_SVC_CLASS_0;
    }

};

/**
 * <p>The PAbortReqBlock class.</p>
 */
class PAbortReqBlock : public GBlock {

public:

    /**
     * Creates a new instance of PAbortReqBlock.
     * @param *dialog Dialog
     */
    PAbortReqBlock(Dialog *dialog) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, GMAP_P_ABORT, 0, -1) {}

    void getGBlock(gblock_t *gb) {
        GBlock::getGBlock(gb);
        gb->parameter.pAbortArg.providerReason=ressourceLimitation;
        gb->parameter.pAbortArg.source=networkServiceProblem;
    }

};

/**
 * <p>The UAbortReqBlock class.</p>
 */
class UAbortReqBlock : public GBlock {

public:

    /**
     * Creates a new instance of UAbortReqBlock.
     * @param *dialog Dialog
     */
    UAbortReqBlock(Dialog *dialog) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, GMAP_U_ABORT, 0, -1) {}

    void getGBlock(gblock_t *gb) {
        GBlock::getGBlock(gb);
        gb->parameter.uAbortArg.userReason.choice=MAP_UserAbortChoice_applicationProcedureCancellation_chosen;
        gb->parameter.uAbortArg.userReason.u.applicationProcedureCancellation=ProcedureCancellationReason_remoteOperationsFailure;
        gb->parameter.uAbortArg.qualityOfService=CL_SVC_CLASS_0;
    }

};

/**
 * <p>The ErrorBlock class.</p>
 */
class ErrorBlock : public GBlock {

public:

    /**
     * Creates a new instance of ErrorBlock.
     * @param *dialog Dialog
     */
    ErrorBlock(Dialog *dialog, const int &serviceMsg) : GBlock(dialog, 0, GMAP_ERROR, serviceMsg, 0, -1) {}

    void getGBlock(gblock_t *gb) {
        GBlock::getGBlock(gb);
    }

};

#endif	/* GBLOCKSM_HPP */
