#include "../../includes/main.hpp"

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

Location::Location(const Location &other)
{
	*this = other;
}

Location &Location::operator=(const Location &rhs)
{
	if (this != &rhs)
	{
		path = rhs.path;
		root = rhs.root;
		autoindex = rhs.autoindex;
		index = rhs.index;
		_methods = rhs._methods;
		allowed_methods = rhs.allowed_methods;
		_return = rhs._return;
		_alias = rhs._alias;
		cgi_path = rhs.cgi_path;
		cgi_extension = rhs.cgi_extension;
		clientMaxBodySize = rhs.clientMaxBodySize;
		_ext_path = rhs._ext_path;
	}
	return (*this);
}

Location::~Location() {}

// setters
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

void Location::setAllowedMethods(std::vector<std::string> methods)
{
	allowed_methods = methods;
}

std::vector<std::string> Location::getAllowedMethods() const
{
	return (allowed_methods);
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

void Location::setRootLocation(std::string parametr) { root = parametr; }

void Location::setPath(std::string parametr) { path = parametr; }

void Location::setIndexLocation(std::string parametr) { index = parametr; }

void Location::setReturn(std::string parametr) { _return = parametr; }

void Location::setAlias(std::string parametr) { _alias = parametr; }

void Location::setCgiPath(std::vector<std::string> path) { cgi_path = path; }

void Location::setCgiExtension(std::vector<std::string> extension) { cgi_extension = extension; }

void Location::setMaxBodySize(std::string parametr)
{
	std::istringstream iss(parametr);
	iss >> clientMaxBodySize;
}

// void Location::setMaxBodySize(unsigned long parametr) { clientMaxBodySize = parametr; }

// getters
const std::string &Location::getPath() const { return (path); }

const std::string &Location::getRootLocation() const { return (root); }

const std::vector<bool> &Location::getMethods() const { return (_methods); }

const bool &Location::getAutoindex() const { return (autoindex); }

const std::string &Location::getIndexLocation() const { return (index); }

const std::string &Location::getReturn() const { return (_return); }

const std::string &Location::getAlias() const { return (_alias); }

const std::vector<std::string> &Location::getCgiPath() const { return (cgi_path); }

const std::vector<std::string> &Location::getCgiExtension() const { return (cgi_extension); }

const std::map<std::string, std::string> &Location::getExtensionPath() const
{
	return (this->_ext_path);
}

const unsigned long &Location::getMaxBodySize() const { return (clientMaxBodySize); }

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
