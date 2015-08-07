/*
 * File:   socket.hpp
 *
 * NuevaTel PCS de Bolivia S.A. (C) 2010
 */

#ifndef _SOCKET_HPP
#define	_SOCKET_HPP

#include "exception.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * <p>The SocketException class.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-16-2010
 */
class SocketException : public Exception {

public:

    SocketException() : Exception() {
        this->name="SocketException";
    }

    SocketException(const std::string &message, const std::string &filename, const int &line) : Exception(message, filename, line) {
        this->name="SocketException";
    }

};

/**
 * <p>The Socket class.</p>
 *
 * @author  unascribed
 * @version 1.0, 04-16-2010
 */
class Socket {

    /** The socket FD. */
    int socketFD;

    /** The addressInfo. */
    addrinfo *addressInfo;

public:

    /**
     * Creates an unconnected socket.
     */
    Socket() {
        socketFD=-1;
    }

    /**
     * Creates a connected socket for the given host and port.
     * @param &host const std::string
     * @param &port const int
     * @throws SocketException
     */
    Socket(const std::string &host, const int &port) throw(SocketException) {
        socketFD=-1;
        connect(host, port);
    }

    virtual ~Socket() {
        freeaddrinfo(addressInfo);
    }

    /**
     * Connects this socket to the given host and port.
     * @param &host const std::string
     * @param &port const int
     * @throws SocketException
     */
    void connect(const std::string &host, const int &port) throw(SocketException) {
        if(!isConnected()) {
            std::stringstream sstream;
            sstream << port;
            std::string strPort=sstream.str();

            addrinfo hints;

            memset(&hints, 0, sizeof(hints));
            hints.ai_family=AF_UNSPEC;
            hints.ai_socktype=SOCK_STREAM;

            addrinfo *ai;
            if(int rv=getaddrinfo(host.c_str(), strPort.c_str(), &hints, &ai)==0) {
                for(addressInfo=ai; addressInfo!=NULL; addressInfo=ai->ai_next) {
                    if((socketFD=socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol))!=-1) {
                        if(::connect(socketFD, addressInfo->ai_addr, addressInfo->ai_addrlen)!=-1) break;
                        else close();
                    }
                }
                if(addressInfo==NULL || socketFD==-1) throw SocketException("connect failed", __FILE__, __LINE__);
            }
            else throw SocketException("getaddrinfo error: " + std::string(gai_strerror(rv)), __FILE__, __LINE__);
        }
        else throw SocketException("socket already connected", __FILE__, __LINE__);
    }

    /**
     * Closes this socket.
     */
    void close() {
        if(socketFD!=-1) ::close(socketFD);
        socketFD=-1;
    }

    /**
     * Returns the remote host address.
     * @return std::string
     */
    std::string getRemoteHostAddress() {
        if(addressInfo!=NULL) {
            char str[INET6_ADDRSTRLEN];
            inet_ntop(addressInfo->ai_family, get_in_addr((sockaddr *)addressInfo->ai_addr), str, sizeof(str));
            return std::string(str);
        }
        else return "";
    }

    /**
     * Returns true if the socket is connected.
     * @return bool
     */
    bool isConnected() {
        if(socketFD!=-1) return true;
        else return false;
    }

    /**
     * Reads a byte from the socket and returns its integer representation.
     * Returns -1 if the socket is closed.
     * @return int
     */
    int read() {
        unsigned char ch[1];
        int res=read(ch, 1);
        if(res > 0) return ch[0];
        else return res;
    }

    /**
     * Reads len bytes from the socket into the ch array. Returns -1 if the socket is closed.
     * @param *ch unsigned char
     * @param &len const int
     * @return int
     */
    int read(unsigned char *ch, const int &len) {
        while(isConnected()) {
            int res=recv(socketFD, ch, len, 0);
            if(res<=0) {
                if(errno!=EINTR) {
                    close();
                    throw SocketException("read error: " + std::string(strerror(errno)), __FILE__, __LINE__);
                }
            }
            else return res;
        }
        return -1;
    }

    /**
     * Reads len bytes from the socket into the ch array starting at off position.
     * Returns -1 if the socket is closed.
     * @param *ch unsigned char
     * @param &off const int
     * @param &len const int
     * @return int
     */
    int read(unsigned char *ch, const int &off, const int &len) {
        return read(ch + off, len);
    }

    /**
     * Writes len bytes form the ch array in the socket.
     * @param *ch unsigned char
     * @param &len const int
     */
    void write(unsigned char *ch, const int &len) {
        if(isConnected()) {
            int res=send(socketFD, ch, len, 0);
            if(res==-1) {
                close();
                throw SocketException("write error: " + std::string(strerror(errno)), __FILE__, __LINE__);
            }
        }
    }

    /**
     * Writes len bytes form the ch array in the socket starting at off position.
     * @param *ch unsigned char
     * @param &off const int
     * @param &len const int
     */
    void write(unsigned char *ch, const int &off, const int &len) {
        write(ch + off, len);
    }

private:

    /**
     * Returns the sockaddr according to the sa_family.
     * @param *sa sockaddr
     * @return void*
     */
    void *get_in_addr(sockaddr *sa) {
        if(sa->sa_family==AF_INET) {
            return &(((sockaddr_in*)sa)->sin_addr);
        }
        return &(((sockaddr_in6*)sa)->sin6_addr);
    }

};

#endif	/* _SOCKET_HPP */
