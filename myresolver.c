/*
CS457DL Project 3
Tapestry Group Submission
4/26/2012
*/


/* This is the client program portion of the CS457DL Lab/Programming Assignnment 3 */

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

uint32_t inputNumber;           /* -x number */
char connectionType[4];         /* -t [udp | tcp] */
int portNumber;                 /* -p portnumber */
char *serverName = NULL;        /* -s servername */
int ipAddressEntered = 1;       /* flag to indicate an IP address was entered as -s arg */

/* align the struct to 1 byte boundaries */
#pragma pack(push,1)
struct messageStruct
{                               /* the message sent to server (packed with #pragma directive) */
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

int
printUsage ()
{
    fprintf (stdout, "Usage: proj1_client -x <number> -t <udp | tcp> -s <hostname> -p <port number>\n");
    return 0;
}

void
doTCPClient ()
{
    int sock;                   /* Socket descriptor */
    struct sockaddr_in myServAddr;      /* server address */
    unsigned short myServPort;  /* server port */
    char *servIP;               /* Server IP address */
    char rcvBuffer;             /* Buffer for reply string */
    int bytesRcvd, totalBytesRcvd;      /* Bytes read in single recv()
                                           and total bytes read */
    struct hostent *hostentStruct;      /* helper struct for gethostbyname() */

    servIP = serverName;

    /* Create a stream socket using TCP */
    if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError ("socket() failed");

    /* establish 3 seconds as the timeout value */
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof tv))
        DieWithError ("setsockopt() failed");

    /* Construct the server address structure */
    memset (&myServAddr, 0, sizeof (myServAddr));       /* Zero out structure */
    myServAddr.sin_family = AF_INET;    /* Internet address family */
    if (ipAddressEntered)
      {
          /* load the address */
          myServAddr.sin_addr.s_addr = inet_addr (servIP);
      }
    else
      {
          /* if a hostname was entered, we need to look for IP address */
          hostentStruct = gethostbyname (serverName);
          if (hostentStruct == NULL)
              DieWithError ("gethostbyname() failed");

          /* make some space for the serverName for the new IP address */
          free (serverName);
          serverName = (char *) malloc (16 * sizeof (char));
          if (serverName == NULL)
            {
                fprintf (stderr, "Malloc error - out of memory.\n");
                exit (1);
            }

          /* get the correct "format" of the IP address */
          serverName = inet_ntoa (*(struct in_addr *) (hostentStruct->h_addr_list[0]));
          if (serverName == NULL)
            {
                fprintf (stderr, "inet_ntoa() failed.\n");
                exit (1);
            }

          /* load the address */
          memcpy (&myServAddr.sin_addr, hostentStruct->h_addr_list[0], hostentStruct->h_length);
      }
    /* load the port */
    myServAddr.sin_port = htons (portNumber);   /* Server port */

    /* Establish the connection to the server */
    if (connect (sock, (struct sockaddr *) &myServAddr, sizeof (myServAddr)) < 0)
        DieWithError ("connect() failed");

    /* Send the string to the server */
    msg.version = 1;
    msg.intValue = htonl (msg.intValue);
    if (send (sock, &msg, sizeof (msg), 0) != sizeof (msg))
        DieWithError ("send() sent a different number of bytes than expected");

    /* let everyone know the number was sent */
    fprintf (stdout, "Sent number (%u) to server %s:%d via TCP\n", inputNumber, serverName, portNumber);

    /* sent the bytes to server, look for reply */

    /* Receive the reply string back from the server */
    totalBytesRcvd = 0;
    printf ("Received: ");      /* Setup to print the string returned from server */
    while (totalBytesRcvd < 1)
      {
          if ((bytesRcvd = recv (sock, &rcvBuffer, 1, 0)) <= 0)
              DieWithError ("recv() failed: (exceeded timeout) or connection closed prematurely");
          totalBytesRcvd += bytesRcvd;  /* Keep tally of total bytes */
          printf ("%u", rcvBuffer);     /* Print the received value */
      }
    printf ("\n");

    if (close (sock) != 0)
        DieWithError ("close() failed");
    fprintf (stderr, "Success - exiting.\n");
    exit (0);
}

