1. Type in the following command to compile the code for both the client and server. 


gcc -Wall -o server server.cpp -lstdc++
gcc -Wall -o client client.cpp -lstdc++ 

2. In one terminal window, type the following command and follow the on screen instructions. 

./server 

"file name: " << type in 'filename'.txt, the file containing user name and key.
"server port number: " << type in port number for server side connection. 


3. In another terminal window, type the following command and follow the on screen instructions. 

./client 

"server host name: " << type in the server host name, testing on my computer locally, I would type 'localhost'
"port number: " << Type in the port number associated with the server side. 
"message: " << Type in user that you would like to receive password to. 
