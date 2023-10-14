#include "../../includes/main.hpp"

ConfParser::ConfParser() : nbr_serv(0) {}

ConfParser::~ConfParser() {}

void ConfParser::parse(const std::string &config_file)
{
	std::string content = Server::fileToString(config_file);
	splitServers(content);
	if (serv_conf.size() != nbr_serv)
		throw std::runtime_error("Error: Server config file is not well formated 1");

	for (size_t i = 0; i < nbr_serv; i++)
	{
		Server server;
		createServer(serv_conf[i], server);
		servers.push_back(server);
	}

	for (size_t i = 0; i < servers.size(); i++)
	{
		for (size_t j = i + 1; j < servers.size(); j++)
		{
			if (servers[i].getPort() == servers[j].getPort() && servers[i].getHost() == servers[j].getHost())
				throw std::runtime_error("Error: Server config file is not well formated 11");
		}
	}
}

void ConfParser::splitServers(std::string &content)
{
	size_t start = 0;
	size_t end = 1;

	while (start != end && start < content.length())
	{
		start = startServer(start, content);
		end = endServer(start, content);
		serv_conf.push_back(content.substr(start, end - start + 1));
		nbr_serv++;
		start = end;
		start++;
	}
}

size_t ConfParser::startServer(size_t start, std::string &content)
{
	size_t pos = start;

	for (; pos < content.size() && isspace(content[pos]); pos++)
		if (content[pos] == 's')
			break;
	if (!content[pos])
		return start;
	if (content.compare(pos, 6, "server") != 0)
		throw std::runtime_error("Error: Wrong character 1");
	pos += 6;
	while (pos < content.size() && isspace(content[pos]))
		pos++;
	if (content[pos] != '{')
		throw std::runtime_error("Error: Wrong character");
	return (pos);
}

size_t ConfParser::endServer(size_t start, std::string &content)
{
	size_t pos = start;
	int count = 0;

	for (pos = start + 1; pos < content.size(); pos++)
	{
		if (content[pos] == '{')
			count++;
		if (content[pos] == '}')
		{
			if (count == 0)
				return (pos + 1);
			count--;
		}
	}
	return (start);
}

void ConfParser::createServer(std::string &config, Server &server)
{
	std::vector<std::string> line = splitParametrs(config, "\n");
	std::vector<std::string> error_page;

	for (size_t i = 0; i < line.size(); i++)
	{
		std::string str = line[i];
		Server::checkToken(str);
		std::vector<std::string> param = ft_split(str);
		if (param.size() > 1)
		{
			std::string key = param[0];
			std::vector<std::string> value;
			for (size_t i = 1; i < param.size(); i++)
				value.push_back(param[i]);
			if (key == "listen")
			{
				if (value.size() != 1)
					throw std::runtime_error("Error: Server config file is not well formated 4");
				server.setListen(value[0]);
			}
			else if (key == "server_name")
			{
				if (value.size() != 1)
					throw std::runtime_error("Error: Server config file is not well formated 5");
				server.setServerName(value[0]);
			}
			else if (key == "error_page")
			{
				server.setErrorPages(value);
			}
			else if (key == "location")
			{
				if (value.size() < 2)
					throw std::runtime_error("Error: Server config file is not well formated 6");
				std::string name = value[0];
				if (value[1] != "{")
					throw std::runtime_error("Error: Server config file is not well formated 7");
				std::vector<std::string> location;
				while (i < line.size() && line[i] != "}")
				{
					Server::checkToken(line[i]);
					location.push_back(line[i++]);
				}
				if (line[i] != "}")
					throw std::runtime_error("Error: Server config file is not well formated 8");
				server.setLocation(name, location);
			}
			else if (key == "host")
			{
				server.setHost(value[0]);
			}
			else if (key == "client_max_body_size")
			{
				server.setClientMaxBodySize(value[0]);
			}
			else if (key == "root")
			{
				server.setRoot(value[0]);
			}
			else if (key == "index")
			{
				server.setIndex(value[0]);
			}
			else if (key == "autoindex")
			{
				server.setAutoindex(value[0]);
			}
			else if (key == "upload_path")
			{
				server.setUploadPath(value[0]);
			}
		}
		else
		{
			if (param.empty() || param[0] == "}" || param[0] == "{")
				continue;
			else
				throw std::runtime_error("Error: Server config file is not well formated 9");
		}
	}
	// if (Server::isReadableAndExist(server.getRoot(), server.getIndex()) < 0)
	// 	throw std::runtime_error("Error: Index from config file not found 1");
	if (Server::isReadableAndExist(server.getRoot(), "") < 0)
		throw std::runtime_error("Error: Root from config file not found 2");
	// if (!server.checkLocaitons())
	// 	throw std::runtime_error("Error: in Location from config file");
	if (!server.isValidErrorPages())
		throw std::runtime_error("Error: in ErrorPages from config file");
}

