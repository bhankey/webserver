//
// Created by sergey on 17.03.2021.
//

#include <iostream>
#include "../includes/ServerConfig.hpp"
#include "../includes/Reader.hpp"
#include "../includes/StringUtils.hpp"

ServerConfig::ServerConfig():
    BaseConfig(),
    HostPort(),
    isHostPort(false),
    serverNames(),
    routes(),
    defaultRoute(),
    isAuth(false),
    auth() {
}
ServerConfig::ServerConfig(const ServerConfig &server):
    BaseConfig(server),
    HostPort(server.HostPort),
    isHostPort(server.isHostPort),
    serverNames(server.serverNames),
    routes(server.routes),
    defaultRoute(server.defaultRoute),
    isAuth(server.isAuth),
    auth(server.auth) {
}
ServerConfig &ServerConfig::operator=(const ServerConfig &serverConfig) {
  if (this != &serverConfig) {
    BaseConfig::operator=(serverConfig);
    HostPort = serverConfig.HostPort;
    serverNames = serverConfig.serverNames;
    defaultErrorPages = serverConfig.defaultErrorPages;
    root = serverConfig.root;
    index = serverConfig.index;
    clientMaxBodySize = serverConfig.clientMaxBodySize;
    routes = serverConfig.routes;
    defaultRoute = serverConfig.defaultRoute;
    isHostPort = serverConfig.isHostPort;
    auth = serverConfig.auth;
    isAuth = serverConfig.isAuth;
  }
  return *this;
}
ServerConfig::~ServerConfig() {

}
void ServerConfig::parseServerBlock(std::vector<std::string> &file, size_t &blockStartLine) {
  for (;blockStartLine < file.size(); ++blockStartLine) {
    if (file[blockStartLine].empty()) {
      continue;
    }
    std::vector<std::string> tokens(split(file[blockStartLine], " \t\v\f\r"));
    if (file[blockStartLine].find("}") != std::string::npos) {
      if (tokens.size() > 1)
        throw parsing::ConfigException("close bracket must be on empty line", blockStartLine);
      if (root.empty()) {
        root = "/";
      }
      fillLocationsWithDefaultValues();
      fillDefaultRoute();
      fillDefaultHostPort();
      return;
    }
    if (tokens[0] == "location") {
      if (tokens.size() != 3) {
        throw parsing::ConfigException("invalid location start syntax", blockStartLine);
      }
      if (tokens[2] == "{") {
        ServerRoutes route;
        route.parseLocationBlock(file, ++blockStartLine, tokens[1]);
        routes.push_back(route);
      }
    }
    else {
      parseParameter(tokens, file[blockStartLine], blockStartLine);
    }
  }
  throw parsing::ConfigException("no closing bracket for server block", blockStartLine);
}
void ServerConfig::parseParameter(std::vector<std::string> &tokens, std::string &line, int lineNum) {
  trimString(line);
  if (line[line.size() - 1] != ';') {
    throw parsing::ConfigException("';' not on the end of line:", lineNum);
  }
  line.erase(--line.end(), line.end());
  tokens = split(line, " \t\v\f\r");
  static const std::string parameters[8] = {"listen", "server_name", "error_page", "root", "index", "client_max_body_size", "autoindex", "auth"};
  static void (ServerConfig::*parsingParametersFunc[8])(std::vector<std::string> &, int) = {
      &ServerConfig::parseListen, &ServerConfig::parseServerName, &ServerConfig::parseErrorPage,
      &ServerConfig::parseRoot, &ServerConfig::parseIndex, &ServerConfig::parseClientMaxBodySize,
      &ServerConfig::parseAutoIndex, &ServerConfig::parseAuth
  };
  for (size_t i = 0; i < 8; ++i) {
    if (tokens[0] == parameters[i]) {
      (this->*parsingParametersFunc[i])(tokens, lineNum);
      return;
    }
  }
  throw parsing::ConfigException("invalid directive \"" + tokens[0] +"\"",lineNum);
}
void ServerConfig::parseServerName(std::vector<std::string> &tokens, int line) {
  if (tokens.size() < 2) {
    throw parsing::ConfigException("invalid number of argument in \"index\"", line);
  }
  serverNames.insert(serverNames.end(), ++tokens.begin(), tokens.end());
}
void ServerConfig::parseListen(std::vector<std::string> &tokens, int line) {
  if (isHostPort == true) {
	throw parsing::ConfigException("more than one listen directive in server", line);
  }
  if (tokens.size() != 2) {
	throw parsing::ConfigException("invalid number of argument in \"listen\"", line);
  }
  std::vector<std::string> ip_port = split(tokens[1], ":");
  if (ip_port.size() > 2) {
	throw parsing::ConfigException("invalid arguments in \"listen\" directive", line);
  }
  if (ip_port.size() == 1) {
	listen host_port = {ip_port[0], 80};
	HostPort = host_port;
	isHostPort = true;
	return;
  } else if (ip_port.size() == 2) {
	try {
	  listen host_port = {ip_port[0], static_cast<unsigned int>(ft_stoi(ip_port[1]))};
	  if (isNum(ip_port[1]) == false || host_port.port < 0 || host_port.port > 65535) {
		throw parsing::ConfigException("invalid arguments in \"listen\" directive", line);
	  }
	  HostPort = host_port;
	  isHostPort = true;
	}
	catch (...) {
	  throw parsing::ConfigException("invalid arguments in \"listen\" directive",
									 line);
	}
  } else {
	throw parsing::ConfigException("invalid arguments in \"listen\" directive",
								   line);
  }
}

