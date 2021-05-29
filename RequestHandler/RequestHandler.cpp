
// by aronin

#include "../includes/RequestHandler.hpp"

RequestHandler::RequestHandler()
{
	rparser = new RequestParser;

	if (pipe(pipeIPC) == -1)
		throw std::runtime_error(strerror(errno));
	write(pipeIPC[1], RUN, 1);

	initMethodHandler();
	initStatusDict();
	initMIMETypeDict();
}

RequestHandler::~RequestHandler()
{
	delete rparser;

	close(pipeIPC[0]);
	close(pipeIPC[1]);
}

void	RequestHandler::process(const ServerConfig *server, Connection *client)
{
	reset(server, client);
	parseRequest();

	if (statusCode < 400 && authenticate())
		handleMethod();

	if (statusCode >= 400)
		retrieveErrorPage();

	generateHeaders();
	generateRawResponse();

	client->updateStatusCode(statusCode);
}

void	RequestHandler::reset(const ServerConfig *server, Connection *client)
{
	this->server = server;
	this->client = client;
	statusCode = client->getStatusCode();
	rawRequest = client->getRequest();

	rawResponse.clear();
	responseHeaders.clear();
	responseHeadersCGI.clear();
	responseBody.clear();
	pathToTargetResource.clear();
	representations.clear();
}

void    RequestHandler::parseRequest()
{
	if (this->statusCode >= 400) return;

	statusCode = std::max((int)rparser->ParseRequest(
						rawRequest), statusCode);
	targetResource = rparser->GetTargetOfRequest();
	location = &server->GetRoute(targetResource,
								 rparser->GetMethod());
	pathToTargetResource = location->GetRoot() + targetResource;

	if (rparser->GetBody().size() > location->GetClientMaxBodySize())
		statusCode = 413;
}

bool	RequestHandler::authenticate()
{
	if (!server->IsAuth()) return true;

	std::map<std::string, std::string> authFile
								= server->GetAuth();
	std::map<std::string, std::string>::iterator authCredentials;
	std::vector<RequestParser::sAuthorization> userCredentials
	 							= rparser->GetAuthorization();

	if (!userCredentials.empty()
			&& !userCredentials.front().type.compare("Basic"))
	{
		authCredentials = authFile.find(userCredentials.front().username);
		if (authCredentials != authFile.end()
			&& userCredentials.front().password == authCredentials->second)
			return true;
	}
	statusCode = 401;
	return false;
}

std::string	const & RequestHandler::getRawResponse() const
{
	return rawResponse;
}

void	RequestHandler::handleMethod()
{
	std::string	method = rparser->GetMethod();
	std::vector<std::string> allowedMethods = location->GetRequestMethod();

	if (std::find(allowedMethods.begin(), allowedMethods.end(),
				  method) != allowedMethods.end())
		(this->*methodHandler[method])();
	else if (methodHandler.find(method) != methodHandler.end())
		handleDisallowedMethod();
	else
		handleNonImplementedMethod();
}

void		RequestHandler::GET()
{
	int fileType = getType();

	std::string extCGI = location->GetExtensionCgi();
	if (!extCGI.empty() && extCGI == getExtension()) {
		runCGI(); return;
	}

	if (fileType == DIRECTORY)
		selectIndexFile();
	else
		findAllRepresentations();

	if (!representations.empty()) {
		negotiateContent(); readFromTargetResource();
		responseHeaders[CONTENT_LOCATION]
				= pathToTargetResource.substr(location->GetRoot().length());
	}
	else if (fileType == DIRECTORY && location->IsAutoIndex())
		handleAutoIndex();
	else
		statusCode = 404;
}

void		RequestHandler::HEAD()
{
	GET();
}

void		RequestHandler::POST()
{
	std::string extCGI = location->GetExtensionCgi();

	if (!extCGI.empty() && extCGI == getExtension()) runCGI();
	else uploadTargetResource(AMEND);
}

void		RequestHandler::PUT()
{
	uploadTargetResource(REPLACE);
}

