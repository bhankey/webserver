//
// Created by sergey on 31.03.2021.
//
#include "../includes/BaseConfig.hpp"

BaseConfig::BaseConfig():
  defaultErrorPages(),
  root(),
  index(),
  clientMaxBodySize(1024 * 1024),
  isClientMaxBodySizeMentioned(false),
  autoIndex(false),
  isAutoIndexMentioned(false) {
}
BaseConfig::BaseConfig(const BaseConfig &baseConfig):
  defaultErrorPages(baseConfig.defaultErrorPages),
  root(baseConfig.root),
  index(baseConfig.index),
  clientMaxBodySize(baseConfig.clientMaxBodySize),
  isClientMaxBodySizeMentioned(baseConfig.isClientMaxBodySizeMentioned),
  autoIndex(baseConfig.autoIndex),
  isAutoIndexMentioned(baseConfig.isAutoIndexMentioned) {

}
BaseConfig &BaseConfig::operator=(const BaseConfig &baseConfig) {
  if (this != &baseConfig) {
    defaultErrorPages = baseConfig.defaultErrorPages;
    root = baseConfig.root;
    index = baseConfig.index;
    clientMaxBodySize = baseConfig.clientMaxBodySize;
    isClientMaxBodySizeMentioned = baseConfig.clientMaxBodySize;
    autoIndex = baseConfig.autoIndex;
    isAutoIndexMentioned = baseConfig.isAutoIndexMentioned;
  }
  return *this;
}
BaseConfig::~BaseConfig() {

}

const std::map<int, std::string> &BaseConfig::GetDefaultErrorPages() const {
  return defaultErrorPages;
}
const std::string &BaseConfig::GetRoot() const {
  return root;
}
const std::vector<std::string> &BaseConfig::GetIndex() const {
  return index;
}
unsigned int BaseConfig::GetClientMaxBodySize() const {
  return clientMaxBodySize;
}
bool BaseConfig::IsAutoIndex() const {
  return autoIndex;
}

void BaseConfig::parseErrorPage(std::vector<std::string> &tokens, int line) {
  if (tokens.size() < 3) {
    throw parsing::ConfigException("invalid number of argument in \"error_page\" directive", line);
  }
  for (size_t i = 1; i < tokens.size() - 1; i++) {
    int num;
    try {
      num = ft_stoi(tokens[i]);
    }
    catch (...) {
      throw parsing::ConfigException("invalid arguments in \"error_page\" directive", line);
    }
    if (!isNum(tokens[i]) || num < 300 || num > 599) {
      throw parsing::ConfigException("invalid arguments in \"error_page\" directive", line);
    }
    defaultErrorPages[num] = tokens[tokens.size() - 1];
  }
}
void BaseConfig::parseIndex(std::vector<std::string> &tokens, int line) {
  if (tokens.size() < 2) {
    throw parsing::ConfigException("invalid number of argument in \"index\"", line);
  }
  index.insert(index.end(), ++tokens.begin(), tokens.end());
}
void BaseConfig::parseRoot(std::vector<std::string> &tokens, int line) {
  if (tokens.size() > 2) {
    throw parsing::ConfigException("invalid number of argument in \"root\" directive", line);
  }
  removeDoubles(root, '/');
  root = tokens[1];
  if (*(--root.end()) != '/') {
    root.push_back('/');
  }
}
void BaseConfig::parseClientMaxBodySize(std::vector<std::string> &tokens, int line) {
  if (tokens.size() != 2) {
    throw parsing::ConfigException("invalid number of argument in \"client_max_body_size\"", line);
  }
  int num;
  for (size_t i = 0; i < tokens[1].size(); i++) {
    if (i == tokens[1].size() - 1) {
      if (!(tokens[1][i] == 'm' || tokens[1][i] == 'M' || tokens[1][i] == 'k' || tokens[1][i] == 'K'
          || tokens[1][i] == 'g' || tokens[1][i] == 'G' || isdigit(tokens[1][i]))) {
        throw parsing::ConfigException("invalid arguments in \"client_max_body_size\" directive", line);
      }
    }
    else if (!std::isdigit(tokens[1][i])) {
      throw parsing::ConfigException("invalid arguments in \"client_max_body_size\" directive", line);
    }
  }
  try {
    num = ft_stoi(tokens[1]);
  } catch (...) {
    throw parsing::ConfigException("invalid number if argument in \"client_max_body_size\"", line);
  }
  if (tokens[1][tokens[1].size() - 1] == 'm' || tokens[1][tokens[1].size() - 1] == 'M') {
    num *= 1024 * 1024;
  }
  else if (tokens[1][tokens[1].size() - 1] == 'k' || tokens[1][tokens[1].size() - 1] == 'K') {
    num *= 1024;
  }
  else if (tokens[1][tokens[1].size() - 1] == 'g' || tokens[1][tokens[1].size() - 1] == 'G') {
    num *= 1024 * 1024 * 1024;
  }
  clientMaxBodySize = num;
  isClientMaxBodySizeMentioned = true;
}
void BaseConfig::parseAutoIndex(std::vector<std::string> &tokens, int line) {
  if (tokens.size() != 2) {
    throw parsing::ConfigException("invalid number if argument in \"autoindex\"", line);
  }
  if (tokens[1] == "on") {
    isAutoIndexMentioned = true;
    autoIndex = true;
  }
  else if (tokens[1] == "off") {
    isAutoIndexMentioned = true;
    autoIndex = false;
  }
  else {
    throw parsing::ConfigException("invalid number of argument in \"autoindex\" directive", line);
  }
}

// Setters
void BaseConfig::SetDefaultErrorPages(const std::map<int, std::string> &default_error_pages) {
  defaultErrorPages = default_error_pages;
}
void BaseConfig::SetRoot(const std::string &root) {
  BaseConfig::root = root;
}
void BaseConfig::SetIndex(const std::vector<std::string> &index) {
  BaseConfig::index = index;
}
void BaseConfig::SetClientMaxBodySize(unsigned long long int client_max_body_size) {
  clientMaxBodySize = client_max_body_size;
}
void BaseConfig::SetAutoIndex(bool auto_index) {
  autoIndex = auto_index;
}
bool BaseConfig::IsClientMaxBodySizeMentioned() {
  return isClientMaxBodySizeMentioned;
}
bool BaseConfig::IsAutoIndexMentioned() {
  return isAutoIndexMentioned;
}
