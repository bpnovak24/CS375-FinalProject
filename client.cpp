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
void DeleteMessage(int sockfd);
void SearchSender(int sockfd);
void SearchDate(int sockfd);

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
/* ---------------------------------------------------------------------------*/
	initializeMailbox(sockfd);

	char options_menu[500] = "\nPlease select a number or Type \"EXIT\" to quit.\n\n 1. View All Messages\n 2. Read an Email\n 3. Delete an Email\n 4. Search by Sender\n 5. Search by Date\nResponse: ";
	char user_input[100];
	printf("%s", options_menu);
	fgets(user_input, 100, stdin);//Client user chooses what to do
	while (strncmp(user_input, "EXIT\n",6) != 0){//return to main menu unless EXIT is typed
		if (strncmp(user_input, "1\n",3) == 0){
			ViewMessages(sockfd);
		}
		else if (strncmp(user_input, "2\n",3) == 0){
			ReadMessage(sockfd);
		}
		else if (strncmp(user_input, "3\n",3) == 0){
			DeleteMessage(sockfd);
		}
		else  if (strncmp(user_input, "4\n",3) == 0){
			SearchSender(sockfd);
		}
		else if (strncmp(user_input, "5\n",3) == 0){
			SearchDate(sockfd);
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

/* ====================================================================================================================================================== */
void ViewMessages(int sockfd){
	char response[3000];
	int numbytes;

	//Ask server for header information from all emails
	char command[MAXDATASIZE] = "tag3 FETCH 1:* (BODY[HEADER.FIELDS (From Subject Date)])\n";
	if (send(sockfd, command, sizeof command, 0) == -1)
		perror("send");
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
			perror("recv");
			exit(1);
	}

	//Fix formatting in response
	string res = response;
	//cout << res << endl;
	smatch m;
	regex regexp(":(.*)");//\*\s(\d).*\n(Subject:(.*)\n)?Date:\s(.*)\nFrom:\s(.*)\s*\)
	int i = 1;
	int index = 1;
	cout << index << " ";
	string match;
	while (regex_search(res, m, regexp)){
		match = m.str();
		match.erase(0,1);
		std::cout << match << "    ";
    res = m.suffix().str();
		if (i % 3 == 0){
			printf("\n");
			index++;
			if (regex_search(res, m, regexp) == 1)
				cout << index << " ";
		}
		i++;
	 }
	 memset(response, 0, sizeof response);
  }

/* ====================================================================================================================================================== */

void ReadMessage(int sockfd){
	char response[1000];
	int numbytes;
	printf("\nWhich email would like to read? Please enter the number associated with the email.\n\nResponse: ");
	char user_input[100];
	fgets(user_input, 100, stdin);//user chooses email index number
	printf("\n");

	//construct command to send to server
	user_input[strcspn(user_input,"\n")] = 0;
	char command1[MAXDATASIZE] = " BODY[HEADER.FIELDS (From Subject Date)]\n";
	char command2[MAXDATASIZE] = " BODY[TEXT]\n";
	char header_command[MAXDATASIZE] = "tag4 FETCH ";
	char text_command[MAXDATASIZE] = "tag4 FETCH ";
	strncat(header_command, user_input, MAXDATASIZE);
	strncat(header_command, command1, MAXDATASIZE);
	if (send(sockfd, header_command, sizeof header_command, 0) == -1)
	  perror("send");
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	//Fix formatting in response
	string res = response;
	smatch m;
	regex regexp("(.*:(.*))");

	while (regex_search(res, m, regexp)){
	  //match = m.str();
	  std::cout << m.str() << endl;;
	  res = m.suffix().str();
	 }

	 strncat(text_command, user_input, MAXDATASIZE);
	 strncat(text_command, command2, MAXDATASIZE);
	 char response2[1000];
	 if (send(sockfd, text_command, sizeof text_command, 0) == -1)
	   perror("send");
	 if ((numbytes = recv(sockfd, response2, sizeof response2, 0)) == -1) {
	     perror("recv");
	     exit(1);
	 }

	res = response2;
	regex regexp2("\n(.*)");
	regex_search(res, m, regexp2);
	cout << m[1] << endl;
}

/* ====================================================================================================================================================== */
void DeleteMessage(int sockfd){
		char response[3000];
		int numbytes;

		printf("Which email would like to delete? Please enter the number associated with the email.\nResponse: ");
		char user_input[100];
		fgets(user_input, 100, stdin);
		user_input[strcspn(user_input,"\n")] = 0;

		//construct command to flag message to be deleted
		char command[MAXDATASIZE] = "tag5 STORE ";
		char command1[MAXDATASIZE] = " FLAGS (\\Deleted)\n";
		strncat(command, user_input, MAXDATASIZE);
		strncat(command, command1, MAXDATASIZE);
		if (send(sockfd, command, sizeof command, 0) == -1)
			perror("send");

		//delete flagged messages
		char command3[14] = "tag5 EXPUNGE\n";
		if (send(sockfd, command3, sizeof command, 0) == -1)
			perror("send");
		if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
				perror("recv");
				exit(1);
		}
		//printf("response: %s", response);
		// printf("\nEmail %s deleted\n", user_input);

		string res = response;
		smatch m;
		regex regexp("OK Expunge completed");
		if (regex_search(res, m, regexp) == 1){
			printf("\nDeletion Successful\n\n");
			exit(1);
		}
		else{
			printf("\nDeletion Unsuccessful\n\n");
		}


		memset(response, 0, sizeof response);

}