void		RequestHandler::DELETE()
{
	if (representations.empty()) findAllRepresentations();

	for (size_t i = 0; i < representations.size(); ++i)
	{
		if (!unlink(representations[i].c_str()) || ((errno == EPERM
		|| errno == EISDIR) && !rmdir(representations[i].c_str())))
			statusCode = std::max(204, statusCode);
		else if (errno == ENOTEMPTY) statusCode = 409;
		else if (errno == EACCES || errno == EROFS) statusCode = 403;
		else if (statusCode == 204) statusCode = 404;
	}
}

void		RequestHandler::CONNECT() {}

void		RequestHandler::OPTIONS()
{
	generateAllowHeader();
}

void		RequestHandler::TRACE()
{
	responseBody = rawRequest;
	responseHeaders[CONTENT_TYPE] = "html/message";
}

void		RequestHandler::handleDisallowedMethod()
{
	statusCode = 405;
	generateAllowHeader();
}

void		RequestHandler::handleNonImplementedMethod()
{
	statusCode = 501;
}

void	RequestHandler::findAllRepresentations()
{
	DIR				*dir;
	struct dirent	*entry;
	struct stat 	statbuf;
	std::string		targetDirPath, entryPath,
					targetPath(pathToTargetResource);

	if (!stat(pathToTargetResource.c_str(), &statbuf))
		 representations.push_back(pathToTargetResource);

	if (*pathToTargetResource.rbegin() == '/') {
		pathToTargetResource.erase(pathToTargetResource.end() - 1);
		findAllRepresentations();
		pathToTargetResource = targetPath;
	}

	targetPath += ".";
	targetDirPath = targetPath.substr(0, targetPath.rfind('/'));

	if (!(dir = opendir(targetDirPath.c_str()))) return;
	while ((entry = readdir(dir))) {
		entryPath = targetDirPath + "/" + entry->d_name;
		if (!targetPath.compare(0, targetPath.length(),
						   entryPath.c_str(), targetPath.length()))
			representations.push_back(entryPath);
	}
	closedir(dir);
}

void		RequestHandler::uploadTargetResource(bool mode)
{
	if (!location->IsUpload()) {
		statusCode = 403; return;
	}

	std::string root = location->GetRoot();
	pathToTargetResource = root + location->GetPathForUpload()
										+ targetResource;

	findAllRepresentations();
	if (representations.empty()) statusCode = 201;
	else if (mode == REPLACE) DELETE(); // consider errors in file deletion
	else statusCode = 200;

	bool finalRes(false);
	size_t slashPos = root.length() + 1;
	std::string fullPath(pathToTargetResource);
	removeDoubles(fullPath, '/');
	std::string contentType = rparser->GetContentType();
	contentType.erase(0, contentType.find("/") + 1);

	do {
		if (!(slashPos = fullPath.find('/', slashPos) + 1)
				|| slashPos == fullPath.length()) {
			slashPos = std::string::npos;
			finalRes = true;
		}
		pathToTargetResource = fullPath.substr(0, slashPos);

		if (!finalRes || getExtension().empty() ||
					!contentType.compare("x-directory")) {
			if (finalRes && !rparser->GetBody().empty()) statusCode = 415;
			else if (mkdir(pathToTargetResource.c_str(), 0777) == -1
					&& (finalRes || errno != EEXIST))
				statusCode = errno == EEXIST ? 303 :
							 (errno == ENOTDIR ? 409 : 500);
		}
		else if (mode == AMEND && getType() == DIRECTORY) {
			size_t namePos = pathToTargetResource.rfind('/') + 1;
			pathToTargetResource += "/" + pathToTargetResource.substr(namePos)
					+ (contentType.empty() ? "" : ".") + contentType;
			statusCode = 201;
			writeToTargetResource(mode);
		}
		else if (statusCode < 400)
			writeToTargetResource(mode);

	} while (!finalRes);
}

