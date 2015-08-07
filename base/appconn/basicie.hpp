/*
 * File:   basicie.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _BASICIE_HPP
#define	_BASICIE_HPP

#include "ie.hpp"

#include <sstream>
#include <time.h>

/**
 * <p>The BasicIE abstract class should be used to handle basic 
 * information elements for the application protocol.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-17-2010
 */
class BasicIE : public IE {

protected:

    /**
     * Constructor for use by subclasses.
     */
    BasicIE() {}

    /**
     * Constructor for use by subclasses.
     * @param &type const unsigned char
     */
    BasicIE(const unsigned char &type) {
        this->type=type;
    }

    virtual ~BasicIE() {}

    /**
     * Returns the XML representation for a basicIE type and value.
     * @param basicIE BASIC_IE
     * @param value std::string
     * @return std::string
     */
    std::string toXML(BASIC_IE basicIE, std::string value) {
        std::stringstream sstream;
        sstream << "<basicIE type=\"" << basicIE.getName() << "\" value=\"" << value << "\"/>" + LINE_SEPARATOR;
        return sstream.str();
    }

};

/**
 * <p>The IEByte class should be used to handle byte data type
 * for the application protocol.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-17-2010
 */
class IEByte : public BasicIE {

    /** The ieByte. */
    unsigned char ieByte;

public:

    /**
     * Creates a new instance of IEByte.
     * @param &ieByte const unsigned char
     */
    IEByte(const unsigned char &ieByte) : BasicIE(BASIC_IE::BYTE.getType()) {
        this->ieByte=ieByte;
    }

    /**
     * Creates a new instance of IEByte.
     * @param *ch const unsigned char
     * @param &len const int
     */
    IEByte(const unsigned char *ch, const int &len) : BasicIE(BASIC_IE::BYTE.getType()) {
        ieByte=*ch;
    }

    void getValue(unsigned char *ch, int &len) {
        len=1;
        ch[0]=ieByte;
    }

    std::string toXML() {
        std::stringstream sstream;
        sstream << (int)ieByte;
        return BasicIE::toXML(BASIC_IE::BYTE, sstream.str());
    }

    /**
     * Returns the ieByte.
     * @return unsigned char
     */
    unsigned char getIEByte() {
        return ieByte;
    }

};

/**
 * <p>The IEInteger class should be used to handle integer data type
 * for the application protocol.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-17-2010
 */
class IEInteger : public BasicIE {

    /** The ieInteger. */
    int ieInteger;

public:

    /**
     * Creates a new instance of IEInteger.
     * @param &ieInteger const unsigned char
     */
    IEInteger(const int &ieInteger) : BasicIE(BASIC_IE::INTEGER.getType()) {
        this->ieInteger=ieInteger;
    }

    /**
     * Creates a new instance of IEInteger.
     * @param *ch const unsigned char
     * @param &len const int
     */
    IEInteger(const unsigned char *ch, const int &len) : BasicIE(BASIC_IE::INTEGER.getType()) {
        if(len > 1) {
            ieInteger=ch[0]; ieInteger<<=8;
            ieInteger+=ch[1];
        }
    }

    void getValue(unsigned char *ch, int &len) {
        len=2;
        ch[0]=(ieInteger >> 8) & 0xff;
        ch[1]=ieInteger & 0xff;
    }

    std::string toXML() {
        std::stringstream sstream;
        sstream << ieInteger;
        return BasicIE::toXML(BASIC_IE::INTEGER, sstream.str());
    }

    /**
     * Returns the ieInteger.
     * @return int
     */
    int getIEInteger() {
        return ieInteger;
    }

};

/**
 * <p>The IELong class should be used to handle long data type
 * for the application protocol.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-17-2010
 */
class IELong : public BasicIE {

    /** The ieLong. */
    long ieLong;

public:

    /**
     * Creates a new instance of IELong.
     * @param &ieLong const unsigned char
     */
    IELong(const long &ieLong) : BasicIE(BASIC_IE::LONG.getType()) {
        this->ieLong=ieLong;
    }

    /**
     * Creates a new instance of IELong.
     * @param *ch const unsigned char
     * @param &len const int
     */
    IELong(const unsigned char *ch, const int &len) : BasicIE(BASIC_IE::LONG.getType()) {
        if(len > 3) {
            ieLong=ch[0]; ieLong<<=8;
            ieLong+=ch[1]; ieLong<<=8;
            ieLong+=ch[2]; ieLong<<=8;
            ieLong+=ch[3];
        }
    }

