/* ********************************
 * Mike Thweatt
 * CS344
 * Project 4
 * 06/10/18
 * Encoder Server Process
 * otp_enc_c
 * *******************************/



#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_LEN 150000
char buffer[1000]; //Receives input, 1000 bytes at a time
char fullInput[MAX_LEN]; //Full string received from client
char clientName[10]; //Holds client name, sent from client
char textFile[MAX_LEN/2]; //Text file contents to be encrypted
char keyFile[MAX_LEN/2]; //Key file contents to use for encryption
char encrypted[MAX_LEN/2]; //Holds encrypted text
char* legend = " ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //Allowed characters

int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
socklen_t sizeOfClientInfo;
struct sockaddr_in serverAddress, clientAddress;



// Error function used for reporting issues
void error(const char *msg) { perror(msg); exit(1); } 



//Return position of requested character in the legend
int getIndex(char c){
	int i;
	for (i = 0; i < 27; i++){
		if (legend[i] == c){
			return i; 
		}
	}
	return -1; //Not found
}



//Encrypt plain text
void encrypt(){
	int i, j;
	i = 0;
	memset(encrypted, '\0', sizeof(encrypted));
	while (textFile[i] != '\0'){
		j = getIndex(textFile[i]); //Get legend index
		j += getIndex(keyFile[i]); //Add key index
		if (j > 26){ //If more than 26 (27 including 0), subtract 26
			j -= 26;
		}
		encrypted[i] = legend[j]; //Write encrypted char
		i++;
	}
	//Add terminator
	strcat(encrypted, "**");
}



//Parse the user input into textFile and keyFile
//"^^" denotes end of client name
//"&&" denotes end of plainText
//"**" denotes the end of the string/key
//plainText&&keyText**
void parseInput(){
	char* pos;
	memset(clientName, '\0', sizeof(clientName));
	memset(textFile, '\0', sizeof(textFile));
	memset(keyFile, '\0', sizeof(keyFile));
	//Copy is client name
	int textLen = strstr(fullInput, "^^") - fullInput;
	memcpy(clientName, fullInput, textLen);
	//Copy in plain text
	pos = &fullInput[textLen + 2];
	textLen = strstr(fullInput, "&&") - pos;
	memcpy(textFile, pos, textLen);
	//Find start of key, copy key
	pos = pos + textLen + 2;
	textLen = strstr(fullInput, "**") - pos;
	memcpy(keyFile, pos, textLen);
}



//Child function to handle newly created incoming connection
void goChild(){

	//Get the message from the client
	//Loop until it finds the termimnation signal (**)
	charsRead = 0;
	memset(fullInput, '\0', sizeof(fullInput));
	int counter=0;
	while (strstr(fullInput, "**") == NULL){
		memset(buffer, '\0', sizeof(buffer));
		//Read the client's message from the socket
		charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
		counter += charsRead;
		strcat(fullInput, buffer); //Add chunk to final string
		if (charsRead < 0) error("ERROR reading from socket");
	}


	//Parse the input
	parseInput();


	//Verify correct client is connected
	if (strcmp(clientName, "otp_enc") != 0){
		//Kill connection if not
		charsRead = send(establishedConnectionFD, "Connected to wrong server**", 27, 0);
		if (charsRead < 0) error("ERROR writing to socket");
		close(establishedConnectionFD);
		exit(1);
	}

	//Encrypt the text
	encrypt();


	//Send encrypted text back to client
	charsRead = send(establishedConnectionFD, encrypted, sizeof(encrypted), 0);
	if (charsRead < 0) error("ERROR writing to socket");


	//Close the existing socket which is connected to the client
	close(establishedConnectionFD);

	exit(0); //Exit child
}





int main(int argc, char *argv[]){
	int childExitMethod = 0;
	//Check usage & args
	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } 

	//SigChild signal handler
	//stackoverflow.com/questions/7171722/how-can-i-handle-sigchld-in-c
	signal(SIGCHLD, SIG_IGN); //Silently reap children

	//Set up the address struct for this process (the server)
	//Clear out the address struct
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); 	
	//Get the port number, convert to an integer from a string
	portNumber = atoi(argv[1]);
	//Create a network-capable socket
	serverAddress.sin_family = AF_INET;
	//Store the port number
	serverAddress.sin_port = htons(portNumber); 
 	//Any address is allowed for connection to this process
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	//Set up the socket
	//Create the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0) error("ERROR opening socket");

	//Enable the socket to begin listening
	//Connect socket to port
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
		error("ERROR on binding");

	//Flip the socket on - it can now receive up to 5 connections
	listen(listenSocketFD, 5);

	//Infinite loop to keep server alive
	while (1){
		//Accept a connection, blocking if one is not available until one connects
		//Get the size of the address for the client that will connect
		sizeOfClientInfo = sizeof(clientAddress);
		//Accept
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
		if (establishedConnectionFD < 0) error("ERROR on accept");

		//Fork off child
		pid_t childPID = fork();
		if (childPID == -1){ //Error
			error("Fork error"); break;

		} else if (childPID == 0){ //Child
			goChild();

		} else { //Parent
			waitpid(childPID, &childExitMethod, WNOHANG);	
		}
	}

	//Close the listening socket
	close(listenSocketFD);
	return 0; 
}
