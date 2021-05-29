//
// Created by sergey on 17.03.2021.
//

#include "../includes/Config.hpp"
#include "../includes/Reader.hpp"
#include "../includes/StringUtils.hpp"

Config::Config(const Config &config) :
  servers(config.servers),
  workerProcesses(config.workerProcesses),
  workerConnections(config.workerConnections) {

}
Config::Config():
  servers(),
  workerProcesses(0),
  workerConnections(0) {
}
Config &Config::operator=(const Config &config) {
  if (this != &config) {
    servers = config.servers;
    workerProcesses = config.workerProcesses;
    workerConnections = config.workerConnections;
  }
  return *this;
}
Config::~Config() {

}
void Config::parseConfig(const std::string &pathConfig) {
  Reader reader(pathConfig);
  std::vector<std::string> file;

  try {
	file = reader.parseFile();
  }
  catch (std::exception &exception) {
	throw (std::runtime_error("Config file can't be open"));
  }
  for (size_t i = 0; i < file.size(); i++) {
    if (file[i].empty()) {
      continue;
    }
    std::vector<std::string> tokens(split(file[i], " \t\v\f\r"));
    if (tokens[0] == "server") {
      if (tokens.size() != 2 || tokens[1] != "{") {
        throw parsing::ConfigException(
            "new block don't open with bracket or open bracket don't follow by new line:",
            i);
      }
      servers.push_back(ServerConfig());
      servers[servers.size() - 1].parseServerBlock(file, ++i);
    } else {
      parseParameters(tokens, file[i], i);
    }
  }
  checkParameters();
  if (servers.empty()) {
    throw parsing::ConfigException("no servers blocks in config file", 0);
  }
}
const ServerConfig &Config::GetServer(size_t idx) const {
  return servers.at(idx);
}
size_t Config::GetServerCount() const {
  return servers.size();
}
const std::vector<ServerConfig> &Config::GetServers() {
  return servers;
}
unsigned int Config::GetWorkerProcesses() const {
  return workerProcesses;
}
unsigned int Config::GetWorkerConnections() const {
  return workerConnections;
}
void Config::parseWorkerProcesses(std::vector<std::string> &tokens, int lineNumber) {
  if (workerProcesses != 0) {
    throw parsing::ConfigException("more than one worker_process directive in config", lineNumber);
  }
  if (tokens.size() != 2 || !isNum(tokens[1])) {
    throw parsing::ConfigException("invalid arguments in \"worker_processes\" directive", lineNumber);
  }
  try {
    workerProcesses = ft_stoi(tokens[1]);
  }
  catch (...) {
    throw parsing::ConfigException("invalid arguments in \"worker_processes\" directive", lineNumber);
  }
  if (workerProcesses <= 0) {
    throw parsing::ConfigException("invalid arguments in \"worker_processes\" directive", lineNumber);
  }

}
void Config::parseWorkerConnections(std::vector<std::string> &tokens, int lineNumber) {
  if (workerConnections != 0) {
    throw parsing::ConfigException("more than one worker_process directive in config", lineNumber);
  }
  if (tokens.size() != 2 || !isNum(tokens[1])) {
    throw parsing::ConfigException("invalid argument in \"worker_connections\" directive", lineNumber);
  }
  try {
    workerConnections = ft_stoi(tokens[1]);
  }
  catch (...) {
    throw parsing::ConfigException("invalid argument in \"worker_connections\" directive", lineNumber);
  }
  if (workerConnections <= 0 || workerConnections > FD_SETSIZE) {
    throw parsing::ConfigException("invalid argument in \"worker_connections\" directive", lineNumber);
  }
}
void Config::checkParameters() {
  if (workerProcesses == 0) {
    workerProcesses = 1;
  }
  if (workerConnections == 0) {
    workerConnections = 512;
  }
}
void Config::parseParameters(std::vector<std::string> &tokens, std::string &line, int lineNum) {

  trimString(line);
  if (line[line.size() - 1] != ';') {
    throw parsing::ConfigException("';' not on the end of line:", lineNum);
  }
  line.erase(--line.end(), line.end());
  tokens = split(line, " \t\v\f\r");
  static const size_t parametersCount = 2;
  static const std::string parameters[parametersCount] = {
      "worker_processes", "worker_connections"
  };
  static void (Config::*parsingParametersFunc[parametersCount])(std::vector<std::string> &, int) = {
      &Config::parseWorkerProcesses, &Config::parseWorkerConnections
  };
  for (size_t j = 0; j < parametersCount; ++j) {
    if (parameters[j] == tokens[0]) {
      (this->*parsingParametersFunc[j])(tokens, lineNum);
      return;
    }
  }
  throw parsing::ConfigException("invalid directive \"" + tokens[0] +"\"",lineNum);
}
