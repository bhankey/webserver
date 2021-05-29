
// by aronin

#ifndef WEBSERV_REQUESTHANDLER_REQUESTHANDLER_HPP_
#define WEBSERV_REQUESTHANDLER_REQUESTHANDLER_HPP_

#define REPLACE			0
#define AMEND			1
#define DATESIZE		50
#define REFSIZE			40
#define RECVBUF			100000
#define NON_EXISTENT	0
#define REGULAR_FILE	1
#define DIRECTORY		2
#define RUN				"1"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <exception>

#include "ServerConfig.hpp"
#include "ServerRoutes.hpp"
#include "Connection.hpp"
#include "RequestParser.hpp"
#include "CGI.hpp"

class RequestHandler
{
private:

	enum eheader
	{
		DATE,
		SERVER,
		LOCATION,
		RETRY_AFTER,
		LAST_MODIFIED,
		WWW_AUTHENTICATE,
		ALLOW,
		CONTENT_LANGUAGE,
		CONTENT_LENGTH,
		CONTENT_LOCATION,
		CONTENT_TYPE,
		TRANSFER_ENCODING,
	};

	static const std::string	eheaderAsString(int index) {
		static const std::string eheaderAsString[] = {"Date", "Server", "Location", "Retry-After", "Last-Modified",
                                                    "WWW-Authenticate", "Allow",  "Content-Language", "Content-Length",
                                                    "Content-Location", "Content-Type", "Transfer-Encoding"};
		return eheaderAsString[index];
	}

	const ServerConfig	*server;
	const ServerRoutes  *location;
	Connection          *client;
    RequestParser       *rparser;

	std::string			rawResponse;
	int					statusCode;
	std::map<eheader, std::string>	responseHeaders;
	std::string         responseHeadersCGI;
	std::string			responseBody;

	std::string			rawRequest;
	std::string			targetResource;
	std::string			pathToTargetResource;
	std::vector<std::string> representations;

	int 				pipeIPC[2];

	std::map<std::string, void (RequestHandler::*)()> methodHandler;
	std::map<int, std::string>	        statusDescription;
    std::map<std::string, std::string>	contentTypeMIME;

	void			generateRawResponse();

	void			handleMethod();
	
	void			GET();
	void			HEAD();
	void			PUT();
	void			POST();
	void			DELETE();
	void			CONNECT();
	void			OPTIONS();
	void			TRACE();

	void			handleDisallowedMethod();
	void			handleNonImplementedMethod();

	void			generateHeaders();
	void			generateDateHeader();
	void			generateLastModifiedHeader();
	void			generateContentLengthHeader();
    void			generateContentTypeHeader();
    void 			generateLocationHeader();
	void			generateAllowHeader();

	void			findAllRepresentations();
	void			uploadTargetResource(bool mode);
	void			writeToTargetResource(bool APPEND);
	void			readFromTargetResource();
	void			retrieveErrorPage();
	void			selectIndexFile();
	void			handleAutoIndex();
	bool			authenticate();
	void			negotiateContent();
	void            runCGI();
	void            checkHeadersCGI();
	void            formatLineBreaks();
	std::string		createTimestamp(time_t time);
	std::string     getExtension();
	int 			getType();

	void			initMethodHandler();
	void			initStatusDict();
	void            initMIMETypeDict();
	void			reset(const ServerConfig *server, Connection *client);
	void            parseRequest();

public:
	RequestHandler();
	~RequestHandler();

	void	process(const ServerConfig *server, Connection *client);

	std::string	const &     getRawResponse() const;
};

#endif //WEBSERV_REQUESTHANDLER_REQUESTHANDLER_HPP_