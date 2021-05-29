//
// Created by sergey on 22.03.2021.
//

#ifndef WEBSERV_ENVIRONMENTVARIABLES_ENVP_HPP_
#define WEBSERV_ENVIRONMENTVARIABLES_ENVP_HPP_

#include <string>
#include <vector>

class Envp {
  private:
  std::vector<std::vector<std::string> > envp;
  std::vector<std::string> splitNameVal(const std::string& str);
  public:
  Envp();
  Envp(char **_envp);
  Envp(const Envp &environment);
  Envp &operator=(const Envp &environment);
  ~Envp();

  std::string getenv(const std::string& name);
  void putenv(const std::string& name,const std::string& value);
  char **getenvp(); // need to clear memory after call
  std::vector<std::vector<std::string> > getenvpVector();
};

#endif //WEBSERV_ENVIRONMENTVARIABLES_ENVP_HPP_
