//
// Created by sergey on 17.03.2021.
//

#ifndef WEBSERV_CONFIG_SERVERCONFIG_HPP_
#define WEBSERV_CONFIG_SERVERCONFIG_HPP_

#include <string>
#include <list>
#include <map>
#include "ServerRoutes.hpp"
#include "ParsingException.hpp"
#include "BaseConfig.hpp"

class ServerConfig: public BaseConfig {
 public:
  struct listen {
    std::string host;
    unsigned int port;
  };
 private:
  listen       							HostPort; // ip and port to listen (default: {0.0.0.0, 80))
  bool 									isHostPort;
  std::vector<std::string>  			serverNames; // default : empty;
  std::vector<ServerRoutes> 			routes; // default: empty
  ServerRoutes              			defaultRoute;
  bool 									isAuth; // default: false
  std::map<std::string, std::string>	auth;
 private:
  void parseParameter(std::vector<std::string> &tokens, std::string &line, int lineNum);
  void parseListen(std::vector<std::string> &tokens, int line);
  void parseServerName(std::vector<std::string> &tokens, int line);
  void parseAuth(std::vector<std::string> &tokens, int line);
  void fillLocationsWithDefaultValues();
  void fillDefaultRoute();
  void fillDefaultHostPort();
  bool isMethodSupport(const std::string &method,const ServerRoutes &route) const;
 public:
  ServerConfig();
  ServerConfig(const ServerConfig &server);
  ServerConfig &operator=(const ServerConfig &serverConfig);
  ~ServerConfig();
  void parseServerBlock(std::vector<std::string> &file, size_t &blockStartLine);
  const listen &GetHostPort() const;
  const std::vector<std::string> &GetServerNames() const;
  const ServerRoutes &GetRoute(size_t idx) const;
  const ServerRoutes &GetRoute(std::string path, const std::string &method) const;
  size_t GetRoutesCount() const;
  bool  IsAuth() const;
  std::map<std::string, std::string> GetAuth() const;
};

#endif //WEBSERV_CONFIG_SERVERCONFIG_HPP_
