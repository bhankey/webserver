//
// Created by sergey on 17.03.2021.
//

#ifndef WEBSERV_CONFIG_READER_HPP_
#define WEBSERV_CONFIG_READER_HPP_

#include <vector>
#include <string>
#include <stdexcept>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 512
# endif

class Reader {
 private:
  std::string fileName;
 public:
  Reader();
  Reader(const Reader &reader);
  Reader(std::string _fileName);
  Reader &operator=(const Reader &reader);
  ~Reader();
  void setFile(std::string _fileName);
  const std::string &getFileName();
  std::vector<std::string> parseFile() throw(std::exception);
};

#endif //WEBSERV_CONFIG_READER_HPP_
