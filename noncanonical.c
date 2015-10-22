/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define F 0x7e
#define EM_ADDR 0x03
#define REC_ADDR 0x01
#define EM_CONTROL 0x03
#define REC_CONTROL 0x07
#define EM_BCC EM_ADDR^EM_CONTROL
#define REC_BCC REC_ADDR^REC_CONTROL


volatile int STOP=FALSE;
////asdjasklfhaksf

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS4", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");


	//STATE MACHINE
	int state=0;	
	char chRead;
	unsigned char set[5];
	while(state<5){
		read(fd,&chRead,1);
		switch(state){
			case 0:
				if(chRead==F)
					state++;
				printf("1st Flag received\n");
				break;
			case 1:
				if(chRead==EM_ADDR){
					state++;
					printf("ADDRESS received\n");
				}
				else if(chRead!=F){
					state=0;
					printf("Back to state 0\n");
				}					
				break;
			case 2:
				if(chRead==EM_CONTROL){
					state++;
					printf("Control received\n");	
				}
				else if(chRead==F){
					state--;
					printf("1st Flag received\n");
				}
				else{
					state=0;
					printf("Back to state 0\n");
				}
				
				break;
			case 3:
				if(chRead==EM_BCC){
					state++;
					printf("BCC received\n");
				}
				else if(chRead==F){
					state=1;
					printf("1st Flag received\n");
				}
				else{
					state=0;
					printf("Back to state 0\n");
				}
				break;
			case 4:
				if(chRead==F){
					state++;
					printf("Last Flag received\n");
				}
				else{
					state=0;
					printf("Back to state 0\n");
				}
				break;
		} 
	}
		
	printf("SET received\n");
	unsigned char ua[5];
	ua[0]=F;
	ua[1]=REC_ADDR;
	ua[2]=REC_CONTROL;
	ua[3]=REC_BCC;
	ua[4]=F;
	write(fd,ua,5);
	printf("UA sent\n");
	sleep(2);
	
   /* while (STOP==FALSE) { 
	if(i==4) STOP=TRUE;      
      res = read(fd,buf,1);               
	SET[i]=buf[0];
	i++;
    }*/

	//printf("Values read from SET: %X, %X, %X, %X, %X",SET[0],SET[1],SET[2],SET[3],SET[4]);

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}

