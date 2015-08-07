/*
 * File:   ie.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _IE_HPP
#define	_IE_HPP

#include <string>

/**
 * </p>The IE abstract class should be used to define
 * information elements for the application protocol.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-17-2010
 */
class IE {

protected:

    /** The type. */
    unsigned char type;

public:

    /** The line separator. */
    static std::string LINE_SEPARATOR;

    virtual ~IE() {}

    /**
     * Returns the type of the information element.
     * @return unsigned char
     */
    unsigned char getType() {
        return type;
    }

    /**
     * Returns the value.
     * @param *ch unsigned char
     * @param &len int
     */
    virtual void getValue(unsigned char *ch, int &len)=0;

    /**
     * Returns the XML.
     * @return std::String
     */
    virtual std::string toXML()=0;

};

std::string IE::LINE_SEPARATOR("\r\n");

class BASIC_IE {

    /** The type. */
    unsigned char type;

    /** The name. */
    std::string name;

    BASIC_IE(const unsigned char &type, const std::string &name) {
        this->type=type;
        this->name=name;
    }

public:

    /* basic information element type */
    static BASIC_IE BYTE;               // byte
    static BASIC_IE INTEGER;            // integer
    static BASIC_IE LONG;               // long
    static BASIC_IE BYTE_ARRAY;         // byteArray
    static BASIC_IE DATE;               // date
    static BASIC_IE TIMESTAMP;          // timestamp

    /**
     * Returns the type.
     * @return unsigned char
     */
    unsigned char getType() {
        return type;
    }

    /**
     * Returns the name.
     * @return std::string
     */
    std::string getName() {
        return name;
    }

};

BASIC_IE BASIC_IE::BYTE             =BASIC_IE(0x30, "byte");
BASIC_IE BASIC_IE::INTEGER          =BASIC_IE(0xd0, "integer");
BASIC_IE BASIC_IE::LONG             =BASIC_IE(0xd1, "long");
BASIC_IE BASIC_IE::BYTE_ARRAY       =BASIC_IE(0xd2, "byteArray");
BASIC_IE BASIC_IE::DATE             =BASIC_IE(0xd3, "date");
BASIC_IE BASIC_IE::TIMESTAMP        =BASIC_IE(0xd4, "timestamp");

#endif	/* _IE_HPP */
