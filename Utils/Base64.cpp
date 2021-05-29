//
// Created by sergey on 28.03.2021.
//

#include "Base64.hpp"
#include <vector>

bool isBase64(std::string &str) {
  for (size_t i = 0; i < str.size(); i++) {
    if (!(isalnum(str[i]) || (str[i] == '+') || (str[i] == '/') || (str[i] == '='))) {
      return false;
    }
  }
  return true;
}

std::string decodeBase64(const std::string &str) {
  static const std::string base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::vector<unsigned char> Bytes(4);
  std::string ret;
  size_t i = 0;
  if (str.empty())
    return "";
  for (size_t len = str.size(), initial = 0; len > 0 && str[initial] != '='; --len, ++initial) {
    Bytes[i++] = str[initial];
    if (i == 4) {
      for (i = 0; i < 4; i++) {
        Bytes[i] = base64.find(Bytes[i]);
      }
      Bytes[0] = (Bytes[0] << 2) + ((Bytes[1] & 0x30) >> 4);
      Bytes[1] = ((Bytes[1] & 0xf) << 4) + ((Bytes[2] & 0x3c) >> 2);
      Bytes[2] = ((Bytes[2] & 0x3) << 6) + Bytes[3];
      ret.insert(ret.end(), Bytes.begin(), --Bytes.end());
      i = 0;
    }
  }
  if (i != 0) {
    for (size_t j = 0; j < 4; j++) {
      Bytes[j] = base64.find(Bytes[j]);
    }
    Bytes[0] = (Bytes[0] << 2) + ((Bytes[1] & 0x30) >> 4);
    Bytes[1] = ((Bytes[1] & 0xf) << 4) + ((Bytes[2] & 0x3c) >> 2);
    Bytes[2] = ((Bytes[2] & 0x3) << 6) + Bytes[3];
    for (size_t j = 0; j < i - 1; j++) {
      ret.push_back(Bytes[j]);
    }
  }
  return ret;
}


