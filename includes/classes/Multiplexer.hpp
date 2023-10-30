#pragma once

#include "../main.hpp"

class Server;
class Client;
class Cgi;

class Multiplexer
{
public:
	Multiplexer();
	~Multiplexer();

	void createServers(std::vector<Server> servers);
	void runServers();

	void init_fds();
	void addToSet(const int i, fd_set &set);
	void removeFromSet(const int i, fd_set &set);
	void acceptNewConnection(Server &serv);

	void readRequest(const int &i, Client &c);
	void closeConnection(const int i);
	void sendResponse(const int &i, Client &c);
	void buildTheResponse(Client &client);

	// CGI methods
	void handleReqBody(Client &client);
	void sendCgiBody(Client &client, Cgi &cgi);
	void readCgiResponse(Client &client, Cgi &cgi);

private:
	fd_set _recv_fds;
	fd_set _write_fds;
	std::vector<Server> _servers;
	std::map<int, Server> _servers_map;
	std::map<int, Client> _clients_map;
	std::string ppath;
	int fdmax;
};