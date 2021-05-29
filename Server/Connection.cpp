
// by aronin

#include "../includes/Connection.hpp"

Connection::Connection() {};

Connection::Connection(int fd, struct sockaddr_in addr) :
		fd(fd), addr(addr)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error(strerror(errno));

	reset();
}

Connection::~Connection() { terminate(); }

bool	Connection::transmissionEnded() const
{
	return state == BODY_RECEIVED || statusCode != 200;
};

bool	Connection::isTimedOut()
{
	struct timeval	tv;
	gettimeofday(&tv, 0);

	if (!requestTime || (tv.tv_sec - requestTime < TIMEOUT))
		return false;

	statusCode = 408;
	state = TERMINATED;
	return true;
}

bool	Connection::isTerminated()
{
	return state == TERMINATED;
}

int					    Connection::getClientFD() const { return fd; };
std::string const &     Connection::getRequest() const { return request; };
int					    Connection::getStatusCode() const { return statusCode; }
struct in_addr const &  Connection::getClientIP() const { return addr.sin_addr; };

void			Connection::updateStatusCode(int statusCode)
{
	this->statusCode = statusCode;
}

void			Connection::setRequestTime()
{
	struct timeval	tv;
	gettimeofday(&tv, 0);
	requestTime = tv.tv_sec;
}

void	Connection::setEOBIdentifier()
{
	size_t pos(0);
	
	if (request.rfind("Transfer-Encoding: ", endOfHeaders)
							!= std::string::npos
		&& (pos = request.substr(pos, endOfHeaders - pos).find("chunked"))
				 	!= std::string::npos) {
		if (!request.compare(pos + 8, 1, "\n"))
			EOBIdentifier = CHUNKED_ENCODING;
		else
			statusCode = 400;
	}

	else if ((pos = request.rfind("Content-Length: ", endOfHeaders))
				!= std::string::npos) {
		try {
			bodyBytesExpected = ft_stoi(std::string(&request[pos +
								strlen("Content-Length: ")]));
			if (bodyBytesExpected < 0) throw std::invalid_argument("negative");
			EOBIdentifier = CONTENT_LENGTH;
		}
		catch (std::out_of_range) {
			statusCode = 413;
		}
		catch (std::invalid_argument) {
			statusCode = 400;
		}
	}
}

void	Connection::checkBodyStatus()
{
	int	bodyBytesReceived = request.length() - endOfHeaders;

	switch (EOBIdentifier) {

		case CHUNKED_ENCODING :
			size_t	pos;

			if ((pos = request.find("0\r\n\r\n", endOfHeaders))
								!= std::string::npos)

				(pos + 5 == request.length()) ?  state = BODY_RECEIVED :
													statusCode = 400;
			break;

		case CONTENT_LENGTH :
			if (bodyBytesExpected == bodyBytesReceived)
				state = BODY_RECEIVED;
			else if (bodyBytesExpected < bodyBytesReceived)
				statusCode = 400;
			break;

		case CONNECTION_END :
			if (!bodyBytesReceived) state = BODY_RECEIVED;
			else statusCode = 411;
			break;
	}
}

void	Connection::appendToRequest(std::string const &buffer)
{
	if (request.empty()) setRequestTime();

	request.append(buffer);

	if (state == INITIATED && (endOfHeaders
			= request.find("\r\n\r\n")) != std::string::npos) {
		state = HEADERS_RECEIVED;
		endOfHeaders += 4;
		setEOBIdentifier();
	}

	if (state == HEADERS_RECEIVED && !isTerminated()) checkBodyStatus();
}

void	Connection::reset()
{
	request.clear();
	statusCode = 200;
	endOfHeaders = requestTime = bodyBytesExpected = 0;
	EOBIdentifier = CONNECTION_END;
	state = INITIATED;
}

void	Connection::terminate()
{ 
	state = TERMINATED;
	close(fd);
	fd = -1;
}