void		RequestHandler::negotiateContent()
{
	std::vector<std::string> languages
					= rparser->GetAcceptLanguage();
	size_t 		pos;
	std::string	langExtension;
	
	for (size_t i = 0; i < languages.size(); ++i) {
		langExtension = "." + languages[i].substr(0,2);
		
		for (size_t j = 0; j < representations.size(); ++j) {
			pos = representations[j].rfind(langExtension.c_str());
			
			if (pos != std::string::npos && (pos + 3 == representations[j].length()
					|| representations[j][pos + 3] == '.')) {
				pathToTargetResource = representations[j];
				responseHeaders[CONTENT_LANGUAGE] = langExtension.substr(1, 2);
				return;
			}
		}
	}
	pathToTargetResource = representations[0];
}

void		RequestHandler::writeToTargetResource(bool APPEND)
{
	int				fd, valwrite;
	std::string		requestBody = rparser->GetBody();
	char 			bufPipe[1];

	read(pipeIPC[0], bufPipe, 1);
	fd = open(pathToTargetResource.c_str(), O_WRONLY | O_CREAT
		| (APPEND ? O_APPEND : O_TRUNC), 0666);
	
	valwrite = write(fd, requestBody.c_str(), requestBody.length());
	close(fd);
	write(pipeIPC[1], RUN, 1);

	if (fd == -1 || valwrite == -1) statusCode = 500;
	else generateLastModifiedHeader();
}

void		RequestHandler::readFromTargetResource()
{
	int		fd, valread;
	char	buf[RECVBUF];
	char 	bufPipe[1];

	read(pipeIPC[0], bufPipe, 1);
	fd = open(pathToTargetResource.c_str(), O_RDONLY);

	responseBody.clear();
	while ((valread = read(fd, buf, RECVBUF)) > 0)
		responseBody.append(buf, valread);

	close(fd);
	write(pipeIPC[1], RUN, 1);

	if (fd == -1 || valread == -1) statusCode = 500;
	else generateLastModifiedHeader();
}

std::string	RequestHandler::createTimestamp(time_t time)
{
	struct tm		*nowtm = localtime(&time); 
	char			date[DATESIZE];
	size_t			bytes;

	bytes = strftime(date, sizeof(date), "%a, %d %b %Y %T %Z" , nowtm);
	return std::string(date, bytes);
}

std::string RequestHandler::getExtension()
{
	size_t  extBegin, extEnd;
	std::string extension;

	extBegin = 	pathToTargetResource.find('.',
							location->GetRoot().length()) + 1;
	if (extBegin != std::string::npos) {
		extEnd = std::min(pathToTargetResource.find_first_of(".?", extBegin),
						  pathToTargetResource.length());
		extension = pathToTargetResource.substr(extBegin, extEnd - extBegin);
	}
	else if (*pathToTargetResource.rbegin() != '/')
		extension = "txt";
	return extension;
}

int 	RequestHandler::getType()
{
	struct stat statbuf;

	if (stat(pathToTargetResource.c_str(), &statbuf))
		return NON_EXISTENT;
	else if (S_ISDIR(statbuf.st_mode))
		return DIRECTORY;
	return REGULAR_FILE;
}

void	RequestHandler::retrieveErrorPage()
{
	struct stat statbuf;
	std::map<int, std::string> errorPages = location->GetDefaultErrorPages();
	std::map<int, std::string>::iterator errorPage;

	errorPage = errorPages.find(statusCode);
	if (errorPage != errorPages.end()) {
		pathToTargetResource = location->GetRoot() + errorPage->second;
		if (!stat(pathToTargetResource.c_str(), &statbuf))
			readFromTargetResource();
		else goto generateErrorPage;
	}
	else {
	generateErrorPage :
		responseBody = "<h1>Error " + ft_itoa(statusCode) + "</h1><hr>";
		responseBody += "<p>" + statusDescription[statusCode] + "</p>";
	}

	responseHeaders[CONTENT_TYPE] = "text/html";
}

void		RequestHandler::selectIndexFile()
{
	std::vector<std::string> indexFiles = location->GetIndex();
	std::string	dirPath(pathToTargetResource);
	
	if (indexFiles.empty()) return;
	
	for (size_t i = 0; i < indexFiles.size(); ++i) {
		pathToTargetResource = dirPath + "/" + indexFiles[i];
		findAllRepresentations();
		if (!representations.empty()) {
			statusCode = 303; return;
		}
	}
	pathToTargetResource.swap(dirPath);
}

