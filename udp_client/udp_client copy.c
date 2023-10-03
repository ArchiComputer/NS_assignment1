/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <openssl/md5.h>



#define BUFSIZE 1024
#define MD5_DIGEST_LENGTH 16

/*
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

void send_md5_hash(int sockfd, struct sockaddr_in serveraddr, socklen_t serverlen, char *file_name) {
    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    calculate_md5_hash(file_name, md5_hash);
    char md5_hash_str[MD5_DIGEST_LENGTH * 2 + 1];

    // Convert the binary MD5 hash to a string for sending
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(md5_hash_str + 2 * i, "%02x", md5_hash[i]);
    }
    md5_hash_str[MD5_DIGEST_LENGTH * 2] = '\0';

    // Send the MD5 hash to the server
    sendto(sockfd, md5_hash_str, strlen(md5_hash_str), 0, (const struct sockaddr *)&serveraddr, serverlen);
}



int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
    char receive_buf[BUFSIZE];
    char sending_buf[BUFSIZE];
    char transmit_buf[BUFSIZE];


    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    while(1) {
          /* get a message from the user */
    bzero(buf, BUFSIZE);
    bzero(transmit_buf, BUFSIZE);
    bzero(receive_buf, BUFSIZE);
    printf("Please enter msg: ");
    fgets(buf, BUFSIZE, stdin);

    /* send the message to the server */
    serverlen = sizeof(serveraddr);
    /* n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen); */
    n = sendto(sockfd, buf, strlen(buf), 0, (const struct sockaddr *)&serveraddr, serverlen);
    if (n < 0) 
      error("ERROR in sendto");
    
        // Check if the received message starts with "get "
    if (strncmp(buf, "get ", 4) == 0) {
      printf("entering get\n");
      // Extract the file name from the message
      char *file_name = buf + 4; // Skip the "get " prefix

      // Remove any trailing newline character if present
      char *newline = strchr(file_name, '\n');
      if (newline != NULL) {
        *newline = '\0';
      }
      recvfrom(sockfd, receive_buf, BUFSIZE, 0, &serveraddr, &serverlen);
      printf("received: %s\n", receive_buf);
      int file = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      write(file, receive_buf, strlen(receive_buf));
    }




        // Check if the received message starts with "put "
    if (strncmp(buf, "put ", 4) == 0) {
      // Extract the file name from the message
      char *file_name = buf + 4; // Skip the "put " prefix


      // Remove any trailing newline character if present
      char *newline = strchr(file_name, '\n');
      if (newline != NULL) {
        *newline = '\0';
      }
      /* open the file for reading */
      int file = open(file_name, O_RDONLY);
      //sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
      read(file, transmit_buf, BUFSIZE);
      printf("sending: %s\n", transmit_buf);
      sendto(sockfd, transmit_buf, strlen(transmit_buf), 0, &serveraddr, serverlen);
    }


    /* print the server's reply */
    /* n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen); */
    n = recvfrom(sockfd, buf, BUFSIZE - 1, 0, (struct sockaddr *)&serveraddr, &serverlen);
    if (n < 0) 
      error("ERROR in recvfrom");

    if (strncmp(buf, "READY\n", 6) == 0) {
        // Server is ready to receive the file, send the file name
        char file_name[BUFSIZE];
        printf("Enter the file name to request: ");
        fgets(file_name, BUFSIZE, stdin);
        n = sendto(sockfd, file_name, strlen(file_name), 0, (const struct sockaddr *)&serveraddr, serverlen);
        if (n < 0) 
          error("ERROR in sendto");

        // Receive and save the file
        FILE *file = fopen("received_file", "wb"); // Replace "received_file" with your desired file name
        if (file == NULL) {
            error("ERROR opening file for writing");
        }

        while (1) {
            n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);
            if (n < 0) 
                error("ERROR in recvfrom");
            if (strncmp(buf, "END_OF_FILE\n", 12) == 0) {
                break; // End of file transfer
            }
            fwrite(buf, 1, n, file);
        }

        fclose(file);
        printf("File received and saved as 'received_file'.\n");
    } else {
        printf("Server is not ready to receive the file.\n");
    }

    printf("Echo from server: %s", buf);
    return 0;
  }
}


