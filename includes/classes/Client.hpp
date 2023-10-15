#pragma once

#include "../main.hpp"

class Server;
class Request;
class Response;

class Client
{
    public:
        Client();
        Client(const Client &other);
        Client(Server &server);
		Client &operator=(const Client & rhs);
        ~Client();

        const int					&getSocket() const;
        const struct sockaddr_in	&getAddress() const;
        const Request				&getRequest() const;
        const time_t				&getLastTime() const;

        void                setSocket(int &);
        void                setAddress(sockaddr_in &);
        void                setServer(Server &);
        void                buildResponse();
        void                updateTime();
        void                clearClient();

        Response            response;
        Request         	request;
        Server        		server;
        size_t                bytes;
        std::ifstream                 file;
        bool        isHeadSent;
        bool        isFileOpened;
        std::streamsize bytesRead;
        size_t      content_len;
        ssize_t      bytes_sent;
        bool        flag;
        bool     _rem;
        std::string __name;
    private:
        int                 _client_socket;
        struct sockaddr_in  _client_address;
        time_t              _last_msg_time;
        // std::string             ppath;
};