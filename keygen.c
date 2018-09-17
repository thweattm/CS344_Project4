/* **************************************************************
 * Mike Thweatt
 * CS344
 * 06/10/18
 * Program 4 - keygen
 * Description: Generates a random key from char options in 
 * defined legend for a user given number of chars
 * *************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]){
	char* legend = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int keyLen = atoi(argv[1]) + 1; //Add one for new line char
	char key[keyLen];

	srand(time(NULL));
	int i, r;
	for (i = 0; i < keyLen - 1; i++){
		//Random int between 0 and length of legend
		r = rand() % strlen(legend); 		
		key[i] = legend[r];
	}

	//Add newline char to end of key
	key[keyLen-1] = '\n';
	//Print out key
	printf("%s", key);
	
	return(0);
}
