/***********************
   This is the implementation of the client applicaion that uses,
   Unix domain stream sockets for Inter Process Communication (IPC) with a server application. 
   This client application sends the sequence of integers to the server that needs to be verified whether its the sequence of prime fibonacci numbers of provided length !
   
***********************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

#define SERVER_SOCKET_PATH "fibonacci_server"

/******Main entry point of the application ***********/
int main(int argc, char *argv[])
{
	
	/*We initialize the sockaddr_un struct that will contain the address of the server */

	struct sockaddr_un serv_addr;

	/*At first we set everything to zero. */
	memset(&serv_addr, 0x00, sizeof(struct sockaddr_un));

	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, SERVER_SOCKET_PATH, strlen(SERVER_SOCKET_PATH));

	/*First we create a socket object. */
	int serv_sock = socket(AF_UNIX, SOCK_STREAM, 0);

	if (serv_sock == -1)
	{
		perror("ERROR creating UNIX family socket ");
		return 1;
	}

	/*We attempt to connect to our server... */
	printf("\n[*]Connecting to server that has socket path: '%s'\n", SERVER_SOCKET_PATH);

	if (connect(serv_sock, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_un)) == -1)
	{
		perror("[!]Cannot connect to server ");
		return 2;
	}

	/*Inform the user that we have successfully connected to our server. */
	printf("[*]Successfully connected to server... \n");

	/*We then send the user provided sequence of integers to the server for processing... This is done until the user confirms to stop. */

	int num = 0;

	/*The user provided sequence of integers that needs to get verified by the server process. */
	int *user_seq = NULL;

	while (1)
	{
		printf("\n##### VERIFY SEQUENCE IS THE FIRST 'n' fibonacci numbers #####\n");
		printf("\n\nEnter the number of fibonacci numbers you want to calculate (0 to exit application): ");
		scanf("%d", &num);

		/*Inform the server the number of integers will be sent. */
		if (write(serv_sock, &num, sizeof(int)) < 0)
		{
			perror("write() ");
			break;
		}

		if (num == 0)
		{
			/*User wants to exit ! */
			break;
		}

		/*We allocate space on the heap.*/
		user_seq = (int *)malloc(num * sizeof(int));

		printf("\n");

		/*We read the sequence integer by integer from the client and fill the allocated array. */
		for (int i = 0; i < num; i++)
		{
			printf("Enter integer #%d: ", (i + 1));
			scanf("%d", &user_seq[i]);
		}

		char buff[32];
		memset(buff, 0x00, 32);

		/*We wait for the server to send the "SEND" response, so that we can send the sequence to the server*/

		if (read(serv_sock, buff, 32) < 0)
		{
			perror("read() ");
			break;
		}

		if (strcmp(buff, "SEND") != 0)
		{
			/*An error occured with the communication */
			printf("\n[!]ERROR in communication with server ! \n");
			break;
		}

        printf("\n[*]Sending sequence of %d integers to server for verification... \n", num); 
		/*We send the sequence to the server in one shot. */
		if (write(serv_sock, user_seq, num * sizeof(int)) < 0)
		{
			perror("write() ");
			break;
		}

		/*We read the response string from the server and print it out to terminal. */
		memset(buff, 0x00, 32);

		if (read(serv_sock, buff, 32) < 0)
		{
			perror("read() ");
			break;
		}
		
		/*Now we print the servers response on the client application's terminal*/
		printf("\n\n[*]Response from server: \"%s\"\n", buff); 
		

		/*Free heap allocated memory every iteration (if allocated) */
		if (user_seq != NULL)
		{
			free(user_seq);
			user_seq = NULL;
			num = 0;
		}
	}

	printf("\n# Thank you for using the client application #\n");

	/*We close the server socket*/
	close(serv_sock);
	return 0;
}
