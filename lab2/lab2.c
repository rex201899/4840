/*
*
* CSEE 4840 Lab 2 for 2019
*
* Lancelot Wathieu, lbw2148
* Zhongtai Ren, zr2208
*/
#include "fbputchar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "usbkeyboard.h"
#include <pthread.h>

/* Update SERVER_HOST to be the IP address of
* the chat server you are connecting to
*/
/* micro36.ee.columbia.edu */
//#define SERVER_HOST "160.39.132.244"
//#define SERVER_HOST "160.39.132.102"
#define SERVER_HOST "128.59.148.182"
#define SERVER_PORT 42000

#define BUFFER_SIZE 128
#define MAX_MSG_LEN BUFFER_SIZE - 17

/*
* References:
*
* http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
* http://www.thegeekstuff.com/2011/12/c-socket-programming/
* 
*/

int sockfd; /* Socket file descriptor */

struct libusb_device_handle *keyboard;
uint8_t endpoint_address;

pthread_t network_thread;
void *network_thread_f(void *);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char lower[37] = "abcdefghijklmnopqrstuvwxyz1234567890";
char upper[37] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()";

char symbol[14]       = " -=[]\\\0;'`,./";
char symbol_shift[14] = " _+{}|\0:\"~<>?";



void deleteChar (char * strA, int pos) {
	memmove(strA + pos, strA + pos + 1, strlen(strA) - pos);
}

void insertChar(char* destination, int pos, char ch) //char* seed)
{
	char seed[2] = {ch, 0};
	char strC[BUFFER_SIZE];

	strncpy(strC,destination,pos);
	strC[pos] = '\0';
	strcat(strC,seed);
	strcat(strC,destination+pos);
	strcpy(destination,strC);
}

char debug( int modifier, int key) 
{
	int shift = (modifier == 2 || modifier == 32);
	if (key > 3 && key < 30) {
		if (shift) {
			return upper[key-4];
		}
		else {
			return lower[key-4];
		}
	}
	else if (key == 44) {
		return ' ';
	}
	return 0;
}

void translate( int modifier, int key, int * cursor, char * msg) //int msgLen) 
{
	int shift = (modifier == 2 || modifier == 32);

	if (key == 42) {        //backspace key
		if ((*cursor) > 0)
		deleteChar(msg, --(*cursor));
	}
	else if (key == 79) {   //right arrow
		if ((*cursor) < strlen(msg)) 
			(*cursor)++;
	} 
	else if (key == 80) {   //left arrow
		if ((*cursor) > 0)
			(*cursor)--;
	}
	else if( strlen(msg) < MAX_MSG_LEN ) {

		if (key > 3 && key < 40 ) {
			if (shift) {
				insertChar(msg, (*cursor)++, upper[key-4]);
			}
			else {
				insertChar(msg, (*cursor)++, lower[key-4]);
			}
		}

		if (key > 43 && key < 57 && key != 50) {
			if (shift) {
				insertChar(msg, (*cursor)++, symbol_shift[key-44]);
			}
			else {
				insertChar(msg, (*cursor)++, symbol[key-44]);
			}
		}
	}
}

void clearScreen() 
{
	for (int col = 0 ; col < 64 ; col++) {
		for (int row = 0 ; row < 24 ; row++ ) {
			fbputchar(' ', row, col);
		}
	}
}

void clearLowerScreen() 
{
	for (int col = 0 ; col < 64 ; col++) {
		fbputchar(' ', 22, col);
		fbputchar(' ', 23, col);
	}
}

void clearUpperScreen() 
{
	for (int col = 0 ; col < 64 ; col++) {
		for (int row = 0 ; row < 21 ; row++ ) {
			fbputchar(' ', row, col);
		}
	}
}

void split()
{
	for (int col = 0 ; col < 64; col++) {
		fbputchar('-', 21, col);
	}
}

