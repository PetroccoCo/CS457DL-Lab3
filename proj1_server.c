
/*
CS457DL Project 1
Tapestry Group Submission
14Feb2012
*/


/* This is the server program portion of the CS457DL Lab/Programming Assignnment 1 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>

#define MAXPENDING 10           // how many pending connections queue will hold

uint32_t inputNumber;           /* the command line -x value from the client */
char connectionType[4];         /* are we using tcp or udp */
int portNumber;                 /* port number for connection */

/* align the struct elements on 1 byte boundaries */
#pragma pack(push,1)
struct msgStruct
{
    uint8_t version;
    uint32_t intValue;
} msg;
#pragma pack(pop)

void
DieWithError (char *errorMessage)
{
    perror (errorMessage);
    exit (1);
}

void
doUDPServer ()
{
    int sock;                   /* Socket descriptor */
    struct sockaddr_in myServAddr;      /* Local address */
    struct sockaddr_in myClntAddr;      /* Client address */
    unsigned short myServPort;  /* Server port */
    unsigned int clntLen;       /* Length of client address data structure */
    int recvMsgSize;            /* size of message received */

    /* Create socket for sending and receiving datagrams */
    if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError ("socket() failed");

    /* Fill address structure */
    memset (&myServAddr, 0, sizeof (myServAddr));       /* Zero out structure */
    myServAddr.sin_family = AF_INET;    /* Internet address family */
    myServAddr.sin_addr.s_addr = htonl (INADDR_ANY);    /* Any incoming interface */
    myServAddr.sin_port = htons (portNumber);   /* Local port */

    /* Bind to the local address */
    if (bind (sock, (struct sockaddr *) &myServAddr, sizeof (myServAddr)) < 0)
        DieWithError ("bind() failed");

    for (;;)
      {
          /* Set the size of the parameter */
          clntLen = sizeof (myClntAddr);

          /* Block until receive message from a client */
          if ((recvMsgSize = recvfrom (sock, &msg, sizeof (msg), 0, (struct sockaddr *) &myClntAddr, &clntLen)) < 0)
              DieWithError ("recvfrom() failed");

          /* message received */
          msg.intValue = ntohl (msg.intValue);
          char *clientName = inet_ntoa (myClntAddr.sin_addr);
          if (clientName == NULL)
              DieWithError ("inet_ntoa() failed");
          printf ("Received message from client %s\n", clientName);
          printf ("Integer value: %u\n", msg.intValue);

          /* Send reply message datagram back to the client */
          if (sendto (sock, &msg, 1, 0, (struct sockaddr *) &myClntAddr, sizeof (myClntAddr)) != 1)
              DieWithError ("sendto() sent a different number of bytes than expected");
      }
}

void
ProcessTCPClient (int clientSocket)
{
    int recvMsgSize;            /* Size of received message */

    /* Receive message from client */
    if ((recvMsgSize = recv (clientSocket, &msg, sizeof (msg), 0)) < 0)
        DieWithError ("recv() failed");

    msg.intValue = ntohl (msg.intValue);
    fprintf (stdout, "The number is: %u \n", msg.intValue);

    /* test timeout requirement */
    //    sleep(4);

    /* Send message back to client (REPLY) */
    if (send (clientSocket, &msg, 1, 0) != 1)
        DieWithError ("send() failed");

    /* Close client socket */
    if (close (clientSocket) != 0)
        DieWithError ("close() failed");
}

