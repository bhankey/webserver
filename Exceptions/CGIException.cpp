//
// Created by sergey on 23.03.2021.
//

#include "../includes/CGIException.hpp"

const char *CGIException::what() const throw() {
  return message.data();
}
CGIException::CGIException(const std::string &_message) throw() {
  message = "CGI failed: " + _message;
}
CGIException::~CGIException() throw() {

}
