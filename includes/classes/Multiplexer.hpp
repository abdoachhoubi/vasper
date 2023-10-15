#pragma once

#include "../main.hpp"

class Server;
class Client;

class Multiplexer {
    public:
        Multiplexer();
        ~Multiplexer();

        void    setupServers(std::vector<Server> servers);
        void    runServers();

        void    init_fds();
        void	addToSet(const int i, fd_set &set);
        void	removeFromSet(const int i, fd_set &set);
        void    acceptNewConnection(Server &serv);

        void    readRequest(const int &i, Client &c);
        void    closeConnection(const int i);
        // void    assignServer(Client &c);
        void    sendResponse(const int &i, Client &c);
		void    sendAstro(const int &i, Client &client);
        void    buildTheResponse(Client &client);
        
    private:
        std::vector<Server>		_servers;
        std::map<int, Server>	_servers_map;
        std::map<int, Client>	_clients_map;

        fd_set					_recv_fds;
        fd_set					_write_fds;
        int						fdmax;
        std::string             ppath;
        
};