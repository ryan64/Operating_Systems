#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <queue>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
using namespace std;

queue<string> inputQ;
map<string, string> key;

bool has_suffix(const std::string &str, const std::string &suffix){ //function handles if user inputs name of text file without including '.txt'
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0; }

int main(){
	int portNumber = 0; 

	string currentText;
    string userName;
    string temporaryKey;
    string fileName;

	cout << "File Name: ";
	cin >> fileName;

	if (has_suffix(fileName, ".txt") == 0) {fileName = fileName + ".txt"; } 

	cout << "Server Port Number: ";
	cin >> portNumber;

	ifstream inputFile (fileName.c_str());
	if(inputFile.is_open()){ //reads data into a queue data structure. 
		cout << "File Found. Server is ready for client." << endl;
	    while(getline(inputFile, currentText)) { inputQ.push(currentText); }
	}
	else{
		cout << "File not found. Did you type it in correctly?" << endl;
		return 0; }

	while (!inputQ.empty()){ //takes data from queue and pushes into a map data structure. 
        istringstream iin(inputQ.front());
        iin >> userName;
        iin >> temporaryKey;
        key[userName] = temporaryKey;
        inputQ.pop(); }

    int sockerListener;
    int socketAccept;
    int message;
    char handler[2048];

    socklen_t client_addressLength;
    struct sockaddr_in server_Address;
    struct sockaddr_in client_Address;
    sockerListener = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&server_Address, 0, sizeof(struct sockaddr_in));
    server_Address.sin_family = AF_INET;
    server_Address.sin_addr.s_addr = INADDR_ANY;
    server_Address.sin_port = htons(portNumber);
    if (sockerListener < 0){
 		perror("Error Opening socket");
 		return(-1);
 	}
 	if (bind(sockerListener, (struct sockaddr *) &server_Address ,sizeof(server_Address)) < 0) {
		 close(sockerListener);
		 return(-1);
	}
 	listen(sockerListener, 5);
 	client_addressLength = sizeof(client_Address);
	string clientMessage;

 	while(sockerListener > 0){ //while loop that runs until client specifies that it wants to shut down server. 
		socketAccept = accept(sockerListener, (struct sockaddr *) &client_Address, &client_addressLength);

		if(socketAccept < 0){ cout << "Accept Error" << endl; }
		else{ cout << "Connection Accepted" << endl; }

	 	message = read(socketAccept, handler, sizeof(handler));
	 	if(message < 0){ cout << "Error currentText" << endl; }
	 	else{ cout << "Message: " << handler << endl;}

	 	clientMessage = handler;
	 	strncpy(handler, key[clientMessage].c_str(), sizeof(handler) - 1);
		handler[sizeof(handler) - 1] = 0;
	 	message = write(socketAccept, handler, sizeof(handler));
	 	if(message < 0){ cout << "Error writing" << endl; }
	 	else{ cout << "Message Sent" << endl; }

		close(socketAccept);
		if(clientMessage == "Terminate"){
			close(sockerListener);
			cout<<"Server Terminated" << endl;;
			return 0; }
	} 	
return 0;
}