void
doTCPServer ()
{

    int serverSocket;           /* Socket descriptor for server */
    int clientSocket;           /* Socket descriptor for client */
    struct sockaddr_in myServAddr;      /* Local address */
    struct sockaddr_in myClntAddr;      /* Client address */
    unsigned short myServPort;  /* Server port */
    unsigned int clntLen;       /* Length of client address data structure */

    /* Create socket for connections */
    if ((serverSocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError ("socket() failed");

    /* Fill address structure */
    memset (&myServAddr, 0, sizeof (myServAddr));       /* Zero out structure */
    myServAddr.sin_family = AF_INET;    /* Internet address family */
    myServAddr.sin_addr.s_addr = htonl (INADDR_ANY);    /* Any incoming interface */
    myServAddr.sin_port = htons (portNumber);   /* Local port */

    /* Bind to the local address */
    if (bind (serverSocket, (struct sockaddr *) &myServAddr, sizeof (myServAddr)) < 0)
        DieWithError ("bind() failed");

    /* set up the socket to listen */
    if (listen (serverSocket, MAXPENDING) < 0)
        DieWithError ("listen() failed");

    for (;;)
      {
          /* Set the size of the parameter */
          clntLen = sizeof (myClntAddr);

          /* Wait for a client to connect */
          if ((clientSocket = accept (serverSocket, (struct sockaddr *) &myClntAddr, &clntLen)) < 0)
              DieWithError ("accept() failed");

          /* Announce that message was received */
          printf ("Received message from client %s\n", inet_ntoa (myClntAddr.sin_addr));

          ProcessTCPClient (clientSocket);
      }
}

void
sigchld_handler (int s)
{
    while (waitpid (-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *
get_in_addr (struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
      {
          return &(((struct sockaddr_in *) sa)->sin_addr);
      }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int
printUsage ()
{
    fprintf (stdout, "Usage: proj1_server -t <udp | tcp> -p <port number>\n");
    return 0;
}

int
main (int argc, char **argv)
{

    int c;
    int i;
    int iMaxPortNumber = 65535;
    char *argValue = NULL;
    int tArgEntered = 0;
    int pArgEntered = 0;

    opterr = 0;

    while ((c = getopt (argc, argv, "t:p:")) != -1)
      {

          argValue = optarg;

          switch (c)
            {
            case 't':

                tArgEntered = 1;

                if (strcmp (argValue, "udp") == 0)
                  {
                      strcpy (connectionType, argValue);
                  }
                else if (strcmp (argValue, "tcp") == 0)
                  {
                      strcpy (connectionType, argValue);
                  }
                else
                  {
                      fprintf (stderr, "Value given for -t argument must be \"udp\" or \"tcp\".\n");
                      printUsage ();
                      exit (1);
                  }
                break;
            case 'p':

                pArgEntered = 1;

                for (i = 0; i < strlen (argValue); i++)
                  {
                      if (i == 0)
                        {
                            if (argValue[i] == '-')
                              {
                                  fprintf (stderr, "Value given for port number cannot be negative.\n");
                                  printUsage ();
                                  exit (1);
                              }
                        }
                      if (isdigit (argValue[i]) == 0)
                        {
                            fprintf (stderr, "Value given for port number must only contain digits 0-9.\n");
                            printUsage ();
                            exit (1);
                        }
                  }
                if ((portNumber = atoi (argValue)) == 0)
                  {
                      fprintf (stderr, "Error returned converting port number argument value to integer. Value given for port number is (%s).\n", argValue);
                      printUsage ();
                      exit (1);
                  }
                if (portNumber > iMaxPortNumber)
                  {
                      fprintf (stderr, "Value given for port number (%s) cannot exceed %i.\n", argValue, iMaxPortNumber);
                      printUsage ();
                      exit (1);
                  }
                break;
            default:
                fprintf (stderr, "Unknown command line option: -%c\n", optopt);
                printUsage ();
                exit (1);
            }
      }

    /* if both command line args were not entered, there is a problem */
    if (tArgEntered == 0)
      {
          fprintf (stderr, "Command line argument '-t <udp | tcp>' is required.\n");
          printUsage ();
          exit (1);
      }
    if (pArgEntered == 0)
      {
          fprintf (stderr, "Command line argument '-p <port number>' is required.\n");
          printUsage ();
          exit (1);
      }

    /* start the actual server portion of the task */
    /* call the appropriate func based on tcp | udp flag value */
    if (strcmp (connectionType, "udp") == 0)
      {
          doUDPServer ();
      }
    else
      {
          doTCPServer ();
      }

    return 0;

}
