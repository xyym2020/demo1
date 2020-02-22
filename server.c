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
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	struct addrinfo hints, *serv,*p;
	struct sockaddr_storage their_addr;
	struct sockaddr_in my_addr;
	socklen_t addr_size;
	int status, getsock_check;
	int yes = 1;
	int socket_connect, socket_mesg;
	char *message;
	char *name, *name_client;

	memset(&hints, 0, sizeof hints);//clear hints
	hints.ai_family = AF_UNSPEC;//IPv4 or TPv6
	hints.ai_socktype = SOCK_STREAM;//TCP

	//get sockaddr and other detailed information
	//[imitate from Beej's Guide to Network Programmin]
	if((status = getaddrinfo("127.0.0.1", "21243", &hints, &serv))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}

	//made a socket(IPv4 or IPv6, TCP), and bind it to the server sockaddr
	//[copy from Beej's Guide to Network Programming]
	for(p = serv; p !=NULL; p = p->ai_next)
	{
		if((socket_connect = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("socket");
			continue;
		}
		//address the "Address already in use" problem, reuse the port
		if(setsockopt(socket_connect, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
		{
			perror("setsockopt");
			exit(1);
		}
		if(bind(socket_connect, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(socket_connect);
			perror("bind");
			continue;
		}	
		break;
	}
	//check port number
	// addr_size = sizeof my_addr;
	// getsock_check=getsockname(socket_connect,(struct sockaddr*)&my_addr, (socklen_t *)&addr_size);
	// if (getsock_check== -1) { perror("getsockname"); exit(1);}
	// printf("port:%d\n", ntohs(my_addr.sin_port));
	
	//free the linked list
	freeaddrinfo(serv);
	//check if there is a avaliable socket
	if(p == NULL)
	{
		fprintf(stderr, "server:failed to bind\n");
		exit(1);
	}
	//listen to other clients, the max of waiting queue is 20
	if((listen(socket_connect, 20))==-1)
	{
		perror("listen");
		exit(1);
	}
	printf("The server is up and listening\n");

	//loop to accept the request from clients
	while(1)
	{
		//accept a request from one client
		//[copy from Beej's Guide to Network Programming]
		addr_size = sizeof their_addr;
		socket_mesg = accept(socket_connect, (struct sockaddr *)&their_addr, &addr_size);

		//exchange information and notify that on the screen
		int len_send, len_recv, bytes_sent, bytes_recv;
		len_recv = 100;
		char buf[len_recv];
		//receive message
		bytes_recv = recv(socket_mesg, buf, len_recv, 0);//[copy from Beej's Guide to Network Programming]
		if(bytes_recv==-1)
		{
			perror("recv");
		}
		else if(bytes_recv>0)
		{
			int i = 0, j = 0;
			buf[bytes_recv] = '\0';

			//get the name of client from received buf[]
			name = strtok(buf, " ");
			while(name != NULL)
			{
				name_client = name;				
				name = strtok(NULL, " ");
			}
			printf("Receive greetings from %s\n", name_client);
		}

		//send reply
		char msg[50] = "Hi, ";
		strcat(msg, name_client);
		strcat(msg,", this is chatbot ");
	    strcat(msg, argv[1]);

		len_send = strlen(msg);//[copy from Beej's Guide to Network Programming]
		bytes_sent = send(socket_mesg, msg, len_send, 0);//[copy from Beej's Guide to Network Programming]

		if(bytes_sent == -1)
		{
			perror("send");
		}
		else
		{
			printf("Send greetings to %s\n\n", name_client);
		}

		close(socket_mesg);//[copy from Beej's Guide to Network Programming]

	}
	return 0;
}