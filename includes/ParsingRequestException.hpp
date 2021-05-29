//
// Created by sergey on 25.03.2021.
//

#ifndef WEBSERV_EXCEPTIONS_PARSINGREQUESTEXCEPTION_HPP
#define WEBSERV_EXCEPTIONS_PARSINGREQUESTEXCEPTION_HPP

#include "exception"
#include <string>

class ParsingRequestException: public std::exception {
  private:
    std::string message;
    int         code;
  public:
    explicit ParsingRequestException(const std::string &message, int code = 0) throw();
    const char *what() const throw();
    int  returnCode() const throw();
    ~ParsingRequestException() throw();
  };

#endif //WEBSERV_EXCEPTIONS_PARSINGREQUESTEXCEPTION_HPP