/* ====================================================================================================================================================== */
void SearchSender(int sockfd){
	char response[1000];
	int numbytes;
	printf("\nWhich sender would you like to see emails from? Enter their first or last name.\n\nResponse: ");
	char user_input[100];
	fgets(user_input, 100, stdin);
	printf("\n");

	//construct command to send to server
	user_input[strcspn(user_input,"\n")] = 0;
	char command1[MAXDATASIZE] = "tag6 SEARCH FROM \"";
	char command2[MAXDATASIZE] = "\"\n";
	strncat(command1, user_input, MAXDATASIZE);
	strncat(command1, command2, MAXDATASIZE);
	if (send(sockfd, command1, sizeof command1, 0) == -1)
	  perror("send");
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}


	//use email indices from response to send fetches
	string nums = response;
	smatch m;
	regex p("\\* SEARCH (.*)");
	regex_search(nums, m, p);
	string indices = m[1];

	//FETCH headers from each email
	for (int i = 0; i < indices.length(); i=i+2){
		memset(response, 0, sizeof response);
		// cout << indices[i] << endl;
		// char *ind = indices;
		// cout << ind << endl;
		string header = "tag6 FETCH ";
		string text_command = " BODY[HEADER.FIELDS (From Subject Date)]\n";
		header = header + indices[i] + text_command;
		//strncat(header_command, text_command, MAXDATASIZE);

		//convert command to char array to send
		char cmd[header.length() + 1];
		strcpy(cmd, header.c_str());
		if (send(sockfd, cmd, sizeof cmd, 0) == -1)
			perror("send");
		if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
				perror("recv");
				exit(1);
		}

		//Fix formatting in response
		string res = response;
		//cout << res << endl;
		smatch m;
		regex regexp(":(.*)");
		cout << indices[i] << " ";
		string match;
		int j = 1;
		while (regex_search(res, m, regexp)){
			match = m.str();
			match.erase(0,1);
			std::cout << match << "    ";
	    res = m.suffix().str();
			if (j % 3 == 0){
				printf("\n");
			}
			j++;
		 }
		 memset(response, 0, sizeof response);//clear response variable
 }
}

/* ====================================================================================================================================================== */
void SearchDate(int sockfd){
	char response[1000];
	int numbytes;
	printf("\nWhich date would you like to see emails from? Use format shown below. Note: Month is always three letters.\nExample: 05-Apr-2022 (DD-MMM-YYYY)\n\nResponse: ");
	char user_input[100];
	fgets(user_input, 100, stdin);
	printf("\n");

	//construct command to send to server
	user_input[strcspn(user_input,"\n")] = 0;
	char command1[MAXDATASIZE] = "tag7 SEARCH ON \"";
	char command2[MAXDATASIZE] = "\"\n";
	strncat(command1, user_input, MAXDATASIZE);
	strncat(command1, command2, MAXDATASIZE);
	if (send(sockfd, command1, sizeof command1, 0) == -1)
	  perror("send");
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}
	string nums = response;
	smatch m;
	regex p("\\* SEARCH (.*)");
	regex_search(nums, m, p);
	string indices = m[1];

	//FETCH headers from messages
	for (int i = 0; i < indices.length(); i=i+2){
		memset(response, 0, sizeof response);
		// cout << indices[i] << endl;
		// char *ind = indices;
		// cout << ind << endl;
		string header = "tag7 FETCH ";
		string text_command = " BODY[HEADER.FIELDS (From Subject Date)]\n";
		header = header + indices[i] + text_command;
		//strncat(header_command, text_command, MAXDATASIZE);

		char cmd[header.length() + 1];
		strcpy(cmd, header.c_str());

		if (send(sockfd, cmd, sizeof cmd, 0) == -1)
			perror("send");
		if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
				perror("recv");
				exit(1);
		}

		//Fix formatting in response
		//printf("response: %s\n", response);
		string res = response;
		//cout << res << endl;
		smatch m;
		regex regexp(":(.*)");
		cout << indices[i] << " ";
		string match;
		int j = 1;
		while (regex_search(res, m, regexp)){
			match = m.str();
			match.erase(0,1);
			std::cout << match << "    ";
	    res = m.suffix().str();
			if (j % 3 == 0){
				printf("\n");
			}
			j++;
		 }
		 memset(response, 0, sizeof response);
	 }
}


/* ====================================================================================================================================================== */
void initializeMailbox(int sockfd){
	char password[100];
	char username[100];
	char response[500];
	int numbytes;
	if ((numbytes = recv(sockfd, response, sizeof response, 0)) == -1) {
			perror("recv");
			exit(1);
	}


	//login
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

	//construct command to send to server
	char mail[MAXDATASIZE] = "tag1 LOGIN ";
	username[strcspn(username,"\n")] = 0;
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

	//simplify response
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
