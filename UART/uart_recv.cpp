#include <wiringPi.h>
#include <wiringSerial.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZE 1024

int main(int argc, char** argv)
{
	int fd;
	int nRec = 0;
	char buf[SIZE];

	/* PINS configuration */

	// pinMode (int pin, int mode) ; wiringpi pins numbers ==> view wiringPi pins table

	//pinMode (15 , OUTPUT); //TxD UART
	//pinMode (16 , INPUT); //RxD UART
	// ...

	//Initialization
	if (wiringPiSetup() < 0)
		perror("WiringPiSetup problem \n ");	
		
	if ((fd = serialOpen("/dev/ttyAMA0", 57600)) < 0)
		//57600 is the Baud used
		perror("device not opened \n");

	bzero(buf, SIZE);
	while (true) {
        nRec = read(fd, buf, SIZE);
        if (nRec == -1) {
            perror("Read Data Error!\n");
            break;
        }
        if (nRec > 0) {
			printf("Len %d\n", nRec);
            buf[nRec] = '\0';
            printf("Recv Data: %s\n", buf);
        }
		//sleep(1);
	}

	return 0;
}
