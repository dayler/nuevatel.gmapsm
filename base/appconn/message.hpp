/*
 * File:   message.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _MESSAGE_HPP
#define	_MESSAGE_HPP

#include "compositeie.hpp"

/**
 * <p>The Message class should be used to handle messages
 * for the application protocol.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-18-2010
 */
class Message : public CompositeIE {

    /** The message version. */
    unsigned char version;

    /** The sequenceNumber. */
    int sequenceNumber;

    /** The timeToLive. */
    int timeToLive;

public:

    /* constants */
    static int TIME_TO_LIVE_1S;
    static int TIME_TO_LIVE_2S;
    static int TIME_TO_LIVE_4S;
    static int DEFAULT_TIME_TO_LIVE;

    /**
     * Creates a new instance of Message.
     * @param *message Message
     */
    Message(Message *message) {
        version=message->getVersion();
        sequenceNumber=message->getSequenceNumber();
        timeToLive=message->getTimeToLive();
        type=message->getType();
        if(type < 128) // Type-Value Representation
            value=message->getValueByte();
        else { // Type-Length-Value Representation
            std::map<unsigned char, IE*>::iterator iter;
            for(iter=message->ieMap.begin(); iter!=message->ieMap.end(); iter++)
                putIE(iter->second);
        }
    }

    /**
     * Creates a new instance of Message.
     * @param &version const unsigned char
     * @param &sequenceNumber const int
     * @param &type const unsigned char
     * @param &value const unsigned char
     * @param *ieVector std::vector<IE*>
     */
    Message(const unsigned char &version, const int &sequenceNumber, const unsigned char &type, const unsigned char &value, std::vector<IE*> *ieVector) {
        this->version=version;
        this->sequenceNumber=sequenceNumber;
        this->timeToLive=DEFAULT_TIME_TO_LIVE;
        this->type=type;
        if(type < 128) // Type-Value Representation
            this->value=value;
        else // Type-Length-Value Representation
            for(unsigned int i=0; i < ieVector->size(); i++) {
                IE *ie=ieVector->operator [](i);
                putIE(ie);
            }
    }

    /**
     * Creates a new instance of Message.
     * @param &version const unsigned char
     * @param &sequenceNumber const int
     * @param &type const unsigned char
     * @param *ch unsigned char
     * @param &len const int
     */
    Message(const unsigned char &version, const int &sequenceNumber, const unsigned char &type, unsigned char *ch, const int &len) {
        this->version=version;
        this->sequenceNumber=sequenceNumber;
        this->timeToLive=DEFAULT_TIME_TO_LIVE;
        this->type=type;
        if(type < 128) { // Type-Value Representation
            if(len > 0) this->value=ch[0];
        }
        else { // Type-Length-Value Representation
            int chIndex=0;
            while(chIndex < len) {
                unsigned char *tmpCh=ch + chIndex;
                unsigned char tmpType=tmpCh[0];
                int tmpLen;
                unsigned char *tmpValue;

                if(tmpType < 128) {
                    tmpLen=1;
                    tmpValue=tmpCh + 1;
                    chIndex+=tmpLen + 1;
                }
                else {
                    tmpLen=tmpCh[1];
                    tmpValue=tmpCh + 2;
                    chIndex+=tmpLen + 2;
                }

                // IE
                IE *ie=getIE(tmpType, tmpValue, tmpLen);
                if(ie!=NULL) {
                    removeIE(ie->getType());
                    ieMap.insert(std::pair<unsigned char, IE*>(ie->getType(), ie));
                }
            }
        }
    }

    std::string toXML() {
        std::stringstream sstream;
        sstream << "<message version=\"" << (int)version << "\" sequenceNumber=\"" << sequenceNumber << "\" type=\"" << (int)type << "\"";
        if(type < 128) sstream << " value=\"" << (int)value << "\"";
        sstream << ">" << LINE_SEPARATOR;
        std::map<unsigned char, IE*>::iterator iter;
        for(iter=ieMap.begin(); iter!=ieMap.end(); iter++) {
            sstream << "<ie>" << LINE_SEPARATOR;
            sstream << iter->second->toXML();
            sstream << "</ie>" << LINE_SEPARATOR;
        }
        sstream << "</message>" << LINE_SEPARATOR;
        return sstream.str();
    }

protected:

