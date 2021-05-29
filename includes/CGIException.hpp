//
// Created by sergey on 23.03.2021.
//

#ifndef WEBSERV_EXCEPTIONS_CGIEXCEPTION_HPP_
#define WEBSERV_EXCEPTIONS_CGIEXCEPTION_HPP_

#include <exception>
#include <string>

class CGIException: public std::exception {
 private:
  std::string message;
 public:
  CGIException(const std::string &message) throw();
  const char *what() const throw();
  ~CGIException() throw();

};

#endif //WEBSERV_EXCEPTIONS_CGIEXCEPTION_HPP_
