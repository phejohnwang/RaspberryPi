#include <cstdint>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <wiringPi.h>
#include <wiringSerial.h>
#include <errno.h>

#include "connection.h"

using namespace std;

#define DATA_SIZE 1024

//CLPC variables
int upper = 26;
int lower = 25;
int step_up = 3;
int step_down = 3;

int power_lookup[3] = { 0, 2, 3 };  //WringPi Pin NO.

int cs_bar = power_lookup[0], sdi_data = power_lookup[1], clk = power_lookup[2];

int powerctrl = 100;	//change this number for test

//CLPC functions
void initialize_ports();
void SDI_data_send(int pwr_ctrl);
bool IsDataComRect(char *data_, int len);
void CtrlDP(char *data_);

bool IsCLPCCom(char *data_, int len);
void ChangeCLPCVar(char* data_);

//uart read
volatile int uart_fd;
int nRec;
uint8_t data[DATA_SIZE];

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void handle_status(size_t status);
void * handle_conns(void * state);

int main(int argc, char * argv[]) {
	pthread_t conn_thread;
	bool running = true;

	/***************** Uart Init - Start ******************/
	/* OPENING SERIAL CNX */
	if (wiringPiSetup() < 0)
		perror("WiringPiSetup problem \n ");
	if ((uart_fd = serialOpen("/dev/ttyAMA0", 115200)) < 0)
	/*device must be replaced by the serial port in the RPi*/
		perror("device not opened \n");

	nRec = 0;
	bzero(data, DATA_SIZE);
	/***************** Uart Init - End ******************/

	//CLPC Init
	initialize_ports();
	SDI_data_send(powerctrl);

	//Start Main Socket
	connection conn_main(6000);
	handle_status(conn_main.open());
	uint8_t* send_buffer_main = conn_main.get_send_buffer();
	//uint8_t* read_buffer_main = conn_main.get_read_buffer();
	ssize_t comm_status_main;

	pthread_create(&conn_thread, NULL, handle_conns, (void *) &running);

	while (true) {
		if (conn_main.connect() == connection::ERR_NO_CONN) continue;
		comm_status_main = 0;

 		while (running && comm_status_main >= 0) {
			nRec = read(uart_fd, send_buffer_main, DATA_SIZE);
			if (nRec == -1) {
				perror("Read Data Error!\n");
				break;
			}
			if (nRec > 0) {
				//printf("Recv Uart Len %d\n", nRec);
				send_buffer_main[nRec] = '\0';
				if (IsDataComRect((char *)send_buffer_main,nRec)) {
					CtrlDP((char *)send_buffer_main);
				}
				//printf("Recv Uart Data: %s\n", data);
				//bzero(send_buffer_main, BUFFER_SIZE);
				//memcpy(send_buffer_main, data, strlen((char *)data));
				comm_status_main = conn_main.send(nRec);
			}
        		}
		conn_main.free();
	}

	conn_main.disconnect();

	running = false;
	pthread_join(conn_thread, NULL);

	return 0;
}

void * handle_conns(void * state) {
	bool * running = (bool *) state;
	int readSize;

	connection conn(5000);
	handle_status(conn.open());
	//uint8_t* send_buffer = conn.get_send_buffer();
	uint8_t* read_buffer = conn.get_read_buffer();
	ssize_t comm_status;

	while (*running) {
		if (conn.connect() == connection::ERR_NO_CONN) continue;

		printf("New Connection----\n");
		comm_status = 0;
		while (*running && comm_status >= 0) {
			readSize = conn.read();
			//printf("%d\n", readSize);
			if (readSize>0) {
    				//printf("Recv Socket Len %d\n", readSize);
				//printf("Recv Socket Data: %s\n", read_buffer);
				if (IsCLPCCom((char *)read_buffer, readSize)) {
					 ChangeCLPCVar((char *)read_buffer);
				} else {
					serialPuts(uart_fd, (char *)read_buffer);
				}
            				bzero(read_buffer, BUFFER_SIZE);
			} else if (readSize == 0) {
				comm_status = -1;
			}
		}

		conn.free();
	}

	conn.disconnect();
	pthread_exit(NULL);
}

void handle_status(size_t status) {
	if (status != connection::SUCCESS) {
		std::cout << connection::error_message(status) << std::endl;
		exit(status);
	}
}

void initialize_ports()
{
	//initialize 3 output ports -- clock, cs_bar, SDI_data
	for (int i = 0; i < 3; i++) {
		pinMode(power_lookup[i], OUTPUT);
	}

	//initialize clock to '0'
	digitalWrite(clk, LOW);

	//initialize CS_bar to '1'
	digitalWrite(cs_bar, HIGH);
}

void SDI_data_send(int pwr_ctrl)
{
	int temp = pwr_ctrl;
	int bit;

	//set CS_bar to 0
	digitalWrite(cs_bar, LOW);

	//send powerctrl  as 8 bit SDI data
	for (int i = 0; i < 8; i++) {
		bit = temp & 128;
		bit = bit >> 7;

		digitalWrite(sdi_data, bit);	//set SDI data bit to "bit"
		usleep(100);

		digitalWrite(clk, HIGH);	//set clock to '1'
		usleep(100);

		digitalWrite(clk, LOW);	//set clock to '0'
		usleep(100);

		temp = temp << 1;
	}

	digitalWrite(cs_bar, HIGH);	//set CS_bar to 1
}

bool IsDataComRect(char *data_, int len)
{
	if ((len % 8) != 0) return false;

	int a1 = data_[0];
	int a2 = data_[1];
	int a3 = data_[2];

	if ((a1 == 1) && (a2 == 1) && (a3 == 1)) return true;
	//if ((a1 == 2) && (a2 == 2) && (a3 == 2)) return true;
	if ((a1 == 3) && (a2 == 3) && (a3 == 3)) return true;
	//if ((a1 == 4) && (a2 == 4) && (a3 == 4)) return true;
	if ((a1 == 5) && (a2 == 5) && (a3 == 5)) return true;

	return false;
}

void CtrlDP(char *data_)
{
	int value = data_[4];
	//printf("Recv Volt: %d\n", value);

	if (value > upper) {
		//cout << "high" << endl;
		if (powerctrl > 10)
			powerctrl = powerctrl - step_down;
		SDI_data_send(powerctrl);
	}
	else if (value < lower) {
		//cout << "low" << endl;
		if (powerctrl < 230)
			powerctrl = powerctrl + step_up;
		SDI_data_send(powerctrl);
	}
	else {
		//cout << "normal" << endl;
	}
	//cout << "powerctrl end:" << powerctrl << endl;
}

bool IsCLPCCom(char *data_, int len)
{
	if (len!=8) return false;

	if ((data_[0]=='C')&&(data_[1]=='L')&&(data_[2]=='P')&&(data_[3]=='C'))	return true;

	return false;
}

void ChangeCLPCVar(char* data_)
{
	int a = data_[5]-'0';
	int b = data_[6]-'0';
	int c = data_[7]-'0';
	int value_new = a * 100 + b * 10 + c;

	if (data_[4]=='U') {
		upper = value_new;
		//cout<<"upper changed: "<< upper<<endl;
	}
	if (data_[4]=='L') {
		lower = value_new;
		//cout<<"lower changed: "<<lower<<endl;
	}
	if (data_[4]=='P') {
		powerctrl = value_new;
		SDI_data_send(powerctrl);
		//cout<<"powertrl changed: "<<powerctrl<<endl;
	}
}
