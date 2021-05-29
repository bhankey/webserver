#include <iostream>

#include <iostream>
#include "includes/Cluster.hpp"

int main(int argc, char **argv) {

    if (argc < 2)
        std::cout << "No config file provided" << std::endl;
    else if (argc > 2)
        std::cout << "Too many arguments" << std::endl;
    else {
        Cluster webserv(argv[1]);

        webserv.start();
    }
}