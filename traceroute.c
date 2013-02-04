/*tracert.c*/   

#include <stdio.h>   
#include <stdlib.h>   
#include <errno.h>   
#include <string.h>   
#include <sys/un.h>   
#include <sys/time.h>   
#include <sys/times.h>   
#include <sys/ioctl.h>   
#include <unistd.h>   
#include <netinet/in.h>   
#include <netdb.h>   

#include "traceroute.h"
//   
// Function: set_ttl   
//   
int set_ttl(int s, int nTimeToLive)   
{   
	int     nRet;   

	nRet = setsockopt(s, IPPROTO_IP, IP_TTL, (int *)&nTimeToLive, sizeof(int));   
	if (nRet < 0)   
	{   
		perror("setsockopt in set_ttl:");   
		return 0;   
	}   
	
	return 1;   
}   

//   
// Function: decode_resp   
//   
int decode_resp(char *buf, int bytes, struct sockaddr_in *from, int ttl)   
{   
	IpHeader       *iphdr = NULL;   
	IcmpHeader     *icmphdr = NULL;   
	unsigned short  iphdrlen;   
	struct hostent *lpHostent = NULL;   
	struct in_addr  inaddr = from->sin_addr;   

	iphdr = (IpHeader *)buf;   
	// Number of 32-bit words * 4 = bytes   
	iphdrlen = iphdr->h_len * 4;    

	icmphdr = (IcmpHeader*)(buf + iphdrlen);   

	switch (icmphdr->i_type)   
	{   
		case ICMP_ECHOREPLY:     // Response from destination   
			return 2;   
			break;   
		case ICMP_TIMEOUT:      // Response from router along the way   
			printf("%2d  %s\n", ttl, inet_ntoa(inaddr));   
			return 0;   
			break;   
		case ICMP_DESTUNREACH:  // Can't reach the destination at all   
			printf("%2d  %s  reports: Host is unreachable\n", ttl,    
					inet_ntoa(inaddr));   
			return 1;   
			break;   
		default:   
			printf("non-echo type %d recvd\n", icmphdr->i_type);   
			return 1;   
			break;   
	}   
	return 0;   
}   

//   
// Function: checksum   
//   
unsigned short checksum(unsigned short *buffer, int size)    
{   
	unsigned long cksum=0;   

	while(size > 1)    
	{   
		cksum += *buffer++;   
		size -= sizeof(unsigned short);   
	}   
	if(size )   
		cksum += *(unsigned char*)buffer;   
	cksum = (cksum >> 16) + (cksum & 0xffff);   
	cksum += (cksum >> 16);   

	return (unsigned short)(~cksum);   
}   

//   
// Function: fill_icmp_data   
//   
void fill_icmp_data(char * icmp_data, int datasize)   
{   
	IcmpHeader *icmp_hdr;   
	char       *datapart;   

	icmp_hdr = (IcmpHeader*)icmp_data;   

	icmp_hdr->i_type = ICMP_ECHO;   
	icmp_hdr->i_code = 0;   
	icmp_hdr->i_id   = (unsigned short)getpid();   
	icmp_hdr->i_cksum = 0;   
	icmp_hdr->i_seq = 0;   

	datapart = icmp_data + sizeof(IcmpHeader);   
	memset(datapart,'E', datasize - sizeof(IcmpHeader));   
}   

long GetTickCount()   
{   
	struct tms tm;   
	return times(&tm);   
}   
//   
// Function: main   
//    
int main(int argc, char **argv)   
{
	return 0;
}