    void getValue(unsigned char *ch, int &len) {
        len=4;
        ch[0]=(ieLong>>24) & 0xff;
        ch[1]=(ieLong>>16) & 0xff;
        ch[2]=(ieLong>>8) & 0xff;
        ch[3]=ieLong & 0xff;
    }

    std::string toXML() {
        std::stringstream sstream;
        sstream << ieLong;
        return BasicIE::toXML(BASIC_IE::LONG, sstream.str());
    }

    /**
     * Returns the ieLong.
     * @return long
     */
    long getIELong() {
        return ieLong;
    }

};

/**
 * <p>The IEByteArray class should be used to handle a byte array data type
 * for the application protocol.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-17-2010
 */
class IEByteArray : public BasicIE {

    /** The ieByteArray. */
    unsigned char *ieByteArray;
    int len;

public:

    /**
     * Creates a new instance of IEByteArray.
     * @param &ieLong const unsigned char
     */
    IEByteArray(const std::string &string) : BasicIE(BASIC_IE::BYTE_ARRAY.getType()) {
        len=string.length();
        if(len > 0) {
            ieByteArray=new unsigned char[len];
            for(int i=0; i < len; i++) ieByteArray[i]=string.at(i);
        }
    }

    /**
     * Creates a new instance of IEByteArray.
     * @param *ch const unsigned char
     * @param &len const int
     */
    IEByteArray(const unsigned char *ch, const int &len) : BasicIE(BASIC_IE::BYTE_ARRAY.getType()) {
        if(len > 0) {
            ieByteArray=new unsigned char[len];
            for(int i=0; i < len; i++) ieByteArray[i]=ch[i];
        }
        this->len=len;
    }

    ~IEByteArray() {
        if(len > 0) delete[] ieByteArray;
    }

    void getValue(unsigned char *ch, int &len) {
        len=this->len;
        for(int i=0; i < this->len; i++) {
            ch[i]=ieByteArray[i];
        }
    }

    std::string toXML() {
        return BasicIE::toXML(BASIC_IE::BYTE_ARRAY, getIEString());
    }

    /**
     * Returns the ieByteArray.
     * @param *ch unsigned char
     * @param &len int
     */
    void getIEByteArray(unsigned char *ch, int &len) {
        len=this->len;
        for(int i=0; i < len; i++) ch[i]=ieByteArray[i];
    }

    /**
     * Returns the ieString.
     * @return std::string
     */
    std::string getIEString() {
        std::string string="";
        if(len > 0) {
            char *tmpByteArray=new char[len + 1];
            for(int i=0; i < len; i++) tmpByteArray[i]=ieByteArray[i];
            tmpByteArray[len]='\0';
            string=std::string(tmpByteArray);
            delete[] tmpByteArray;
        }
        return string;
    }

};

/**
 * <p>The IEDate class should be used to handle date data type
 * for the application protocol.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-17-2010
 */
class IEDate : public BasicIE {

    /* The ieDate. */
    tm ieDate;

public:

    /**
     * Creates a new instance of IEDate.
     * @param &ieDate const time_t
     */
    IEDate(const tm &ieDate) : BasicIE(BASIC_IE::DATE.getType()) {
        this->ieDate=ieDate;
    }

    /**
     * Creates a new instance of IEDate.
     * @param *ch const unsigned char
     * @param &len const int
     */
    IEDate(const unsigned char *ch, const int &len) : BasicIE(BASIC_IE::DATE.getType()) {
        if(len > 3) {
            int year=((ch[0]>>4) & 0x0f); year*=10;
            year+=(ch[0] & 0x0f); year*=10;
            year+=((ch[1]>>4) & 0x0f); year*=10;
            year+=(ch[1] & 0x0f);

            int mon=(ch[2]>>4) & 0x0f; mon*=10;
            mon+=ch[2] & 0x0f;

            int mday=(ch[3]>>4) & 0x0f; mday*=10;
            mday+=ch[3] & 0x0f;

            this->ieDate.tm_year=year - 1900;
            this->ieDate.tm_mon=mon - 1;
            this->ieDate.tm_mday=mday;
        }
    }

    void getValue(unsigned char *ch, int &len) {
        len=4;
        int year=ieDate.tm_year + 1900;
        ch[1]=(year % 10) & 0x0f; year/=10;
        ch[1]+=((year % 10) & 0x0f) << 4; year/=10;
        ch[0]=(year % 10) & 0x0f; year/=10;
        ch[0]+=((year % 10) & 0x0f) << 4;

        int mon=ieDate.tm_mon + 1;
        ch[2]=(mon % 10) & 0x0f; mon/=10;
        ch[2]+=((mon % 10) & 0x0f) << 4;

        int mday=ieDate.tm_mday;
        ch[3]=(mday % 10) & 0x0f; mday/=10;
        ch[3]+=((mday % 10) & 0x0f) << 4;
    }

