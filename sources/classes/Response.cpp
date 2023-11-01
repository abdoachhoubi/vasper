#include "../../includes/main.hpp"

// Constructors

Response::Response()
{
	statusCode = SUCCESS;
	_cgi_state = 0;
};
Response::Response(Request &req, Server server) : _req(req), _server_conf(server)
{
	contentType = getContentTypeFromExtension(req.getPath());
	statusCode = SUCCESS;
	_cgi_state = 0;
}
void Response::setServer(Server server) { _server_conf = server; }
void Response::setRequest(Request req) { _req = req; }
int Response::getStatusCode() const { return statusCode; }
const std::string &Response::getStatusText() const { return statusText; }
const std::string &Response::getBody() const { return body; }
void Response::set_headers(std::string headers) { _headers = headers; }
void Response::set_response(std::string response) { this->_response = response; }
void Response::setPath(std::string path) { this->_path = path; }
void Response::settype(std::string type) { this->_type = type; }
void Response::setHeader(const std::string &key, const std::string &value) { headers[key] = value; }
void Response::setBody(const std::string &body) { this->body = body; }
void Response::cut_response(size_t i) { _response = _response.substr(i); }
std::string Response::getContentType() { return contentType; }
std::string Response::gettype() { return this->_type; }
std::string Response::get_response() { return this->_response; }
std::string Response::getPath() { return this->_path; }
std::string Response::get_headers() { return (_headers); }

bool Response::isResourceFound(const std::string &fullPath)
{
	struct stat fileStat;
	if (stat(fullPath.c_str(), &fileStat) == 0)
	{
		if (S_ISREG(fileStat.st_mode))
			_type = "FILE";
		else if (S_ISDIR(fileStat.st_mode))
			_type = "FOLDER";
	}
	else
		_type = "ERROR";
	return (!(_type == "ERROR"));
}

std::string Response::generateResponse(std::string fullPath, int flag, Location location)
{
	if (flag == 1)
	{
		if (location.getIndexLocation() != "")
			fullPath += location.getIndexLocation();
		else if (_server_conf.getIndex() != "")
			fullPath += _server_conf.getIndex();
		contentType = "text/html";
	}
	if (!fileExists(fullPath))
	{
		set_headers(generateErrorResponse(NOT_FOUND, _server_conf));
		return _headers;
	}
	std::ifstream file(fullPath.c_str(), std::ios::binary | std::ios::ate);
	if (!file)
	{
		set_headers(generateErrorResponse(INTERNAL_SERVER_ERROR, _server_conf));
		return _headers;
	}
	_path = fullPath;
	fileSize = file.tellg();
	if (fileSize > (long long)_server_conf.getClientMaxBodySize())
	{
		set_headers(generateErrorResponse(REQUEST_ENTITY_TOO_LARGE, _server_conf));
		return _headers;
	}
	contentType = getContentTypeFromExtension(fullPath);
	file.close();
	setStatus(statusCode, statusTextGen(statusCode));
	setHeader("Server", "MyWebServer");
	setHeader("Content-Type", contentType);
	setHeader("Content-length", to_string(fileSize));
	return this->toString();
}

const std::string &Response::getHeader(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	if (it != headers.end())
		return it->second;

	static const std::string empty;
	return empty;
}

void Response::setStatus(int statusCode, const std::string &statusText)
{
	this->statusCode = statusCode;
	this->statusText = statusText;
}

std::string Response::toString()
{
	std::string hedears = "HTTP/1.1 " + to_string(statusCode) + " " + statusTextGen(statusCode) + "\r\n";

	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
		hedears += it->first + ": " + it->second + "\r\n";

	hedears += "\r\n"; // End of headers
	set_headers(hedears);
	return hedears;
}

