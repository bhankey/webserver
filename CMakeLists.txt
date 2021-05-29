cmake_minimum_required(VERSION 3.17)
project(webserv)

set(CMAKE_CXX_STANDARD 98)
include_directories(includes)
add_executable(webserv main.cpp Config/Config.cpp Config/BaseConfig.cpp Config/ServerConfig.cpp
        Config/ServerRoutes.cpp Config/Reader.cpp Server/Cluster.cpp Server/Server.cpp
        Server/Connection.cpp RequestHandler/RequestHandler.cpp RequestHandler/RequestParser.cpp
        RequestHandler/CGI.cpp RequestHandler/Envp.cpp Utils/Base64.cpp Utils/StringUtils.cpp
        Exceptions/CGIException.cpp Exceptions/ParsingException.cpp Exceptions/ParsingRequestException.cpp)