void
doUDPClient ()
{
    int sock;                   /* Socket descriptor */
    struct sockaddr_in myServAddr;      /* server address */
    struct sockaddr_in senderAddr;      /* sender address */
    unsigned short myServPort;  /* server port */
    unsigned int addrSize;      /* address size */
    char *servIP;               /* Server IP address */
    char msgBuffer;             /* Buffer for message */
    struct hostent *hostentStruct;      /* helper struct for gethostbyname() */
    int respLen;                /* length of response msg */

    servIP = serverName;

    /* Create a stream socket using TCP */
    if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError ("socket() failed");

    /* establish 3 seconds as the timeout value */
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof tv))
        DieWithError ("setsockopt() failed");

    /* Construct the server address structure */
    memset (&myServAddr, 0, sizeof (myServAddr));       /* Zero out structure */
    myServAddr.sin_family = AF_INET;    /* Internet address family */
    if (ipAddressEntered)
      {
          /* load the address */
          myServAddr.sin_addr.s_addr = inet_addr (servIP);      /* Server IP address */
      }
    else
      {
          /* if a hostname was entered, we need to look for IP address */
          hostentStruct = gethostbyname (serverName);
          if (hostentStruct == NULL)
              DieWithError ("gethostbyname() failed");

          /* make some space for the serverName for the new IP address */
          free (serverName);
          serverName = (char *) malloc (16 * sizeof (char));
          if (serverName == NULL)
            {
                fprintf (stderr, "Malloc error - out of memory.\n");
                exit (1);
            }
          /* get the correct "format" of the IP address */
          serverName = inet_ntoa (*(struct in_addr *) (hostentStruct->h_addr_list[0]));
          if (serverName == NULL)
            {
                fprintf (stderr, "inet_ntoa() failed.\n");
                exit (1);
            }

          /* load the address */
          memcpy (&myServAddr.sin_addr, hostentStruct->h_addr_list[0], hostentStruct->h_length);
      }
    /* load the port */
    myServAddr.sin_port = htons (portNumber);   /* Server port */

    /* Send the message to the server */
    msg.version = 1;
    msg.intValue = htonl (msg.intValue);
    if (sendto (sock, &msg, sizeof (msg), 0, (struct sockaddr *) &myServAddr, sizeof (myServAddr)) != sizeof (msg))
        DieWithError ("sendto() sent a different number of bytes than expected");

    /* let everyone know the number was sent */
    fprintf (stdout, "Sent number (%u) to server %s:%d via UDP\n", inputNumber, serverName, portNumber);

    /* sent the bytes to server, look for reply */

    /* Receive the reply string back from the server */
    addrSize = sizeof (senderAddr);
    if ((respLen = recvfrom (sock, &msg, 1, 0, (struct sockaddr *) &senderAddr, &addrSize)) != 1)
        DieWithError ("recvfrom() failed: (exceeded timeout) or connection closed prematurely");

    if (myServAddr.sin_addr.s_addr != senderAddr.sin_addr.s_addr)
      {
          fprintf (stderr, "Error: received a packet from unknown source.\n");
          exit (1);
      }

    printf ("%u", msg.version); /* Print the received value */
    printf ("\n");

    if (close (sock) != 0)
        DieWithError ("close() failed");

    fprintf (stderr, "Success - exiting.\n");
    exit (0);
}

int
main (int argc, char **argv)
{

    int c;
    int i;
    int iMaxPortNumber = 65535;
    char *argValue = NULL;
    int xArgEntered = 0;
    int tArgEntered = 0;
    int sArgEntered = 0;
    int pArgEntered = 0;

    opterr = 0;

    while ((c = getopt (argc, argv, "x:t:s:p:")) != -1)
      {

          argValue = optarg;

          switch (c)
            {
            case 'x':

                xArgEntered = 1;

                /* make sure this is an unsigned int less than 2^32 */
                for (i = 0; i < strlen (argValue); i++)
                  {
                      if (i == 0)
                        {
                            if (argValue[i] == '-')
                              {
                                  fprintf (stderr, "Value given for number cannot be negative.\n");
                                  printUsage ();
                                  exit (1);
                              }
                        }
                      if (isdigit (argValue[i]) == 0)
                        {
                            fprintf (stderr, "Value given for number must only contain digits 0-9.\n");
                            printUsage ();
                            exit (1);
                        }
                  }
#if 1
                if (strcmp ("0", argValue) == 0)
                  {
                      inputNumber = 0;
                  }
                else
                  {
				      uint64_t tmpNumber = atol(argValue);
					  if (tmpNumber < 0 || tmpNumber > UINT_MAX) 
					    {
						    fprintf(stderr, "Option -x requires a number between 0 and %u.\n", UINT_MAX);
						    exit(1); 
						}
		
                      if ((inputNumber = atoi (argValue)) == 0)
                        {
                            fprintf (stderr, "Error returned from atoi(). Value given for number (%s). Valid range is 0-4294967295\n", argValue);
                            exit (1);
                        }
                  }
#else
                inputNumber = strtol (argValue, NULL, 10);
#endif

                msg.intValue = inputNumber;
                break;
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
            case 's':

                sArgEntered = 1;

                /* ensure each character isalnum or a period */
                for (i = 0; i < strlen (argValue); i++)
                  {
                      if (!isalnum (argValue[i]))
                        {
                            if (argValue[i] != '.')
                              {
                                  fprintf (stderr, "Value given for server hostname (%s) is invalid.\n", argValue);
                                  printUsage ();
                                  exit (1);
                              }
                        }
                  }
                /* if all digits and periods, then IP address was entered */
                for (i = 0; i < strlen (argValue); i++)
                  {
                      if (argValue[i] != '.')
                        {
                            if (isdigit (argValue[i]) == 0)
                              {
                                  ipAddressEntered = 0;
                              }
                        }
                  }
                //      fprintf(stderr, "ipAddressEntered = %i\n",ipAddressEntered);

                /* make some space for the serverName */
                serverName = (char *) malloc ((strlen (argValue) + 1) * sizeof (char));
                if (serverName == NULL)
                  {
                      fprintf (stderr, "Malloc error - out of memory.\n");
                      printUsage ();
                      exit (1);
                  }
                strcpy (serverName, argValue);
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

    /* if all 4 command line args were not entered, there is a problem */

    if (xArgEntered == 0)
      {
          fprintf (stderr, "Command line argument '-x <number>' is required.\n");
          printUsage ();
          exit (1);
      }
    if (tArgEntered == 0)
      {
          fprintf (stderr, "Command line argument '-t <udp | tcp>' is required.\n");
          printUsage ();
          exit (1);
      }
    if (sArgEntered == 0)
      {
          fprintf (stderr, "Command line argument '-s hostname' is required.\n");
          printUsage ();
          exit (1);
      }
    if (pArgEntered == 0)
      {
          fprintf (stderr, "Command line argument '-p <port number>' is required.\n");
          printUsage ();
          exit (1);
      }

    /* start the actual client portion of the task */
    /* call the appropriate function based on tcp | udp value of flag */
    if (strcmp (connectionType, "udp") == 0)
      {
          doUDPClient ();
      }
    else
      {
          doTCPClient ();
      }

    return 0;
}
