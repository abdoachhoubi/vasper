#include "../../includes/main.hpp"

Server::Server() {
	setRoot("/");
	setListen("80");
	setHost("localhost");
	setIndex("index.html");
}

Server::~Server() {}

Server::Server(const Server &other) {
	*this = other;
}

Server &Server::operator=(const Server &other) {
	if (this != &other) {
		port = other.port;
		host = other.host;
		nameServer = other.nameServer;
		root = other.root;
		clientMaxBodySize = other.clientMaxBodySize;
		index = other.index;
		autoindex = other.autoindex;
		_errorPages = other._errorPages;
		locations = other.locations;
		serveraddress = other.serveraddress;
		listenFd = other.listenFd;
		_uploadPath = other._uploadPath;
	}
	return (*this);
}

void Server::setServerName(std::string server_name) {
	nameServer = server_name;
}

void Server::setHost(std::string parametr) {
	struct sockaddr_in addr;

	if (parametr == "localhost")
		parametr = "127.0.0.1";
	if (inet_pton(AF_INET, parametr.c_str(), &addr.sin_addr) == 0)
		throw std::runtime_error("Error: invalid host");
	host = inet_addr(parametr.c_str());
}

void Server::setRoot(std::string root) {
	this->root = root;
}

void Server::setFd(int fd) {
	listenFd = fd;
}

void Server::setListen(std::string parametr) {
	port = atoi(parametr.c_str());
}

void Server::setClientMaxBodySize(std::string parametr) {
	clientMaxBodySize = atoi(parametr.c_str());
}

void	Server::setUploadPath(std::string uploadPath) {
	this->_uploadPath = uploadPath;
};

std::string	&Server::getUploadPath() {
	return _uploadPath;
};

void Server::setErrorPages(std::vector<std::string> &parametr) {
	if (parametr.size() % 2 != 0)
		throw std::runtime_error("Error: invalid error_page 1");
	for (size_t i = 0; i < parametr.size(); i++) {
		for (size_t j = 0; j < parametr[i].size(); j++) {
			if (!std::isdigit(parametr[i][j]))
				throw std::runtime_error("Error: invalid error_page 2");
		}
		if (atoi(parametr[i].c_str()) < 100 || atoi(parametr[i].c_str()) > 599)
			throw std::runtime_error("Error: invalid error_page 3");
		error_pages key = static_cast<error_pages>(atoi(parametr[i].c_str()));
		++i;
		std::string value = parametr[i];
		if (Server::getTypePath(value) != 2)
			throw std::runtime_error("Error: invalid error_page 4");
		if (Server::accessFile(value, F_OK) < 0
			|| Server::accessFile(value, R_OK) < 0)
			throw std::runtime_error("Error: invalid error_page");
		_errorPages[key] = value;

	}
}

void Server::setIndex(std::string index) {
	this->index = index;
}

void Server::setLocation(std::string nameLocation, std::vector<std::string> parametr) {
	Location location;
	for(size_t i = 0; i < parametr.size(); i++) {
		std::string str = parametr[i];
		Server::checkToken(str);
		std::vector<std::string> param = ft_split(str);
		if (param.size() > 1) {
			std::string key = param[0];
			std::vector<std::string> value;
			for (size_t i = 1; i < param.size(); i++)
				value.push_back(param[i]);
			if (key == "root")
				location.setRootLocation(value[0]);
			else if (key == "allow_methods")
				location.setMethods(value);
			else if (key == "autoindex")
				location.setAutoindex(value[0]);
			else if (key == "index")
				location.setIndexLocation(value[0]);
			else if (key == "return")
				location.setReturn(value[0]);
			else if (key == "alias") {
				location.setAlias(value[0]);
			}
			else if (key == "cgi_path")
				location.setCgiPath(value);
			else if (key == "cgi_extension")
				location.setCgiExtension(value);
			else if (key == "max_body_size")
				location.setMaxBodySize(value[0]);

			// if (key == "root")
			// 	location.setRootLocation(value);
			// else if (key == "methods")
			// 	location.setMethods(value);
			// else if (key == "autoindex")
			// 	location.setAutoindex(value);
			// else if (key == "index")
			// 	location.setIndexLocation(value);
			// else if (key == "return")
			// 	location.setReturn(value);
			// else if (key == "alias") {
			// 	std::cout << "alias: " << value << std::endl;
			// 	location.setAlias(value);
			// }
			// else if (key == "cgi_path")
			// 	location.setCgiPath(value);
			// else if (key == "cgi_extension")
			// 	location.setCgiExtension(value);
			// else if (key == "max_body_size")
			// 	location.setMaxBodySize(value);
		}
	}
	location.setPath(nameLocation);
	locations.push_back(location);
}

void Server::setAutoindex(std::string autoindex) {
	if (autoindex == "on")
		this->autoindex = true;
	else if (autoindex == "off")
		this->autoindex = false;
	else
		throw std::runtime_error("Error: invalid autoindex");
}

