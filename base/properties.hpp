/*
 * File:   properties.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _PROPERTIES_HPP
#define	_PROPERTIES_HPP

#include <fstream>
#include <map>
#include <string>

/**
 * <p>The Properties class.</p>
 *
 * @author  Eduardo Marin
 * @version 1.0, 05-17-2010
 */
class Properties {

    /** The properties. */
    std::map<std::string, std::string> properties;

public:

    /**
     * Creates a new instanace of Properties.
     */
    Properties() {
    }

    virtual ~Properties() {}

    /**
     * Returns the property for the given key.
     * @param &key const std::string
     * @return std::string
     */
    std::string getProperty(const std::string &key) {
        std::map<std::string, std::string>::iterator iter;
        iter=properties.find(key);
        if(iter!=properties.end()) return iter->second;
        else return "";
    }

    /**
     * Returns the property for the given key.
     * @param &key const std::string
     * @param &defaultValue const std::string
     * @return std::string
     */
    std::string getProperty(const std::string &key, const std::string &defaultValue) {
        std::map<std::string, std::string>::iterator iter;
        iter=properties.find(key);
        if(iter!=properties.end()) return iter->second;
        else return defaultValue;
    }

    /**
     * Loads the properties from the given filename.
     * @param &filename const std::string
     */
    void load(const std::string &filename) {
        const unsigned char LINE_S =1;
        const unsigned char SPACE  =2;
        const unsigned char ESCAPE =4;
        const unsigned char COMMENT=8;
        const unsigned char KEY    =16;
        const unsigned char VALUE  =32;
        const unsigned char ESC_LS =64;

        std::ifstream in;
        in.open(filename.c_str());
        if(in.is_open()) {
            unsigned char flags=KEY;
            std::string key="";
            std::string value="";
            while(in) {
                int tmpCh=in.get();
                switch(tmpCh) {
                    case '\n':
                        flags|=LINE_S;
                        break;
                    case '\r':
                        flags|=LINE_S;
                        break;
                    case '\\':
                        flags|=ESCAPE;
                        break;
                    case ' ':
                        flags|=SPACE;
                        break;
                    case '\t':
                        flags|=SPACE;
                        break;
                    case '\f':
                        flags|=SPACE;
                        break;
                    case '=':
                        flags|=SPACE;
                        break;
                    case ':':
                        flags|=SPACE;
                        break;
                    case '#':
                        flags|=COMMENT;
                        break;
                    case '!':
                        flags|=COMMENT;
                        break;
                }
                std::string str="";
                if((flags & ESCAPE)==0) str+=(char)tmpCh;
                else { // ESCAPE
                    if(in) {
                        int escapeCh=in.get();
                        switch(escapeCh) {
                            case '\n':
                                flags|=ESC_LS;
                                flags|=SPACE;
                                break;
                            case '\r':
                                flags|=ESC_LS;
                                flags|=SPACE;
                                break;
                            case 'n':
                                str+='\n';
                                break;
                            case 'r':
                                str+='\r';
                                break;
                            case '\\':
                                str+='\\';
                                break;
                            case 't':
                                str+='\t';
                                break;
                            case 'f':
                                str+='\f';
                                break;
                            case '=':
                                str+='=';
                                break;
                            case ':':
                                str+=':';
                                break;
                            default:
                                str+='\\' + (char)escapeCh;
                        }
                    }
                    flags&=~ESCAPE;
                }
                if((flags & LINE_S)==0) {
                    if((flags & COMMENT)==0) {
                        if((flags & SPACE)==0) {
                            if((flags & KEY)!=0) key+=str;
                            else if((flags & VALUE)!=0) value+=str;
                            flags&=~ESC_LS;
                        }
                        else { // SPACE
                            if(((flags & KEY)!=0) && (key.length() > 0)) {
                                flags&=~KEY;
                                flags|=VALUE;
                            }
                            else if(((flags & VALUE)!=0) && ((flags & ESC_LS)==0) && (value.length() > 0)) value+=str;
                            flags&=~SPACE;
                        }
                    }
                }
                else { // LINE_S
                    if(key.length() > 0) setProperty(key, value);
                    key="";
                    value="";
                    flags=KEY;
                }
            }
        }
        in.close();
    }

    /**
     * Sets the given property.
     * @param &key const std::string
     * @param &value const std::string
     */
    void setProperty(const std::string &key, const std::string &value) {
        properties.erase(key);
        properties.insert(std::pair<std::string, std::string>(key, value));
    }

};

#endif	/* _PROPERTIES_HPP */