// Getters
const ServerConfig::listen &ServerConfig::GetHostPort() const {
  return HostPort;
}
const std::vector<std::string> &ServerConfig::GetServerNames() const {
  return serverNames;
}
const ServerRoutes & ServerConfig::GetRoute(size_t idx) const {
  return routes.at(idx);
}
size_t ServerConfig::GetRoutesCount() const {
  return routes.size();
}

void ServerConfig::fillLocationsWithDefaultValues() {
  for (std::vector<ServerRoutes>::iterator it = routes.begin(); it != routes.end(); ++it) {
    if (it->GetDefaultErrorPages().empty()) {
      it->SetDefaultErrorPages(defaultErrorPages);
    }
    if (it->GetRoot().empty()) {
      it->SetRoot(root);
    }
    if (it->GetIndex().empty()) {
      it->SetIndex(index);
    }
    if (it->IsClientMaxBodySizeMentioned() == false && isClientMaxBodySizeMentioned == true) {
      it->SetClientMaxBodySize(clientMaxBodySize);
    }
    if (it->IsAutoIndexMentioned() == false && isAutoIndexMentioned == true) {
      it->SetAutoIndex(autoIndex);
    }
    if (it->GetRequestMethod().empty()) {
      it->SetDefaultRequestMethods();
    }
  }
}
void ServerConfig::fillDefaultRoute() {
  defaultRoute.SetDefaultErrorPages(defaultErrorPages);
  defaultRoute.SetRoot(root);
  defaultRoute.SetIndex(index);
  defaultRoute.SetClientMaxBodySize(clientMaxBodySize);
  defaultRoute.SetAutoIndex(autoIndex);
  defaultRoute.SetDefaultRequestMethods();
}
const ServerRoutes &ServerConfig::GetRoute(std::string path, const std::string &method) const {
  removeDoubles(path, '/');
  if (path.empty() || routes.empty()) {
    return defaultRoute;
  }
  path = path.substr(0,path.rfind('?'));
  size_t maxMatchSupported = 0;
  size_t maxMatchUnsupported = 0;
  std::vector<ServerRoutes>::const_iterator maxMatchItSupported = routes.end();
  std::vector<ServerRoutes>::const_iterator maxMatchItUnsupported = routes.end();
  bool findAsteriskMatchSupported = false;
  bool findAsteriskMatchUnsupported = false;
  std::vector<ServerRoutes>::const_iterator asteriskMatchSupported;
  std::vector<ServerRoutes>::const_iterator asteriskMatchUnsupported;
  std::vector<std::string> pathByTokens = split(path, "/");
  for (std::vector<ServerRoutes>::const_iterator it = routes.begin(); it != routes.end(); ++it) {
    const std::string &locationPath = it->GetPath();
    if (locationPath[0] != '*') {
      size_t tokenTrying = pathByTokens.size();
      while (tokenTrying > 0) {
        std::string toFind;
        for (size_t i = 0; i < tokenTrying; ++i) {
          toFind += pathByTokens[i];
          if (toFind == "" || i != tokenTrying - 1)
            toFind += '/';
        }
        if (toFind == locationPath) {
          if ((maxMatchSupported < tokenTrying) && isMethodSupport(method, *it)) {
            maxMatchSupported = tokenTrying;
            maxMatchItSupported = it;
          } else if ((maxMatchUnsupported < tokenTrying) && !isMethodSupport(method, *it)) {
            maxMatchUnsupported = tokenTrying;
            maxMatchItUnsupported = it;
          }
        }
        --tokenTrying;
      }
    }
    else if (findAsteriskMatchSupported == false){
      std::string ending = locationPath.substr(1, std::string::npos);
      if (path.size() > ending.size() && !path.compare(path.size() - ending.size(), ending.size(), ending)) {
        if (isMethodSupport(method, *it)) {
          asteriskMatchSupported = it;
          findAsteriskMatchSupported = true;
        }
        else {
          asteriskMatchUnsupported = it;
          findAsteriskMatchUnsupported = true;
        }
      }
    }
  }
  if (maxMatchSupported == pathByTokens.size()) {
    return *maxMatchItSupported;
  }
  else if (findAsteriskMatchSupported == true) {
    return *asteriskMatchSupported;
  }
  else if (maxMatchSupported != 0 && maxMatchItSupported != routes.end()) {
    return *maxMatchItSupported;
  }
  else if (maxMatchUnsupported == pathByTokens.size()) {
    return *maxMatchItUnsupported;
  }
  else if (findAsteriskMatchUnsupported == true) {
    return *asteriskMatchUnsupported;
  }
  else if (maxMatchUnsupported != 0 && maxMatchItUnsupported != routes.end()) {
    return *maxMatchItUnsupported;
  }
  else {
    return defaultRoute;
  }
}

