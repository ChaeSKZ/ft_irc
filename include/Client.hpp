#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
	int _fd;
	std::string _nickname;
	std::string _username;
	std::string _hostname;
	std::string _servername;
	std::string _realname;
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
	void setHostname(const std::string &host);
	void setServername(const std::string &server);
	void setRealname(const std::string &realname);
	void setPasswordAccepted(bool ok);

	std::string getNickname() const;
	std::string getUsername() const;
	std::string getHostname() const;
	std::string getServername() const;
	std::string getRealname() const;
	bool isReady() const;
	void authenticate();
	bool isAuthenticated() const;
};

#endif
