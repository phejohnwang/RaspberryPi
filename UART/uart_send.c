#include <wiringPi.h>
#include <wiringSerial.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	int fd;
	char buf[1024] = "Pheno!";

	//Initialization
	if (wiringPiSetup() < 0)
		perror("WiringPiSetup problem \n ");	
		
	if ((fd = serialOpen("/dev/ttyAMA0", 57600)) < 0)
		//57600 is the Baud used
		perror("device not opened \n");

	while (1) {
		scanf("%s",buf);
		serialPuts(fd, buf); //Send message
		printf("Sent Data: %s\n", buf);
	}

	return 0;
}
