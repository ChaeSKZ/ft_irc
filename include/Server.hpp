#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <set>
#include "Client.hpp"

class Server
{
private:
	int _port;
	bool CapEnd;
	bool SentWelcome;
	std::string _password;
	int _serverSocket;
	std::map<int, Client> _clients;
	std::map<std::string, std::set<int> > _channels; // channel -> liste des fds

public:
	Server(int port, const std::string &password);
	~Server();

	void run();
	void handleClientMessage(int fd, const std::string &msg);
	void setCapEnd(bool value);
	void setSentWelcome(bool value);
	bool getCapEnd() const;
	bool getSentWelcome() const;

private:
	void initSocket();

	// Commandes IRC
	void cmdJoin(int fd, const std::string &channel);
	void cmdPrivmsg(int fd, const std::string &target, const std::string &message);
	void cmdQuit(int fd, const std::string &reason);
};

#endif
