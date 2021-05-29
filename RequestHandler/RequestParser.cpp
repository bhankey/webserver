//
// Created by sergey on 24.03.2021.
//


#include "../includes/RequestParser.hpp"


// Constructors, destructors, operations
RequestParser::RequestParser(const RequestParser &request):
    method(request.method),
    targetOfRequest(request.targetOfRequest),
    HTTPVersion(request.HTTPVersion),
    RawAcceptCharset(request.RawAcceptCharset),
    acceptCharset(request.acceptCharset),
    RawAcceptLanguage(request.RawAcceptLanguage),
    acceptLanguage(request.acceptLanguage),
    authorization(request.authorization),
    contentLanguage(request.contentLanguage),
    isContentLength(request.isContentLength),
    contentLength(request.contentLength),
    contentLocation(request.contentLocation),
    contentType(request.contentType),
    date(request.date),
    isDatePresented(request.isDatePresented),
    host(request.host),
    referer(request.referer),
    transferEncoding(request.transferEncoding),
    userAgent(request.userAgent),
    CGIHeaders(request.CGIHeaders) {}
RequestParser::RequestParser() {
  isDatePresented = false;
  isContentLength = false;
  contentLength = 0;
}
RequestParser::~RequestParser() {}
RequestParser &RequestParser::operator=(const RequestParser &request) {
  if (this != &request) {
    method = request.method;
    targetOfRequest = request.targetOfRequest;
    HTTPVersion = request.HTTPVersion;
    RawAcceptCharset = request.RawAcceptCharset;
    acceptCharset = request.acceptCharset;
    RawAcceptLanguage = request.RawAcceptLanguage;
    acceptLanguage = request.acceptLanguage;
    authorization = request.authorization;
    contentLanguage = request.contentLanguage;
    isContentLength = request.isContentLength;
    contentLength = request.contentLength;
    contentLocation = request.contentLocation;
    contentType = request.contentType;
    date = request.date;
    isDatePresented = request.isDatePresented;
    host = request.host;
    referer = request.referer;
    transferEncoding = request.transferEncoding;
    userAgent = request.userAgent;
    body = request.body;
    CGIHeaders = request.CGIHeaders;
  }
  return *this;
}

// interface to work with parsing
int RequestParser::ParseRequest(std::string request) {
    ClearRequest();
    try {
        parseStartLine(getLine(request));
        std::string line(getLine(request));
        while (!line.empty()) {
            try {
                parseHeader(line);
            }
            catch (ParsingRequestException &exception) {
                std::cerr << exception.what() << std::endl; // Need to comment
                if (exception.returnCode() != 0) {
                    return (exception.returnCode());
                }
            }
            line = getLine(request);
        }
        normingAcceptCharset();
        normingAcceptLanguage();
        if (!request.empty()) {
            bodyParsing(request);
        }
        if (host.empty()) {
          throw ParsingRequestException("No Host header", 400);
        }
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl; // Write only for debug
        return 400;
    }
    return 0;
}
void RequestParser::ClearRequest() {
  method.clear();
  targetOfRequest.clear();
  HTTPVersion.clear();
  RawAcceptCharset.clear();
  acceptCharset.clear();
  RawAcceptLanguage.clear();
  acceptLanguage.clear();
  authorization.clear();
  contentLanguage.clear();
  isContentLength = false;
  contentLength = 0;
  contentLocation.clear();
  contentType.clear();
  isDatePresented = false;
  host.clear();
  referer.clear();
  transferEncoding.clear();
  userAgent.clear();
  body.clear();
}

