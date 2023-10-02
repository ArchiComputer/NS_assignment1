/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  int sockfd; /* socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    // bzero(buf, BUFSIZE);
    memset(buf, 0, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");

    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", 
	   hostp->h_name, hostaddrp);
    printf("server received %zu/%d bytes: %s\n", strlen(buf), n, buf);

    // Check if the received message starts with "get "
    if (strncmp(buf, "get ", 4) == 0) {
      // Extract the file name from the message
      char *file_name = buf + 4; // Skip the "get " prefix

      // Remove any trailing newline character if present
      char *newline = strchr(file_name, '\n');
      if (newline != NULL) {
        *newline = '\0';
      }

      // Open the requested file for reading
      FILE *file = fopen(file_name, "rb");
      if (file == NULL) {
        printf("File not found or unable to open: %s\n", file_name);

        // Send an error message to the client
        n = sendto(sockfd, "File not found or unable to open.\n", 38, 0, 
                  (struct sockaddr *) &clientaddr, clientlen);
        if (n < 0) 
          error("ERROR in sendto");
      } else {
        // Send the file in small chunks
        char chunk[5120]; // 5KB chunk size
        size_t read_size;

        while ((read_size = fread(chunk, 1, sizeof(chunk), file)) > 0) {
          n = sendto(sockfd, chunk, read_size, 0, 
                    (struct sockaddr *) &clientaddr, clientlen);
          if (n < 0) 
            error("ERROR in sendto");
        }

        // Close the file
        fclose(file);

        printf("Sent file: %s\n", file_name);
      }
    }


    // Check if the received message starts with "delete "
    if (strncmp(buf, "delete ", 7) == 0) {
      // Extract the file name from the message
      char *file_name = buf + 7; // Skip the "delete " prefix

      // Remove any trailing newline character if present
      char *newline = strchr(file_name, '\n');
      if (newline != NULL) {
        *newline = '\0';
      }

      // Attempt to delete the file
      if (remove(file_name) == 0) {
        printf("Deleted file: %s\n", file_name);

        // Send a success message to the client
        n = sendto(sockfd, "File deleted successfully.\n", 27, 0, 
                  (struct sockaddr *) &clientaddr, clientlen);
        if (n < 0) 
          error("ERROR in sendto");
      } else {
        printf("File not found or unable to delete: %s\n", file_name);

        // Send an error message to the client
        n = sendto(sockfd, "File not found or unable to delete.\n", 38, 0, 
                  (struct sockaddr *) &clientaddr, clientlen);
        if (n < 0) 
          error("ERROR in sendto");
      }
    }

    // Check if the received message is "ls"
    if (strcmp(buf, "ls\n") == 0) {
      printf("Received 'ls' message. Sending file list to client...\n");

      // Execute the 'ls' command and capture the output
      FILE *ls_output = popen("ls", "r");
      if (ls_output == NULL) {
        error("ERROR executing 'ls'");
      }

      // Read the output of 'ls' and send it to the client
      while (fgets(buf, BUFSIZE, ls_output) != NULL) {
        n = sendto(sockfd, buf, strlen(buf), 0,
          (struct sockaddr *) &clientaddr, clientlen);
        if (n < 0)
          error("ERROR in sendto");
      }

      // Close the 'ls' output and send an end-of-list marker
      pclose(ls_output);
      n = sendto(sockfd, "END_OF_LIST\n", 12, 0,
                (struct sockaddr *) &clientaddr, clientlen);
      if (n < 0)
        error("ERROR in sendto");

      printf("File list sent.\n");
    }

     
    // Check if the received message is "exit"
    if (strcmp(buf, "exit\n") == 0) {
      printf("Received 'exit' message. Server is shutting down.\n");

      
      n = sendto(sockfd, buf, strlen(buf), 0, 
          (struct sockaddr *) &clientaddr, clientlen);
          // 수정사항, BUF에 정상종료 되었습니다.
      break; // Exit the loop and close the server
    
    // IF 문 동일 ls\n 이면 > 리스트를 확인하는 명령어 > buf에 넣고, n 해주기
    // DELETE -> 파일을 삭제하는 처리로직 -> 처리 완료되었습니다 MSG -> BUF
    // GET / PUT 


    if (n < 0) 
      error("ERROR in sendto");
    }
  }
}