void	RequestHandler::handleAutoIndex()
{
	DIR				*dir;
	struct dirent	*entry;
	struct stat     statbuf;
	std::string		parentDir, lastModified, root;

	if (!(dir = opendir(pathToTargetResource.c_str()))) {
		statusCode = 500; return;
	}

	root = "http://" + server->GetHostPort().host + ":"
				+ ft_itoa(server->GetHostPort().port);
	parentDir = targetResource.substr(0, targetResource.rfind('/'));

	responseBody = "<h1>Index of " + targetResource + "</h1><hr><pre><a href=\""
					+ root + parentDir + "\">../</a><br>";

	if (*targetResource.rbegin() != '/')
		targetResource.append(1, '/');

	while ((entry = readdir(dir))) {
		std::string entryName(entry->d_name);

		if (!entryName.compare(0, 1, "."))
			continue;

		if (entryName.length() > REFSIZE) {
			entryName.resize(REFSIZE);
			entryName.replace(REFSIZE - 3, 3, "...");
		}
		responseBody += "<a href=\"" + root + targetResource + entry->d_name
									+ "\">" + entryName + "</a>";
		responseBody += std::string(REFSIZE - entryName.length(), ' ');

		stat((pathToTargetResource + "/" + entry->d_name).c_str(), &statbuf);
		lastModified = createTimestamp(statbuf.st_mtime);
		lastModified.resize(40, ' ');
		responseBody += lastModified;

		responseBody += entry->d_type == DT_DIR ? "-" :
								ft_itoa(statbuf.st_size);
		responseBody += "<br>";
	}
	responseBody += "</pre><hr>";
	closedir(dir);

	responseHeaders[CONTENT_TYPE] = "text/html";
}

void		RequestHandler::runCGI()
{
	pathToTargetResource = location->GetPathCgi();
	if (pathToTargetResource[0] != '/')
		pathToTargetResource.insert(0,location->GetRoot().c_str());

	CGI CGIhandler(*server, *location, *client,
							*rparser, pathToTargetResource);
	std::string		CGIresponse;
	size_t			endOfHeaders;
	bool			CRLF(false);
	char 			buf[1];
	static int 		i(0);

	read(pipeIPC[0], buf, 1);
	usleep(100);
	if ((CGIresponse = CGIhandler.CGICall()).empty()) {
		write(pipeIPC[1], RUN, 1);
		if (i++ < 5) {
			runCGI(); return;
		}
		i = 0; statusCode = 500;
		return;
	}
	write(pipeIPC[1], RUN, 1);

	endOfHeaders = CGIresponse.find("\r\n\r\n");
	if (endOfHeaders != std::string::npos) {
		endOfHeaders += 4; CRLF = true;
	}
	else endOfHeaders = CGIresponse.find("\n\n") + 2;

	responseBody = CGIresponse.substr(endOfHeaders);
	responseHeadersCGI = CGIresponse.erase(endOfHeaders
						- (CRLF ? 2 : 1));

	checkHeadersCGI();
	if (!CRLF) formatLineBreaks();
}

void        RequestHandler::checkHeadersCGI()
{
	std::string		value;
	size_t			posStatus, valueLength;

	posStatus = responseHeadersCGI.find("Status: ");
	if (posStatus != std::string::npos) {
		valueLength = responseHeadersCGI.find(posStatus, '\n')
					  - posStatus - 8;
		value = responseHeadersCGI.substr(posStatus + 8, valueLength);
		statusCode = ft_stoi(value);
		responseHeadersCGI.erase(posStatus, valueLength + 8);
	}

	if (responseHeadersCGI.find("Content-Length: ")
				== std::string::npos && !responseBody.empty())
		generateContentLengthHeader();
}

