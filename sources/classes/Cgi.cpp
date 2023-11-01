#include "../../includes/main.hpp"

/* Constructor */
Cgi::Cgi()
{
	this->_cgi_pid = -1;
	this->_exit_status = 0;
	this->_cgi_path = "";
	this->_ch_env = NULL;
	this->_argv = NULL;
}

Cgi::Cgi(std::string path)
{
	this->_cgi_pid = -1;
	this->_exit_status = 0;
	this->_cgi_path = path;
	this->_ch_env = NULL;
	this->_argv = NULL;
}

Cgi::~Cgi()
{

	if (this->_ch_env)
	{
		for (int i = 0; this->_ch_env[i]; i++)
			free(this->_ch_env[i]);
		free(this->_ch_env);
	}
	if (this->_argv)
	{
		for (int i = 0; this->_argv[i]; i++)
			free(_argv[i]);
		free(_argv);
	}
	this->_env.clear();
}

Cgi::Cgi(const Cgi &other)
{
	this->_env = other._env;
	this->_ch_env = other._ch_env;
	this->_argv = other._argv;
	this->_cgi_path = other._cgi_path;
	this->_cgi_pid = other._cgi_pid;
	this->_exit_status = other._exit_status;
}

Cgi &Cgi::operator=(const Cgi &other)
{
	if (this != &other)
	{
		this->_env = other._env;
		this->_ch_env = other._ch_env;
		this->_argv = other._argv;
		this->_cgi_path = other._cgi_path;
		this->_cgi_pid = other._cgi_pid;
		this->_exit_status = other._exit_status;
	}
	return (*this);
}

/*Set functions */
void Cgi::setCgiPid(pid_t cgi_pid) {this->_cgi_pid = cgi_pid;}
void Cgi::setCgiPath(const std::string &cgi_path) {this->_cgi_path = cgi_path;}

/* Get functions */
const std::map<std::string, std::string> &Cgi::getEnv() const {return (this->_env);}
const pid_t &Cgi::getCgiPid() const {return (this->_cgi_pid);}
const std::string &Cgi::getCgiPath() const {return (this->_cgi_path);}

/* initialization environment variable */
void Cgi::initEnv(Request &req, Location &location)
{
	int poz;
	std::string extension;
	std::string ext_path;

	extension = this->_cgi_path.substr(this->_cgi_path.find("."));
	ext_path = location._ext_path[extension];

	this->_env["AUTH_TYPE"] = "Basic";
	this->_env["CONTENT_LENGTH"] = req.getHeader("content-length");
	this->_env["CONTENT_TYPE"] = req.getHeader("content-type");
	this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	poz = findStart(this->_cgi_path, "cgi-bin/");
	this->_env["SCRIPT_NAME"] = this->_cgi_path;
	this->_env["SCRIPT_FILENAME"] = ((poz < 0 || (size_t)(poz + 8) > this->_cgi_path.size()) ? "" : this->_cgi_path.substr(poz + 8, this->_cgi_path.size())); // check dif cases after put right parametr from the response
	this->_env["PATH_INFO"] = getPathInfo(req.getPath(), location.getCgiExtension());
	this->_env["PATH_TRANSLATED"] = location.getRootLocation() + (this->_env["PATH_INFO"] == "" ? "/" : this->_env["PATH_INFO"]);
	this->_env["QUERY_STRING"] = decode(req.getQuery());
	this->_env["REMOTE_ADDR"] = req.getHeader("host");
	poz = findStart(req.getHeader("host"), ":");
	this->_env["SERVER_NAME"] = (poz > 0 ? req.getHeader("host").substr(0, poz) : "");
	this->_env["SERVER_PORT"] = (poz > 0 ? req.getHeader("host").substr(poz + 1, req.getHeader("host").size()) : "");
	this->_env["REQUEST_METHOD"] = req.getMethodStr();
	this->_env["HTTP_COOKIE"] = req.getHeader("cookie");
	this->_env["DOCUMENT_ROOT"] = location.getRootLocation();
	this->_env["REQUEST_URI"] = req.getPath() + req.getQuery();
	this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	this->_env["REDIRECT_STATUS"] = "SUCCESS";
	this->_env["SERVER_SOFTWARE"] = "vasper";

	this->_ch_env = (char **)calloc(sizeof(char *), this->_env.size() + 1);
	std::map<std::string, std::string>::const_iterator it = this->_env.begin();
	for (int i = 0; it != this->_env.end(); it++, i++)
	{
		std::string tmp = it->first + "=" + it->second;
		this->_ch_env[i] = strdup(tmp.c_str());
	}
	this->_argv = (char **)malloc(sizeof(char *) * 3);
	this->_argv[0] = strdup(ext_path.c_str());
	this->_argv[1] = strdup(this->_cgi_path.c_str());
	this->_argv[2] = NULL;
}