    IE* getIE(const unsigned char &type, unsigned char *ch, const int &len) {
        IE *ie=CompositeIE::getIE(type, ch, len);
        if(ie==NULL) ie=new CompositeIE(type, ch, len);
        return ie;
    }

public:

    /**
     * Returns a CompositeIE of this information element.
     * @param &ieType const unsigned char
     * @return *CompositeIE
     */
    CompositeIE* getCompositeIE(const unsigned char &ieType) {
        IE *ie=CompositeIE::getIE(ieType);
        if(ie!=NULL) {
            try {
                return dynamic_cast<CompositeIE*>(ie);
            } catch(std::bad_cast) {
                return NULL;
            }
        }
        else return NULL;
    }

    void putIE(IE *ie) {
        try {
            CompositeIE *compositeIE;
            if((compositeIE=dynamic_cast<CompositeIE*>(ie))!=NULL) {
                removeIE(compositeIE->getType());
                ieMap.insert(std::pair<unsigned char, IE*>(compositeIE->getType(), new CompositeIE(compositeIE)));
            }
            else CompositeIE::putIE(ie);
        } catch(std::bad_cast) {}
    }

    /**
     * Returns the message.
     * @param *ch unsigned char
     * @param &len int
     */
    void getMessage(unsigned char *ch, int &len) {
        unsigned char sn[2];
        sn[0]=(sequenceNumber >> 8) & 0xff;
        sn[1]=sequenceNumber & 0xff;
        if(type < 128) { // Type-Value Representation
            len=5;
            ch[0]=version;
            ch[1]=sn[0];
            ch[2]=sn[1];
            ch[3]=type;
            ch[4]=value;
        }
        else { // Type-Length-Value Representation
            int tmpLen;
            ch[0]=version;
            ch[1]=sn[0];
            ch[2]=sn[1];
            ch[3]=type;
            getValue(&ch[5], tmpLen);
            ch[4]=tmpLen;
            len=tmpLen + 5;
        }
    }

    /**
     * Returns the version of this message.
     * @return unsigned char
     */
    unsigned char getVersion() {
        return version;
    }

    /**
     * Sets the sequenceNumber.
     * @param &sequenceNumber const int
     */
    void setSequenceNumber(const int &sequenceNumber) {
        this->sequenceNumber=sequenceNumber;
    }

    /**
     * Returns the sequenceNumber of this message.
     * @return int
     */
    int getSequenceNumber() {
        return sequenceNumber;
    }

    /**
     * Sets the timeToLive.
     * @param &timeToLive const int
     */
    void setTimeToLive(const int &timeToLive) {
        this->timeToLive=timeToLive;
    }

    /**
     * Returns the timeToLive.
     * @return int
     */
    int getTimeToLive() {
        return timeToLive;
    }

};

int Message::TIME_TO_LIVE_1S=1000;
int Message::TIME_TO_LIVE_2S=2000;
int Message::TIME_TO_LIVE_4S=4000;
int Message::DEFAULT_TIME_TO_LIVE=TIME_TO_LIVE_4S;

class Group {

    /** The group id. */
    unsigned char id;

public:

    static Group ADVICE;
    static Group REQUEST;
    static Group RESPONSE;

    /**
     * Creates a new instance of Group.
     */
    Group(const unsigned char &id) {
        this->id=id;
    }

    /**
     * Returns the id.
     * @return unsigned char
     */
    unsigned char getId() {
        return id;
    }

};

Group Group::ADVICE=Group(1);
Group Group::REQUEST=Group(2);
Group Group::RESPONSE=Group(3);

/**
 * <p>The MessageType class should be used to classify message types.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-21-2010
 */
class MessageType {

    /** The type. */
    unsigned char type;

    /** The linkedType. */
    unsigned char linkedType;

    /** The group */
    Group* group;

public:

