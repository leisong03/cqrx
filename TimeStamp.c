#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include "rs232.h"
#include "CQmsg.h"
int main(int argc, char** argv){
    cqmsg mymsg;
    int n,cport_nr=0,bdrate=19200;
    unsigned char buf[4096];
    FILE* fn;
    struct timeval tv;

    if(argc<2){
	fprintf(stderr,"Usage: %s output_filename\n",argv[0]);
	return(0);
    }
    fn=fopen(argv[1],"w");
    if(RS232_OpenComport(cport_nr, bdrate)){
	printf("Can not open comport %d\n",cport_nr);
	return(0);
    }


    while(1){
	n = RS232_PollComport(cport_nr, buf, 4095);
	if(n>0){
	    buf[n]=0;
	    gettimeofday(&tv,NULL);
	    if (n==sizeof(cqmsg))
		printf("size matched\n");
	    else
		printf("expect size %d, got size %d",sizeof(cqmsg),n);

	    printf("%ld %ld: %s\n",(long int)tv.tv_sec,(long int)tv.tv_usec,(char *) buf);
	    fflush(stdout);
	    fprintf(fn,"%ld %ld: %s\n",(long int)tv.tv_sec,(long int)tv.tv_usec,(char *) buf);
	    fflush(fn);
	    usleep(10000);
	}
    }
    fclose(fn);
}