bool Response::isResourceDeletable(const std::string &resourcePath)
{
	struct stat fileStat;
	if (stat(resourcePath.c_str(), &fileStat) == 0)
	{
		if (S_ISREG(fileStat.st_mode) || S_ISDIR(fileStat.st_mode))
		{
			if (fileStat.st_uid == getuid())
			{
				if (fileStat.st_mode & S_IWUSR)
					return true;
			}
		}
	}
	return false;
}

bool Response::deleteResource(const std::string &resourcePath)
{
	// Check if the resource is deletable
	if (isResourceDeletable(resourcePath))
	{
		// Attempt to delete the resource
		if (remove(resourcePath.c_str()) == 0)
			return true;
	}
	return false;
}

bool Response::fileExists(const std::string &f)
{
	struct stat buffer;
	return (stat(f.c_str(), &buffer) == 0);
}

// saveDataToFile
void Response::SaveDataToFile(const std::string &filePath, const std::string &data)
{
	std::ofstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open())
		return;
	file << data;
	file.close();
}

void Response::AppendDataToFile(const std::string &filePath, const std::string &data)
{
	std::ofstream file(filePath.c_str(), std::ios::binary | std::ios::app);
	if (!file.is_open())
		return;
	file << data;
	file.close();
}

int Response::myFind(std::string src, std::string target)
{
	int i = 0;
	int j = 0;
	int src_len = src.length();
	int target_len = target.length();
	while (i < src_len)
	{
		if (src[i] == target[j])
		{
			while (src[i] == target[j] && i < src_len && j < target_len)
			{
				i++;
				j++;
			}
			if (j == target_len)
				return (i - j);
			else
				j = 0;
		}
		i++;
	}
	return (-1);
}

std::string Response::parseBoundary(std::string &body, std::string &boundary)
{
	int start = 0;
	int end = 0;
	std::string filename;
	std::string key;
	std::string value;
	std::string new_body;

	while (true)
	{
		start = body.find(boundary, end);
		if (start == -1)
			break;
		end = body.find(boundary, start + boundary.length());
		if (end == -1)
			break;
		std::string part = body.substr(start, end - start);
		std::string content_disposition = part.substr(part.find("Content-Disposition: ") + 21, part.find("\r\n") - part.find("Content-Disposition: ") - 21);
		if (myFind(content_disposition, "filename=") != -1)
		{
			std::string content_type = part.substr(part.find("Content-Type: ") + 14, part.find("\r\n") - part.find("Content-Type: ") - 14);
			content_type = content_type.substr(0, content_type.find("\r\n"));
			std::string file_extension = content_type.substr(content_type.find("/") + 1, content_type.length() - content_type.find("/") - 1);
			filename = content_disposition.substr(content_disposition.find("filename=") + 10, content_disposition.find("\r\n") - content_disposition.find("filename=") - 10);
			filename = filename.substr(0, filename.find("\""));
			key = content_disposition.substr(content_disposition.find("name=") + 6, content_disposition.find("\r\n") - content_disposition.find("name=") - 6);
			key = key.substr(0, key.find("\""));
			if (file_extension != "octet-stream")
				key += "." + file_extension;
			std::string data = part.substr(part.find("\r\n\r\n") + 4, part.length() - part.find("\r\n\r\n") - 4);
			if (file_extension == "octet-stream")
				data = data.substr(0, data.length() - 2);
			saveDataToFile(key, data);
			new_body += "name: " + key + "\n";
			new_body += "filename: " + filename + "\n";
			new_body += "Content-Type: " + content_type + "\n";
			new_body += "data: " + data + "\n\n";
		}
		else
		{
			key = content_disposition.substr(content_disposition.find("name=") + 6, content_disposition.find("\r\n") - content_disposition.find("name=") - 6);
			key = key.substr(0, key.find("\""));
			value = part.substr(part.find("\r\n\r\n") + 4, part.length() - part.find("\r\n\r\n") - 4);
			value = value.substr(0, value.length() - 2);
			saveDataToFile(key, value);
			new_body += "name: " + key + "\n";
			new_body += "data: " + value + "\n\n";
		}
	}
	return (new_body);
}

