
// by aronin

#ifndef WEBSERV_SERVER_CONNECTION_HPP_
#define WEBSERV_SERVER_CONNECTION_HPP_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

#include <iostream>
#include <string>
#include <algorithm>
#include <exception>

#include "StringUtils.hpp"

#define TIMEOUT 100000

class	Connection
{	
private:
	Connection();

    enum state
    {
       INITIATED,
       HEADERS_RECEIVED,
       BODY_RECEIVED,
       TERMINATED,
    };

    enum EOBIdentifier
    {
        CHUNKED_ENCODING,
        CONTENT_LENGTH,
        CONNECTION_END,
    };

	int				    fd;
    struct sockaddr_in 	addr;

	int				requestTime;
	std::string		request;
	int				statusCode;
	state			state;
	
	EOBIdentifier	EOBIdentifier;
	size_t			endOfHeaders;
	int				bodyBytesExpected;

	void			setRequestTime();
	void			setEOBIdentifier();
	void			checkBodyStatus();

public:
	Connection(int fd, struct sockaddr_in addr);
	~Connection();

	bool				    transmissionEnded() const;
	bool				    isTimedOut();
	bool				    isTerminated();
	
	int					    getClientFD() const;
    struct in_addr const &  getClientIP() const;
	std::string const &	    getRequest() const;
	int					    getStatusCode() const;
	void				    updateStatusCode(int statusCode);

	void				    appendToRequest(std::string const &buffer);
	void				    reset();
	void				    terminate();
};

#endif //WEBSERV_SERVER_CONNECTION_HPP_
