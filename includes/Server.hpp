
// by aronin

#ifndef WEBSERV_SERVER_SERVER_HPP_
#define WEBSERV_SERVER_SERVER_HPP_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <list>
#include <algorithm>
#include <exception>

#include "ServerConfig.hpp"
#include "Connection.hpp"
#include "RequestHandler.hpp"
#include "Cluster.hpp"

#define BACKLOG 100000
#define RECVBUF 100000

class Server {

friend class Cluster;

private:
    const ServerConfig  &   serverConfig;

    int						fd;
    struct sockaddr_in 		addr;
    std::list<Connection *>	connections;

    RequestHandler  *       rhandler;

    void			configureSocket(int port, std::string const & host);
    void			addNewConnection(RWFDset *FDset);
    void			receiveRequest(Connection *client);
    void			sendResponse(int clientFD, std::string const & response);
    void			closeSocket();
    static void 	handleSIGPIPE(int);

public:
    Server(const ServerConfig &_config);
    ~Server();

    void    	registerSocket(RWFDset *FDset);
    void		handleConnections(RWFDset *FDset);
};

#endif //WEBSERV_SERVER_SERVER_HPP_