std::string Response::decodePath(std::string path)
{
	std::string decoded_path;
	int i = 0;
	while (i < (int)path.length())
	{
		if (path[i] == '%')
		{
			std::string hex = path.substr(i + 1, 2);
			int dec = strtol(hex.c_str(), NULL, 16);
			decoded_path += (char)dec;
			i += 3;
		}
		else
		{
			decoded_path += path[i];
			i++;
		}
	}
	return decoded_path;
}

int Response::getController(Location location)
{
	if (gettype() == "FILE")
	{
		// CGI STARTS HERE
		std::string file_extension = getPath().substr(getPath().find_last_of(".") + 1);
		std::vector<std::string> exts = location.getCgiExtension();
		std::vector<std::string> paths = location.getCgiPath();
		int i = 0;
		while (i < (int)exts.size())
		{
			if (((exts[i] == "*." + file_extension) || (exts[i] == "." + file_extension)) && location.getCGI())
			{
				_cgi_state = 1;
				handleCgi(location);
				std::string res = "HTTP/1.1 " + to_string(statusCode) + " " + statusTextGen(statusCode) + "\r\n";
				res += "Content-Type: text/html\r\n";
				res += "Content-Length: " + to_string(_response.length()) + "\r\n";
				res += "\r\n";
				res += _response;
				set_headers(res);
				return 0;
			}
			i++;
		}
		generateResponse(getPath(), 0, location);
		_check = false;
		return 0;
	}
	else if (gettype() == "FOLDER")
	{
		bool index_exist = false;
		if (location.getIndexLocation() != "" || _server_conf.getIndex() != "")
		{
			std::string index = location.getIndexLocation() != "" ? location.getIndexLocation() : _server_conf.getIndex();
			std::string index_path;
			if (getPath()[(int)(getPath().size() - 1)] != '/')
				index_path = getPath() + "/" + index;
			else
				index_path = getPath() + index;
			if (fileExists(index_path))
				index_exist = true;
		}
		if (getPath()[(int)(getPath().size() - 1)] != '/')
			setPath(getPath() + "/");
		if (!Server::isReadableAndExist(getPath(), location.getIndexLocation()) && index_exist)
		{
			set_headers(generateResponse(getPath(), 1, location));
			return 0;
		}
		else
		{
			if (location.getAutoindex())
			{
				std::string response_body = autoindex_body((char *)getPath().c_str(), _req.getPath());
				if (response_body.size() > _server_conf.getClientMaxBodySize())
				{
					set_headers(generateErrorResponse(REQUEST_ENTITY_TOO_LARGE, _server_conf));
					return 0;
				}
				std::string response = "HTTP/1.1 " + to_string(statusCode) + " " + statusTextGen(statusCode) + "\r\n";
				response += "Content-Type: text/html\r\n";
				response += "Content-Length: " + to_string(response_body.length()) + "\r\n";
				response += "\r\n";
				response += response_body;
				set_headers(response);
				return 0;
			}
			else
			{
				set_headers(generateErrorResponse(FORBIDDEN, _server_conf));
				return 0;
			}
		}
	}
	else
	{
		set_headers(generateErrorResponse(NOT_FOUND, _server_conf));
		return 0;
	}
}

