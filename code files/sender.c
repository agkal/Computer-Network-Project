
#include <stdio.h>
#include <time.h>
#include <sys/types.h>

#include <stdlib.h>
#include <errno.h>

#include <fcntl.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/time.h>
#include <sys/sendfile.h>

#include <arpa/inet.h>
#include <sys/types.h>

//function declaration
void programUtility(int rec_sokt);

//packat structure
struct struct_pkt{
	int sequenceNo; //Sequence number of the packet
	char data[500]; //Data of packet in bytes
	int payLoadSIze; //Size of payload in data variable
};


/* Ending identifier */
char *END_FLAG = "~~~~~~flag value for ending~~~~";

//main function
int main(int argc, char **argv)
{
        int send_sokt;
        /* Creating the sender socket */
        send_sokt = socket(AF_INET, SOCK_DGRAM, 0);
	//calling function program Utility.
	programUtility(send_sokt);
        return 0;
}
void programUtility(int send_sokt){
	
	ssize_t ssize;//p
        struct sockaddr_in      sendAdrs;//p
        int fileDescriptr;//p


	//To specify a time in nano-seconds and seconds timespec structure is used. 
        struct timespec tSpecVar1, tSpecVar2;//.

        tSpecVar1.tv_sec = 0;//.
        tSpecVar1.tv_nsec = 500000L;

        int ACK;//.
        int arrayAck[5];//.

	int tempInt;//p
        socklen_t     sock_length;//p
        
        off_t size_file;
        struct stat statFile;
        struct struct_pkt pkts_structVar[5];
        struct timeval Value_time;
		Value_time.tv_sec = 0;
		Value_time.tv_usec = 1000000;

        int pkt_countVar;
	int counter = 0;
	
        /* assigning the sendAdrs struct to zero */
        memset(&sendAdrs, 0, sizeof(sendAdrs));
        /* Constructing sendAdrs struct */
		bzero(&sendAdrs, sizeof(sendAdrs));
        sendAdrs.sin_family = AF_INET;
        inet_pton(AF_INET, "127.0.0.1", &sendAdrs.sin_addr);

        sendAdrs.sin_port = htons(29314);
        sock_length = sizeof(struct sockaddr_in);
        
        /* Opening audio file as read-only*/
        fileDescriptr = open("audiosend.mp3", O_RDONLY);
        
        /* Getting the statistics of file */
        fstat(fileDescriptr, &statFile);
        
        /* Printing payLoadSIze of file to be sent*/
        fprintf(stdout, "File Size: \n%ld bytes\n", statFile.st_size);
        
        /* Sending file payLoadSIze to receiver */
        size_file = statFile.st_size;
        sendto(send_sokt, &size_file, sizeof(off_t), 0, (struct sockaddr *) &sendAdrs, sock_length);
        
        /* Setting timeout for send_sokt */
		setsockopt(send_sokt, SOL_SOCKET, SO_RCVTIMEO, &Value_time, sizeof(Value_time));
        
        /* Packet defined for receiving Acks */
        struct struct_pkt pkt;
        
        /* Reading file data into pkts_structVar in a buffer array */
		tempInt = 1;
        ssize = 1;
        pkt_countVar = 0;
    	while ( counter < 5){
        	ssize = read(fileDescriptr, pkts_structVar[counter].data, 500);
        	pkts_structVar[counter].sequenceNo = tempInt;
        	pkts_structVar[counter].payLoadSIze = ssize;
        	tempInt += 1;
            pkt_countVar++;
		counter++;
    	}
        ////////////////////////////////////////to use func here///////////////
        /* while loop for sending data receiving Acks and reading further data */
        while (ssize > 0 && tempInt > 1) 
        {
            /* Resend tag */
            RESEND:
            counter = 0;
            /* Loop for sending pkts_structVar in the buffer array */
        	while ( counter < 5){
                /* Check if ACK for that struct_pkt is received or not */
                if (arrayAck[counter]== 0){
                    printf("Sending packet no: %d\n", pkts_structVar[counter].sequenceNo);
                    sendto(send_sokt, &pkts_structVar[counter], sizeof(struct struct_pkt), 0, (struct sockaddr *) &sendAdrs, sock_length);
                    nanosleep(&tSpecVar1, &tSpecVar2);
                }
	counter++;
            }
            printf("\n-----------------------------------------------------\n");
	counter = 0;
            /* Loop for receiving Acks */
            while (counter < 5){
            	ACK = recvfrom(send_sokt, &pkt, sizeof(struct struct_pkt), 0, (struct sockaddr *)&sendAdrs , &sock_length);
                if (strcmp(pkt.data, "Ack") == 0 && ACK > 0){
            		printf("Ack number received: %d\n", pkt.sequenceNo);
                    arrayAck[pkt.sequenceNo - 1] = 1;
            	}else{
                    /* If Ack is not received */
                    goto RESEND;
                }
		counter++;
            }
            
		printf("\n-----------------------------------------------------\n");
            /* Zeroing array of Acks */
            memset(arrayAck, 0, sizeof(arrayAck));
            
            /* Zeroing array of pkts_structVar */
            memset(pkts_structVar, 0, sizeof(pkts_structVar));
            
            /* Loop for reading next 5 pkts_structVar */
            tempInt = 1;
		counter = 0;
    		while (counter < 5){
        		ssize = read(fileDescriptr, pkts_structVar[counter].data, 500);
                pkts_structVar[counter].sequenceNo = tempInt;
                pkts_structVar[counter].payLoadSIze = ssize;
                tempInt += 1;
                if (ssize > 0){
                    pkt_countVar++;
                }
		counter++;
    		}
   		}
        printf("Total packets: %d\n", pkt_countVar);
        /* Sending the end flag  */
   		struct struct_pkt end_pack;
        strcpy(end_pack.data, END_FLAG);
   		printf("Sending the end flag\n");
		sendto(send_sokt, &end_pack, sizeof(struct struct_pkt), 0,  (struct sockaddr *) &sendAdrs, sock_length);
        
        /* Closing the socket */
        close(send_sokt);

return;
}





