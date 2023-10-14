#include "../../includes/main.hpp"

Multiplexer::Multiplexer() {}

Multiplexer::~Multiplexer() {}

void Multiplexer::setupServers(std::vector<Server> servers)
{
    std::cout << "Creating Servers" << std::endl;
    _servers = servers;
    char buf[INET_ADDRSTRLEN];
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        // std::cout << BLUE_BOLD << "CREATING SERVER ==> " << i++ << RESET << std::endl;
        it->setupServer();
        std::cout << "Server Created: ServerName " << inet_ntop(AF_INET, &it->getHost(), buf, INET_ADDRSTRLEN) << "Host " << it->getServerName().c_str() << "Port " << it->getPort() << std::endl;
    }
}

void Multiplexer::init_fds()
{
    FD_ZERO(&_recv_fds);
    FD_ZERO(&_write_fds);

    for (size_t i = 0; i < _servers.size();i++)
    {
        if (listen(_servers[i].getFd(), SOMAXCONN) == -1)
        {
            std::cerr << "webserv: listen error: " << strerror(errno) << std::endl;
            close(_servers[i].getFd());
            continue;
        }
        if (fcntl(_servers[i].getFd(), F_SETFL, O_NONBLOCK) < 0)
        {
            std::cerr << "webserv: fcntl error: " << strerror(errno) << std::endl;
            close(_servers[i].getFd());
            continue; 
        }
        addToSet(_servers[i].getFd(), _recv_fds);
        _servers_map[_servers[i].getFd()] = _servers[i];
    }
}

void Multiplexer::addToSet(const int i, fd_set &set)
{
    FD_SET(i, &set);
    fdmax = (fdmax < i) ? i : fdmax;
}

void	Multiplexer::removeFromSet(const int i, fd_set &set)
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
        // std::cout << ORANGE_BOLD << "ana fl while lwlaa " << RESET << std::endl;
        int i = 2;
        while (i <= fdmax)
        {
            std::cout << GREEN_BOLD << "the i is : " << i << RESET << std::endl;
            if (FD_ISSET(i, &_write_temp))
                sendResponse(i, _clients_map[i]);
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

void    Multiplexer::closeConnection(const int i)
{
    if (FD_ISSET(i, &_write_fds))
        removeFromSet(i, _write_fds);
    if (FD_ISSET(i, &_recv_fds))
        removeFromSet(i, _recv_fds);
    close(i);
    _clients_map.erase(i);
}

void    Multiplexer::readRequest(const int &i, Client &client)
{
    char    buffer[BUFFER_SIZE];
    int     bytes_read = 0;
    bytes_read = read(i, buffer, BUFFER_SIZE);
    // std::cout << GREEN_BOLD <<"the read of the request  "  << RESET << std::endl;
    if (bytes_read < 1)
    {
        std::cerr << "webserv: fd " << i << " read error " << strerror(errno) << std::endl;
        closeConnection(i);
        return ;
    }
    else
    {
        client.request.parse(buffer, bytes_read);
        memset(buffer, 0, sizeof(buffer));
        if (client.request.parsingCompleted() || client.request.errorCode()) // 1 = parsing completed and we can work on the response.
        {
            client.buildResponse();
            removeFromSet(i, _recv_fds);
            addToSet(i, _write_fds);
        }
    }

  
}

void    Multiplexer::sendResponse(const int &i, Client &client)
{
    std::string response;
    std::cout << WHITE_BOLD << "ana dkhalt l send response " << RESET << std::endl;
    if (client._rem ==  false)
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
            std::cout << RED_BOLD << "settng the body " << RESET << std::endl;
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
            {
                std::cerr << "ERROR HERE : IN READ BYTES " << std::endl;
            }
            client.response._response = std::string(buffer, client.bytesRead);
        }
    }
    ssize_t result = write(i, client.response._response.c_str(), client.response._response.size());
    std::cout << RED_BOLD << i << " send :  " << client.bytes_sent << "  ||  " << result << RESET  << std::endl;
    if (client.flag == true)
        client.bytes_sent += result;

    if (result == -1)
    {
        std::cout << BLUE_BOLD << "Probleme in send function " << strerror(errno) << std::endl;
        closeConnection(i);
    }
    else if (result != (ssize_t)client.response._response.size())
    {
        // std::cout << ORANGE_BOLD << "baqa chi haja f remaining " << RESET << std::endl;
        client._rem = true;
        client.response._response = client.response._response.substr(result,client.response._response.size()); 
    }
    else if (client.bytes_sent >= (ssize_t)client.content_len ||  client.response._response.empty())
    {
        std::cout << "the content length is : " << client.content_len << std::endl;
        std::cout << "the bytes send  is : " << client.bytes_sent << std::endl;
        std::cout << "the response is sent " << std::endl;
        closeConnection(i);
    }
    else
    {
        client._rem = false;
        std::cout << "sendat gha chwiyaa  " << std::endl;
    }
}

void    Multiplexer::assignServer(Client &client)
{
    for (std::vector<Server>::iterator it = _servers.begin();
        it != _servers.end(); ++it)
    {
        if (client.server.getHost() == it->getHost() &&
            client.server.getPort() == it->getPort() &&
            client.request.getServerName() == it->getServerName())
        {
            client.setServer(*it);
			client.response.setServer(*it);
            return ;
        }
    }
}

void    Multiplexer::acceptNewConnection(Server &serv)
{
    struct sockaddr_in client_address;
    long  client_address_size = sizeof(client_address);
    int client_sock;
    Client  new_client(serv);
    // std::cout << CYAN_BOLD << "ana f accept function  " << RESET << std::endl; 
    char    buf[INET_ADDRSTRLEN];

    if ( (client_sock = accept(serv.getFd(), (struct sockaddr *)&client_address,
     (socklen_t*)&client_address_size)) == -1)
    {
        std::cerr << "webserv: accept error " << strerror(errno) << std::endl;
        return ;
    }
    std::cout << YELLOW_BOLD << "New Connection From " << inet_ntop(AF_INET, &client_address, buf, INET_ADDRSTRLEN) << "Assigned Socket " << client_sock << RESET << std::endl;

    addToSet(client_sock, _recv_fds);

    if (fcntl(client_sock, F_SETFL, O_NONBLOCK) < 0)
    {
        std::cerr << "webserv: fcntl error " << strerror(errno) << std::endl;
        removeFromSet(client_sock, _recv_fds);
        close(client_sock);
        return ;
    }
    new_client.setSocket(client_sock);
    _clients_map[client_sock] = new_client;
}