int Response::postController(Location location)
{
	if (_req.getBody().size() > _server_conf.getClientMaxBodySize())
	{
		set_headers(generateErrorResponse(REQUEST_ENTITY_TOO_LARGE, _server_conf));
		return 0;
	}

	std::string file_extension = getPath().substr(getPath().find_last_of(".") + 1);
	std::vector<std::string> exts = location.getCgiExtension();
	std::vector<std::string> paths = location.getCgiPath();
	int i = 0;
	while (i < (int)exts.size())
	{
		if (((exts[i] == "*." + file_extension) || (exts[i] == "." + file_extension)) && location.getCGI())
		{
			_cgi_state = 1;
			handleCgi(location);
			std::string res = "HTTP/1.1 " + to_string(statusCode) + " " + statusTextGen(statusCode) + "\r\n";
			res += "Content-Type: text/html\r\n";
			res += "Content-Length: " + to_string(_response.length()) + "\r\n";
			res += "\r\n";
			res += _response;
			set_headers(res);
			return 0;
		}
		i++;
	}

	std::string upload_path = _server_conf.getUploadPath();
	if (upload_path[upload_path.length() - 1] == '/')
		upload_path = upload_path.substr(0, upload_path.length() - 1);
	std::string _target_file = location.getRootLocation() + "/" + upload_path + _req.getPath();
	if (fileExists(_target_file))
	{
		set_headers(generateErrorResponse(NO_CONTENT, _server_conf));
		return 0;
	}
	std::ofstream file(_target_file.c_str(), std::ios::binary);
	if (file.fail())
	{
		set_headers(generateErrorResponse(INTERNAL_SERVER_ERROR, _server_conf));
		return 0;
	}
	if (_req.getMultiformFlag())
	{
		std::string body = _req.getBody();
		body = parseBoundary(body, _req.getBoundary());
		file << body;
		set_headers(generateErrorResponse(SUCCESS, _server_conf));
		return 0;
	}
	else
	{
		file << _req.getBody();
		set_headers(generateErrorResponse(SUCCESS, _server_conf));
		return 0;
	}
}

int Response::deleteController(Location location)
{
	(void)location;
	if (gettype() == "FILE")
	{
		std::string resourcePath = getPath();
		if (deleteResource(resourcePath))
		{
			// Resource deleted successfully
			std::string res = "HTTP/1.1 SUCCESS OK\r\n";
			res += "Server: AstroServer\r\n";
			res += "Content-Length: 0\r\n";
			res += "Content-Type: application/json\r\n";
			res += "\r\n";
			set_headers(res);
			return 0;
		}
		else
		{
			set_headers(generateErrorResponse(INTERNAL_SERVER_ERROR, _server_conf));
			return 0;
		}
	}
	else if (gettype() == "FOLDER")
	{
		set_headers(generateErrorResponse(FORBIDDEN, _server_conf));
		return (0);
	}
	else
	{
		std::string res = "HTTP/1.1 NOT_FOUND Not Found\r\n";
		res += "Content-Length: 0\r\n";
		res += "Content-Type: application/json\r\n";
		res += "\r\n";
		set_headers(res);
		return (0);
	}
}

int Response::redirect(std::vector<Location> loc, std::vector<std::string> sub_uris, std::string loc_path)
{
	std::vector<Location>::iterator itx;
	bool flag = true;

	for (size_t i = 0; i < sub_uris.size(); ++i)
	{
		if (!flag)
			break;
		for (itx = loc.begin(); itx != loc.end(); ++itx)
		{
			if (!flag)
				break;
			std::string sub_uri = sub_uris[i].substr(0, sub_uris[i].length() - 1);
			if (sub_uri == "")
				sub_uri = "/";
			if (itx->getPath() == sub_uri)
			{
				flag = false;
				bool redir = false;
				if (!itx->getReturn().empty())
				{
					std::vector<Location>::iterator ity;
					for (ity = loc.begin(); ity != loc.end(); ++ity)
					{
						if (ity->getPath() == itx->getReturn())
						{
							statusCode = 302;
							loc_path = ity->getPath();
							_req.setPath(loc_path);
							redir = true;
							std::string res = "HTTP/1.1 " + to_string(statusCode) + " Found\r\n";
							res += "Server: " + _server_conf.getServerName() + "\r\n";
							res += "Location: " + ity->getPath() + "\r\n";
							res += "Content-Length: 0\r\n";
							res += "Content-Type: application/json\r\n";
							res += "\r\n";
							set_headers(res);
							return (1);
						}
					}
				}
				else
					redir = true;
				if (!redir)
				{
					set_headers(generateErrorResponse(MOVED_PERMANENTLY, _server_conf));
					return (1);
				}
			}
		}
	}
	return (0);
}