// Main parsing functions
void RequestParser::parseStartLine(const std::string &startLine) {
  std::vector<std::string> tokens(split(startLine, " "));
  if (tokens.size() != 3) {
    throw ParsingRequestException("start line in request is incorrect", 400);
  }
  method = tokens[0];
  targetOfRequest = tokens[1];
  HTTPVersion = tokens[2];
  if (!((HTTPVersion != "HTTP/1.0") || (HTTPVersion != "HTTP/1.1"))) {
    throw ParsingRequestException("incorrect version of HTTP", 505);
  }
}
void RequestParser::parseHeader(const std::string &line) {
  static const size_t implementHeadersCount = 12;
  static const std::string implementHeaders[implementHeadersCount] = {
      "Accept-Charset", "Accept-Language", "Authorization", "Content-Language",
      "Content-Length", "Content-Type", "Date", "Host",
      "Referer", "Transfer-Encoding", "User-Agent", "Content-Location"};
  static void  (RequestParser::*headersParseFunctions[implementHeadersCount])(const std::vector<std::string> &) = {
       &RequestParser::acceptCharSetParsing, &RequestParser::acceptLanguageParsing, &RequestParser::authorizationParsing,
       &RequestParser::contentLanguageParsing, &RequestParser::contentLengthParsing,
       &RequestParser::contentTypeParsing, &RequestParser::dateParsing, &RequestParser::hostParsing,
       &RequestParser::refererParsing, &RequestParser::transferEncodingParsing, &RequestParser::userAgentParsing,
       &RequestParser::contentLocationParsing};

  size_t pos;
  if ((pos = line.find(':')) == std::string::npos) {
    throw ParsingRequestException("not valid header");
  }
  std::vector<std::string> tokens(2);
  tokens[0] = line.substr(0, pos);
  tokens[1] = line.substr(pos + 1, std::string::npos);
  trimString(tokens[1]);
  if (tokens[0].empty() || tokens[1].empty()) {
    throw ParsingRequestException("not valid header");
  }
  for (size_t i = 0; i < implementHeadersCount; i++) {
    if (tokens[0] == implementHeaders[i]) {
      (this->*headersParseFunctions[i])(tokens);
      return;
    }
  }
  CGIHeaders[tokens[0]] = tokens[1];
}

