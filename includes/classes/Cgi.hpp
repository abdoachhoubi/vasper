#pragma once

#include "../main.hpp"

class Request;
class Cgi {
	private:
		std::map<std::string, std::string>	_env;
		char**								_ch_env;
		char**								_argv;
		int									_exit_status;
		std::string							_cgi_path;
		pid_t								_cgi_pid;

	public:
		int	pipe_in[2];
		int	pipe_out[2];

		Cgi();
		Cgi(std::string path);
		~Cgi();
		Cgi(Cgi const &other);
		Cgi &operator=(Cgi const &rhs);

		void initEnv(Request& req);
		void initEnvCgi(Request& req);
		void execute(short &error_code);
		void clear();

		void setCgiPid(pid_t cgi_pid);
		void setCgiPath(const std::string &cgi_path);

		const std::map<std::string, std::string> &getEnv() const;
		const pid_t &getCgiPid() const;
		const std::string &getCgiPath() const;
		std::string getCgiResponse();

		std::string	getPathInfo(std::string& path, std::vector<std::string> extensions);
		int findStart(const std::string path, const std::string delim);
		std::string decode(std::string &path);
};
