//
// Created by sergey on 22.03.2021.
//

#include "../includes/Envp.hpp"
Envp::Envp(char **_envp) {
  for (int i = 0; _envp[i] != NULL; i++) {
    envp.push_back(splitNameVal(_envp[i]));
  }
}
Envp::Envp(const Envp &environment): envp(environment.envp) {

}
Envp &Envp::operator=(const Envp &environment) {
  if (this != &environment) {
    envp = environment.envp;
  }
  return *this;
}
std::string Envp::getenv(const std::string& name) {
  for (std::vector<std::vector<std::string> >::iterator it = envp.begin(); it != envp.end(); ++it) {
    if ((*it)[0] == name) {
      return ((*it)[1]);
    }
  }
  return ("");
}
std::vector<std::string> Envp::splitNameVal(const std::string& str) {
  std::vector<std::string> tokens(2);
  size_t pos = str.find_first_of('=');
  tokens[0] = str.substr(0, pos);
  tokens[1] = str.substr(pos + 1, std::string::npos);
  return (tokens);
}
void Envp::putenv(const std::string &name, const std::string &value) {
  for (std::vector<std::vector<std::string> >::iterator it = envp.begin(); it != envp.end(); ++it) {
    if ((*it)[0] == name) {
      (*it)[1] = value;
      return;
    }
  }
  std::vector<std::string> env(2);
  env[0] = name;
  env[1] = value;
  envp.push_back(env);
}
char **Envp::getenvp() {
  char **CEnvp = new char *[envp.size() + 1]();
  for (size_t i = 0; i < envp.size(); i++) {
    std::string env = envp[i][0] + "=" + envp[i][1];
    CEnvp[i] = new char[env.size() + 1]();
    env.copy(CEnvp[i], env.size());
    CEnvp[i][env.size()] = '\0';
  }
  CEnvp[envp.size()] = NULL;
  return CEnvp;
}
std::vector<std::vector<std::string> > Envp::getenvpVector() {
  return envp;
}
Envp::~Envp() {

}
Envp::Envp() {

}
