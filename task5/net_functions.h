void printTerminalOptions(int argc, char *argv[]);
int Socket(int family, int type, int proto);
int Bind(int sockfd, const struct sockaddr *myaddr, int addrlen);
int Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr* cliaddr, socklen_t *addrlen);
int Connect(int sockfd, const struct sockaddr *server, socklen_t addrlen);


