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
#include <iostream>
#include <arpa/inet.h>
#include <termios.h>
#include <regex>

using namespace std;

#define PORT "143" // the port client will be connecting to

#define MAXDATASIZE 500 // max number of bytes we can get at once

void initializeMailbox(int sockfd);
void ViewMessages(int sockfd);
void ReadMessage(int sockfd);

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[])
{
	/* ====================================================================== */
	// Connect to IMAP email server
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

	freeaddrinfo(servinfo);
/* ====================================================================== */
	initializeMailbox(sockfd);

	char options_menu[500] = "\nPlease select a number or Type \"EXIT\" to quit.\n\n 1. View All Messages\n 2. Read an Email\n Response: ";
	char user_input[100];
	printf("%s", options_menu);
	fgets(user_input, 100, stdin);
	while (strncmp(user_input, "EXIT\n",6) != 0){
		if (strncmp(user_input, "1\n",3) == 0){
			ViewMessages(sockfd);
		}
		else if (strncmp(user_input, "2\n",3) == 0){
			ReadMessage(sockfd);
		}
		else{
			printf("Not an eligible option. Please choose 1-5 or EXIT\n");
		}
		printf("%s", options_menu);
		fgets(user_input, 100, stdin);
	}

	close(sockfd);
	return 0;
}

void ReadMessage(int sockfd){
	char response[1000];
	int numbytes;
	printf("Which email would like to read? Please enter the number associated with the email.\n Response: ");
	char user_input[100];
	fgets(user_input, 100, stdin);
	user_input[strcspn(user_input,"\n")] = 0;
	char command[MAXDATASIZE] = "tag4 FETCH ";
	char command1[MAXDATASIZE] = " BODY[TEXT]\n";
	strncat(command, user_input, MAXDATASIZE);
	strncat(command, command1, MAXDATASIZE);
	printf("Command: %s", command);
	if (send(sockfd, command, sizeof command, 0) == -1)
		perror("send");
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
			perror("recv");
			exit(1);
	}
	printf("\n%s\n", response);
}

void ViewMessages(int sockfd){
	char response[3000];
	int numbytes;
	//char command[MAXDATASIZE] = "tag3 FETCH 1:* (ENVELOPE)\n";//fetch 1 to n
	char command[MAXDATASIZE] = "tag3 FETCH 1:* (BODY[HEADER.FIELDS (From Subject Date)])\n";

	if (send(sockfd, command, sizeof command, 0) == -1)
		perror("send");
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
			perror("recv");
			exit(1);
	}
	string res = response;
	//cout << res << endl;
	smatch m;
	regex regexp(":(.*)");
	int i = 1;
	int index = 1;
	cout << index << " ";
	string match;
	while (regex_search(res, m, regexp)){
		match = m.str();
		match.erase(0,2);
		std::cout << match << "    ";
    res = m.suffix().str();
		if (i % 3 == 0){
			printf("\n");
			index++;
			cout << index << " ";
		}
		i++;
	 }
  }


void initializeMailbox(int sockfd)
{
	char password[100];
	char username[100];
	char response[500];
	int numbytes;
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
			perror("recv");
			exit(1);
	}
	//printf("\n%s\n", response);

	//printf("Client: received '%s'\n",buf);
	printf("Please enter your Denison username: ");
	fgets(username, MAXDATASIZE, stdin);
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
	char mail[MAXDATASIZE] = "tag1 LOGIN ";
	username[strcspn(username,"\n")] = 0;
	//password[strcspn(password,"\n")] = 0;
	strncat(username, " ", 23);
	strncat(mail, username, MAXDATASIZE);
	strncat(mail, password, 100);

	if (send(sockfd, mail, sizeof mail, 0) == -1)
		perror("send");
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}
	//printf("\n%s\n", response);

	string res = response;
	smatch m;
	regex regexp("AUTHENTICATIONFAILED");
	if (regex_search(res, m, regexp) == 1){
		printf("\nAuthentification Failed: Invalid username or password.\n\n");
		exit(1);
	}
	else{
		printf("\nAuthentification Successful.\n\n");
	}

	//look at emails
	char command[MAXDATASIZE] = "tag2 SELECT INBOX EXISTS\n";
	//char command[MAXDATASIZE] = "tag2 SEARCH ALL";
	if (send(sockfd, command, sizeof command, 0) == -1)
		perror("send");
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1){
	    perror("recv");
	    exit(1);
	}
	//printf("%s\n", response);

	res = response;
	regex regexp2("\\*\\s(\\d)\\sEXISTS");
	regex_search(res, m, regexp2);
	cout << "There are " << m[1] << " emails in your inbox."<< endl;
}