// Helping parsing functions
void RequestParser::userAgentParsing(const std::vector<std::string> &tokens) {
  userAgent = tokens.at(1);
  CGIHeaders[tokens[0]] = tokens[1];
}
void RequestParser::transferEncodingParsing(const std::vector<std::string> &tokens) {
  static const std::string availableEncodingChunks = "chunked";
  if (!transferEncoding.empty()) {
    throw ParsingRequestException("several Transfer-Encoding", 400);
  }
  std::string encoding = tokens[1];
  trimString(encoding);
  if (encoding != "chunked") {
    throw ParsingRequestException("Not Implemented", 501);
  }
  transferEncoding = encoding;
  CGIHeaders[tokens[0]] = encoding;
}
void RequestParser::refererParsing(const std::vector<std::string> &tokens) {
  referer = tokens[1]; // How i can check referer (it can be not absolute url)
  CGIHeaders[tokens[0]] = tokens[1];
}
void RequestParser::hostParsing(const std::vector<std::string> &tokens) {
  if (host.empty()) {
    host = tokens[1]; // Maybe make structure of {std::string host, size_t port} ???
    CGIHeaders[tokens[0]] = tokens[1];
  }
  else {
    throw ParsingRequestException("Host header was too times", 400);
  }
}
void RequestParser::dateParsing(const std::vector<std::string> &tokens) {
  if (tokens[1].size() != 29) {
    throw ParsingRequestException("error in Date header");
  }
  char datestr[30];
  for (size_t i = 0; i < tokens[1].size(); i++) {
    datestr[i] = tokens[1][i];
  }
  datestr[29] = '\0';
  if (*strptime(datestr, "%a, %d %b %Y %T GMT", &date) != '\0') {
    throw ParsingRequestException("error in Date header");
  }
  isDatePresented = true;
  CGIHeaders[tokens[0]] = tokens[1];
}
void RequestParser::contentTypeParsing(const std::vector<std::string> &tokens) {
  contentType = tokens[1];
  CGIHeaders[tokens[0]] = tokens[1];
}
void RequestParser::contentLocationParsing(const std::vector<std::string> &tokens) {
  contentLocation = tokens[1];
  CGIHeaders[tokens[0]] = tokens[1];
}
void RequestParser::contentLengthParsing(const std::vector<std::string> &tokens) {
  long long int numeric;
  if (isContentLength == false && isNum(tokens[1]) == true) {
    try {
      numeric = ft_stoll(tokens[1]);
    }
    catch (...) {
      throw ParsingRequestException("error in content-length", 400);
    }
    if (numeric >= 0) {
      contentLength = numeric;
      isContentLength = true;
      return;
    }
  }
  throw ParsingRequestException("error in content-length", 400);
}
void RequestParser::contentLanguageParsing(const std::vector<std::string> &tokens) {
  std::vector<std::string> languages = split(tokens[1], ",");
  for (size_t i = 0; i < languages.size(); i++) {
    trimString(languages[i]);
    contentLanguage.push_back(languages[i]);
  }
  CGIHeaders[tokens[0]] = tokens[1];
}
void RequestParser::authorizationParsing(const std::vector<std::string> &tokens) {
  std::vector<std::string> authorization(2);
  if (tokens[1].find(' ') == std::string::npos) {
    throw ParsingRequestException("authorization header incorrect");
  }
  authorization[0] = tokens[1].substr(0, tokens[1].find(' '));
  authorization[1] = tokens[1].substr(tokens[1].find(' ') + 1, std::string::npos);
  trimString(authorization[0]);
  trimString(authorization[1]);
  if (authorization[0].empty() || authorization[1].empty()) {
    throw ParsingRequestException("authorization header incorrect");
  }
  if (authorization[1].find(':') == std::string::npos) {
    if (isBase64(authorization[1]))
      authorization[1] = decodeBase64(authorization[1]);
    else {
      throw ParsingRequestException("authorization header incorrect");
    }
  }
  if (authorization[1].find(':')) {
    std::vector<std::string> userPasswd(2);
    userPasswd[0] = authorization[1].substr(0, authorization[1].find(':'));
    userPasswd[1] = authorization[1].substr(authorization[1].find(':') + 1, std::string::npos);
    if (userPasswd[0].empty() || userPasswd[1].empty()) {
      throw ParsingRequestException("authorization header incorrect");
    }
    sAuthorization authStruct;
    authStruct.type = authorization[0];
    authStruct.username = userPasswd[0];
    authStruct.password = userPasswd[1];
    CGIHeaders[tokens[0]] = tokens[1];
    this->authorization.push_back(authStruct);
  }
  else {
    throw ParsingRequestException("authorization header incorrect");
  }
}
void RequestParser::acceptCharSetParsing(const std::vector<std::string> &tokens) {
  std::vector<std::string> sets = split(tokens[1], ",");
  std::vector<data> data;
  bool isCorrect = true;
  for (size_t i = 0; i < sets.size(); i++) {
    try {
      data.push_back(parseDataStruct(sets[i]));
    }
    catch (ParsingRequestException &exception) {
      std::cerr << exception.what() << std::endl; // Need to comment
      isCorrect = false;
      break;
    }
  }
  if (isCorrect) {
    CGIHeaders[tokens[0]] = tokens[1];
    RawAcceptCharset.insert(RawAcceptCharset.end(), data.begin(), data.end());
  }
}
RequestParser::data RequestParser::parseDataStruct(const std::string &str) {
  data set;
  if (str.find(";q=") == std::string::npos) {
      set.data = str;
      set.factoryWeighting = 1.0f;
    return set;
  }
  else {
      set.data = str.substr(0, str.find(";q="));
      trimString(set.data);
      set.factoryWeighting = parseFactoryWeight(str.substr(str.find(";q=") + 3, std::string::npos));
      return set;
  }
}
void RequestParser::acceptLanguageParsing(const std::vector<std::string> &tokens) {
  std::vector<std::string> sets = split(tokens[1], ",");
  std::vector<data> data;
  bool isCorrect = true;
  for (size_t i = 0; i < sets.size(); i++) {
    try {
      data.push_back(parseDataStruct(sets[i]));
    }
    catch (ParsingRequestException &exception) {
      std::cerr << exception.what() << std::endl; // Need to comment
      isCorrect = false;
      break;
    }
  }
  if (isCorrect) {
    CGIHeaders[tokens[0]] = tokens[1];
    RawAcceptLanguage.insert(RawAcceptLanguage.end(), data.begin(), data.end());
  }
}

