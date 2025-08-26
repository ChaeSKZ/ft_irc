#include "Server.hpp"
#include "Utils.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sstream>

#define BUFFER_SIZE 512

Server::Server(int port, const std::string &password) : _port(port), _password(password)
{
	initSocket();
}

Server::~Server()
{
	close(_serverSocket);
}

void Server::setCapEnd(bool value)
{
	CapEnd = value;
}

void Server::setSentWelcome(bool value)
{
	SentWelcome = value;
}

bool Server::getCapEnd() const
{
	return CapEnd;
}

bool Server::getSentWelcome() const
{
	return SentWelcome;
}

void Server::initSocket()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket < 0)
		throw std::runtime_error("socket() failed");

	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt() failed");

	sockaddr_in serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(_port);

	if (bind(_serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
		throw std::runtime_error("bind() failed");

	if (listen(_serverSocket, 10) < 0)
		throw std::runtime_error("listen() failed");

	std::cout << "Server listening on port " << _port << std::endl;
}

void Server::run()
{
	std::vector<struct pollfd> fds;
	struct pollfd serverPoll;
	serverPoll.fd = _serverSocket;
	serverPoll.events = POLLIN;
	fds.push_back(serverPoll);

	while (true)
	{
		int ret = poll(&fds[0], fds.size(), -1);
		if (ret < 0)
			throw std::runtime_error("poll() failed");

		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == _serverSocket)
				{
					int clientFd = accept(_serverSocket, NULL, NULL);
					if (clientFd >= 0)
					{
						struct pollfd clientPoll;
						clientPoll.fd = clientFd;
						clientPoll.events = POLLIN;
						fds.push_back(clientPoll);
						_clients[clientFd] = Client(clientFd);

						struct sockaddr_in addr;
						socklen_t len = sizeof(addr);
						getpeername(clientFd, (struct sockaddr*)&addr, &len);
						std::string ip = inet_ntoa(addr.sin_addr);
						_clients[clientFd].setHostname(ip);
						std::cout << "New client connected (fd=" << clientFd << ")" << std::endl;
					}
				}
				else
				{
					char buffer[BUFFER_SIZE];
					int bytes = recv(fds[i].fd, buffer, BUFFER_SIZE - 1, 0);
					if (bytes <= 0)
					{
						std::cout << "Client disconnected (fd=" << fds[i].fd << ")" << std::endl;
						close(fds[i].fd);
						_clients.erase(fds[i].fd);
						fds.erase(fds.begin() + i);
						i--;
					}
					else
					{
						buffer[bytes] = '\0';
						std::string msg(buffer);
						handleClientMessage(fds[i].fd, msg);
					}
				}
			}
		}
	}
}

