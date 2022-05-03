/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <termios.h>

#define PORT "143" // the port client will be connecting to

#define MAXDATASIZE 500 // max number of bytes we can get at once


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes, new_fd;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"Need destination name\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("Client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("Client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "Client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("Client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	// if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	//     perror("recv");
	//     exit(1);
	// }

	//buf[numbytes] = '\0';

	//printf("Client: received '%s'\n",buf);
	char password[100];
	char response[500];
	printf("Please enter your Denison username: ");
	fgets(buf, MAXDATASIZE, stdin);
	printf("Enter password: ");

	//hide password from terminal
	struct termios term;
	tcgetattr(fileno(stdin), &term);
	term.c_lflag &= ~ECHO;
	tcsetattr(fileno(stdin), 0, &term);
	fgets(password, 100, stdin);
	term.c_lflag |= ECHO;
	tcsetattr(fileno(stdin), 0, &term);
	printf("\n");
	//command tag -> tag

	//make email
	//char domain[24]  = "@rogue1.cs.denison.edu ";
	char mail[MAXDATASIZE] = "tag LOGIN ";
	buf[strcspn(buf,"\n")] = 0;
	strncat(buf, " ", 23);
	strncat(mail, buf, MAXDATASIZE);
	strncat(mail, password, 100);

	//printf("mail: %s\n", mail);

	if (send(sockfd, mail, sizeof mail, 0) == -1)
		perror("send");
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}
	printf("%s\n", response);
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
			perror("recv");
			exit(1);
	}
	printf("%s\n\n\n\n", response);

	//printf("Checking mailbox\n");

	//look at emails
	//char command[10] = "tag1 NOOP";
	char command[18] = "tag1 SELECT INBOX";
	printf("%s\n\n", command);
	if (send(sockfd, command, sizeof command, 0) == -1)
		perror("send");
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1){
	    perror("recv");
	    exit(1);
	}
	printf("%s\n", response);
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1){
	    perror("recv");
	    exit(1);
	}
	printf("%s\n", response);


	close(sockfd);

	return 0;
}
