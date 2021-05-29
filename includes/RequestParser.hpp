//
// Created by sergey on 24.03.2021.
//

#ifndef WEBSERV_REQUESTHANDLER_REQUESTPARSER_HPP_
#define WEBSERV_REQUESTHANDLER_REQUESTPARSER_HPP_

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <ctime>
#include <algorithm>
#include "StringUtils.hpp"
#include "ParsingRequestException.hpp"
#include "Base64.hpp"

class RequestParser {
 public:
  struct sAuthorization {
    std::string type;
    std::string username;
    std::string password;
  };

 private:
  // start line of request
  std::string method;
  std::string targetOfRequest;
  std::string HTTPVersion;

  // structure for charset and language
  struct data {
    std::string data; // Accept-Charsets - charset, Accept-Language - language
    double factoryWeighting;
  };

  // implement headers
  std::vector<data>           RawAcceptCharset;
  std::vector<std::string>    acceptCharset;
  std::vector<data>           RawAcceptLanguage;
  std::vector<std::string>    acceptLanguage;
  std::vector<sAuthorization> authorization;
  std::vector<std::string>    contentLanguage;
  bool                        isContentLength;
  long long int               contentLength;
  std::string                 contentLocation;
  std::string                 contentType;
  struct tm                   date;
  bool                        isDatePresented;
  std::string                 host;
  std::string                 referer;
  std::string                 transferEncoding; // Nginx for default support only chunked
  std::string                 userAgent;
  std::map<std::string, std::string> CGIHeaders;

  // body of request
  std::string body;

//  std::vector<std::map<std::string, std::string> > headers;

  std::string getLine(std::string &request);
  size_t GetEndOfLine(const std::string &request, size_t startOfLine, size_t &offset);
  void        parseStartLine(const std::string &line);
  void        parseHeader(const std::string &line);

  // Parsing every headers function
  void        acceptCharSetParsing(const std::vector<std::string> &tokens);
  void        acceptLanguageParsing(const std::vector<std::string> &tokens);
  void        authorizationParsing(const std::vector<std::string> &tokens);
  void        contentLanguageParsing(const std::vector<std::string> &tokens);
  void        contentLengthParsing(const std::vector<std::string> &tokens);
  void        contentTypeParsing(const std::vector<std::string> &tokens);
  void        contentLocationParsing(const std::vector<std::string> &tokens);
  void        dateParsing(const std::vector<std::string> &tokens);
  void        hostParsing(const std::vector<std::string> &tokens);
  void        refererParsing(const std::vector<std::string> &tokens);
  void        transferEncodingParsing(const std::vector<std::string> &tokens);
  void        userAgentParsing(const std::vector<std::string> &tokens);

  // Parse body
  void        bodyParsing(std::string &str);
  void chunkedParsing(unsigned long long int size,
                      const std::string &request,
                      size_t StartOfLine); // I don't really know how to parse incorrect body

  // Utils functions
  void        normingAcceptCharset();
  void        normingAcceptLanguage();
  static bool isDataLess(const data &first, const data &second);
  data        parseDataStruct(const std::string &str);
  float       parseFactoryWeight(const std::string &str);
  bool        isFactoryWeight(const std::string &str);
public:
  RequestParser();
  RequestParser(const RequestParser &request);
  RequestParser &operator=(const RequestParser &request);
  ~RequestParser();

  // One function to parse
  int   ParseRequest(std::string request); // return code if error in request, else return 0
  void  ClearRequest();
  // Getters
  const std::string                 &GetMethod() const;
  const std::string                 &GetTargetOfRequest() const;
  const std::string                 &GetHttpVersion() const;
  const std::vector<std::string>    &GetAcceptCharset() const;
  const std::vector<std::string>    &GetAcceptLanguage() const;
  const std::vector<sAuthorization> &GetAuthorization() const;
  const std::vector<std::string>    &GetContentLanguage() const;
  size_t                            GetContentLength() const;
  const std::string                 &GetContentLocation() const;
  const std::string                 &GetContentType() const;
  const tm                          &GetDate() const;
  const std::string                 &GetHost() const;
  const std::string                 &GetReferer() const;
  const std::string                 &GetTransferEncoding() const;
  const std::string                 &GetUserAgent() const;
  const std::string                 &GetBody() const;
  const std::map<std::string, std::string> &GetCGIHeaders() const;
};

// Body parsing. TEST TEST TEST
#endif //WEBSERV_REQUESTHANDLER_REQUESTPARSER_HPP_
