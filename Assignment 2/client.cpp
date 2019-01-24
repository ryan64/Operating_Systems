#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
using namespace std;

int main(){
	int portNumber;
	int socket_systemCall;
	int read_message;
	int write_message;
	char host_name[256];
	char handler[2048];
	struct sockaddr_in server_Addr;
	struct hostent *server;

	while (true) {
		 cout << "Server Host Name: "; 
		 cin >> host_name;
		 if((server = gethostbyname(host_name)) != NULL){
			 break;
		 }
 	}

	cout << "Port Number: ";
	cin >> portNumber; 

	memset(&server_Addr,0,sizeof(server_Addr));
	memcpy( (char *) &server_Addr.sin_addr, server->h_addr, server->h_length);
	server_Addr.sin_family = server->h_addrtype;
	server_Addr.sin_port = htons(portNumber);

	if ((socket_systemCall = socket(server->h_addrtype, SOCK_STREAM, 0) ) < 0) { return(-1); }
	if (connect(socket_systemCall, (struct sockaddr *) &server_Addr, sizeof(server_Addr)) < 0) {
		perror("Error connecting"); 	
	 	return(-1);
	}

	bool nextIn = true;
	if(nextIn != false){
		cout << "Enter Username: ";
		cin >> handler;
		string userName = handler;
		if(handler == "Terminate"){
			write_message = write(socket_systemCall, handler, sizeof(handler));
			if (write_message < 0){ perror("Error: writing to socket"); }
			nextIn = false;
		}
		else{
			write_message = write(socket_systemCall, handler, sizeof(handler));
			if (write_message < 0) { perror("Error: writing to socket"); }

		    read_message = read(socket_systemCall, handler, sizeof(handler));
			if (read_message < 0) { perror("Error: writing to socket"); }

		    cout << "Public Key For " << userName << ": " << handler << endl;
		}
	}
	close(socket_systemCall); 	
}

