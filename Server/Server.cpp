
// by aronin

#include "../includes/Server.hpp"

bool 	sigpipe(false);

Server::Server(const ServerConfig &config): serverConfig(config)
{
	rhandler = new RequestHandler;

	const ServerConfig::listen listen = serverConfig.GetHostPort();
	configureSocket(static_cast<int>(listen.port), listen.host);
}

Server::~Server()
{
	closeSocket();
	delete rhandler;
}

void	Server::registerSocket(RWFDset *FDset)
{
	FD_SET(fd, &FDset->read_active);
	FDset->MAXFD = std::max(fd, FDset->MAXFD);
}

void	Server::configureSocket(int port, std::string const & host)
{
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(host.c_str());
	addr.sin_port = htons(port);

	int enable = 1;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1
		|| setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1
		|| bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1
		|| listen(fd, BACKLOG) == -1
		|| fcntl(fd, F_SETFL, O_NONBLOCK) == -1)

		throw std::runtime_error(strerror(errno));
}

void	Server::handleConnections(RWFDset *FDset)
{
	if (FD_ISSET(fd, &FDset->read_ready))
		addNewConnection(FDset);

	std::list<Connection *>::iterator connect
			= connections.begin();
	while (connect != connections.end()) {

		Connection *c = *connect;
		int fd = c->getClientFD();

		if (FD_ISSET(fd, &FDset->read_ready)) {
			receiveRequest(c);
			if (c->transmissionEnded())
				FD_SET(fd, &FDset->write_active);
		}
		if (c->isTimedOut() || FD_ISSET(fd, &FDset->write_ready)) {
			rhandler->process(&serverConfig, c);
			sendResponse(fd, rhandler->getRawResponse());

			if (c->getStatusCode() < 400) c->reset();
			else c->terminate();

			FD_CLR(fd, &FDset->write_active);
		}
		if (c->isTerminated()) {
			FD_CLR(fd, &FDset->read_active);
			FD_CLR(fd, &FDset->write_active);
			delete *connect;
			connections.erase(connect++);
		}
		else ++connect;
	}
}

void	Server::addNewConnection(RWFDset *FDset)
{
	int					newClientFD;
	struct sockaddr_in	clientAddr;
	socklen_t			size = sizeof(clientAddr);

	if ((newClientFD = accept(fd, (struct sockaddr *) &clientAddr, &size)) == -1)
		return;

	FD_SET(newClientFD, &FDset->read_active);
	FDset->MAXFD = std::max(newClientFD, FDset->MAXFD);

	connections.push_back(new Connection(newClientFD, clientAddr));

	if (connections.size() > BACKLOG - 10)
		connections.back()->updateStatusCode(503);

	usleep(1000);
}

void	Server::receiveRequest(Connection *client)
{
	char		buffer[RECVBUF];
	ssize_t		valrecv;
	std::string bufTotal;
	int			clientFD = client->getClientFD();

	while ((valrecv = recv(clientFD, buffer,
						RECVBUF, MSG_DONTWAIT)) > 0) {
		if (valrecv > 0) bufTotal.append(buffer, valrecv);
	}
	client->appendToRequest(bufTotal);
	if (!valrecv) client->terminate();
}

void	Server::sendResponse(int clientFD, std::string const & response)
{
	const char *  response_msg = response.c_str();
	ssize_t bytesToSend = response.length();
	ssize_t	valsend, bytesSent(0);

	while (bytesToSend > 0 && !sigpipe) {
		if ((valsend = send(clientFD, response_msg + bytesSent,
							bytesToSend, MSG_DONTWAIT)) == -1)
			continue;
		bytesSent += valsend;
		bytesToSend -= valsend;
	}
	sigpipe = false;
}

void	Server::closeSocket()
{
	std::list<Connection *>::iterator c = connections.begin();
	while (c != connections.end()) {
		(*c)->terminate();
		delete *c;
		connections.erase(c++);
	}
	close(fd);
	fd = -1;
}

void 	Server::handleSIGPIPE(int)
{
	sigpipe = true;
}
