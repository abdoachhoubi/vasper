#include "../../includes/main.hpp"

Multiplexer::Multiplexer()
{
	fdmax = 0;
	_recv_fds = fd_set();
	_write_fds = fd_set();
}

Multiplexer::~Multiplexer() {}

void Multiplexer::setupServers(std::vector<Server> servers)
{
	std::cout << RED_BOLD << "Creating Servers" << RESET << std::endl;
	_servers = servers;
	char buf[INET_ADDRSTRLEN];
	for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		it->setupServer();
		std::cout << BLUE_BOLD << "Server " << it->getServerName().c_str() << " Created and listening on: http://" << inet_ntop(AF_INET, &it->getHost(), buf, INET_ADDRSTRLEN) << ":" << it->getPort() << RESET << std::endl;
	}
}

void Multiplexer::init_fds()
{
	FD_ZERO(&_recv_fds);
	FD_ZERO(&_write_fds);

	for (size_t i = 0; i < _servers.size(); i++)
	{
		addToSet(_servers[i].getFd(), _recv_fds);
		_servers_map[_servers[i].getFd()] = _servers[i];
	}
}

void Multiplexer::addToSet(const int i, fd_set &set)
{
	FD_SET(i, &set);
	fdmax = (fdmax < i) ? i : fdmax;
}

void Multiplexer::removeFromSet(const int i, fd_set &set)
{
	FD_CLR(i, &set);
	if (i == fdmax)
		fdmax--;
}

void Multiplexer::runServers()
{
	fd_set _recv_temp;
	fd_set _write_temp;

	init_fds();

	while (true)
	{
		_recv_temp = _recv_fds;
		_write_temp = _write_fds;

		if (select(fdmax + 1, &_recv_temp, &_write_temp, NULL, NULL) < 0)
		{
			std::cerr << "webserv: select error " << strerror(errno) << std::endl;
			continue;
		}
		int i = 2;
		while (i <= fdmax)
		{
			if (FD_ISSET(i, &_write_temp))
			{
				// if (_clients_map[i].request.getMethod() == GET && _clients_map[i].response._check == false)
				sendResponse(i, _clients_map[i]);
				// else
				// 	sendAstro(i, _clients_map[i]);
			}
			else if (FD_ISSET(i, &_recv_temp))
			{
				if (_servers_map.count(i))
					acceptNewConnection(_servers_map.find(i)->second);
				else
					readRequest(i, _clients_map[i]);
			}
			++i;
		}
	}
}

void Multiplexer::closeConnection(const int i)
{
	if (FD_ISSET(i, &_write_fds))
		removeFromSet(i, _write_fds);
	if (FD_ISSET(i, &_recv_fds))
		removeFromSet(i, _recv_fds);
	close(i);
	_clients_map.erase(i);
}

void Multiplexer::readRequest(const int &i, Client &client)
{
	char buffer[BUFFER_SIZE];
	int bytes_read = 0;
	bytes_read = read(i, buffer, BUFFER_SIZE);
	// std::cout << GREEN_BOLD <<"the read of the request  "  << RESET << std::endl;
	if (bytes_read < 1)
	{
		std::cerr << "webserv: fd " << i << " read error " << strerror(errno) << std::endl;
		closeConnection(i);
		return;
	}
	else
	{
		client.request.parse(buffer, bytes_read);
		memset(buffer, 0, sizeof(buffer));
		if (client.request.parsingCompleted() || client.request.errorCode()) // 1 = parsing completed and we can work on the response.
		{
			// DEBUGGING STARTS
			std::cout << "PARSING COMPLETE" << std::endl;
			// DEBUGGING ENDS

			client.buildResponse();
			if (client.response.getCgiState())
			{
				// DEBUGGING STARTS
				std::cout << "CGI STATE: " << client.response.getCgiState() << std::endl;
				// DEBUGGING ENDS
				handleReqBody(client);
				addToSet(client.response._cgi_obj.pipe_in[1], _write_fds);
				addToSet(client.response._cgi_obj.pipe_out[0], _recv_fds);
			}
			removeFromSet(i, _recv_fds);
			addToSet(i, _write_fds);
		}
	}
}

void Multiplexer::handleReqBody(Client &client)
{
	if (client.request.getBody().length() == 0)
	{
		std::string tmp;
		std::fstream file;
		(client.response._cgi_obj.getCgiPath().c_str());
		std::stringstream ss;
		ss << file.rdbuf();
		tmp = ss.str();
		client.request.setBody(tmp);
	}
}

void Multiplexer::buildTheResponse(Client &client)
{
	if (client.isHeadSent == false)
	{
		client.flag = false;
		std::string headers = client.response.get_headers();
		client.response._response = headers;
		client.content_len = client.response.fileSize;
		client.isHeadSent = true;
	}
	else
	{
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));
		if (client.isFileOpened == false)
		{
			client.file.close();
			client.file.open(client.response.getPath().c_str(), std::ios::in | std::ios::binary);
			client.isFileOpened = true;
		}
		client.file.read(buffer, BUFFER_SIZE - 1);
		client.bytesRead = client.file.gcount();
		if (client.bytesRead < 0)
			std::cerr << "ERROR HERE : IN READ BYTES " << std::endl;
		client.response._response = std::string(buffer, client.bytesRead);
	}
}

