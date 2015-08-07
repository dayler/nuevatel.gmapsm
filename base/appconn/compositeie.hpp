/*
 * File:   compositeie.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _COMPOSITEIE_HPP
#define	_COMPOSITEIE_HPP

#include "basicie.hpp"

#include <map>
#include <vector>

/**
 * <p>The CompositeIE class should be used to handle composite 
 * information elements for the application protocol.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-18-2010
 */
class CompositeIE : public IE {

protected:

    /** The information element value. */
    unsigned char value;

    /** The data types information elements map. */
    std::map<unsigned char, IE*> ieMap;

    /**
     * Creates an empty instance of CompositeIE.
     */
    CompositeIE() {}

public:

    /**
     * Creates a new instance of CompositeIE.
     * @param *compositeIE CompositeIE
     */
    CompositeIE(CompositeIE *compositeIE) {
        this->type=compositeIE->getType();
        if(type < 128) // Type-Value Representation
            this->value=compositeIE->getValueByte();
        else { // Type-Length-Value Representation
            std::map<unsigned char, IE*>::iterator iter;
            for(iter=compositeIE->ieMap.begin(); iter!=compositeIE->ieMap.end(); iter++)
                putIE(iter->second);
        }
    }

    /**
     * Creates a new instance of CompositeIE.
     * @param &type const unsigned char
     * @param &value const unsigned char
     * @param *ieVector std::vector<IE*>
     */
    CompositeIE(const unsigned char &type, const unsigned char &value, std::vector<IE*> *ieVector) {
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
     * Creates a new instance of CompositeIE.
     * @param &type const unsigned char
     * @param *ch unsigned char
     * @param &len const int
     */
    CompositeIE(const unsigned char &type, unsigned char *ch, const int &len) {
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

    virtual ~CompositeIE() {
        std::map<unsigned char, IE*>::iterator iter;
        for(iter=ieMap.begin(); iter!=ieMap.end(); iter++) delete iter->second;
    }

    void getValue(unsigned char *ch, int &len) {
        if(type < 128) { // Type-Value Representation
            len=1;
            ch[0]=value;
        }
        else { // Type-Length-Value Representation
            int chIndex=0;
            std::map<unsigned char, IE*>::iterator iter;
            for(iter=ieMap.begin(); iter!=ieMap.end(); iter++) {
                IE *ie=iter->second;
                unsigned char ieValue[255];
                int ieLen;
                ie->getValue(ieValue, ieLen);
                if(ie->getType() < 128) {
                    ch[chIndex + 0]=ie->getType();
                    ch[chIndex + 1]=ieValue[0];
                    chIndex+=2;
                }
                else {
                    ch[chIndex + 0]=ie->getType();
                    ch[chIndex + 1]=ieLen;
                    for(int i=0; i < ieLen; i++) ch[chIndex + 2 + i]=ieValue[i];
                    chIndex+=(2 + ieLen);
                }
            }
            len=chIndex;
        }
    }

    virtual std::string toXML() {
        std::stringstream sstream;
        sstream << "<compositeIE type=\"" << (int)type << "\"";
        if(type < 128) sstream << " value=\"" << (int)value << "\"";
        sstream << ">" << LINE_SEPARATOR;
        std::map<unsigned char, IE*>::iterator iter;
        for(iter=ieMap.begin(); iter!=ieMap.end(); iter++) sstream << "\t" << iter->second->toXML();
        sstream << "</compositeIE>" << LINE_SEPARATOR;
        return sstream.str();
    }

protected:

    /**
     * Returns the ie for the given type and value.
     * @param &type const unsigned char
     * @param *ch unsigned char
     * @param &len const int
     * @return *IE
     */
    virtual IE* getIE(const unsigned char &type, unsigned char *ch, const int &len) {
        IE *ie;
        ie=NULL;
        if(type==BASIC_IE::BYTE.getType()) ie=new IEByte(ch, len);                  // byte
        else if(type==BASIC_IE::INTEGER.getType()) ie=new IEInteger(ch, len);       // integer
        else if(type==BASIC_IE::LONG.getType()) ie=new IELong(ch, len);             // long
        else if(type==BASIC_IE::BYTE_ARRAY.getType()) ie=new IEByteArray(ch, len);  // byteArray
        else if(type==BASIC_IE::DATE.getType()) ie=new IEDate(ch, len);             // date
        else if(type==BASIC_IE::TIMESTAMP.getType()) ie=new IETimestamp(ch, len);   // timestamp
        return ie;
    }

public:

    /**
     * Returns the value of this information element.
     * @return unsigned char
     */
    unsigned char getValueByte() {
        return value;
    }

    /**
     * Puts a IE in this information element.
     * @param *ie IE
     */
    virtual void putIE(IE *ie) {
        try {
            // IE
            IE *tmpIE;
            tmpIE=NULL;
            if(ie->getType()==BASIC_IE::BYTE.getType()) {               // byte
                IEByte *ieByte;
                if((ieByte=dynamic_cast<IEByte*>(ie))!=NULL)
                    tmpIE=new IEByte(ieByte->getIEByte());
            }
            else if(ie->getType()==BASIC_IE::INTEGER.getType()) {       // integer
                IEInteger *ieInteger;
                if((ieInteger=dynamic_cast<IEInteger*>(ie))!=NULL)
                    tmpIE=new IEInteger(ieInteger->getIEInteger());
            }
            else if(ie->getType()==BASIC_IE::LONG.getType()) {          // long
                IELong *ieLong;
                if((ieLong=dynamic_cast<IELong*>(ie))!=NULL)
                    tmpIE=new IELong(ieLong->getIELong());
            }
            else if(ie->getType()==BASIC_IE::BYTE_ARRAY.getType()) {    // byteArray
                IEByteArray *ieByteArray;
                if((ieByteArray=dynamic_cast<IEByteArray*>(ie))!=NULL) {
                    unsigned char ch[255];
                    int len;
                    ieByteArray->getIEByteArray(ch, len);
                    tmpIE=new IEByteArray(ch, len);
                }
            }
            else if(ie->getType()==BASIC_IE::DATE.getType()) {          // date
                IEDate *ieDate;
                if((ieDate=dynamic_cast<IEDate*>(ie))!=NULL)
                    tmpIE=new IEDate(ieDate->getIEDate());
            }
            else if(ie->getType()==BASIC_IE::TIMESTAMP.getType()) {     // timestamp
                IETimestamp *ieTimestamp;
                if((ieTimestamp=dynamic_cast<IETimestamp*>(ie))!=NULL)
                    tmpIE=new IETimestamp(ieTimestamp->getIETimestamp());
            }
            if(tmpIE!=NULL) {
                removeIE(tmpIE->getType());
                ieMap.insert(std::pair<unsigned char, IE*>(tmpIE->getType(), tmpIE));
            }
        } catch(std::bad_cast) {}
    }

    /**
     * Returns true if the compositeIE contains the specified ie.
     * @param &ieType const unsigned char
     * @return boolean
     */
    bool containsIE(const unsigned char &ieType) {
        if(ieMap.find(ieType)!=ieMap.end()) return true;
        else return false;
    }

    /**
     * Returns an IE of this information element.
     * @param &ieType const unsigned char
     * @return *IE
     */
    IE* getIE(const unsigned char &ieType) {
        IE *ie;
        ie=NULL;
        std::map<unsigned char, IE*>::iterator iter;
        iter=ieMap.find(ieType);
        if(iter!=ieMap.end()) ie=iter->second;
        return ie;
    }

    /**
     * Removes an IE of this information element.
     * @param &ieType const unsigned char
     */
    void removeIE(const unsigned char &ieType) {
        std::map<unsigned char, IE*>::iterator iter;
        iter=ieMap.find(ieType);
        if(iter!=ieMap.end()) {
            ieMap.erase(iter);
            delete iter->second;
        }
    }

    /**
     * Returns the ieByte, otherwise returns 0.
     * @return unsigned char
     */
    unsigned char getIEByte() {
        IE *ie=getIE(BASIC_IE::BYTE.getType());
        if(ie!=NULL) {
            try {
                IEByte *ieByte=dynamic_cast<IEByte*>(ie);
                return ieByte->getIEByte();
            } catch(std::bad_cast) {
                return 0;
            }
        }
        else return 0;
    }

    /**
     * Returns the ieInteger, otherwise returns 0.
     * @return int
     */
    int getIEInteger() {
        IE *ie=getIE(BASIC_IE::INTEGER.getType());
        if(ie!=NULL) {
            try {
                IEInteger *ieInteger=dynamic_cast<IEInteger*>(ie);
                return ieInteger->getIEInteger();
            } catch(std::bad_cast) {
                return 0;
            }
        }
        else return 0;
    }

    /**
     * Returns the ieLong, otherwise returns 0.
     * @return long
     */
    long getIELong() {
        IE *ie=getIE(BASIC_IE::LONG.getType());
        if(ie!=NULL) {
            try {
                IELong *ieLong=dynamic_cast<IELong*>(ie);
                return ieLong->getIELong();
            } catch(std::bad_cast) {
                return 0l;
            }
        }
        else return 0l;
    }

    /**
     * Returns the ieByteArray, otherwise NULL pointer.
     * @param *ch unsigned char
     * @param &len int
     */
    void getIEByteArray(unsigned char *ch, int &len) {
        IE *ie=getIE(BASIC_IE::BYTE_ARRAY.getType());
        if(ie!=NULL) {
            try {
                IEByteArray *ieByteArray=dynamic_cast<IEByteArray*>(ie);
                ieByteArray->getIEByteArray(ch, len);
            } catch(std::bad_cast) {
                ch=NULL;
                len=0;
            }
        }
        else {
            ch=NULL;
            len=0;
        }
    }

    /**
     * Returns the ieString, otherwise empty string.
     * @return std::string
     */
    std::string getIEString() {
        IE *ie=getIE(BASIC_IE::BYTE_ARRAY.getType());
        if(ie!=NULL) {
            try {
                IEByteArray *ieByteArray=dynamic_cast<IEByteArray*>(ie);
                return ieByteArray->getIEString();
            } catch(std::bad_cast) {
                return "";
            }
        }
        else return "";
    }

    /**
     * Returns the ieDate, otherwise current tm.
     * @return tm
     */
    tm getIEDate() {
        IE *ie=getIE(BASIC_IE::DATE.getType());
        if(ie!=NULL) {
            try {
                IEDate *ieDate=dynamic_cast<IEDate*>(ie);
                return ieDate->getIEDate();
            } catch(std::bad_cast) {
                tm tmpTM;
                return tmpTM;
            }
        }
        else {
            tm tmpTM;
            return tmpTM;
        }
    }

    /**
     * Returns the ieTimestamp, otherwise current tm.
     * @return tm
     */
    tm getIETimestamp() {
        IE *ie=getIE(BASIC_IE::TIMESTAMP.getType());
        if(ie!=NULL) {
            try {
                IETimestamp *ieTimestamp=dynamic_cast<IETimestamp*>(ie);
                return ieTimestamp->getIETimestamp();
            } catch(std::bad_cast) {
                tm tmpTM;
                return tmpTM;
            }
        }
        else {
            tm tmpTM;
            return tmpTM;
        }
    }

};

#endif	/* _COMPOSITEIE_HPP */
