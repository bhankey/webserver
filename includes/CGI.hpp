//
// Created by sergey on 22.03.2021.
//

#ifndef WEBSERV_CGI_CGI_HPP_
#define WEBSERV_CGI_CGI_HPP_

#include "Envp.hpp"
#include "Config.hpp"
#include "Connection.hpp"
#include "RequestHandler.hpp"
#include "CGIException.hpp"
#include <sys/wait.h>

class CGI {
 private:
  const ServerConfig    &server;
  const ServerRoutes	&location;
  const Connection	    &client;
  const RequestParser   &requestParser;
  Envp                  envp;
  std::string           pathToExecutable;
  std::string           ft_inet_ntop(unsigned int addr);
  void setEnvp();
  CGI();
 public:
  CGI(const ServerConfig &_server, const ServerRoutes &_location, const Connection &_client, const RequestParser &_request, std::string _pathToExecutable);
  CGI(const CGI &cgi);
  CGI &operator=(const CGI &cgi);
  ~CGI();
  std::string CGICall();
};

#endif //WEBSERV_CGI_CGI_HPP_
