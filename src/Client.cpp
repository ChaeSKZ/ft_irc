#include "Client.hpp"

Client::Client() : _fd(-1), _authenticated(false), _passOk(false), _nickOk(false), _userOk(false) {}

Client::Client(int fd) : _fd(fd), _authenticated(false), _passOk(false), _nickOk(false), _userOk(false) {}

Client::~Client() {}

int Client::getFd() const { return _fd; }

void Client::setNickname(const std::string &nick) { _nickname = nick; _nickOk = true; }

void Client::setUsername(const std::string &user) { _username = user; _userOk = true; }

void Client::setHostname(const std::string &host) { _hostname = host; }

void Client::setServername(const std::string &server) { _servername = server; }

void Client::setRealname(const std::string &realname) { _realname = realname; }

void Client::setPasswordAccepted(bool ok) { _passOk = ok; }

std::string Client::getNickname() const { return _nickname; }

std::string Client::getUsername() const { return _username; }

std::string Client::getHostname() const { return _hostname; }

std::string Client::getServername() const { return _servername; }

std::string Client::getRealname() const { return _realname; }

bool Client::isReady() const { return _passOk && _nickOk && _userOk && !_authenticated; }

void Client::authenticate() { _authenticated = true; }

bool Client::isAuthenticated() const { return _authenticated; }
