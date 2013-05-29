ReadSerial: rs232.c TimeStamp.c
	gcc -wall -D__linux__ rs232.c TimeStamp.c -o ReadSerial