void Multiplexer::sendResponse(const int &i, Client &client)
{
	std::string response;
	if (client._rem == false)
		buildTheResponse(client);
	ssize_t result = write(i, client.response._response.c_str(), client.response._response.size());
	if (client.flag == true)
		client.bytes_sent += result;

	if (result == -1)
		closeConnection(i);
	else if (result != (ssize_t)client.response._response.size())
	{
		client._rem = true;
		client.response._response = client.response._response.substr(result, client.response._response.size());
	}
	else if (client.bytes_sent >= (ssize_t)client.content_len || client.response._response.empty())
		closeConnection(i);
	else
		client._rem = false; // send just a buffer size not all the response
}

void Multiplexer::sendAstro(const int &i, Client &client)
{
	int bytes_sent;
	std::string response = client.response.get_response();

	if (response.length() >= BUFFER_SIZE)
		bytes_sent = write(i, response.c_str(), BUFFER_SIZE);
	else
		bytes_sent = write(i, response.c_str(), response.length());

	if (bytes_sent < 0)
		closeConnection(i);
	else if (bytes_sent == 0 || (size_t)bytes_sent == response.length())
	{
		if (client.request.keepAlive() == false || client.request.errorCode())
			closeConnection(i);
		else
		{
			removeFromSet(i, _write_fds);
			addToSet(i, _recv_fds);
		}
	}
	else
		client.response.cut_response(bytes_sent);
}

void Multiplexer::acceptNewConnection(Server &serv)
{
	struct sockaddr_in client_address;
	long client_address_size = sizeof(client_address);
	int client_sock;
	Client new_client(serv);
	char buf[INET_ADDRSTRLEN];

	if ((client_sock = accept(serv.getFd(), (struct sockaddr *)&client_address,
							  (socklen_t *)&client_address_size)) == -1)
	{
		std::cerr << "webserv: accept error " << strerror(errno) << std::endl;
		return;
	}
	std::cout << YELLOW_BOLD << "New Connection From " << inet_ntop(AF_INET, &client_address, buf, INET_ADDRSTRLEN) << "Assigned Socket " << client_sock << RESET << std::endl;
	addToSet(client_sock, _recv_fds);
	if (fcntl(client_sock, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "webserv: fcntl error " << strerror(errno) << std::endl;
		removeFromSet(client_sock, _recv_fds);
		close(client_sock);
		return;
	}
	new_client.setSocket(client_sock);
	_clients_map[client_sock] = new_client;
}

// _recv_fds

// _write_fds

void Multiplexer::sendCgiBody(Client &client, Cgi &cgi)
{
	int bytes_sent;
	std::string &req_body = client.request.getBody();

	if (req_body.length() == 0)
		bytes_sent = 0;
	else if (req_body.length() >= BUFFER_SIZE)
		bytes_sent = write(cgi.pipe_in[1], req_body.c_str(), BUFFER_SIZE);
	else
		bytes_sent = write(cgi.pipe_in[1], req_body.c_str(), req_body.length());

	if (bytes_sent < 0)
	{
		std::cerr << RED_BOLD << "sendCgiBody() Error Sending: " << strerror(errno) << RESET << std::endl;
		removeFromSet(cgi.pipe_in[1], _write_fds);
		close(cgi.pipe_in[1]);
		close(cgi.pipe_out[1]);
		client.response.generateErrorResponse(500);
	}
	else if (bytes_sent == 0 || (size_t)bytes_sent == req_body.length())
	{
		removeFromSet(cgi.pipe_in[1], _write_fds);
		close(cgi.pipe_in[1]);
		close(cgi.pipe_out[1]);
	}
	else
	{
		// client.updateTime();
		req_body = req_body.substr(bytes_sent);
	}
}

/* Reads outpud produced by the CGI script */
void Multiplexer::readCgiResponse(Client &client, Cgi &cgi)
{
	char buffer[BUFFER_SIZE * 2];
	int bytes_read = 0;
	bytes_read = read(cgi.pipe_out[0], buffer, BUFFER_SIZE * 2);

	if (bytes_read == 0)
	{
		removeFromSet(cgi.pipe_out[0], _recv_fds);
		close(cgi.pipe_in[0]);
		close(cgi.pipe_out[0]);
		int status;
		waitpid(cgi.getCgiPid(), &status, 0);
		if (WEXITSTATUS(status) != 0)
		{
			client.response.generateErrorResponse(502);
		}
		client.response.setCgiState(2);
		if (client.response._response.find("HTTP/1.1") == std::string::npos)
			client.response._response.insert(0, "HTTP/1.1 200 OK\r\n");
		return;
	}
	else if (bytes_read < 0)
	{
		std::cerr << RED_BOLD << "readCgiResponse() Error Reading From CGI Script: " << strerror(errno) << RESET << std::endl;
		removeFromSet(cgi.pipe_out[0], _recv_fds);
		close(cgi.pipe_in[0]);
		close(cgi.pipe_out[0]);
		client.response.setCgiState(2);
		client.response.generateErrorResponse(500);
		return;
	}
	else
	{
		// client.updateTime();
		client.response._response.append(buffer, bytes_read);
		memset(buffer, 0, sizeof(buffer));
	}
}