void ServerConfig::fillDefaultHostPort()
{
	if (isHostPort == false) {
		listen host_port = {"127.0.0.1", 80};
		HostPort = host_port;
		isHostPort = true;
	}
}
bool ServerConfig::IsAuth() const {
  return isAuth;
}
std::map<std::string, std::string> ServerConfig::GetAuth() const {
  return auth;
}
void ServerConfig::parseAuth(std::vector<std::string> &tokens, int line) {
  if (isAuth == true) {
	throw parsing::ConfigException("more than one auth directive in server", line);
  }
  try {
	Reader reader(tokens[1]);
	std::vector<std::string> file = reader.parseFile();
	for (std::vector<std::string>::iterator it = file.begin(); it != file.end(); ++it) {
	  if (it->find(':') != std::string::npos) {
		auth[it->substr(0, it->find(':'))] = it->substr((it->find(':') + 1), std::string::npos);
	  }
	}
	isAuth = true;
  }
  catch (std::exception &exception) {
	throw parsing::ConfigException("Can't open authentication file file", line);
  }
}
bool ServerConfig::isMethodSupport(const std::string &method, const ServerRoutes &route) const {
  for (std::vector<std::string>::const_iterator it = route.GetRequestMethod().begin(); it != route.GetRequestMethod().end(); ++it) {
    if (*it == method) {
      return true;
    }
  }
  return false;
}