int Response::matchLocation(std::vector<Location> loc, std::vector<std::string> sub_uris)
{
	bool flag = true;
	std::string _req_path = _req.getPath();
	std::vector<Location>::iterator it;

	for (size_t i = 0; i < sub_uris.size(); ++i)
	{
		if (!flag)
			break;
		std::vector<Location>::iterator it;
		for (it = loc.begin(); it != loc.end(); ++it)
		{
			if (!flag)
				break;
			// remove last character from sub_uri
			std::string sub_uri = sub_uris[i].substr(0, sub_uris[i].length() - 1);
			if (sub_uri == "")
				sub_uri = "/";
			if (it->getPath() == sub_uri)
			{
				if (it->getRootLocation() != "")
				{
					// replace the req path matched location with the location's root path
					// example: if _req_path is "/in/lol.html" and the location that was matched is "/in" then "/in" should be replaced with it->getRootPath()
					_req_path = _req_path.replace(0, sub_uri.length(), it->getRootLocation());
				}
				else
					_req_path = _req_path.replace(0, sub_uri.length() - 1, _server_conf.getRoot());
				flag = false;
				_path = _req_path;
				isResourceFound(_path);
				if (!isMethodAllowed(it->getAllowedMethods(), _req.getMethodStr()))
				{
					set_headers(generateErrorResponse(METHOD_NOT_ALLOWED, _server_conf));
					return (1);
				}
				if (_req.getMethodStr() == "GET")
					return (!getController(*it));
				else if (_req.getMethodStr() == "DELETE")
					return (!deleteController(*it));
				else if (_req.getMethodStr() == "POST")
					return (!postController(*it));
				else
				{
					set_headers(generateErrorResponse(METHOD_NOT_ALLOWED, _server_conf));
					return (1);
				}
			}
		}
	}
	return (0);
}

int Response::respond()
{
	_req.setPath(decodePath(_req.getPath()));
	std::string loc_path = _req.getPath();
	if (loc_path[loc_path.length() - 1] == '/')
		loc_path = loc_path.substr(0, loc_path.length() - 1);
	loc_path = loc_path == "" ? "/" : loc_path;
	_check = true;
	std::vector<Location> loc = _server_conf.getLocations();
	std::vector<std::string> sub_uris = generateSubUris(loc_path);
	std::reverse(sub_uris.begin(), sub_uris.end());
	if (redirect(loc, sub_uris, loc_path))
		return (0);

	sub_uris = generateSubUris(loc_path);
	std::reverse(sub_uris.begin(), sub_uris.end());

	if (matchLocation(loc, sub_uris))
		return (0);
	set_headers(generateErrorResponse(METHOD_NOT_ALLOWED, _server_conf));
	return (0);
}

std::string Response::generateUniqueFilename()
{
	std::srand(static_cast<unsigned int>(std::time(NULL)));
	std::stringstream ss;
	ss << "/upload_" << std::rand() << ".dat";
	return ss.str();
}

// Helper function to save data to a file
bool Response::saveDataToFile(const std::string &filePath, const std::string &data)
{
	std::string final_path = _server_conf.getRoot() + "/" + _server_conf.getUploadPath() + filePath;
	std::ofstream file(final_path.c_str(), std::ios::binary);
	if (!file.is_open())
		return false;
	file << data;
	file.close();
	return true;
}

// Helper method to get the content type from the file extension

bool Response::isMethodAllowed(std::vector<std::string> methods, std::string req_method)
{
	for (size_t i = 0; i < methods.size(); ++i)
	{
		if (req_method == methods[i])
			return (true);
	}
	return (false);
}