bool Server::isValidErrorPages() {
	std::map<error_pages, std::string>::iterator it = _errorPages.begin();
	std::map<error_pages, std::string>::iterator ite = _errorPages.end();
	for (; it != ite; ++it) {
		if (Server::accessFile(it->second, F_OK) < 0
			|| Server::accessFile(it->second, R_OK) < 0)
			return false;
	}
	return true;
}

int Server::isValidLocation(Location &location) const {
	if (location.getPath().empty())
		return 0;
	if (location.getRootLocation().empty())
		return 0;
	if (location.getMethods().empty())
		return 0;
	if (location.getIndexLocation().empty())
		return 0;
	if (location.getAlias().empty())
		return 0;
	if (location.getCgiPath().empty())
		return 0;
	if (location.getCgiExtension().empty())
		return 0;
	if (location.getMaxBodySize() == 0)
		return 0;
	return 1;
}

// getters
const std::string &Server::getServerName() {
	return nameServer;
}

const uint16_t &Server::getPort() {
	return port;
}

const in_addr_t &Server::getHost() {
	return host;
}

const size_t &Server::getClientMaxBodySize() {
	return clientMaxBodySize;
}

const std::vector<Location> &Server::getLocations() {
	return locations;
}

const std::string &Server::getRoot() {
	return root;
}

const std::map<error_pages, std::string> &Server::getErrorPages() {
	return _errorPages;
}

const std::string &Server::getIndex() {
	return index;
}

const bool &Server::getAutoindex() {
	return autoindex;
}

const std::string &Server::getPathErrorPage(error_pages key) {
	return _errorPages[key];
}

const std::vector<Location>::iterator Server::getLocationKey(std::string key) {
	std::vector<Location>::iterator it = locations.begin();
	std::vector<Location>::iterator ite = locations.end();
	for (; it != ite; ++it) {
		if (it->getPath() == key)
			return it;
	}
	return it;
}

void Server::checkToken(std::string &parametr) {
	std::string::size_type pos = parametr.find('#');
	if (pos != std::string::npos)
		parametr.erase(pos, parametr.find('\n', pos) - pos);
	pos = parametr.find(";");
	if (pos != std::string::npos)
		parametr.erase(pos);
}

bool Server::checkLocaitons() const {
	std::vector<Location>::const_iterator it = locations.begin();
	std::vector<Location>::const_iterator ite = locations.end();
	for (; it != ite; ++it) {
		if (isValidLocation(const_cast<Location &>(*it)) == 0)
			return false;
	}
	return true;
}

// void Server::setupServer() {
// 	serveraddress.sin_family = AF_INET;
// 	serveraddress.sin_addr.s_addr = host;
// 	serveraddress.sin_port = htons(port);
// }

int Server::getFd() {
	return listenFd;
}

// define is path is directory or file
int Server::getTypePath(std::string const path) {
	struct stat buf;

	if (stat(path.c_str(), &buf) == -1)
		return -1;
	if (S_ISDIR(buf.st_mode))
		return 1;
	if (S_ISREG(buf.st_mode))
		return 2;
	return 0;
}

// Check file exist and readable
int Server::accessFile(std::string const path, int mode) {
	if (access(path.c_str(), mode) == -1)
		return -1;
	return 0;
}

// Read file to string
std::string Server::fileToString(std::string path) {
	std::string		file;
	std::string		line;
	std::ifstream	in(path.c_str());

	if (!in.is_open())
		throw std::runtime_error("Error: file " + path + " not found");
	while (getline(in, line))
		file += line + "\n";
	in.close();
	return file;
}

int Server::isReadableAndExist(std::string const path, std::string const index) {
	int		type;
	std::string path_p = path + "/" + index;

	type = getTypePath(path_p);
	if (type == -1)
		return -1;
	if (type == 1)
		if ((accessFile(path_p, R_OK | X_OK)) == -1)
			return -1;
	if (type == 2)
		if ((accessFile(path_p, R_OK)) == -1)
			return -1;
	if (type == 0)
		if ((accessFile(path_p, R_OK)) == -1)
			return -1;
	return 0;
}

void	Server::setupServer(void)
{
	if ((listenFd = socket(AF_INET, SOCK_STREAM, 0) )  == -1 )
    {
		std::cerr << "webserv: socket error " << strerror(errno)<< " Closing ...." << std::endl;
        exit(EXIT_FAILURE);
    }

    int option_value = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int)) < 0)
	{
		std::cerr << "Error: cannot reuse socket" << std::endl;
		exit(1);
	}
    memset(&serveraddress, 0, sizeof(serveraddress));
    serveraddress.sin_family = AF_INET;
    serveraddress.sin_addr.s_addr = host;
    serveraddress.sin_port = htons(port);
    if (bind(listenFd, (struct sockaddr *) &serveraddress, sizeof(serveraddress)) == -1)
    {
		std::cerr << "webserv: bind error " << strerror(errno) << " Closing ...." << std::endl;
        exit(EXIT_FAILURE);
    }
}
