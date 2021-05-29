
// by aronin

#include "../includes/Cluster.hpp"

Cluster	*THIS;

Cluster::Cluster(std::string const &pathToConfig)
{
	try {
		THIS = this;
		config.parseConfig(pathToConfig);
	}
	catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		stop(0);
	}
}

Cluster::~Cluster()
{
	stop(0);
};

void	Cluster::start()
{
	try {
		const std::vector<ServerConfig> &serverConfigs = config.GetServers();
		std::vector<ServerConfig>::const_iterator server = serverConfigs.begin();

		for ( ; server != serverConfigs.end(); ++server)
			servers.push_back(new Server(*server));

		signal(SIGINT, Cluster::stop);
		signal(SIGTERM, Cluster::stop);
		signal(SIGABRT, Cluster::stop);
		signal(SIGKILL, Cluster::stop);
		signal(SIGPIPE, Server::handleSIGPIPE);

		spawnWorkers();
	}
	catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		stop(0);
	}
}

void	Cluster::stop(int)
{
	if (!THIS->workers.empty() && getpid() < THIS->workers[0]) {
		for (size_t i = 0; i < THIS->workers.size(); ++i)
			kill(THIS->workers[i], SIGKILL);
	}

	std::vector<Server *>::iterator server = THIS->servers.begin();
	for ( ; server != THIS->servers.end(); ++server)
		delete *server;

	exit(0);
}

void 	Cluster::spawnWorkers()
{
	pid_t	pid;
	size_t  totalWorkers = config.GetWorkerProcesses();

	workers.reserve(totalWorkers);

	for (size_t i = 0; i < totalWorkers; ++i) {
		if (!(pid = fork())) manageServers(i);
		else if (pid > 0) workers.push_back(pid);
	}

	for (size_t i = 0; i < workers.size(); ++i)
		waitpid(workers[i], 0, 0);

	workers.clear();
	THIS->stop(0);
}

void	Cluster::manageServers(int i)
{
	(void)i;
	RWFDset			FDset;
	bzero(&FDset, sizeof(FDset));

	std::vector<Server *>::iterator server = servers.begin();
	for ( ; server != servers.end(); ++server)
	  (*server)->registerSocket(&FDset);

	while (true) {
		FDset.read_ready = FDset.read_active;
		FDset.write_ready = FDset.write_active;

		if (select(FDset.MAXFD + 1, &FDset.read_ready,
						&FDset.write_ready, NULL, NULL) == -1)
			throw std::runtime_error(strerror(errno));
		
		for (server = servers.begin();
				server != servers.end(); ++server)
		  (*server)->handleConnections(&FDset);
	}
	exit(0);
}