std::string Response::getContentTypeFromExtension(const std::string &filePath)
{
	std::map<std::string, std::string> extensionToContentType;
	extensionToContentType[".html"] = "text/html";
	extensionToContentType[".htm"] = "text/html";
	extensionToContentType[".css"] = "text/css";
	extensionToContentType[".js"] = "application/javascript";
	extensionToContentType[".jpg"] = "image/jpeg";
	extensionToContentType[".png"] = "image/png";
	extensionToContentType[".gif"] = "image/gif";
	extensionToContentType[".svg"] = "image/svg+xml";
	extensionToContentType[".pdf"] = "application/pdf";
	extensionToContentType[".txt"] = "text/plain";
	extensionToContentType[".zip"] = "application/zip";
	extensionToContentType[".gz"] = "application/gzip";
	extensionToContentType[".tar"] = "application/x-tar";
	extensionToContentType[".xml"] = "application/xml";
	extensionToContentType[".xhtml"] = "application/xhtml+xml";
	extensionToContentType[".ico"] = "image/x-icon";
	extensionToContentType[".avi"] = "video/x-msvideo";
	extensionToContentType[".bmp"] = "image/bmp";
	extensionToContentType[".doc"] = "application/msword";
	extensionToContentType[".gz"] = "application/x-gzip";
	extensionToContentType[".jpeg"] = "image/jpeg";
	extensionToContentType[".mp3"] = "audio/mp3";
	extensionToContentType[".mp4"] = "video/mp4";
	extensionToContentType[".mpeg"] = "video/mpeg";
	extensionToContentType[".pdf"] = "application/pdf";
	extensionToContentType["default"] = "text/html";

	size_t lastDot = filePath.rfind('.');
	if (lastDot != std::string::npos)
	{
		std::string extension = filePath.substr(lastDot);
		std::map<std::string, std::string>::iterator it = extensionToContentType.find(extension);
		if (it != extensionToContentType.end())
			return it->second;
	}
	return "text/plain";
}
std::string Response::statusTextGen(int code)
{
	switch (code)
	{
	case 100:
		return "Continue";
	case 101:
		return "Switching Protocols";
	case SUCCESS:
		return "OK";
	case 201:
		return "Created";
	case 202:
		return "Accepted";
	case 203:
		return "Non-Authoritative Information";
	case NO_CONTENT:
		return "No Content";
	case 205:
		return "Reset Content";
	case 206:
		return "Partial Content";
	case 300:
		return "Multiple Choices";
	case MOVED_PERMANENTLY:
		return "Moved Permanently";
	case 302:
		return "Found";
	case 303:
		return "See Other";
	case 304:
		return "Not Modified";
	case 305:
		return "Use Proxy";
	case 307:
		return "Temporary Redirect";
	case BAD_REQUEST:
		return "Bad Request";
	case 401:
		return "Unauthorized";
	case 402:
		return "Payment Required";
	case FORBIDDEN:
		return "Forbidden";
	case NOT_FOUND:
		return "Not Found";
	case METHOD_NOT_ALLOWED:
		return "Method Not Allowed";
	case 406:
		return "Not Acceptable";
	case 407:
		return "Proxy Authentication Required";
	case 408:
		return "Request Timeout";
	case 409:
		return "Conflict";
	case 410:
		return "Gone";
	case 411:
		return "Length Required";
	case 412:
		return "Precondition Failed";
	case REQUEST_ENTITY_TOO_LARGE:
		return "Payload Too Large";
	case 414:
		return "URI Too Long";
	case 415:
		return "Unsupported Media Type";
	case 416:
		return "Range Not Satisfiable";
	case 417:
		return "Expectation Failed";
	case INTERNAL_SERVER_ERROR:
		return "Internal Server Error";
	case NOT_IMPLEMENTED:
		return "Not Implemented";
	case BAD_GATEWAY:
		return "Bad Gateway";
	case SERVICE_UNAVAILABLE:
		return "Service Unavailable";
	case GATEWAY_TIMEOUT:
		return "Gateway Timeout";
	case HTTP_VERSION_NOT_SUPPORTED:
		return "HTTP Version Not Supported";
	default:
		return "Unknown Status";
	}
}

