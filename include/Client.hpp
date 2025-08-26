#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
	int _fd;
	std::string _nickname;
	std::string _username;
	bool _authenticated;
	bool _passOk;
	bool _nickOk;
	bool _userOk;

public:
	Client();
	Client(int fd);
	~Client();

	int getFd() const;
	void setNickname(const std::string &nick);
	void setUsername(const std::string &user);
	void setPasswordAccepted(bool ok);

	std::string getNickname() const;
	bool isReady() const;
	void authenticate();
	bool isAuthenticated() const;
};

#endif
