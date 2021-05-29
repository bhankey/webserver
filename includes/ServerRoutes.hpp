//
// Created by sergey on 17.03.2021.
//

#ifndef WEBSERV_CONFIG_SERVERROUTES_HPP_
#define WEBSERV_CONFIG_SERVERROUTES_HPP_

#include <string>
#include <vector>
#include "BaseConfig.hpp"

class ServerRoutes: public BaseConfig {
 private:
  std::string path; //locations ONLY LOCATION
  std::vector<std::string> requestMethod; // if no request methods was mentioned, vector will be empty  ONLY LOCATION
  bool isUpload; // default: false ONLY LOCATION
  std::string pathForUpload; // default: "/" ONLY LOCATION
  std::string pathCGI; // default: empty ONLY LOCATION
  std::string extensionCGI; // default: empty ONLY LOCATION

  void parseParameter(std::vector<std::string> &tokens, std::string &line, int lineNum);
  void parseRequestMethod(std::vector<std::string> &tokens, int line);
  void parseUploadStatus(std::vector<std::string> &tokens, int line);
  void parsePathForUpload(std::vector<std::string> &tokens, int line);
  void parsePathCGI(std::vector<std::string> &tokens, int line);
  void parseExtensionCGI(std::vector<std::string> &tokens, int line);
 public:
  ServerRoutes();
  ServerRoutes(const ServerRoutes &location);
  ServerRoutes &operator=(const ServerRoutes &location);
  ~ServerRoutes();
  void parseLocationBlock(std::vector<std::string> &file, size_t &blockStartLine, const std::string& _path);
  const std::string &GetPath() const;
  const std::vector<std::string> &GetRequestMethod() const;
  bool IsUpload() const;
  const std::string &GetPathForUpload() const;
  const std::string &GetPathCgi() const;
  const std::string &GetExtensionCgi() const;
  void SetDefaultRequestMethods();
};

#endif //WEBSERV_CONFIG_SERVERROUTES_HPP_