std::string Response::generateErrorResponse(error_pages code, Server server)
{

	std::map<error_pages, std::string> err_pages = server.getErrorPages();
	if (err_pages[code] != "")
	{
		std::string path_to_error_page = server.getRoot() + "/" + err_pages[code];
		std::ifstream file(path_to_error_page.c_str(), std::ios::binary);
		if (!file)
			return (generateErrorResponse(INTERNAL_SERVER_ERROR, server));
		std::string body;
		std::string line;
		while (getline(file, line))
			body += line + "\n";
		file.close();
		std::string res = "HTTP/1.1 " + to_string(code) + " " + statusTextGen(code) + "\r\n";
		res += "Content-Type: text/html\r\n";
		res += "Content-Length: " + to_string(body.length()) + "\r\n";
		res += "\r\n";
		res += body;
		return res;
	}
	// TODO: CHANGE FLAG
	bool flag = true;
	std::string code_string = to_string(code);
	std::string res = "HTTP/1.1 " + code_string + " " + statusTextGen(code) + "\r\n";
	res += "Server: AstroServer\r\n";
	std::string body = "<!DOCTYPE html>\r\n";
	body += "<html lang='en'>\r\n";
	body += "<head>\r\n";
	body += "<meta charset='UTF-8'>\r\n";
	body += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\r\n";
	body += "<title>" + code_string + " - " + statusTextGen(code) + "</title>\r\n";
	body += "<style>\r\n";
	body += "*{margin:0;padding:0;box-sizing:border-box;}\r\n";
	body += "body {\r\n";
	body += "width: 100%;\r\n";
	body += "height: 100vh;\r\n";
	body += "display: flex;\r\n";
	body += "align-items: center;\r\n";
	body += "justify-content: center;\r\n";
	body += "background-color: #2e2e2e;\r\n";
	body += "color: #fff;\r\n";
	body += "}\r\n";
	body += "h1 {\r\n";
	body += "font-family: 'Courier New', Courier, monospace;\r\n";
	body += "}\r\n";
	body += "</style>\r\n";
	body += "</head>\r\n";
	body += "<body>\r\n";
	body += "<h1>" + code_string + " - " + statusTextGen(code) + "</h1>\r\n";
	body += "</body>\r\n";
	body += "</html>\r\n";

	if (flag)
	{
		res += "Content-Type: text/html\r\n";
		res += "Content-Length: " + to_string(body.length()) + "\r\n\r\n";
		res += body;
	}
	else
		res += "\r\n";
	return res;
}

void Response::setCgiState(int state)
{
	_cgi_state = state;
}

int Response::getCgiState()
{
	return (_cgi_state);
}

/* check a file for CGI (the extension is supported, the file exists and is executable) and run the CGI */
int Response::handleCgi(Location location)
{
	std::string path;
	size_t pos;

	path = this->_req.getPath();
	if (path[0] && path[0] == '/')
		path.erase(0, 1);

	path = _path;
	pos = path.find(".");
	if (pos == std::string::npos)
	{
		statusCode = NOT_IMPLEMENTED;
		return (1);
	}
	if (ConfParser::getTypePath(path) != 1)
	{
		statusCode = NOT_FOUND;
		set_headers(generateErrorResponse(NOT_FOUND, _server_conf));
		return (1);
	}
	_cgi_obj.clear();
	_cgi_obj.setCgiPath(path);
	_cgi_state = 1;
	if (pipe(_cgi_fd) < 0)
	{
		statusCode = INTERNAL_SERVER_ERROR;
		set_headers(generateErrorResponse(INTERNAL_SERVER_ERROR, _server_conf));
		return (1);
	}
	_cgi_obj.initEnv(_req, location); // + URI
	_cgi_obj.execute(statusCode);
	_response = _cgi_obj.getResponse();
	set_headers(_response);
	remove("./vasper.cgi");
	return (0);
}