int main()
{
	int err;

	struct sockaddr_in serv_addr;

	struct usb_keyboard_packet packet;
	int transferred;
	char keystate[12];

	if ((err = fbopen()) != 0) {
		fprintf(stderr, "Error: Could not open framebuffer: %d\n", err);
		exit(1);
	}

	clearScreen();
	split();

	/* Open the keyboard */
	if ( (keyboard = openkeyboard(&endpoint_address)) == NULL ) {
		fprintf(stderr, "Did not find a keyboard\n");
		fbputs("Did not find a keyboard", 22, 19);
		exit(1);
	}

	/* Create a TCP communications socket */
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		fprintf(stderr, "Error: Could not create socket\n");
		exit(1);
	}

	/* Get the server address */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERVER_PORT);
	if ( inet_pton(AF_INET, SERVER_HOST, &serv_addr.sin_addr) <= 0) {
		fprintf(stderr, "Error: Could not convert host IP \"%s\"\n", SERVER_HOST);
		exit(1);
	}

	/* Connect the socket to the server */
	if ( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "Error: connect() failed.  Is the server running?\n");
		fbputs("Failed - Is the server running?",22,27);
		exit(1);
	}


	int chatLineNum = 0;
	/* Start the network thread */

	pthread_create(&network_thread, NULL, network_thread_f, &chatLineNum);
	char msg[BUFFER_SIZE];
	memset( msg, 0, BUFFER_SIZE);

	int cursor = 0;
	int key0_old = 0;
	int key1_old = 0;

	//int writeReady = 1;

	/* Look for and handle keypresses */
	fbputchar('_', 22, cursor);
	fbputs("<- Type your message here", 22, 2);

	for (;;) {
		libusb_interrupt_transfer(keyboard, endpoint_address,
			(unsigned char *) &packet, sizeof(packet),
			&transferred, 0);
		if (transferred == sizeof(packet)) {

			//debugging
			sprintf(keystate, "%02x %02x %02x", packet.modifiers, packet.keycode[0],
			  packet.keycode[1]);
			printf("%s, %c\n", keystate, debug(packet.modifiers, packet.keycode[0]));
			if ( packet.keycode[0] == 0 || 
			     (packet.keycode[0] == key0_old && packet.keycode[1] == key1_old) || //use shift means nothing
			     (packet.keycode[0] == key0_old && packet.keycode[1] == 0) ||	//release the second key, 1st condition
			     packet.keycode[0] == key1_old ||
			     packet.keycode[2] != 0) {	//release the second key, 2nd condition
				//do nothing, still have to update
			     if (packet.keycode[2]!=0) {
						printf("ignore this\n");
						continue;
			     } 
			}
			else if (packet.keycode[0] == 0x29) { /* ESC pressed? */
				clearLowerScreen();
				fbputs("Have a good day!", 22, 23);
				//fbputchar('_', 22, 0);
				break;
			}
			else if (packet.keycode[0] == 0x28) { /* return pressed */
				if (strlen(msg) == 0)
					continue;


				//update to chat window
				char buf[BUFFER_SIZE];
				memset( buf, 0, BUFFER_SIZE);
				strcpy(buf,"<  My Message  > ");
				strcat(buf, msg);

				//lock and write to the upper screen, so network thread doesn't write
				pthread_mutex_lock(&mutex); 

				if (chatLineNum > 19) {  
					clearUpperScreen();
					chatLineNum = 0;
				}
				fbputs(buf, chatLineNum++, 0);
				if (strlen(buf) > 64) {
					chatLineNum++;
				}
				pthread_mutex_unlock(&mutex);

				//write to network
				insertChar(msg, cursor, '\n');
				if  ( write(sockfd, &msg, strlen(msg) + 1) != strlen(msg) + 1 ) {
					printf("Problem sending to server...\n");
					break;
				}
				else { //Sent to server. nice :)
					//update the input
					cursor = 0;
					memset( msg, 0, BUFFER_SIZE);
					clearLowerScreen();
					fbputchar('_', 22, 0);
				}
			}
			else { //update the string and cursor (only key1 or key0 changed)

				int key_to_write = 0;
				if (packet.keycode[0] != key0_old)
					key_to_write = packet.keycode[0];
				else if (packet.keycode[1] != key1_old)
					key_to_write = packet.keycode[1];

				//translate function updates cursor and msg
				translate(packet.modifiers, key_to_write, &cursor, msg);

				clearLowerScreen();
				fbputs(msg, 22, 0);

				if ( cursor < 64) 
					fbputchar('_', 22, cursor);
				else 
					fbputchar('_', 23, cursor - 64);
			}

			key0_old = packet.keycode[0];
			key1_old = packet.keycode[1];

		}

	}

	/* Terminate the network thread */
	pthread_cancel(network_thread);

	/* Wait for the network thread to finish */
	pthread_join(network_thread, NULL);

	return 0;
}

void *network_thread_f(void *ptr)
{
	char recvBuf[BUFFER_SIZE];
	int n;
	int *lineNum = (int *)ptr;
	/* Receive data */
	while ( (n = read(sockfd, &recvBuf, BUFFER_SIZE - 1)) > 0 ) { 
		if ( n > 1 && recvBuf[n-2] == '\r' )
			recvBuf[n-2] = 0;
		if ( recvBuf[n-1] == '\n' )
			recvBuf[n-1] = 0;
		if ( strlen(recvBuf) > 0 && recvBuf[strlen(recvBuf)-1] == '\n' )
			recvBuf[strlen(recvBuf)-1] = 0;

		pthread_mutex_lock(&mutex); 
		if ((*lineNum) > 19) {  
			clearUpperScreen();
			(*lineNum) = 0;
		}
		recvBuf[n] = '\0';
		printf("%s", recvBuf);
		fbputs(recvBuf, (*lineNum)++, 0);
		if (strlen(recvBuf) > 64)
			(*lineNum)++;
		pthread_mutex_unlock(&mutex);

	}

	return NULL;
}

