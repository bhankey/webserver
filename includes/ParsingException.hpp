//
// Created by sergey on 18.03.2021.
//

#ifndef WEBSERV_EXCEPTIONS_PARSINGEXCEPTION_HPP_
#define WEBSERV_EXCEPTIONS_PARSINGEXCEPTION_HPP_

#include <exception>
#include <string>
#include "StringUtils.hpp"

namespace parsing {

class ConfigException : public std::exception {
 private:
  std::string message;
 public:
  explicit ConfigException(const std::string &message, int line = 0) throw();
  const char *what() const throw();
  ~ConfigException() throw();
};

}
#endif //WEBSERV_EXCEPTIONS_PARSINGEXCEPTION_HPP_
