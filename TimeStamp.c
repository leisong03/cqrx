#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include "rs232.h"
#include "CQmsg.h"
int findapkt(int* start, int *length,unsigned char* msgbuf,int bufsize);
int main(int argc, char** argv){
    cqmsg mymsg;
    int i,n,cport_nr=0,bdrate=19200;
    unsigned char buf[4096];
    unsigned char msgbuf[4096];
    uint16_t msgcount=0;
    FILE* fn;
    struct timeval tv;

    if(argc<3){
	fprintf(stderr,"Usage: %s portnum output_filename\n",argv[0]);
	return(0);
    }
    cport_nr=atoi(argv[1]);
    fn=fopen(argv[2],"w");
    if(RS232_OpenComport(cport_nr, bdrate)){
	printf("Can not open comport %d\n",cport_nr);
	return(0);
    }

    while(1){
	n = RS232_PollComport(cport_nr, buf, 4095);
	if(n>0){
	    memcpy(msgbuf+msgcount,buf,n);
	    msgcount=msgcount+n;
#ifdef debug
	    fprintf(stderr,"read something\n");
	    printf("   buf=");
	    for(i=0;i<n;i++){
		printf("%02x ",buf[i]);
	    }
	    printf("\n");
	    printf("msgbuf=");
	    for(i=0;i<msgcount;i++){
		printf("%02x ",msgbuf[i]);
	    }
	    printf("\n");
#endif
	    if(msgcount>=sizeof(cqmsg)+2){
		int start,length;
		while(findapkt(&start,&length,msgbuf,msgcount)){
		    gettimeofday(&tv,NULL);
#ifdef debug
		    fprintf(stderr,"get a pkt\n");
#endif
		    printf("%ld %ld: ",(long int)tv.tv_sec,(long int)tv.tv_usec);
		    fprintf(fn,"%ld %ld: ",(long int)tv.tv_sec,(long int)tv.tv_usec);
		    memcpy(&mymsg,msgbuf+start+1,length-2);
		    msgcount=msgcount-length-start+1;
		    memcpy(msgbuf,msgbuf+start+length,msgcount);
		    printf("%d %d %d ",mymsg.ID,mymsg.Seq,mymsg.CQLEN);
		    fprintf(fn,"%d %d %d ",mymsg.ID,mymsg.Seq,mymsg.CQLEN);
		    for (i=0;i<mymsg.CQLEN;i++){
			printf("%d %d ",mymsg.symbol[i],mymsg.counter[i]);
			fprintf(fn,"%d %d ",mymsg.symbol[i],mymsg.counter[i]);
		    }
		    printf("\n");
		    fprintf(fn,"\n");
		    fflush(stdout);
		    fflush(fn);
		}
	    }
	}
	usleep(10000);
    }
}
int findapkt(int* start, int *length,unsigned char* msgbuf,int bufsize){
    int startpoint=0,endpoint=0;
    while(msgbuf[startpoint]!=0x7E && startpoint<bufsize)
	startpoint++;
    while(msgbuf[startpoint]==0x7E && startpoint<bufsize)
	startpoint++;
#ifdef debug
    printf("startpoint=%d\n",startpoint);
#endif
    if(startpoint>=bufsize){
	return 0;
    }
    else{
	endpoint=startpoint;
	while(msgbuf[endpoint]!=0x7E && startpoint<bufsize)
	    endpoint++;
	if(endpoint>=bufsize)
	    return 0;
	*start=startpoint-1;
	*length=endpoint-startpoint+2;
#ifdef debug
    printf("length=%d\n",*length);
#endif
	return 1;
    }
}

