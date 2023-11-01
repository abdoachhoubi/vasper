#include "../../includes/main.hpp"

Location::~Location() {}

Location::Location(const Location &other) {*this = other;}

std::vector<std::string> Location::getAllowedMethods() const {return (allowed_methods);}

void Location::setAllowedMethods(std::vector<std::string> methods) {allowed_methods = methods;}

void Location::setRootLocation(std::string parametr) { root = parametr; }

void Location::setPath(std::string parametr) { path = parametr; }

void Location::setIndexLocation(std::string parametr) { index = parametr; }

void Location::setReturn(std::string parametr) { _return = parametr; }

void Location::setAlias(std::string parametr) { _alias = parametr; }

void Location::setCgiExtension(std::vector<std::string> extension) { cgi_extension = extension; }

const std::string &Location::getPath() const { return (path); }

const std::string &Location::getRootLocation() const { return (root); }

const std::string &Location::getIndexLocation() const { return (index); }

const std::string &Location::getReturn() const { return (_return); }

const std::string &Location::getAlias() const { return (_alias); }

const bool &Location::getAutoindex() const { return (autoindex); }

const std::vector<bool> &Location::getMethods() const { return (_methods); }

const std::vector<std::string> &Location::getCgiPath() const { return (cgi_path); }

const std::vector<std::string> &Location::getCgiExtension() const { return (cgi_extension); }

const std::map<std::string, std::string> &Location::getExtensionPath() const {return (this->_ext_path);}

const unsigned long &Location::getMaxBodySize() const { return (clientMaxBodySize); }

bool Location::getCGI() const { return (this->cgi); }

Location::Location()
{
	path = "";
	root = "";
	autoindex = false;
	index = "";
	_methods = std::vector<bool>(3, false);
	_return = "";
	_alias = "";
	cgi_path = std::vector<std::string>();
	cgi_extension = std::vector<std::string>();
	clientMaxBodySize = MAX_CONTENT_LENGTH;
	_ext_path = std::map<std::string, std::string>();
}

Location &Location::operator=(const Location &other)
{
	if (this != &other)
	{
		path = other.path;
		root = other.root;
		autoindex = other.autoindex;
		index = other.index;
		_methods = other._methods;
		allowed_methods = other.allowed_methods;
		_return = other._return;
		_alias = other._alias;
		cgi_path = other.cgi_path;
		cgi_extension = other.cgi_extension;
		clientMaxBodySize = other.clientMaxBodySize;
		_ext_path = other._ext_path;
		cgi = other.cgi;
	}
	return (*this);
}

void Location::setMethods(std::vector<std::string> methods)
{
	allowed_methods = methods;
	_methods = std::vector<bool>(3, false);
	for (size_t i = 0; i < methods.size(); i++)
	{
		if (methods[i] == "GET")
			_methods[0] = true;
		else if (methods[i] == "POST")
			_methods[1] = true;
		else if (methods[i] == "DELETE")
			_methods[2] = true;
		else
			throw std::runtime_error("Error: invalid method");
	}
}

void Location::setAutoindex(std::string parametr)
{
	if (parametr == "on")
		autoindex = true;
	else if (parametr == "off")
		autoindex = false;
	else
		throw std::runtime_error("Error: invalid autoindex");
}

void Location::setCgiPath(std::vector<std::string> path)
{
	for (size_t i = 0; i < path.size(); i++)
	{
		// check if path exists
		struct stat buffer;
		if (stat(path[i].c_str(), &buffer))
			throw std::runtime_error("Error: invalid cgi_path");
		i++;
	}
	cgi_path = path;
}

void Location::setMaxBodySize(std::string parametr)
{
	std::istringstream iss(parametr);
	iss >> clientMaxBodySize;
}

std::string Location::getPrintMethods() const
{
	std::string result = "";
	if (_methods[0])
		result += "GET ";
	if (_methods[1])
		result += "POST ";
	if (_methods[2])
		result += "DELETE ";
	return (result);
}

void Location::setCGI(std::string parametr)
{
	if (parametr == "on")
		cgi = true;
	else if (parametr == "off")
		cgi = false;
	else
		throw std::runtime_error("Error: invalid cgi");
}
