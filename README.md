# IMAP Email Client Project
## CS 375 - Computer Networking Final Project

### Olivia Strasburg and Brandon Novak

For the final project, we decided to implement an interface for an IMAP email client connected to an IMAP server. Below is a link to the RFC in which the IMAP protocol is based on. Our program has mainly 5 different functionalities along with initializing the mailbox:

1. `ViewMessages(int sockfd)` - View one line headers of each message.
2. `ReadMessage(int sockfd)` - Open and read an email.
3. `DeleteMessage(int sockfd)` - Delete an email.
4. `SearchSender(int sockfd)` -  View one line headers of messages from a certain sender.
5. `SearchData(int sockfd)` -  View one line headers of messages on a certain day.

A. `initializeMailbox(int sockfd)` - Login into IMAP server and select itself into the correct inbox. 

**Link for RFC 3501 - IMAP**
https://datatracker.ietf.org/doc/html/rfc3501#page-4 

#### `main()`

Inside the main function, we use a TCP connection to connect the client to the server. After doing so we call `initializeMailbox()` to connect to the user's mailbox. Once inside the mailbox, the user has the option to return a number 1 through 5. Each number allows the user to perform a different function as listed above. Once a user is ready to quit, they can type "EXIT" in order to quit the program and gracefully close the connection. Inside of the loop, an options menu is displayed to remind the user which numbers perform which function.

#### `initializeMailbox(int sockfd)`

The `initializeMailbox()` is called after a connection is established with a TCP connection. When it is called, it requests the username and password for the user. In order to preserve confidentiallity, the password does not appear as the user types it. After the username and password are acquired and cleaned (removing the `\n` character from the char array), we format a `LOGIN` command so that we can send it to the server to grant us access to the mailbox. The string will appear as such: "tag1 LOGIN *username password*". If incorrect usernames or passwords are given, then the server responds with message that says so. We `regex` the response string from server. If the message contains `AUTHENTICATIONFAILED`, then the program notifies the user it provided incorrect information and exits the program. If the `LOGIN` command succeeds then the program continues. 

Then the client sends the server the following message "tag2 SELECT INBOX EXISTS". This function selects the primary inbox which will allow us to access messages from the inbox. The server's response contains a message of how many messages appear in our inbox. Again, we use `regex` to identify this number and print out the number of messages that are in the inbox.

#### `ViewMessages(int sockfd)`

This function is initialized if the user inputs `1` after the options menu is displayed. The function sends the following message to return the `From`, `Subject`, and `Date` from each email: "tag3 FETCH 1:* (BODY\[HEADER.FIELDS (From Subject Date)])". Using `regex`, we print out each of the select header fields into the terminal so that each message occupies one line gracefully. Unfortunately, our `regex` expression currently only allows us to print emails that have `Subject`. So if an email is sent to the client without a subject, the formatting of our print statements will not be aligned and messages would get mixed together on lines. 

####  `ReadMessage(int sockfd)`

This function returns a select amount of headers and the text of an email to the client. First, the function request which email, the client would like to read. It must choose the number identifier that is associated with the email. This identifier can be found in the output of `ViewMessages()`. Once the number is provided, the headers, `Subject`, `From`, and `Date` are returned and printed into the terminally gracefully using a `regex` expression. Afterwards, the following command is sent to the server: "tag4 FETCH *n* BODY\[TEXT]" where *n* is the identifier for an email. The text from the email is then printing gracefully using a `regex` expression. 
