#include "../../includes/main.hpp"

Client::Client() {}
Client::~Client() {}
Client::Client(const Client &other) { *this = other; }
void Client::setSocket(int &sock) { _client_socket = sock; }
void Client::clearClient() { request.clear(); }
void Client::setAddress(sockaddr_in &addr) { _client_address = addr; }
void Client::setServer(Server &server) { response.setServer(server); }
const int &Client::getSocket() const { return (_client_socket); }
const Request &Client::getRequest() const { return (request); }
const struct sockaddr_in &Client::getAddress() const { return (_client_address); }
/* Assinment operator */
Client &Client::operator=(const Client &rhs)
{
	if (this != &rhs)
	{
		this->_client_socket = rhs._client_socket;
		this->_client_address = rhs._client_address;
		this->request = rhs.request;
		this->response = rhs.response;
		this->server = rhs.server;
		this->isHeadSent = rhs.isHeadSent;
		this->isFileOpened = rhs.isFileOpened;
		this->_rem = rhs._rem;
		this->bytesRead = rhs.bytesRead;
		this->content_len = rhs.content_len;
		this->bytes_sent = rhs.bytes_sent;
		this->flag = rhs.flag;
	}
	return (*this);
}

Client::Client(Server &server)
{
	isHeadSent = false;
	isFileOpened = false;
	_rem = false;
	bytesRead = 0;
	content_len = 0;
	bytes_sent = 0;
	flag = false;
	response.setServer(server);
	request.setMaxBodySize(server.getClientMaxBodySize());
}

void Client::buildResponse()
{
	response.setRequest(this->request);
	response.respond();
}