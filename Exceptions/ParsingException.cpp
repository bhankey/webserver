//
// Created by sergey on 18.03.2021.
//

#include "../includes/ParsingException.hpp"
parsing::ConfigException::ConfigException(const std::string &_massage, int line) throw(): std::exception(), message("Not valid configure file: ") {
  message += _massage + ":" + ft_itoa(line + 1);
}
const char *parsing::ConfigException::what() const throw() {
  return message.data();
}
parsing::ConfigException::~ConfigException() throw() {

}