// Body parsing

// RFC 7230
//4.1.3.  Decoding Chunked
//
//A process for decoding the chunked transfer coding can be represented
//in pseudo-code as:
//
//length := 0
//read chunk-size, chunk-ext (if any), and CRLF
//while (chunk-size > 0) {
//    read chunk-data and CRLF
//    append chunk-data to decoded-body
//    length := length + chunk-size
//    read chunk-size, chunk-ext (if any), and CRLF
//}
//read trailer field
//while (trailer field is not empty) {
//  if (trailer field is allowed to be sent in a trailer) {
//    append trailer field to existing header fields
//  }
//  read trailer-field
// }
//Content-Length := length
//    Remove "chunked" from Transfer-Encoding
//    Remove Trailer from existing header fields

void RequestParser::bodyParsing(std::string &request) {

  body.clear();
  if (transferEncoding.empty()) {
    body.insert(body.end(), request.begin(), request.end());
  }
  else {
    size_t lineStart = 0;
    size_t lineEnd;
    size_t offset;
    while ((lineEnd = GetEndOfLine(request, lineStart + 1, offset)) != std::string::npos) {
      std::string sizeOfNextLine = request.substr(lineStart, lineEnd - lineStart - offset);
      if (sizeOfNextLine.empty()) {
        continue;
      }
      unsigned long long int size = ft_stoll_hex(sizeOfNextLine);
      if (size == 0) {
        break;
      }
      lineStart = lineEnd + 1;
      chunkedParsing(size, request, lineStart);
      lineStart += size;
      if (lineStart + 3 < request.size() && request[lineStart] == '\r' && request[lineStart + 1] == '\n') {
        lineStart += 2;
      }
      else if (lineStart + 2 < request.size() && request[lineStart] == '\n') {
        lineStart += 1;
      }
      else {
        break;
      }
    }
  }
}

void RequestParser::chunkedParsing(unsigned long long int size, const std::string &request, size_t StartOfLine) {
  if (size + StartOfLine < request.size()) {
    body.insert(body.size(), request, StartOfLine, size);
  }
}


// Utils for parsing

