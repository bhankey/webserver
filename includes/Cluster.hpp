
#ifndef WEBSERV_SERVER_CLUSTER_HPP_
#define WEBSERV_SERVER_CLUSTER_HPP_

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <semaphore.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <exception>

#define WORKERS 8

struct RWFDset
{
	fd_set	read_active;
	fd_set	read_ready;
	fd_set	write_active;
	fd_set	write_ready;
	int		MAXFD;
};

#include "Config.hpp"
class Server;
#include "Server.hpp"


class Cluster
{
private:
	Config					config;
	std::vector<Server *>	servers;
	std::vector<pid_t>		workers;

	void			spawnWorkers();
	void			manageServers(int i);
	static void		stop(int);

	Cluster();

public:
	Cluster(const std::string &pathToConfig);
	~Cluster();

	void	start();
};

#endif  //WEBSERV_SERVER_CLUSTER_HPP_
