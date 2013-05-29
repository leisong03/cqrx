#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <sys/time.h>
#include <netinet/in.h> 
#include <unistd.h>
#include "rs232.h"
#include "CQmsg.h"
int findapkt(int* start, int *length,unsigned char* msgbuf,int bufsize);
int pktdecode(unsigned char* inbuf,unsigned char* outbuf,int inbufsize);
int main(int argc, char** argv){
    cqmsg mymsg;
    int i,n,cport_nr=0,bdrate=19200;
    unsigned char buf[4096];
    unsigned char msgbuf[4096];
    uint16_t msgcount=0;
    FILE* fn;
    struct timeval tv;
    struct tm* ptm;
    char time_string[40];

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
		    strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
		    unsigned char outbuf[4096];
		    int outbufsize;
		    outbufsize=pktdecode(msgbuf+start+1,outbuf,length-2);
		    if(!outbufsize){
			fprintf(stderr,"pkt decode error\n");
		    }
		    //printf("%s: ",(long int)tv.tv_sec,(long int)tv.tv_usec);
		    printf("%s: ",time_string);
		    fprintf(fn,"%ld %ld: ",(long int)tv.tv_sec,(long int)tv.tv_usec);
		    memcpy(&mymsg,outbuf,outbufsize);
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
//0x7E->0x7D 0X5E and  0x7D->0x7D 0X5D
int pktdecode(unsigned char* inbuf,unsigned char* outbuf,int inbufsize){
    int outidx=0,inidx=0;

    while(inidx<inbufsize-1){
	if(inbuf[inidx]!=0x7D){
	    outbuf[outidx]=inbuf[inidx];
	    outidx++;
	    inidx++;
	}else{
	    if(inbuf[inidx+1]==0x5D){
		outbuf[outidx]=0x7D;
		outidx++;
		inidx+=2;
	    }
	    if(inbuf[inidx+1]==0x5E){
		outbuf[outidx]=0x7E;
		outidx++;
		inidx+=2;
	    }
	}
    }
    if(inidx==inbufsize-1){
	outbuf[outidx]=inbuf[inidx];
	outidx++;
    }
#ifdef debug
    int i;
    fprintf(stderr,"inbuf=");
    for(i=0;i<inbufsize;i++){
	fprintf(stderr,"%02x ",inbuf[i]);
    }
    fprintf(stderr,"\n");
    fprintf(stderr,"outbuf=");
    for(i=0;i<outidx;i++){
	fprintf(stderr,"%02x ",outbuf[i]);
    }
    fprintf(stderr,"\n");
#endif
    return outidx;
}
