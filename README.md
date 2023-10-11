# NS_assignment1

Working Functions

 - get[file_name]
    1. complie and run server and client using "make all" in NS_assignment directory folder.
    2. change directory using "cd udp_client" for running client, and then type "./client [hostname or IP.address] [port number]".
    3. change directory using "cd udp_server" for running server, and then type "./server [port number]".
    4. type "get[file_name]" after "Please enter msg", then you can get the file in udp_client directory folder from udp_server directory folder

 - put[file_name]
    1. complie and run server and client using "make all" in NS_assignment directory folder.
    2. change directory using "cd udp_client" for running client, and then type "./client [hostname or IP.address] [port number]".
    3. change directory using "cd udp_server" for running server, and then type "./server [port number]".
    4. type "put[file_name]" after "Please enter msg", then you can send the file from udp_client directory folder to udp_server directory folder
    
 - delete[file_name]
    1. complie and run server and client using "make all" in NS_assignment directory folder.
    2. change directory using "cd udp_client" for running client, and then type "./client [hostname or IP.address] [port number]".
    3. change directory using "cd udp_server" for running server, and then type "./server [port number]".
    4. type "delete[file_name]" after "Please enter msg", then you can delete [file_name] in the udp_server directory folder

 - ls
    1. complie and run server and client using "make all" in NS_assignment directory folder.
    2. change directory using "cd udp_client" for running client, and then type "./client [hostname or IP.address] [port number]".
    3. change directory using "cd udp_server" for running server, and then type "./server [port number]".
    4. type "ls" after "Please enter msg", then you can get the list of files in the udp_server directory folder

 - exit
    1. complie and run server and client using "make all" in NS_assignment directory folder.
    2. change directory using "cd udp_client" for running client, and then type "./client [hostname or IP.address] [port number]".
    3. change directory using "cd udp_server" for running server, and then type "./server [port number]".
    4. type "exit" after "Please enter msg", then you can shut down the server.


Partially Working Function(working on udp_client copy.c and udp_server copy.c)

 - Implementing a re-send request if the MD5 calculation doesn't match between the client and server.


 