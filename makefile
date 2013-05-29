ReadSerial: rs232.c TimeStamp.c
	gcc -Wall -D__linux__ rs232.c TimeStamp.c -o ReadSerial