void  RequestParser::normingAcceptCharset() {
  std::sort(RawAcceptCharset.begin(), RawAcceptCharset.end(), isDataLess);
  acceptCharset.clear();
  acceptCharset.reserve(RawAcceptCharset.size());
  for (std::vector<data>::iterator it = RawAcceptCharset.begin(); it != RawAcceptCharset.end(); it++) {
    acceptCharset.push_back(it->data);
  }
}
void  RequestParser::normingAcceptLanguage() {
  std::sort(RawAcceptLanguage.begin(), RawAcceptLanguage.end(), &RequestParser::isDataLess);
  acceptLanguage.clear();
  acceptLanguage.reserve(RawAcceptLanguage.size());
  for (std::vector<data>::iterator it = RawAcceptLanguage.begin(); it != RawAcceptLanguage.end(); it++) {
    acceptLanguage.push_back(it->data);
  }
}
bool  RequestParser::isDataLess(const data &first, const data &second) {
  if (first.factoryWeighting > second.factoryWeighting) {
    return true;
  }
  return false;
}
float RequestParser::parseFactoryWeight(const std::string &str) {
  std::vector<std::string> floatNum(split(str, "."));
  double value;
  if (floatNum.size() == 1) {
    try {
      value = static_cast<float>(ft_stoi(floatNum[0]));
    }
    catch (...) {
      throw ParsingRequestException("not valid factory weight");
    }
    if ((value != 0 && value != 1) || (!isFactoryWeight(floatNum[0]))) {
      throw ParsingRequestException("not valid factory weight");
    }
    return (static_cast<float>(value));
  }
  else if (floatNum.size() == 2) {
    if (!isFactoryWeight(floatNum[0])) {
      return false;
    }
    int intPart;
    double floatPart;
    try {
      intPart = ft_stoi(floatNum[0]);
      floatPart = ft_stoi(floatNum[1]);
    }
    catch (...) {
      throw ParsingRequestException("not valid factory weight");
    }
    for (size_t i = 0; i < floatNum[1].size(); i++) {
      floatPart /= 10;
    }
    value = static_cast<float>(intPart) + floatPart;
    if (value > 1 || value < 0) {
      throw ParsingRequestException("not valid factory weight");
    }
    return (value);
  }
  else {
    throw ParsingRequestException("not valid factory weight");
  }
}
bool  RequestParser::isFactoryWeight(const std::string &str) {
  size_t i = 0;
  bool wasPoint = false;
  for (; i < str.size(); i++) {
    if (!std::isdigit(str[i]) && (str[i] != '.')) {
      return false;
    }
    if (str[i] == '.' && wasPoint) {
      return false;
    }
    if (str[i] == '.') {
      wasPoint = true;
    }
  }
  return true;
}
std::string RequestParser::getLine(std::string &request) {
  size_t CRPos = request.find('\r');
  size_t LFPos = request.find('\n');
  if (LFPos != std::string::npos) {
    if (CRPos != std::string::npos && LFPos - CRPos == 1)  {
      std::string requestLine = request.substr(0, CRPos);
      request.erase(0, LFPos + 1);
      return requestLine;
    }
    else {
      std::string requestLine = request.substr(0, LFPos);
      request.erase(0, LFPos + 1);
      return requestLine;
    }
  }
  return "";
}

size_t RequestParser::GetEndOfLine(const std::string &request, size_t startOfLine, size_t &offset) {
  size_t CRPos = request.find('\r', startOfLine);
  size_t LFPos = request.find('\n', startOfLine);
  size_t end;
  if (LFPos != std::string::npos) {
    if (CRPos != std::string::npos && LFPos - CRPos == 1)  {
      end = LFPos;
      offset = 1;
      return end;
    }
    else {
      end = LFPos + 1;
      offset = 0;
      return end;
    }
  }
  return std::string::npos;
}

// Getters
const std::string &RequestParser::GetMethod() const {
  return method;
}
const std::string &RequestParser::GetTargetOfRequest() const {
  return targetOfRequest;
}
const std::string &RequestParser::GetHttpVersion() const {
  return HTTPVersion;
}
const std::vector<std::string> &RequestParser::GetAcceptCharset() const {
  return acceptCharset;
}
const std::vector<std::string> &RequestParser::GetAcceptLanguage() const {
  return acceptLanguage;
}
const std::vector<RequestParser::sAuthorization> &RequestParser::GetAuthorization() const {
  return authorization;
}
const std::vector<std::string> &RequestParser::GetContentLanguage() const {
  return contentLanguage;
}
size_t RequestParser::GetContentLength() const {
  return contentLength;
}
const std::string &RequestParser::GetContentLocation() const {
  return contentLocation;
}
const std::string &RequestParser::GetContentType() const {
  return contentType;
}
const tm & RequestParser::GetDate() const {
  return date;
}
const std::string &RequestParser::GetHost() const {
  return host;
}
const std::string &RequestParser::GetReferer() const {
  return referer;
}
const std::string &RequestParser::GetTransferEncoding() const {
  return transferEncoding;
}
const std::string &RequestParser::GetUserAgent() const {
  return userAgent;
}
const std::string &RequestParser::GetBody() const {
  return body;
}
const std::map<std::string, std::string> &RequestParser::GetCGIHeaders() const {
  return CGIHeaders;
}

