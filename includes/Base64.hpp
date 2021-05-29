//
// Created by sergey on 28.03.2021.
//

#ifndef WEBSERV_UTILS_BASE64_HPP_
#define WEBSERV_UTILS_BASE64_HPP_

#include <string>

std::string decodeBase64(const std::string &str);
bool        isBase64(std::string &str);
#endif //WEBSERV_UTILS_BASE64_HPP_