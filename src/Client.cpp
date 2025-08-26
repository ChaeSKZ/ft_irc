#include "Client.hpp"

Client::Client() : _fd(-1), _authenticated(false), _passOk(false), _nickOk(false), _userOk(false) {}

Client::Client(int fd) : _fd(fd), _authenticated(false), _passOk(false), _nickOk(false), _userOk(false) {}

Client::~Client() {}

int Client::getFd() const { return _fd; }

void Client::setNickname(const std::string &nick) { _nickname = nick; _nickOk = true; }

void Client::setUsername(const std::string &user) { _username = user; _userOk = true; }

void Client::setPasswordAccepted(bool ok) { _passOk = ok; }

std::string Client::getNickname() const { return _nickname; }

bool Client::isReady() const { return _passOk && _nickOk && _userOk && !_authenticated; }

void Client::authenticate() { _authenticated = true; }

bool Client::isAuthenticated() const { return _authenticated; }
