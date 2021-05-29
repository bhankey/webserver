//
// Created by sergey on 22.03.2021.
//

#include <cstring>
#include "../includes/CGI.hpp"
CGI::CGI(const ServerConfig &_server, const ServerRoutes &_location,
         const Connection &_client, const RequestParser &_request, std::string _pathToExecutable):
        server(_server),
        location(_location),
        client(_client),
        requestParser(_request),
        envp(),
        pathToExecutable(_pathToExecutable) {
  setEnvp();
}
CGI::CGI(const CGI &cgi):
          server(cgi.server),
          location(cgi.location),
          client(cgi.client),
          requestParser(cgi.requestParser),
          envp(cgi.envp),
          pathToExecutable(cgi.pathToExecutable) {
}
CGI &CGI::operator=(const CGI &cgi) {
  static_cast<void>(cgi);
  return *this;
}
CGI::~CGI() {

}
void CGI::setEnvp() {
  // Authentication
  if (!requestParser.GetAuthorization().empty()) {
    envp.putenv("AUTH_TYPE", requestParser.GetAuthorization()[0].type);
    envp.putenv("REMOTE_USER", requestParser.GetAuthorization()[0].username);
    envp.putenv("REMOTE_IDENT", requestParser.GetAuthorization()[0].password);
  }
  if (requestParser.GetContentType().empty()) {
    envp.putenv("CONTENT_TYPE", requestParser.GetContentType());
  }
  envp.putenv("GATEWAY_INTERFACE", "CGI/1.1");
  envp.putenv("REQUEST_METHOD", requestParser.GetMethod());
  envp.putenv("CONTENT_LENGTH", ft_itoa(requestParser.GetContentLength()));
  envp.putenv("SERVER_PORT", ft_itoa(static_cast<int>(server.GetHostPort().port)));
  envp.putenv("SERVER_PROTOCOL", "HTTP/1.1");
  envp.putenv("SERVER_SOFTWARE", "Webserv_21");
  if (!server.GetServerNames().empty()) {
    envp.putenv("SERVER_NAME", server.GetServerNames()[0]);
  }
  else {
    envp.putenv("SERVER_NAME", server.GetHostPort().host);
  }
  envp.putenv("REMOTE_ADDR", ft_inet_ntop(client.getClientIP().s_addr));

  std::string Target = requestParser.GetTargetOfRequest();
  envp.putenv("REQUEST_URI", Target);
  size_t query;
  if ((query = Target.find('?')) != std::string::npos) {
    envp.putenv("QUERY_STRING", Target.substr(query + 1, std::string::npos));
    Target.erase(query, std::string::npos);
  }

  envp.putenv("PATH_INFO", Target);
  envp.putenv("PATH_TRANSLATED", Target);
  envp.putenv("SCRIPT_NAME", Target);
  for (std::map<std::string, std::string>::const_iterator it = requestParser.GetCGIHeaders().begin(); it != requestParser.GetCGIHeaders().end(); ++it) {
    envp.putenv("HTTP_" + it->first, it->second);
  }
  envp.putenv("SCRIPT_FILENAME", location.GetRoot() + Target); // For php-cgi
  envp.putenv("REDIRECT_STATUS", "200"); //https://www.php.net/security.cgi-bin
}
std::string CGI::CGICall() {

  pid_t pid;
  char **envp_buf;

  char **argv = new char *[2];
  argv[0] = new char [pathToExecutable.size() + 1];
  ft_strcpy(argv[0], pathToExecutable.data());
  argv[1] = NULL;

  envp_buf = envp.getenvp();
  int FileInput = open("webserv_cgi_temp_2", O_CREAT | O_TRUNC | O_RDWR, 0777);
  if (FileInput < 0) {
    std::cerr << "Can't open temporary file for cgi" << std::endl;
    return "";
  }
  if (write(FileInput,requestParser.GetBody().c_str(), requestParser.GetBody().size())
                                                          != static_cast<ssize_t>(requestParser.GetBody().size())) {
    close(FileInput);
    std::cerr << "Can't write to temporary file for cgi" << std::endl;
    return "";
  }
  close(FileInput);
  pid = fork();
  if (pid < 0) {
    std::cerr << "Fork failed in cgi call" << std::endl;
    return "";
  }
  else if (pid == 0) {
    int FileInput = open("webserv_cgi_temp_2", O_RDONLY, 0777);
    int FileOutput = open("webserv_cgi_temp", O_CREAT | O_TRUNC | O_RDWR, 0777);
    if (FileInput < 0 || FileOutput < 0) {
      std::cerr << "Can't open temporary files in child process" << std::endl;
      exit(1);
    }
    if (dup2(FileInput, STDIN_FILENO) < 0 || dup2(FileOutput, STDOUT_FILENO) < 0) {
      std::cerr << "System call dup2 failed in child process" << std::endl;
      exit(1);
    }
    execve(pathToExecutable.c_str(), argv, envp_buf);
    exit(1);
  }
  waitpid(-1, NULL, 0);
  std::string output;
  int OutputCGI = open("webserv_cgi_temp", O_RDONLY);
  if (OutputCGI < 0) {
    std::cerr << "Can't open temporary file for reading cgi output" << std::endl;
    return "";
  }
  char buf[513];
  int bytes;
  while ((bytes = read(OutputCGI, buf, 512)) > 0) {
    buf[bytes] = '\0';
    output += buf;
  }
  if (bytes < 0) {
    close(OutputCGI);
    std::cerr << "Can't read results of cgi work" << std::endl;
    return "";
  }
  close(OutputCGI);
  delete2dArr(argv);
  delete2dArr(envp_buf);
  return output;
}

std::string CGI::ft_inet_ntop(unsigned int addr) {
    std::string result;
    unsigned char byte[4];
    byte[0] = static_cast<unsigned char>(addr & 0xFF) ;
    byte[1] = static_cast<unsigned char>((addr >> 8) & 0xFF) ;
    byte[2] = static_cast<unsigned char>((addr >> 16) & 0xFF) ;
    byte[3] = static_cast<unsigned char>((addr >> 24) & 0xFF) ;
    result = ft_itoa(byte[0]) + "." + ft_itoa(byte[1]) + "."
        + ft_itoa(byte[2]) + "." + ft_itoa(byte[3]);
    return result;
}

