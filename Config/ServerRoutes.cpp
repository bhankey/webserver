//
// Created by sergey on 17.03.2021.
//
#include <iostream>
#include "../includes/StringUtils.hpp"
#include "../includes/ParsingException.hpp"
#include "../includes/ServerRoutes.hpp"

ServerRoutes::ServerRoutes():
  BaseConfig(),
  path(),
  requestMethod(),
  isUpload(false),
  pathForUpload("/"),
  pathCGI(),
  extensionCGI() {
}
ServerRoutes::ServerRoutes(const ServerRoutes &location):
    BaseConfig(location),
    path(location.path),
    requestMethod(location.requestMethod),
    isUpload(location.isUpload),
    pathForUpload(location.pathForUpload),
    pathCGI(location.pathCGI),
    extensionCGI(location.extensionCGI) {

}
ServerRoutes &ServerRoutes::operator=(const ServerRoutes &location) {
  if (this != &location) {
    BaseConfig::operator=(location);
    path = location.path;
    requestMethod = location.requestMethod;
    root = location.root;
    index = location.index;
    clientMaxBodySize = location.clientMaxBodySize;
    autoIndex = location.autoIndex;
    isUpload = location.isUpload;
    pathForUpload = location.pathForUpload;
    pathCGI = location.pathCGI;
    extensionCGI = location.extensionCGI;
  }
  return *this;
}
ServerRoutes::~ServerRoutes() {

}

void ServerRoutes::parseLocationBlock(std::vector<std::string> &file, size_t &blockStartLine, const std::string& _path) {
  path = _path;
  removeDoubles(path, '/');
  for (;blockStartLine < file.size(); ++blockStartLine) {
    if (file[blockStartLine].empty()) {
      continue;
    }
    std::vector<std::string> tokens(split(file[blockStartLine], " \t\v\f\r"));
    if (file[blockStartLine].find("}") != std::string::npos) {
      if (tokens.size() > 1)
        throw parsing::ConfigException("close bracket must be on empty line", blockStartLine);
      return;
    }
    parseParameter(tokens, file[blockStartLine], blockStartLine);
  }
  throw parsing::ConfigException("no ';' at end of parameter", blockStartLine);
}

// Parse all parameters
void ServerRoutes::parseParameter(std::vector<std::string> &tokens, std::string &line, int lineNum) {
  trimString(line);
  if (line[line.size() - 1] != ';') {
    throw parsing::ConfigException("';' not on the end of line:", lineNum);
  }
  line.erase(--line.end(), line.end());
  tokens = split(line, " \t\v\f\r");
  static const std::string parameters[9] = {
      "root", "index", "client_max_body_size", "request_method", "autoindex", "is_upload",
      "path_for_upload", "path_CGI", "extension_CGI"};
  static void (ServerRoutes::*parsingParametersFunc[9])(std::vector<std::string> &, int) = {
      &ServerRoutes::parseRoot, &ServerRoutes::parseIndex, &ServerRoutes::parseClientMaxBodySize,
      &ServerRoutes::parseRequestMethod, &ServerRoutes::parseAutoIndex, &ServerRoutes::parseUploadStatus,
      &ServerRoutes::parsePathForUpload, &ServerRoutes::parsePathCGI, &ServerRoutes::parseExtensionCGI
  };
  for (size_t i = 0; i < 9; ++i) {
    if (tokens[0] == parameters[i]) {
      (this->*parsingParametersFunc[i])(tokens, lineNum);
      return;
    }
  }
  throw parsing::ConfigException("invalid directive \"" + tokens[0] +"\"",lineNum);
}
void ServerRoutes::parseRequestMethod(std::vector<std::string> &tokens, int line) {
  if (tokens.size() < 2) {
    throw parsing::ConfigException("invalid number of argument in \"request_methods\" directive", line);
  }
  static const std::string httpMethods[8] = {"OPTIONS", "GET", "HEAD", "POST",
                                             "PUT", "DELETE", "TRACE", "CONNECT"};
  for (size_t i = 1; i < tokens.size(); ++i) {
    bool isMethodsExist = false;
    for (int j = 0; j < 8; j++) {
      if (tokens[i] == httpMethods[j]) {
        isMethodsExist = true;
        break;
      }
    }
    if (!isMethodsExist) {
      throw parsing::ConfigException("invalid arguments in \"request_methods\" directive", line);
    }
    bool isMethodsInit = false;
    for (size_t j = 0; j < requestMethod.size(); j++) {
      if (tokens[i] == requestMethod[j]) {
        isMethodsInit = true;
        break;
      }
    }
    if (!isMethodsInit) {
      requestMethod.push_back(tokens[i]);
    }
  }
}
void ServerRoutes::parseUploadStatus(std::vector<std::string> &tokens, int line) {
  if (tokens.size() != 2) {
    throw parsing::ConfigException("invalid number if argument in \"upload_status\"", line);
  }
  if (tokens[1] == "on") {
    isUpload = true;
  }
  else if (tokens[1] == "off") {
    isUpload = false;
  }
  else {
    throw parsing::ConfigException("invalid number of argument in \"upload_status\" directive", line);
  }
}
void ServerRoutes::parsePathForUpload(std::vector<std::string> &tokens, int line) {
  if (tokens.size() != 2) {
    throw parsing::ConfigException("invalid number if argument in \"upload_path\"", line);
  }
  pathForUpload = tokens[1];
}
void ServerRoutes::parsePathCGI(std::vector<std::string> &tokens, int line) {
  if (tokens.size() != 2) {
    throw parsing::ConfigException("invalid number if argument in \"upload_path\"", line);
  }
  pathCGI = tokens[1];
}
void ServerRoutes::parseExtensionCGI(std::vector<std::string> &tokens, int line) {
  if (tokens.size() != 2) {
    throw parsing::ConfigException("invalid number if argument in \"upload_path\"", line);
  }
  extensionCGI = tokens[1];
}

// Getters
const std::string &ServerRoutes::GetPath() const {
  return path;
}
const std::vector<std::string> &ServerRoutes::GetRequestMethod() const {
  return requestMethod;
}
bool ServerRoutes::IsUpload() const {
  return isUpload;
}
const std::string &ServerRoutes::GetPathForUpload() const {
  return pathForUpload;
}
const std::string &ServerRoutes::GetPathCgi() const {
  return pathCGI;
}
const std::string &ServerRoutes::GetExtensionCgi() const {
  return extensionCGI;
}
void ServerRoutes::SetDefaultRequestMethods() {
  if (requestMethod.empty()) {
    static const std::string httpMethods[8] = {"OPTIONS", "GET", "HEAD", "POST",
                                               "PUT", "DELETE", "TRACE", "CONNECT"};
    for (size_t i = 0; i < 8; ++i) {
      requestMethod.push_back(httpMethods[i]);
    }
  }
}