void Server::handleClientMessage(int fd, const std::string &msg)
{
	std::cout << "[Recv fd=" << fd << "] " << msg;

	Client &client = _clients[fd];

	std::istringstream iss(msg);
	std::string line;
	while (std::getline(iss, line))
	{
		if (!line.empty() && line[line.size()-1] == '\r')
			line.erase(line.size()-1);
		if (line.find("PASS ") == 0)
		{
			std::string pass = line.substr(5);
			if (pass != _password)
			{
				std::string err = ":ircserv 464 * :Password incorrect\r\n";
				send(fd, err.c_str(), err.size(), 0);
				close(fd);
				_clients.erase(fd);
				return;
			}
			else
				client.setPasswordAccepted(true);
		}
		else if (line.find("NICK ") == 0 )
			client.setNickname(line.substr(5));
		else if (line.find("USER ") == 0)
		{
			std::istringstream iss(line.substr(5));
			std::string username, hostname, servername, realname;
			iss >> username >> hostname >> servername;
			std::getline(iss, realname);
			if (!realname.empty() && realname[0] == ' ')
				realname = realname.substr(1);
			if (!realname.empty() && realname[0] == ':')
				realname = realname.substr(1);
			client.setUsername(username);
			client.setServername(servername);
			client.setRealname(realname);
		}
		else if (line.find("PING ") == 0 && getSentWelcome())
		{
			std::string pong = "PONG " + line.substr(5) + "\r\n";
			send(fd, pong.c_str(), pong.size(), 0);
		}
		else if (line.find("JOIN ") == 0 && getSentWelcome())
		{
			std::string chan = line.substr(5);
			cmdJoin(fd, chan);
		}
		else if (line.find("PRIVMSG ") == 0 && getSentWelcome())
		{
			size_t pos = line.find(" :");
			if (pos != std::string::npos)
			{
				std::string target = line.substr(8, pos - 8);
				std::string message = line.substr(pos + 2);
				cmdPrivmsg(fd, target, message);
			}
		}
		else if (line.find("QUIT") == 0 && getSentWelcome())
		{
			std::string reason;
			size_t pos = line.find(" :");
			if (pos != std::string::npos)
				reason = line.substr(pos + 2);
			else
				reason = "Client Quit";
			cmdQuit(fd, reason);
			return;
		}
		else if (line.find("CAP LS") == 0)
		{
			std::string capls = "CAP * LS :\r\n";
			send(fd, capls.c_str(), capls.size(), 0);
		}
		else if (line.find("CAP END") == 0)
			setCapEnd(true);
		if (client.isReady() && getCapEnd() && !getSentWelcome())
		{
			std::string welcome =
			":ircserv 001 " + client.getNickname() + " :Welcome to ft_irc, " + client.getNickname() + "\r\n";
			send(fd, welcome.c_str(), welcome.size(), 0);
			std::string host = ":ircserv 002 " + client.getNickname() + " :Your host is " + client.getHostname() + ", running version 1.0\r\n";
			send(fd, host.c_str(), host.size(), 0);
			std::string created = ":ircserv 003 " + client.getNickname() + " :This server was created just now\r\n";
			send(fd, created.c_str(), created.size(), 0);
			std::string motd_start = ":ircserv 375 " + client.getNickname() + " :- ircserv Message of the Day -\r\n";
			send(fd, motd_start.c_str(), motd_start.size(), 0);
			std::string motd_line = ":ircserv 372 " + client.getNickname() + " :- Welcome to ft_irc!\r\n";
			send(fd, motd_line.c_str(), motd_line.size(), 0);
			std::string motd_end = ":ircserv 376 " + client.getNickname() + " :End of /MOTD command.\r\n";
			send(fd, motd_end.c_str(), motd_end.size(), 0);
			client.authenticate();
		}
	}
}

void Server::cmdJoin(int fd, const std::string &channel)
{
	Client &client = _clients[fd];
	_channels[channel].insert(fd);

	std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " JOIN :" + channel + "\r\n";
	for (std::set<int>::iterator it = _channels[channel].begin(); it != _channels[channel].end(); ++it)
		std::cout << *it << " ";
	std::cout << std::endl;
	for (std::set<int>::iterator it = _channels[channel].begin(); it != _channels[channel].end(); ++it)
	{
		send(*it, joinMsg.c_str(), joinMsg.size(), 0);
	}
}

void Server::cmdPrivmsg(int fd, const std::string &target, const std::string &message)
{
	Client &client = _clients[fd];
	std::string msg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " PRIVMSG " + target + " :" + message + "\r\n";

	if (_channels.find(target) != _channels.end())
	{
		for (std::set<int>::iterator it = _channels[target].begin(); it != _channels[target].end(); ++it)
		{
			if (*it != fd)
			{
				int ret = send(*it, msg.c_str(), msg.size(), 0);
				if (ret == -1)
					perror("send failed");
				else
					std::cout << "Sent: " << msg;
			}
		}
	}
	else
	{
		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			if (it->second.getNickname() == target)
			{
				int ret = send(it->first, msg.c_str(), msg.size(), 0);
				if (ret == -1)
					perror("send failed");
				else
					std::cout << "Sent: " << msg;
				return;
			}
		}
		std::string err = ":ircserv 401 " + client.getNickname() + " " + target + " :No such nick/channel\r\n";
		send(fd, err.c_str(), err.size(), 0);
	}
}

void Server::cmdQuit(int fd, const std::string &reason)
{
	Client &client = _clients[fd];
	std::string quitMsg = ":" + client.getNickname() + " QUIT :" + reason + "\r\n";

	for (std::map<std::string, std::set<int> >::iterator ch = _channels.begin(); ch != _channels.end(); ++ch)
	{
		if (ch->second.find(fd) != ch->second.end())
		{
			for (std::set<int>::iterator it = ch->second.begin(); it != ch->second.end(); ++it)
			{
				if (*it != fd)
					send(*it, quitMsg.c_str(), quitMsg.size(), 0);
			}
			ch->second.erase(fd);
		}
	}

	close(fd);
	_clients.erase(fd);
	std::cout << "Client QUIT (fd=" << fd << "): " << reason << std::endl;
}