void        RequestHandler::formatLineBreaks()
{
	std::string headersFormatted;
	headersFormatted.reserve(responseHeadersCGI.length());

	size_t lastPos(0), findPos;

	while((findPos = responseHeadersCGI.find('\n', lastPos))
											!= std::string::npos) {
		headersFormatted.append(responseHeadersCGI,
								lastPos, findPos - lastPos);
		headersFormatted += "\r\n";
		lastPos = findPos + 1;
	}
	headersFormatted += responseHeadersCGI.substr(lastPos);
	responseHeadersCGI.swap(headersFormatted);
}

void		RequestHandler::generateRawResponse()
{
	rawResponse = "HTTP/1.1 " + ft_itoa(statusCode) + " "
					+ statusDescription[statusCode] + "\r\n";

	std::map<eheader, std::string>::iterator header = responseHeaders.begin();
	for ( ; header != responseHeaders.end(); ++header)
		rawResponse += eheaderAsString(header->first) + ": "
						+ header->second + "\r\n";
	rawResponse += responseHeadersCGI;
	rawResponse += "\r\n";
	if (rparser->GetMethod().compare("HEAD"))
		rawResponse += responseBody;
}

void	RequestHandler::generateHeaders()
{
	generateDateHeader();
	responseHeaders[SERVER] = "webserv/1.0";

	if (!responseHeadersCGI.empty()) return;

	switch (statusCode) {
		case 201 :
		case 303 :
			generateLocationHeader();
			break;
		case 401 :
			responseHeaders[WWW_AUTHENTICATE] = "Basic realm=\"access to website\"";
			break;
		case 503 :
			responseHeaders[RETRY_AFTER] = ft_itoa(120);
	}

	generateContentLengthHeader();
	if (!responseBody.empty()) {
		generateContentTypeHeader();
	}
}

void		RequestHandler::generateDateHeader()
{
	struct timeval 	tv;
	gettimeofday(&tv, 0);

	responseHeaders[DATE] = createTimestamp(tv.tv_sec);
}

void		RequestHandler::generateLastModifiedHeader()
{
	struct stat statbuf;

	stat(pathToTargetResource.c_str(), &statbuf);
	responseHeaders[LAST_MODIFIED] = createTimestamp(statbuf.st_mtime);
}

void		RequestHandler::generateAllowHeader()
{
	std::vector<std::string> allowedMethods
									= location->GetRequestMethod();

	for (size_t i = 0; i < allowedMethods.size(); ++i) {
		responseHeaders[ALLOW] += allowedMethods[i];
		if (i != allowedMethods.size() - 1)
			responseHeaders[ALLOW] += ", ";
	}
}

void		RequestHandler::generateContentLengthHeader()
{
	responseHeaders[CONTENT_LENGTH] = ft_itoa(responseBody.length());
}

void		RequestHandler::generateContentTypeHeader()
{
	if (responseHeaders.find(CONTENT_TYPE)
						!= responseHeaders.end())
			return;

	std::map<std::string, std::string>::iterator contentType;

	contentType = contentTypeMIME.find(getExtension());
	if (contentType != contentTypeMIME.end())
		responseHeaders[CONTENT_TYPE] = contentType->second;
	else
		responseHeaders[CONTENT_TYPE] = "application/octet-stream";
}

void 		RequestHandler::generateLocationHeader()
{
	std::string	locationHeader;

	locationHeader = pathToTargetResource.substr(
							location->GetRoot().length());
	removeDoubles(locationHeader, '/');
	responseHeaders[LOCATION] = locationHeader;
}

void		RequestHandler::initMethodHandler()
{
	methodHandler["GET"] = &RequestHandler::GET;
	methodHandler["HEAD"] = &RequestHandler::HEAD;
	methodHandler["PUT"] = &RequestHandler::PUT;
	methodHandler["POST"] = &RequestHandler::POST;
	methodHandler["DELETE"] = &RequestHandler::DELETE;
	methodHandler["CONNECT"] = &RequestHandler::CONNECT;
	methodHandler["OPTIONS"] = &RequestHandler::OPTIONS;
	methodHandler["TRACE"] = &RequestHandler::TRACE;
}

