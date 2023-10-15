#include "../../includes/main.hpp"

char fromHexToDec(std::string hex)
{
	char decimal = 0;
	for (int i = 0; i < 2; i++)
	{
		if (hex[i] >= '0' && hex[i] <= '9')
			decimal += (hex[i] - '0') * pow(16, 1 - i);
		else if (hex[i] >= 'A' && hex[i] <= 'F')
			decimal += (hex[i] - 'A' + 10) * pow(16, 1 - i);
		else if (hex[i] >= 'a' && hex[i] <= 'f')
			decimal += (hex[i] - 'a' + 10) * pow(16, 1 - i);
		else
			return (0);
	}
	return (decimal);
}

std::vector<std::string> ft_split(const std::string& s) {
	std::vector<std::string> tokens;
	std::string token;
	std::stringstream ss(s);
	while (ss >> token)
		tokens.push_back(token);
	return tokens;
}

std::vector<std::string> splitParametrs(std::string line, std::string sep)
{
	std::vector<std::string>	str;
	size_t		start, end;

	start = end = 0;
	while (1)
	{
		end = line.find_first_of(sep, start);
		if (end == std::string::npos)
			break;
		std::string tmp = line.substr(start, end - start);
		str.push_back(tmp);
		start = line.find_first_not_of(sep, end);
		if (start == std::string::npos)
			break;
	}
	return (str);
}