    /**
     * Creates a new instance of MessageType.
     * @param &type const unsigned char
     * @param &linkedType const unsigned char
     * @param *group Group
     */
    MessageType(const unsigned char &type, const unsigned char &linkedType, Group *group) {
        this->type=type;
        this->linkedType=linkedType;
        this->group=group;
    }

    virtual ~MessageType() {}

    /**
     * Returns the type.
     * @return unsigned char
     */
    unsigned char getType() {
        return type;
    }

    /**
     * Returns the linkedType.
     * @return unsigned char
     */
    unsigned char getLinkedType() {
        return linkedType;
    }

    /**
     * Returns true if the group is an advice.
     * @return bool
     */
    bool isAdvice() {
        if(group->getId()==Group::ADVICE.getId()) return true;
        else return false;
    }

    /**
     * Returns true if the group is a request.
     * @return bool
     */
    bool isRequest() {
        if(group->getId()==Group::REQUEST.getId()) return true;
        else return false;
    }

    /**
     * Returns true if the group is a response.
     * @return bool
     */
    bool isResponse() {
        if(group->getId()==Group::RESPONSE.getId()) return true;
        else return false;
    }

};

/**
 * <p>The TypeCollection class should be used to define supported message types collection.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-20-2010
 */
class TypeCollection {

    /** The messageTypeMap. */
    std::map<unsigned char, MessageType*> messageTypeMap;

public:

    /* constants */
    static MessageType VERSION_NOT_SUPPORTED_ADVICE;    // versionNotSupportedAdvice
    static MessageType ECHO_REQUEST;                    // echoRequest
    static MessageType ECHO_RESPONSE;                   // echoResponse
    static MessageType REGISTER_REQUEST;                // registerRequest
    static MessageType REGISTER_RESPONSE;               // registerResponse

    static int NODE;
    static unsigned char REQUEST_FAILED;
    static unsigned char REQUEST_ACCEPTED;

    /**
     * Creates a new instance of TypeCollection.
     */
    TypeCollection() {
        put(&VERSION_NOT_SUPPORTED_ADVICE);
        put(&ECHO_REQUEST);
        put(&ECHO_RESPONSE);
        put(&REGISTER_REQUEST);
        put(&REGISTER_RESPONSE);
    }

    virtual ~TypeCollection() {}

    /**
     * Puts a messageType.
     * @param *messageType MessageType
     */
    void put(MessageType *messageType) {
        messageTypeMap.erase(messageType->getType());
        messageTypeMap.insert(std::pair<unsigned char, MessageType*>(messageType->getType(), messageType));
    }

    /**
     * Returns the messageType for the given message type, otherwise NULL.
     * @param &type const unsigned char
     * @return *MessageType
     */
    MessageType* get(const unsigned char &type) {
        std::map<unsigned char, MessageType*>::iterator iter;
        iter=messageTypeMap.find(type);
        if(iter!=messageTypeMap.end()) return iter->second;
        else return NULL;
    }

    /**
     * Returns the linkedType for the given message type.
     * @return unsigned char
     */
    unsigned char getLinkedType(int type) {
        MessageType *messageType=get(type);
        if(messageType!=NULL) return messageType->getLinkedType();
        else return 0;
    }

};

MessageType TypeCollection::VERSION_NOT_SUPPORTED_ADVICE(0x00, 0xff, &Group::ADVICE);
MessageType TypeCollection::ECHO_REQUEST(0x01, 0x02, &Group::REQUEST);
MessageType TypeCollection::ECHO_RESPONSE(0x02, 0x01, &Group::RESPONSE);
MessageType TypeCollection::REGISTER_REQUEST(0xc0, 0x20, &Group::REQUEST);
MessageType TypeCollection::REGISTER_RESPONSE(0x20, 0xc0, &Group::RESPONSE);

int TypeCollection::NODE=0xc0;
unsigned char TypeCollection::REQUEST_FAILED=0;
unsigned char TypeCollection::REQUEST_ACCEPTED=1;

#endif	/* _MESSAGE_HPP */
