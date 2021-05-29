//
// Created by sergey on 17.03.2021.
//

#include <climits>
#include <stdexcept>
#include "StringUtils.hpp"

std::vector<std::string> split(std::string str, const std::string &delims) {
  std::vector<std::string> tokens;
  trimString(str);
  for (size_t pos = str.find_first_of(delims); pos != std::string::npos; pos = str.find_first_of(delims)) {
    tokens.push_back(str.substr(0, pos));
    str.erase(0, pos + 1);
    while (str.find_first_of(delims) == 0) {
      str.erase(0, 1);
    }
  }
  if (!str.empty()) {
    tokens.push_back(str);
  }
  return tokens;
}
void trimString(std::string &str) {
  str.erase(0, str.find_first_not_of("\t\n\v\f\r "));
  str.erase(str.find_last_not_of("\t\n\v\f\r ") + 1);
}
int ft_stoi(const std::string &str) {
  long long int num = 0;
  bool sign = false;
  size_t i = 0;
  if (str[i] == '-'){
    i++;
    sign = true;
  }
  if (str[i] == '+'){
    i++;
  }
  if (!std::isdigit(str[i])) {
    throw std::invalid_argument("invalid argument");
  }
  for (; i < str.size() && std::isdigit(str[i]); ++i) {
    if ((num > INT_MAX + 1ll && sign == true) || (sign == false && num > INT_MAX)) {
      throw std::out_of_range("out of range");
    }
    num = num * 10 + str[i] - '0';
  }
  if ((num > INT_MAX + 1ll && sign == true) || (sign == false && num > INT_MAX)) {
    throw std::out_of_range("out of range");
  }
  if (sign == true)
    num *= -1;
  return static_cast<int>(num);
}
bool isNum(const std::string &str) {
  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] < '0' || str[i] > '9')
      return false;
  }
  return true;
}
std::string ft_itoa(const int num) {
  bool sign = false;
  long long int num_buf = num;
  if (num == 0)
    return "0";
  if (num < 0) {
    sign = true;
    num_buf = -num;
  }
  else {
    num_buf = num;
  }
  std::string res;
  do {
    res.insert(res.begin(), (num_buf % 10) + '0');
  } while (num_buf /= 10);
  if (sign) {
    res.insert(res.begin(), '-');
  }
  return (res);
}
void delete2dArr(char **arr) {
  for (int i = 0; arr[i] != NULL; i++) {
    delete[] arr[i];
  }
  delete[] arr;
}
long long int ft_stoll(const std::string &str) {
  unsigned long long int num = 0;
  bool sign = false;
  size_t i = 0;
  if (str[i] == '-'){
    i++;
    sign = true;
  }
  for (; i < str.size() && std::isdigit(str[i]); ++i) {
    if ((num > LLONG_MAX + 1ull && sign == true) || (sign == false && num > LLONG_MAX)) {
      throw std::exception();
    }
    num = num * 10 + str[i] - '0';
  }
  if ((num > LLONG_MAX + 1ull && sign == true) || (sign == false && num > LLONG_MAX)) {
    throw std::exception();
  }
  long long int res;
  if (sign == true)
    res = static_cast<long long int>(num) * -1ll;
  else
    res = static_cast<long long int>(num);
  return res;
}
void removeDoubles(std::string &str, char doubles) {
  std::string doublesStr;
  doublesStr.push_back(doubles);
  doublesStr.push_back(doubles);
  size_t find;
  while (( find = str.find(doublesStr)) != std::string::npos) {
    str.erase(find, 1);
  }
}
long long int ft_stoll_hex(const std::string &str) {
  unsigned long long int num = 0;
  bool sign = false;
  size_t i = 0;
  if (str[i] == '-') {
    i++;
    sign = true;
  }
  for (; i < str.size() && (std::isdigit(str[i]) || (str[i] >= 'A' && str[i] <= 'F') || (str[i] >= 'a' && str[i] <= 'f')); ++i) {
    if ((num > LLONG_MAX + 1ull && sign == true) || (sign == false && num > LLONG_MAX)) {
      throw std::exception();
    }
    num *= 16;
    if (std::isdigit(str[i])) {
      num += str[i] - '0';
    }
    else if ((str[i] >= 'A' && str[i] <= 'F')) {
      num += str[i] - 'A' + 10;
    }
    else {
      num += str[i] - 'a' + 10;
    }
  }
  long long int res;
  if (sign == true)
    res = static_cast<long long int>(num) * -1ll;
  else
    res = static_cast<long long int>(num);
  return res;
}
char *ft_strcpy(char *dst, const char *str) {

  char *buf = dst;
  while (*str != '\0') {
    *dst = *str;
    dst++;
    str++;
  }
  *dst = '\0';
  return buf;
}