void	RequestHandler::initStatusDict()
{
	statusDescription[100] = "Continue";
	statusDescription[101] = "Switching Protocols";
	statusDescription[200] = "OK";
	statusDescription[201] = "Created";
	statusDescription[202] = "Accepted";
	statusDescription[203] = "Non-Authoritative Information";
	statusDescription[204] = "No Content";
	statusDescription[205] = "Reset Content";
	statusDescription[206] = "Partial Content";
	statusDescription[300] = "Multiple Choices";
	statusDescription[301] = "Moved Permanently";
	statusDescription[302] = "Found";
	statusDescription[303] = "See Other";
	statusDescription[304] = "Not Modified";
	statusDescription[305] = "Use Proxy";
	statusDescription[307] = "Temporary Redirect";
	statusDescription[400] = "Bad Request";
	statusDescription[401] = "Unauthorized";
	statusDescription[402] = "Payment Required";
	statusDescription[403] = "Forbidden";
	statusDescription[404] = "Not Found";
	statusDescription[405] = "Method Not Allowed";
	statusDescription[406] = "Not Acceptable";
	statusDescription[407] = "Proxy Authentication Required";
	statusDescription[408] = "Request Timeout";
	statusDescription[409] = "Conflict";
	statusDescription[410] = "Gone";
	statusDescription[411] = "LengthRequired";
	statusDescription[412] = "Precondition Failed";
	statusDescription[413] = "Payload Too Large";
	statusDescription[414] = "URI Too Long";
	statusDescription[415] = "Unsupported Media Type";
	statusDescription[416] = "Range Not Satisfiable";
	statusDescription[417] = "Expectation Failed";
	statusDescription[426] = "Upgrade Required";
	statusDescription[500] = "Internal Server Error";
	statusDescription[501] = "Not Implemented";
	statusDescription[502] = "Bad Gateway";
	statusDescription[503] = "Service Unavailable";
	statusDescription[504] = "Gateway Timeout";
	statusDescription[505] = "HTTP Version Not Supported";
}

void		RequestHandler::initMIMETypeDict()
{
	// https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types

	contentTypeMIME["bin"] = "application/octet-stream";
	contentTypeMIME["bmp"] = "image/bmp";
	contentTypeMIME["css"] = "text/css";
	contentTypeMIME["csv"] = "text/csv";
	contentTypeMIME["doc"] = "application/msword";
	contentTypeMIME["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	contentTypeMIME["epub"] = "application/epub+zip";
	contentTypeMIME["gz"] = "application/gzip";
	contentTypeMIME["gif"] = "image/gif";
	contentTypeMIME["htm"] = "text/html";
	contentTypeMIME["html"] = "text/html";
	contentTypeMIME["ico"] = "image/vnd.microsoft.icon";
	contentTypeMIME["ics"] = "text/calendar";
	contentTypeMIME["jar"] = "application/java-archive";
	contentTypeMIME["jpeg"] = "image/jpeg";
	contentTypeMIME["jpg"] = "image/jpeg";
	contentTypeMIME["js"] = "text/javascript";
	contentTypeMIME["json"] = "application/json";
	contentTypeMIME["jsonld"] = "application/ld+json";
	contentTypeMIME["mp3"] = "audio/mpeg";
	contentTypeMIME["mp4"] = "video/mp4";
	contentTypeMIME["png"] = "image/png";
	contentTypeMIME["pdf"] = "application/pdf";
	contentTypeMIME["php"] = "application/x-httpd-php";
	contentTypeMIME["ppt"] = "application/vnd.ms-powerpoint";
	contentTypeMIME["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	contentTypeMIME["rar"] = "application/vnd.rar";
	contentTypeMIME["rtf"] = "application/rtf";
	contentTypeMIME["sh"] = "application/x-sh";
	contentTypeMIME["svg"] = "image/svg+xml";
	contentTypeMIME["tar"] = "application/x-tar";
	contentTypeMIME["txt"] = "text/plain";
	contentTypeMIME["xls"] = "application/vnd.ms-excel";
	contentTypeMIME["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	contentTypeMIME["xml"] = "application/xml";
	contentTypeMIME["zip"] = "application/zip";
	contentTypeMIME["7z"] = "application/x-7z-compressed";
}