//
//Function which do traceroute
//liste_found sera allouee avec pour taille maxhop
//
int traceroute(char* adresse_ip, int maxhop, int **liste_found)   
{   
	int sockRaw;   
	struct hostent *hp = NULL;   
	struct sockaddr_in dest, from;   
	int ret, datasize, fromlen = sizeof(from), done = 0, maxhops, ttl = 1;   
	char *icmp_data = NULL, *recvbuf = NULL;   
	int bOpt;   
	unsigned short seq_no = 0;   
	struct timeval timeout;   

	maxhops = maxhop;

	sockRaw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);   
	if (sockRaw < 0)    
	{   
		perror("socket");   
		exit(-1);   
	}   
	//   
	// Set the receive and send timeout values to a second   
	//   
	timeout.tv_sec = 1;   
	timeout.tv_usec = 0;   
	ret = setsockopt(sockRaw, SOL_SOCKET, SO_RCVTIMEO,    
			&timeout, sizeof(struct timeval));   
	if (ret == -1)   
	{   
		perror("setsockopt in main receive:");   
		return -1;   
	}   
	timeout.tv_sec = 1;   
	timeout.tv_usec = 0;   
	ret = setsockopt(sockRaw, SOL_SOCKET, SO_SNDTIMEO,    
			&timeout, sizeof(struct timeval));   
	if (ret == -1)   
	{   
		perror("setsockopt in send:");   
		return -1;   
	} 
	//
	//Set the target ip
	//
	memset(&dest, 0, sizeof(struct sockaddr_in));   
	dest.sin_family = AF_INET;   
	if ((dest.sin_addr.s_addr = inet_addr(adresse_ip)) == INADDR_NONE)   
	{   
		hp = gethostbyname(adresse_ip);   
		if (hp)   
		{
			memcpy(&dest.sin_addr, hp->h_addr, hp->h_length);   
		}
		else   
		{   
			return -1;   
		}   
	}   
	//   
	// Set the data size to the default packet size.   
	// We don't care about the data since this is just traceroute/ping   
	//   
	datasize = DEF_PACKET_SIZE;   

	datasize += sizeof(IcmpHeader);     
	//   
	// Allocate the sending and receiving buffers for ICMP packets   
	//   
	icmp_data = malloc(MAX_PACKET*sizeof(char));   
	recvbuf = malloc(MAX_PACKET*sizeof(char));     
	(*liste_found) = malloc(maxhops*sizeof(int));
	if ((!icmp_data) || (!recvbuf) || (!(*liste_found)) )   
	{   
		perror("malloc:");   
		return -1;   
	}   
	//     
	// Here we are creating and filling in an ICMP header that is the    
	// core of trace route.   
	//   
	memset(icmp_data, 0, MAX_PACKET);   
	fill_icmp_data(icmp_data, datasize);   

	printf("\nTracing route to %s over a maximum of %d hops:\n\n",    
			adresse_ip, maxhops);   

	for(ttl = 1; ((ttl < maxhops) && (!done)); ttl++)   
	{   
		int bwrote;   

		// Set the time to live option on the socket   
		//   
		set_ttl(sockRaw, ttl);   

		//   
		// Fill in some more data in the ICMP header   
		//   
		((IcmpHeader*)icmp_data)->i_cksum = 0;   
		((IcmpHeader*)icmp_data)->timestamp = GetTickCount();   

		((IcmpHeader*)icmp_data)->i_seq = seq_no++;   
		((IcmpHeader*)icmp_data)->i_cksum = checksum((unsigned short*)icmp_data,    
			datasize);   
		//   
		// Send the ICMP packet to the destination   
		//   
		bwrote = sendto(sockRaw, icmp_data, datasize, 0,    
				(struct sockaddr *)&dest, sizeof(dest));   
		if (bwrote < 0)   
		{   
			perror("sendto:");   
			return -1;   
		}   
		// Read a packet back from the destination or a router along    
		// the way.   
		//   
		ret = recvfrom(sockRaw, recvbuf, MAX_PACKET, 0,    
				(struct sockaddr*)&from, &fromlen);   
		if (ret < 0)   
		{   
			perror("recvfrom:");   
			return -1;   
		}   
		//   
		// Decode the response to see if the ICMP response is from a    
		// router along the way or whether it has reached the destination.   
		//   
		done = decode_resp(recvbuf, ret, &from, ttl);   
		//ajouter au tableau
		if(done == 2)
		{
			(*liste_found)[ttl] = htonl(from.sin_addr.s_addr);	    
		}
		else
		{
			(*liste_found)[ttl] = 0;	    
		}
		sleep(1);   
	}   
	free(recvbuf);   
	free(icmp_data);   
	return 0;   
}   
