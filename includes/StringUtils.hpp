//
// Created by sergey on 17.03.2021.
//

#ifndef WEBSERV_UTILS_STRINGUTILS_HPP_
#define WEBSERV_UTILS_STRINGUTILS_HPP_

#include <vector>
#include <string>
#include <exception>

std::vector<std::string> split(std::string str, const std::string &delimiter);
void  trimString(std::string &str);
int   ft_stoi(const std::string &str);
bool  isNum(const std::string &str);
std::string ft_itoa(const int num);
void  delete2dArr(char **arr);
long long int ft_stoll(const std::string &str);
long long int ft_stoll_hex(const std::string &str);
void removeDoubles(std::string &str, char doubles);
char *ft_strcpy(char *dst, const char *str);
#endif //WEBSERV_UTILS_STRINGUTILS_HPP_
