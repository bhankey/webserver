//
// Created by sergey on 17.03.2021.
//

#ifndef WEBSERV_CONFIG_CONFIG_HPP_
#define WEBSERV_CONFIG_CONFIG_HPP_

#include "ServerConfig.hpp"
#include "ParsingException.hpp"
#include <map>
#include <iostream>
#include <unistd.h>
#include <sys/select.h>

class Config {
 private:
  std::vector<ServerConfig> servers;
  unsigned int              workerProcesses;
  unsigned int              workerConnections;

  void parseWorkerProcesses(std::vector<std::string> &tokens, int lineNumber);
  void parseWorkerConnections(std::vector<std::string> &tokens, int lineNumber);
  void parseParameters(std::vector<std::string> &tokens, std::string &line, int lineNum);
  void checkParameters();
 public:
  Config(const Config &config);
  Config();
  Config &operator=(const Config &config);
  ~Config();

  void parseConfig(const std::string &pathConfig);

  const std::vector<ServerConfig> &GetServers();
  const ServerConfig &GetServer(size_t idx) const;
  unsigned int GetWorkerProcesses() const;
  unsigned int GetWorkerConnections() const;
  size_t GetServerCount() const;

};

#endif //WEBSERV_CONFIG_CONFIG_HPP_
