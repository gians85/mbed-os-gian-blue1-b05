#include "mbed.h"
#include "colorlib.h"
#include "imu.h"


Serial pc(USBTX, USBRX);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalIn but1(PUSH1);
DigitalIn but2(PUSH2);
SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS); // mosi, miso, sclk, cs
DigitalOut cs(SPI_CS);


void init();
void Rx_interrupt();
void printMenu();
uint8_t IMU_register(char, char);
void IMU_readAcc(uint16_t *);
void IMU_readGyr(uint16_t *);
float IMU_readTemp();
void IMU_config();
void IMU_device();


uint8_t rx = 0;
bool flag = 0;


int main() {
	init();
	IMU_device();
}



void init(){
	// Buttons
	but1.mode(NoPull);
	but2.mode(NoPull);
	// Serial interface
	pc.attach(&Rx_interrupt, Serial::RxIrq);
	// SPI Interface
	spi.format(8, 0);
	spi.frequency(100000);
	// Ready
	led2 = 1;
}


// Interupt Routine to read in data from serial port
void Rx_interrupt() {
	rx = pc.getc();
	flag = 1;
}


void printMenu(){
	pc.printf(CLS CURSOR_OFF GREEN
			"*************************************************************\n\r"
			"                    BLUENRG1 & LSM6DS3\n\r"
			"*************************************************************\n\r"
			"\n"
			" A - Accelerometro\n\r"
			" G - Giroscopio\n\r"
			" T - Temperatura\n\r"
			" H - Help/Menu\n\r"
			" C - Cancella\n\r"
			"\n"
			"\tASSE X\tASSE Y\tASSE Z\r\n"
			WHITE);
}


void IMU_config(){
	IMU_register(0x11, 0x12);
	IMU_register(0x19, 0x38);
	IMU_register(0x10, 0x10);
	IMU_register(0x18, 0x38);
}


uint8_t IMU_register(char reg_name, char data){
	char buf[2];
	if (data == READ){
		reg_name |= READ_1B;
		spi.write(&reg_name, 1, &data, 1);
	}
	else{
		buf[0] = reg_name;
		buf[1] = data;
		spi.write(buf, 2, &data, 0);
	}
	return (uint8_t)data;
}


void IMU_readAcc(uint16_t * buf){
	buf[0]  = IMU_register(OUT_X_H_A, READ);
	buf[0] <<= 8;
	buf[0] |= IMU_register(OUT_X_L_A, READ);
	buf[1]  = IMU_register(OUT_Y_H_A, READ);
	buf[1] <<= 8;
	buf[1] |= IMU_register(OUT_Y_L_A, READ);
	buf[2]  = IMU_register(OUT_Z_H_A, READ);
	buf[2] <<= 8;
	buf[2] |= IMU_register(OUT_Z_L_A, READ);
}


void IMU_readGyr(uint16_t * buf){
	buf[0]  = IMU_register(OUT_X_H_G, READ);
	buf[0] <<= 8;
	buf[0] |= IMU_register(OUT_X_L_G, READ);
	buf[1]  = IMU_register(OUT_Y_H_G, READ);
	buf[1] <<= 8;
	buf[1] |= IMU_register(OUT_Y_L_G, READ);
	buf[2]  = IMU_register(OUT_Z_H_G, READ);
	buf[2] <<= 8;
	buf[2] |= IMU_register(OUT_Z_L_G, READ);
}


float IMU_readTemp(){
	int16_t temp = 0;
	temp  = IMU_register(OUT_TEMP_H, READ);
	temp <<= 8;
	temp |= IMU_register(OUT_TEMP_L, READ);
	return (25.0+(((float)temp)/16));
}

void IMU_device(){
	IMU_config();
	float a;
	uint16_t buf[3];
	printMenu();
	while (1){
		if (flag){
			flag = 0;
		}
		switch (rx){
		case 'h': case 'H':
		printMenu();
		break;
		case 'a': case 'A':
			pc.printf(CLEARLINE2 CR " ACC");
			a = 0.000061;
			while (!flag){
				IMU_readAcc(buf);
				pc.printf("\r\t%.4f  %.4f  %.4f", (buf[0]*a), (buf[1]*a), (buf[2]*a));
			}
			break;
		case 'g': case 'G':
			pc.printf(CLEARLINE2 CR " GYR");
			a = 0.00875;
			while (!flag){
				IMU_readGyr(buf);
				pc.printf("\r\t%.4f  %.4f  %.4f", (buf[0]*a), (buf[1]*a), (buf[2]*a));
			}
			break;
		case 't': case 'T':
			pc.printf(CLEARLINE2 CR " TEMP ");
			while (!flag){
				pc.printf("\r\t%.1f", IMU_readTemp());
				wait_ms(200);
			}
			break;
		case 'c': case 'C':
			pc.printf(CLEARLINE2 CR);
			break;
		default:
			break;
		}
		rx = 0x00;
		led1 = !led1;
		wait_ms(200);
	}
}