/* Pipe and execute CGI */
void Cgi::execute(short &error_code)
{
	if (this->_argv[0] == NULL || this->_argv[1] == NULL)
	{
		error_code = INTERNAL_SERVER_ERROR;
		return;
	}
	filex = open("./vasper.cgi", O_CREAT | O_TRUNC | O_RDWR, 0777);
	if (filex < 0)
	{
		std::cerr << RED_BOLD << "pipe() failed" << RESET << std::endl;
		close(filex);
		error_code = INTERNAL_SERVER_ERROR;
		return;
	}
	this->_cgi_pid = fork();
	if (this->_cgi_pid == 0)
	{
		dup2(filex, STDOUT_FILENO);
		close(filex);
		this->_exit_status = execve(this->_argv[0], this->_argv, this->_ch_env);
		exit(this->_exit_status);
	}
	else if (this->_cgi_pid > 0)
		waitpid(this->_cgi_pid, &this->_exit_status, 0);
	else
	{
		std::cout << "Fork failed" << std::endl;
		error_code = INTERNAL_SERVER_ERROR;
	}
}

int Cgi::findStart(const std::string path, const std::string delim)
{
	if (path.empty())
		return (-1);
	size_t poz = path.find(delim);
	if (poz != std::string::npos)
		return (poz);
	else
		return (-1);
}

/* Translation of parameters for QUERY_STRING environment variable */
std::string Cgi::decode(std::string &path)
{
	size_t token = path.find("%");
	while (token != std::string::npos)
	{
		if (path.length() < token + 2)
			break;
		char decimal = fromHexToDec(path.substr(token + 1, 2));
		path.replace(token, 3, to_string(decimal));
		token = path.find("%");
	}
	return (path);
}

/* Isolation PATH_INFO environment variable */
std::string Cgi::getPathInfo(std::string &path, std::vector<std::string> extensions)
{
	std::string tmp;
	size_t start, end;

	for (std::vector<std::string>::iterator it_ext = extensions.begin(); it_ext != extensions.end(); it_ext++)
	{
		start = path.find(*it_ext);
		if (start != std::string::npos)
			break;
	}
	if (start == std::string::npos)
		return "";
	if (start + 3 >= path.size())
		return "";
	tmp = path.substr(start + 3, path.size());
	if (!tmp[0] || tmp[0] != '/')
		return "";
	end = tmp.find("?");
	return (end == std::string::npos ? tmp : tmp.substr(0, end));
}

void Cgi::clear()
{
	this->_cgi_pid = -1;
	this->_exit_status = 0;
	this->_cgi_path = "";
	this->_ch_env = NULL;
	this->_argv = NULL;
	this->_env.clear();
}

std::string Cgi::getResponse()
{
	std::string response;

	close(filex);
	std::ifstream file("./vasper.cgi");
	if (file.is_open())
	{
		std::string line;
		while (getline(file, line))
		{
			response += line;
		}
		file.close();
	}
	else if (!file.good())
		std::cout << "Unable to open CGI file" << std::endl;
	return (response);
}