    std::string toXML() {
        std::stringstream sstream;
        sstream << (ieDate.tm_year + 1900) << "-" << (ieDate.tm_mon + 1) << "-" << ieDate.tm_mday;
        return BasicIE::toXML(BASIC_IE::DATE, sstream.str());
    }

    /**
     * Returns the ieDate.
     * @return tm
     */
    tm getIEDate() {
        return ieDate;
    }

};

/**
 * <p>The IETimestamp class should be used to handle timestamp data type
 * for the application protocol.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-17-2010
 */
class IETimestamp : public BasicIE {

    /* The ieTimestamp. */
    tm ieTimestamp;

public:

    /**
     * Creates a new instance of IETimestamp.
     * @param &ieTimestamp const time_t
     */
    IETimestamp(const tm &ieTimestamp) : BasicIE(BASIC_IE::TIMESTAMP.getType()) {
        this->ieTimestamp=ieTimestamp;
    }

    /**
     * Creates a new instance of IETimestamp.
     * @param *ch const unsigned char
     * @param &len const int
     */
    IETimestamp(const unsigned char *ch, const int &len) : BasicIE(BASIC_IE::TIMESTAMP.getType()) {
        if(len > 7) {
            int year=((ch[0]>>4) & 0x0f); year*=10;
            year+=(ch[0] & 0x0f); year*=10;
            year+=((ch[1]>>4) & 0x0f); year*=10;
            year+=(ch[1] & 0x0f);

            int mon=(ch[2]>>4) & 0x0f; mon*=10;
            mon+=ch[2] & 0x0f;

            int mday=(ch[3]>>4) & 0x0f; mday*=10;
            mday+=ch[3] & 0x0f;

            int hour=(ch[4]>>4) & 0x0f; hour*=10;
            hour+=ch[4] & 0x0f;

            int min=(ch[5]>>4) & 0x0f; min*=10;
            min+=ch[5] & 0x0f;

            int sec=(ch[6]>>4) & 0x0f; sec*=10;
            sec+=ch[6] & 0x0f;

            this->ieTimestamp.tm_year=year - 1900;
            this->ieTimestamp.tm_mon=mon - 1;
            this->ieTimestamp.tm_mday=mday;
            this->ieTimestamp.tm_hour=hour;
            this->ieTimestamp.tm_min=min;
            this->ieTimestamp.tm_sec=sec;
            this->ieTimestamp.tm_wday=ch[7] - 1;
        }
    }

    void getValue(unsigned char *ch, int &len) {
        len=8;
        int year=ieTimestamp.tm_year + 1900;
        ch[1]=(year % 10) & 0x0f; year/=10;
        ch[1]+=((year % 10) & 0x0f) << 4; year/=10;
        ch[0]=(year % 10) & 0x0f; year/=10;
        ch[0]+=((year % 10) & 0x0f) << 4;

        int mon=ieTimestamp.tm_mon + 1;
        ch[2]=(mon % 10) & 0x0f; mon/=10;
        ch[2]+=((mon % 10) & 0x0f) << 4;

        int mday=ieTimestamp.tm_mday;
        ch[3]=(mday % 10) & 0x0f; mday/=10;
        ch[3]+=((mday % 10) & 0x0f) << 4;

        int hour=ieTimestamp.tm_hour;
        ch[4]=(hour % 10) & 0x0f; hour/=10;
        ch[4]+=((hour % 10) & 0x0f) << 4;

        int min=ieTimestamp.tm_min;
        ch[5]=(min % 10) & 0x0f; min/=10;
        ch[5]+=((min % 10) & 0x0f) << 4;

        int sec=ieTimestamp.tm_sec;
        ch[6]=(sec % 10) & 0x0f; sec/=10;
        ch[6]+=((sec % 10) & 0x0f) << 4;

        ch[7]=ieTimestamp.tm_wday + 1;
    }

    std::string toXML() {
        std::stringstream sstream;
        sstream << (ieTimestamp.tm_year + 1900) << "-" << (ieTimestamp.tm_mon + 1) << "-" << ieTimestamp.tm_mday << " " << ieTimestamp.tm_hour << ":" << ieTimestamp.tm_min << ":" << ieTimestamp.tm_sec;
        return BasicIE::toXML(BASIC_IE::TIMESTAMP, sstream.str());
    }

    /**
     * Returns the ieTimestamp.
     * @return tm
     */
    tm getIETimestamp() {
        return ieTimestamp;
    }

};

#endif	/* _BASICIE_HPP */