std::vector<Server> ConfParser::getServers()
{
	return servers;
}

int ConfParser::print()
{
	std::cout << "nb server: " << nbr_serv << std::endl;

	for (size_t i = 0; i < serv_conf.size(); i++)
	{
		std::cout << "getRoot: " << servers[i].getRoot() << std::endl;
		std::cout << "getPort: " << servers[i].getPort() << std::endl;
		std::cout << "getHost: " << servers[i].getHost() << std::endl;
		std::cout << "getServerName: " << servers[i].getServerName() << std::endl;
		std::cout << "getClientMaxBodySize: " << servers[i].getClientMaxBodySize() << std::endl;
		std::cout << "getIndex: " << servers[i].getIndex() << std::endl;
		std::cout << "getAutoindex: " << servers[i].getAutoindex() << std::endl;
		std::map<error_pages, std::string> errors = servers[i].getErrorPages();
		std::map<error_pages, std::string>::iterator it = errors.begin();
		std::map<error_pages, std::string>::iterator ite = errors.end();
		for (; it != ite; ++it)
		{
			std::cout << "error code: " << it->first << "\nerror page: " << it->second << std::endl;
		}
		std::vector<Location> locations = servers[i].getLocations();
		std::vector<Location>::iterator itl = locations.begin();
		std::vector<Location>::iterator itle = locations.end();
		for (; itl != itle; itl++)
		{
			std::cout << "location: " << itl->getPath() << std::endl;
			std::cout << "location root: " << itl->getRootLocation() << std::endl;
			std::cout << "location autoindex: " << itl->getAutoindex() << std::endl;
			std::cout << "location index: " << itl->getIndexLocation() << std::endl;
			std::cout << "location return: " << itl->getReturn() << std::endl;
			std::cout << "location alias: " << itl->getAlias() << std::endl;
			std::vector<std::string> getCgi = itl->getCgiPath();
			std::vector<std::string>::iterator itc = getCgi.begin();
			std::vector<std::string>::iterator itce = getCgi.end();
			for (; itc != itce; itc++)
			{
				std::cout << "location cgi path: " << *itc << std::endl;
			}
			std::vector<std::string> getCgiExt = itl->getCgiExtension();
			std::vector<std::string>::iterator itce2 = getCgiExt.begin();
			std::vector<std::string>::iterator itce2e = getCgiExt.end();
			for (; itce2 != itce2e; itce2++)
			{
				std::cout << "location cgi extension: " << *itce2 << std::endl;
			}
			// std::cout << "location cgi path: " << itl->getCgiPath()[0]<< std::endl;
			// std::cout << "location cgi extension: " << itl->getCgiExtension()[0]<< std::endl;
			std::cout << "location max body size: " << itl->getMaxBodySize() << std::endl;
			std::vector<bool> getMethods = itl->getMethods();
			std::vector<bool>::iterator itm = getMethods.begin();
			std::vector<bool>::iterator itme = getMethods.end();
			for (; itm != itme; itm++)
			{
				std::cout << "location methods: " << *itm << std::endl;
			}
			std::cout << "location methods: " << itl->getPrintMethods() << std::endl;
		}
	}
	return 0;
}
