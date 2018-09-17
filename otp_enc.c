/* ********************************
 * Mike Thweatt
 * CS344
 * Project4
 * 06/10/18
 * Encoder Client Process
 * otp_enc
 * *******************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define MAX_LEN 150000
char textFile[50];
char plainText[MAX_LEN/2];
char keyFile[50];
char key[MAX_LEN/2];
char sendString[MAX_LEN];
char* legend = " ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //Allowed characters



//Error function used for reporting issues
void error(const char *msg) { perror(msg); exit(0); }



void createString(){
	FILE* fp;
	char* line = NULL;
	size_t len;
	ssize_t read;

	memset(sendString, '\0', sizeof(sendString));
	memset(plainText, '\0', sizeof(plainText));
	memset(plainText, '\0', sizeof(key));

	//Add client name
	strcat(sendString, "otp_enc^^");

	//Open text file
	fp = fopen(textFile, "r");
	if (fp == NULL){fprintf(stderr, "Could not open %s\n", textFile); exit(1);}

	//Read in lines of text file
	while ((read = getline(&line, &len, fp)) != -1){
		strcat(plainText, line);
		//Remove possible new line char
		char* pos;
		if ((pos=strchr(plainText, '\n')) != NULL)
			*pos = '\0';
	}

	fclose(fp); //Close file
	
	//Open key file
	fp = fopen(keyFile, "r");
	if (fp == NULL){fprintf(stderr, "Could not open %s\n", keyFile); exit(1);}

	//Read in lines of text file
	while ((read = getline(&line, &len, fp)) != -1){
		strcat(key, line);
		//Remove possible new line char
		char* pos;
		if ((pos=strchr(key, '\n')) != NULL)
			*pos = '\0';

	}

	fclose(fp); //Close file

	//Compile one string
	strcat(sendString, plainText);
	strcat(sendString, "&&");
	strcat(sendString, key);
	strcat(sendString, "**");
	//Remove new line char
	char* pos;
	if ((pos=strchr(sendString, '\n')) != NULL)
		*pos = '\0';

}


void verifyInput(){
	//Make sure key length is >= plain text length
	if (strlen(key) < strlen(plainText)){
		fprintf(stderr, "Key file does not meet length requirements.\n");
		exit(1);
	}

	//Make sure no bad characters in plain text
	int i=0, j, x;
	char c;
	while (plainText[i] != '\0'){
		x = 1; //Not found flag
		c = plainText[i];
		for (j = 0; j < 27; j++){
			if (legend[j] == plainText[i]){
				x = 0; //Mark flag as found
			}	
		}
		if (x){
			fprintf(stderr, "Bad characters found in %s.\n", textFile);
			exit(1);
		}
		i++;
	}
}




int main(int argc, char *argv[]){
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[MAX_LEN/2];
	char encrypted[MAX_LEN/2];

	//Check usage & args
	if (argc < 3) { fprintf(stderr, "USAGE: %s textFile keyFile port\n", argv[0]); exit(1);}

	memset(textFile, '\0', sizeof(textFile));
	strcpy(textFile, argv[1]); //Text file name
	memset(keyFile, '\0', sizeof(keyFile));
	strcpy(keyFile, argv[2]); //Key file name

	//Compile the data to send to server
	createString();	

	//Verify text input
	verifyInput();


	//Set up the server address struct
	//Clear out the address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress));
	//Get the port number, convert to an integer from a string
	portNumber = atoi(argv[3]);
	//Create a network-capable socket
	serverAddress.sin_family = AF_INET;
	//Store the port number
	serverAddress.sin_port = htons(portNumber);
	//Convert the machine name into a special form of address
	//serverHostInfo = gethostbyname(argv[1]);
	serverHostInfo = gethostbyname("localhost");
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	//Copy in the address
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);


	//Create the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	

	//Connect to server
	//Connect socket to address
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
		error("CLIENT: ERROR connecting");



	//Send text to server
	charsWritten = 0;
	do {
		int c = send(socketFD, sendString, sizeof(sendString), 0);
		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		charsWritten += c;
	} while (charsWritten < sizeof(sendString));

	
	//Get encrypted text back
	memset(encrypted, '\0', sizeof(encrypted));
	//Read data from the socket, until terminator is found
	while (strstr(encrypted, "**") == NULL){
		memset(buffer, '\0', sizeof(buffer));
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		strcat(encrypted, buffer);
	}

	//Remove terminator from string
	int term = strstr(encrypted, "**") - encrypted;
	encrypted[term] = '\0';


	//Report error if connected to wrong server
	if (strcmp(encrypted, "Connected to wrong server") == 0){
		fprintf(stderr, "%s, client now exiting.\n", encrypted);
		close(socketFD);
		exit(2);
	}
	
	//Print encrypted text
	printf("%s\n", encrypted);


	//Close the socket
	close(socketFD);
	return 0;
}
