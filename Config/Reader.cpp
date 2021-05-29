//
// Created by sergey on 17.03.2021.
//

#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include "../includes/Reader.hpp"

Reader::Reader(): fileName() {

}
Reader::Reader(const Reader &reader): fileName(reader.fileName) {

}
Reader::Reader(std::string _fileName): fileName(_fileName) {

}
Reader &Reader::operator=(const Reader &reader) {
  if (this != &reader) {
    fileName = reader.fileName;
  }
  return *this;
}
Reader::~Reader() {

}
void Reader::setFile(std::string _fileName) {
  fileName = _fileName;
}
const std::string &Reader::getFileName() {
  return fileName;
}
std::vector<std::string> Reader::parseFile() throw(std::exception){
  int flag;
  int fd;
  std::vector<std::string> fileBuf;
  std::vector<char> buf(BUFFER_SIZE);

  if ((fd = open(fileName.c_str(), O_RDONLY)) < 0) {
    throw std::invalid_argument("File can't be open");
  }
  std::string stringBuf;
  stringBuf.reserve(BUFFER_SIZE);
  while ((flag = read(fd, buf.data(), BUFFER_SIZE)) > 0) {
    std::vector<char>::iterator findEndl;
    while ((findEndl = std::find(buf.begin(), buf.end(), '\n')) != buf.end()) {
      stringBuf.append(buf.begin(), findEndl);
      fileBuf.push_back(stringBuf);
      stringBuf.clear();
      buf.erase(buf.begin(), findEndl + 1);
    }
    if (buf[0] == '\0') {
      buf.clear();
      buf.resize(BUFFER_SIZE);
      continue;
    }
    else {
      stringBuf.append(buf.begin(), buf.end());
      buf.clear();
      buf.resize(BUFFER_SIZE);
    }
  }
  if (flag < 0) {
    throw std::runtime_error("Error while reading file");
  }
  if (!stringBuf.empty()) {
    fileBuf.push_back(stringBuf);
  }
  return (fileBuf);
}
