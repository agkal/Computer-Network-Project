#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>

void programUtility(int rec_sokt);
int recFromsendr(int rec_sokt,  struct sockaddr_in recv_adres, socklen_t     skt_len);
/*identifier for ending*/
char *END_FLAG = "~~~~~~flag value for ending~~~~";

/*this is the structure for the packet that contains 3 things:
	1)	sequence number.
	2)	data
	3)	payload
*/
struct struct_pkt{
	int sequenceNo; //shows Sequence number for struct_pkt
	char data[500]; //Data of size 500
	int payLoadSIze; //total payload size
};

int main(int argc, char **argv)
{
        int rec_sokt;
        /* Create server socket */
        rec_sokt = socket(AF_INET, SOCK_DGRAM, 0);
	
	//program utility function calling.	
	programUtility(rec_sokt);

          return 0;
}	//ending main function

/*this funciton receives input integer value of 
receiver socket as parameter and returns nothing. */
void programUtility(int rec_sokt){
	
	int dataRemaining;
	socklen_t     skt_len;		//socket length variable
        ssize_t ssize;			//socket size variable
        struct sockaddr_in      recv_adres;
        int rcv_data_byte = 0;		
     
	int fileDescriptr;
     
        struct struct_pkt pkts_structVar[5];
        struct struct_pkt acks_structVar[5];
 
        int NAcks;
        int totalACKs;
        int size_send;
	int counter;
	printf("open to receive....\n\n");
        // appending Zero to recv_adres struct
        memset(&recv_adres, 0, sizeof(recv_adres));
        // making recv_adres structure
		bzero(&recv_adres, sizeof(recv_adres));
        recv_adres.sin_family = AF_INET;
        recv_adres.sin_addr.s_addr=htonl(INADDR_ANY);
        recv_adres.sin_port = htons(29314);	//here port number used is 29314
        skt_len = sizeof(struct sockaddr_in);

        // Binding receiver socket
        bind(rec_sokt, (struct sockaddr *)&recv_adres, sizeof(struct sockaddr));

        //opening file if not exist otherwise create new file.
        fileDescriptr = open("audioreceive.mp3", O_RDWR | O_CREAT, 0666);
        
	//calling function to receiving from sender.
	dataRemaining = recFromsendr(rec_sokt, recv_adres,skt_len);

        // Creating struct_pkt for data receiving
        struct struct_pkt pkt;
        totalACKs = 0;
	        
	while(ssize != -1 && dataRemaining > 0)
		{	
            label1:
           		counter = 0;	
			while(counter < 5 ){		
	ssize = recvfrom(rec_sokt, &pkt, sizeof(struct struct_pkt), 0, (struct sockaddr *)&recv_adres , &skt_len);
                if (ssize > 0){
					if (strcmp(pkt.data, END_FLAG) == 0){
		        		ssize = -1;
                        break;
		    		}
                     		pkts_structVar[pkt.sequenceNo - 1] = pkt;
		    	}
			counter++;
		    }
            
            NAcks = 0;
		    	counter = 0;
			while(counter < 5){
		    		strcpy(acks_structVar[counter].data, "Ack");
		    		acks_structVar[counter].sequenceNo = counter + 1;
		    		size_send = sendto(rec_sokt, &acks_structVar[counter], sizeof(acks_structVar[counter]), 0,  (struct sockaddr *) &recv_adres, skt_len);
                    if (size_send > 0){
                        NAcks++;
                        totalACKs++;
                        printf("Ack no: %d sent successfully..\n", acks_structVar[counter].sequenceNo);
                    }
			counter++;
		    }
            
            if (NAcks < 5){
                goto label1;
            }
			counter = 0;                	
			while(counter < 5 && NAcks == 5 ){
		/* Check for data into struct_pkt */
				if (pkts_structVar[counter].payLoadSIze != 0){
					printf("Writing packet no: %d\n", pkts_structVar[counter].sequenceNo);
					write(fileDescriptr, pkts_structVar[counter].data, pkts_structVar[counter].payLoadSIze);
                    rcv_data_byte += pkts_structVar[counter].payLoadSIze;
                    dataRemaining -= pkts_structVar[counter].payLoadSIze;
				}
				counter++;
			}
            
            /* append zeros to pkts_structVar */
            memset(pkts_structVar, 0, sizeof(pkts_structVar));
            
            printf("overall Acks used are: %d\n", totalACKs);
            printf("overall bytes received are: %d\nRemaining data: %d bytes\n", rcv_data_byte, dataRemaining);
        }
	
        close(rec_sokt);
        close(fileDescriptr);
	

return; 
}

//this function returns the integer value of dataRemaining
int recFromsendr(int rec_sokt,  struct sockaddr_in recv_adres, socklen_t     skt_len){
	   int dataRemaining;
	   off_t size_file;
       struct timeval Value_time;
		Value_time.tv_sec = 0;
		Value_time.tv_usec = 1000000;

// Receive size_file from sender
        recvfrom(rec_sokt, &size_file, sizeof(off_t), 0, (struct sockaddr *)&recv_adres , &skt_len);
		dataRemaining = size_file;
        printf("Size of File: %ld\n", size_file);
        
        // Setting timeout for sender_socket
	/*setsockopt() function used in setting options that are related with a socket. now these Options can exist at multiple 	levels of protocol*/
		setsockopt(rec_sokt, SOL_SOCKET, SO_RCVTIMEO, &Value_time, sizeof(Value_time));

return	dataRemaining;
        
};


