//
// Created by sergey on 25.03.2021.
//

#include "../includes/ParsingRequestException.hpp"
ParsingRequestException::ParsingRequestException(const std::string &_message, int _code) throw() {
  message = _message;
  code = _code;
}
const char *ParsingRequestException::what() const throw() {
  return message.c_str();
}
ParsingRequestException::~ParsingRequestException() throw() {

}
int ParsingRequestException::returnCode() const throw() {
  return code;
}
