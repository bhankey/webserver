//
// Created by sergey on 31.03.2021.
//

#ifndef WEBSERV_CONFIG_BASECONFIG_HPP_
#define WEBSERV_CONFIG_BASECONFIG_HPP_

#include <map>
#include <string>
#include <vector>
#include "ParsingException.hpp"

class BaseConfig {
 protected:
  std::map<int, std::string> defaultErrorPages; // default: empty error code - path to page
  std::string root; // default - "/"
  std::vector<std::string> index;
  long long unsigned int clientMaxBodySize;
  bool isClientMaxBodySizeMentioned; // if autoindex wasn't in  config - false
  bool autoIndex; // default: false
  bool isAutoIndexMentioned; // if autoindex wasn't in  config - false

  // Functions to parse
  virtual void parseParameter(std::vector<std::string> &tokens, std::string &line, int lineNum) = 0;
  void parseErrorPage(std::vector<std::string> &tokens, int line);
  void parseRoot(std::vector<std::string> &tokens, int line);
  void parseIndex(std::vector<std::string> &tokens, int line);
  void parseClientMaxBodySize(std::vector<std::string> &tokens, int line);
  void parseAutoIndex(std::vector<std::string> &tokens, int line);
 public:
  BaseConfig();
  BaseConfig(const BaseConfig &baseConfig);
  BaseConfig &operator=(const BaseConfig &baseConfig);
  virtual ~BaseConfig();

  // Setters
  void SetDefaultErrorPages(const std::map<int, std::string> &default_error_pages);
  void SetRoot(const std::string &root);
  void SetIndex(const std::vector<std::string> &index);
  void SetClientMaxBodySize(unsigned long long int client_max_body_size);
  void SetAutoIndex(bool auto_index);

  // Getters
  const std::map<int, std::string> &GetDefaultErrorPages() const;
  const std::string &GetRoot() const;
  const std::vector<std::string> &GetIndex() const;
  unsigned int GetClientMaxBodySize() const;
  bool IsAutoIndex() const;
  bool IsClientMaxBodySizeMentioned();
  bool IsAutoIndexMentioned();
};

#endif //WEBSERV_CONFIG_BASECONFIG_HPP_
