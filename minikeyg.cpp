/*
"Commons Clause" License Condition v1.0

The Software is provided to you by the Licensor under the License, as defined below, subject to the following condition.

Without limiting other conditions in the License, the grant of rights under the License will not include, and the License does not grant to you, the right to Sell the Software.

For purposes of the foregoing, "Sell" means practicing any or all of the rights granted to you under the License to provide to third parties, for a fee or other consideration (including without limitation fees for hosting or consulting/ support services related to the Software), a product or service whose value derives, entirely or substantially, from the functionality of the Software. Any license notice or attribution required by the License must also include this Commons Clause License Condition notice.

Software: minikeyg

License: Apache 2.0

Licensor: Luis Alberto <alberto.bsd@gmail.com>

*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>

#include <unistd.h>
#include <pthread.h>

#include "hash/sha256.h"
#include "hash/ripemd160.h"

const char *Ccoinbuffer_default = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

char *Ccoinbuffer;

int Ccoinbuffer_length,Ccoinbuffer_mod;


int THREADS = 1;

int FLAG_COINBUFFER = 0;
int FLAG_VERIFIED = 1;
int FLAG_RANDOM = 0;
int FLAG_MINIKEY = 0;
int FLAG_SIZE = 0;
int SIZE_VALUE = 0;
int INCREMENT_OFFSET = 0;

char *user_minikey;
int user_size;
FILE *fd;

char minikey[40] = {0};
unsigned char raw_minikey[40];

pthread_mutex_t thread_mutex;

bool increment_minikey_index(char *buffer,unsigned char *rawbuffer,int index);

void bin2alphabet(unsigned char *buffer,int length);

void sha256sse_23(uint8_t *src0, uint8_t *src1, uint8_t *src2, uint8_t *src3, uint8_t *dst0, uint8_t *dst1, uint8_t *dst2, uint8_t *dst3);
void sha256sse_31(uint8_t *src0, uint8_t *src1, uint8_t *src2, uint8_t *src3, uint8_t *dst0, uint8_t *dst1, uint8_t *dst2, uint8_t *dst3);

void *process_verified22(void *vargp);
void *process_verified30(void *vargp);
void *process_unverified(void *vargp);

void *process_verified22_random(void *vargp);
void *process_verified30_random(void *vargp);
void *process_unverified_random(void *vargp);


int main(int argc, char **argv)	{
	
	char *r_str;
	int len,i,s;
	pthread_t tid ;
	char c;
	while ((c = getopt(argc, argv, "a:m:s:t:ur")) != -1) {
		switch(c){
			case 'r':
				FLAG_RANDOM = 1;
			break;
			case 'u':
				FLAG_VERIFIED = 0;
			break;
			case 'm':
				FLAG_MINIKEY = 1;
				user_minikey = optarg;
			break;
			case 'a':
				FLAG_COINBUFFER = 1;
				Ccoinbuffer = optarg;
				Ccoinbuffer_length = strlen(Ccoinbuffer);
				Ccoinbuffer_mod = Ccoinbuffer_length -1;

			break;
			case 's':
				user_size = (int)strtol(optarg,NULL,10);
				switch(user_size)	{
					case 22:
					case 30:
						FLAG_SIZE = 1;
						SIZE_VALUE = user_size;
						INCREMENT_OFFSET = SIZE_VALUE -1;
					break;
					default:
						fprintf(stderr,"Invalid size %i\n",user_size);
						exit(0);
					break;
				}
			break;
			case 't':
				THREADS = (int) strtol(optarg,NULL,10);
				if(THREADS < 1)	{
					THREADS = 1;
				}
			break;
		}
	}
	pthread_mutex_init(&thread_mutex,NULL);
	
	if(FLAG_SIZE == 0)	{
		SIZE_VALUE = 22;	//default size
		INCREMENT_OFFSET = SIZE_VALUE - 1;
	}
	if(FLAG_COINBUFFER == 0)	{
		Ccoinbuffer = (char*) Ccoinbuffer_default;
		Ccoinbuffer_length = strlen(Ccoinbuffer);
		Ccoinbuffer_mod = Ccoinbuffer_length -1;
	}

	
	if(FLAG_RANDOM)	{
		fd = fopen("/dev/urandom","r");
		if(fd == NULL)	{
			fprintf(stderr,"Can't open /dev/urandom\n");
			exit(0);
		}
	}
	else	{
		if(FLAG_MINIKEY)	{
			len = strlen(user_minikey);
			switch(len)	{
				case 22:
				case 30:
					if(FLAG_SIZE && SIZE_VALUE != len)	{
						fprintf(stderr,"Size mismatch %i != %i (%s)\n",SIZE_VALUE,len,user_minikey);
						exit(0);
					}
					else	{
						SIZE_VALUE = len;
						INCREMENT_OFFSET = SIZE_VALUE -1;
					}
					strcpy(minikey,user_minikey);
					for(i = 0; i< SIZE_VALUE; i++)	{
						r_str = strchr(Ccoinbuffer,user_minikey[i]);
						if(r_str == NULL)	{
							fprintf(stderr,"Invalid minikey %s have invalid character %c\n",user_minikey,user_minikey[i]);
							exit(0);
						}
						raw_minikey[i] = (int)(r_str - Ccoinbuffer) % Ccoinbuffer_length;
					}
				break;
				default:
					fprintf(stderr,"Invalid size %i: %s\n",len,user_minikey);
					exit(0);
				break;
			}
		}
		else	{
			fd = fopen("/dev/urandom","r");
			if(fd == NULL)	{
				fprintf(stderr,"Can't open /dev/urandom\n");
				exit(0);
			}
			fread(raw_minikey,sizeof(char),SIZE_VALUE,fd);
			fclose(fd);
			raw_minikey[0] = 25;
			minikey[0] = 'S';
			for(i = 1; i < SIZE_VALUE; i++)	{
				raw_minikey[i] = raw_minikey[i] % Ccoinbuffer_length;
				minikey[i] = Ccoinbuffer[raw_minikey[i]];
			}
		}
	}
	for(i= 0;i < THREADS; i++)	{
		if(FLAG_VERIFIED)	{
			if(SIZE_VALUE == 22)	{
				if(FLAG_RANDOM)	{
					s = pthread_create(&tid,NULL,process_verified22_random,NULL);
				}
				else	{
					s = pthread_create(&tid,NULL,process_verified22,NULL);
				}
			}
			else	{
				if(FLAG_RANDOM)	{
					s = pthread_create(&tid,NULL,process_verified30_random,NULL);
				}
				else	{
					s = pthread_create(&tid,NULL,process_verified30,NULL);
				}
			}
		}
		else	{
			if(FLAG_RANDOM)	{
				s = pthread_create(&tid,NULL,process_unverified_random,NULL);
			}
			else	{
				s = pthread_create(&tid,NULL,process_unverified,NULL);
			}
		}
	}
	pthread_join(tid,NULL);
}

void *process_verified22(void *vargp)	{
	char current_minikey[4][40];
	uint8_t keyvalue[4][32];
	int i;
	for(i = 0; i < 4; i++){
		current_minikey[i][0] = 'S';
		current_minikey[i][SIZE_VALUE] = '?';
	}
	do {
		pthread_mutex_lock(&thread_mutex);
		memcpy(current_minikey[0],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		memcpy(current_minikey[1],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		memcpy(current_minikey[2],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		memcpy(current_minikey[3],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		pthread_mutex_unlock(&thread_mutex);
		sha256sse_23((uint8_t*)current_minikey[0],(uint8_t*)current_minikey[1],(uint8_t*)current_minikey[2],(uint8_t*)current_minikey[3],keyvalue[0],keyvalue[1],keyvalue[2],keyvalue[3]);
		for(i = 0; i < 4; i++){
			if(keyvalue[i][0] == 0x00)	{
				current_minikey[i][SIZE_VALUE] = '\0';
				fprintf(stdout,"%s\n",current_minikey[i]);
				current_minikey[i][SIZE_VALUE] = '?';
			}
		}
	}while(1);
	return NULL;
}

void *process_verified30(void *vargp)	{
	char current_minikey[4][40];
	uint8_t keyvalue[4][32];
	int i;
	for(i = 0; i < 4; i++){
		current_minikey[i][0] = 'S';
		current_minikey[i][SIZE_VALUE] = '?';
	}
	do {
		pthread_mutex_lock(&thread_mutex);
		memcpy(current_minikey[0],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		memcpy(current_minikey[1],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		memcpy(current_minikey[2],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		memcpy(current_minikey[3],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		pthread_mutex_unlock(&thread_mutex);
		sha256sse_31((uint8_t*)current_minikey[0],(uint8_t*)current_minikey[1],(uint8_t*)current_minikey[2],(uint8_t*)current_minikey[3],keyvalue[0],keyvalue[1],keyvalue[2],keyvalue[3]);
		for(i = 0; i < 4; i++){
			if(keyvalue[i][0] == 0x00)	{
				current_minikey[i][SIZE_VALUE] = '\0';
				fprintf(stdout,"%s\n",current_minikey[i]);
				current_minikey[i][SIZE_VALUE] = '?';
			}
		}
	}while(1);
	return NULL;
}

void *process_verified22_random(void *vargp)	{
	char random_buffer[4][2048];
	char current_minikey[4][40];
	uint8_t keyvalue[4][32];
	int i,j,limit;
	for(i = 0; i < 4; i++)	{
		current_minikey[i][0] = 'S';
		current_minikey[i][SIZE_VALUE] = '?';
	}
	limit = 2048 -SIZE_VALUE;
	do {
		pthread_mutex_lock(&thread_mutex);
		fread(random_buffer[0],1,2048,fd);
		fread(random_buffer[1],1,2048,fd);
		fread(random_buffer[2],1,2048,fd);
		fread(random_buffer[3],1,2048,fd);
		pthread_mutex_unlock(&thread_mutex);
		for(i = 0; i < 4; i++){
			bin2alphabet((unsigned char*)random_buffer[i],2048);
		}
		for(j = 0; j < limit; j++ )	{
			memcpy(current_minikey[0]+1,random_buffer[0]+j,INCREMENT_OFFSET);
			memcpy(current_minikey[1]+1,random_buffer[1]+j,INCREMENT_OFFSET);	
			memcpy(current_minikey[2]+1,random_buffer[2]+j,INCREMENT_OFFSET);
			memcpy(current_minikey[3]+1,random_buffer[3]+j,INCREMENT_OFFSET);
			sha256sse_23((uint8_t*)current_minikey[0],(uint8_t*)current_minikey[1],(uint8_t*)current_minikey[2],(uint8_t*)current_minikey[3],keyvalue[0],keyvalue[1],keyvalue[2],keyvalue[3]);
			for(i = 0; i < 4; i++){
				if(keyvalue[i][0] == 0x00)	{
					current_minikey[i][SIZE_VALUE] = '\0';
					fprintf(stdout,"%s\n",current_minikey[i]);
					current_minikey[i][SIZE_VALUE] = '?';
				}
			}
		}
	}while(1);
	return NULL;
}


void *process_verified30_random(void *vargp)	{
	char random_buffer[4][2048];
	char current_minikey[4][40];
	uint8_t keyvalue[4][32];
	int i,j,limit;
	
	for(i = 0; i < 4; i++)	{
		current_minikey[i][0] = 'S';
		current_minikey[i][SIZE_VALUE] = '?';
	}
	limit = 2048 -SIZE_VALUE;
	do {
		pthread_mutex_lock(&thread_mutex);
		fread(random_buffer[0],1,2048,fd);
		fread(random_buffer[1],1,2048,fd);
		fread(random_buffer[2],1,2048,fd);
		fread(random_buffer[3],1,2048,fd);
		pthread_mutex_unlock(&thread_mutex);
		for(i = 0; i < 4; i++){
			bin2alphabet((unsigned char*)random_buffer[i],2048);
		}
		for(j = 0; j < limit; j++ )	{
			memcpy(current_minikey[0]+1,random_buffer[0]+j,INCREMENT_OFFSET);
			memcpy(current_minikey[1]+1,random_buffer[1]+j,INCREMENT_OFFSET);	
			memcpy(current_minikey[2]+1,random_buffer[2]+j,INCREMENT_OFFSET);
			memcpy(current_minikey[3]+1,random_buffer[3]+j,INCREMENT_OFFSET);
			sha256sse_31((uint8_t*)current_minikey[0],(uint8_t*)current_minikey[1],(uint8_t*)current_minikey[2],(uint8_t*)current_minikey[3],keyvalue[0],keyvalue[1],keyvalue[2],keyvalue[3]);
			for(i = 0; i < 4; i++){
				if(keyvalue[i][0] == 0x00)	{
					current_minikey[i][SIZE_VALUE] = '\0';
					fprintf(stdout,"%s\n",current_minikey[i]);
					current_minikey[i][SIZE_VALUE] = '?';
				}
			}
		}
	}while(1);
	return NULL;
}

void *process_unverified(void *vargp)	{
	char current_minikey[4][40];
	int i;
	for(i = 0; i < 4; i++){
		memset(current_minikey[i],0,40);
	}
	do {
		pthread_mutex_lock(&thread_mutex);
		memcpy(current_minikey[0],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		memcpy(current_minikey[1],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		memcpy(current_minikey[2],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		memcpy(current_minikey[3],minikey,SIZE_VALUE);
		increment_minikey_index(minikey,raw_minikey,INCREMENT_OFFSET);
		pthread_mutex_unlock(&thread_mutex);
		for(i = 0; i < 4; i++)	{
			fprintf(stdout,"%s\n",current_minikey[i]);
		}
	}while(1);
	return NULL;
}

void *process_unverified_random(void *vargp)	{
	char random_buffer[4][2048];
	char current_minikey[4][40];
	int i,j,limit;
	for(i = 0; i < 4; i++){
		memset(current_minikey[i],0,40);
		current_minikey[i][0] = 'S';
	}
	limit = 2048 -SIZE_VALUE;
	do {
		pthread_mutex_lock(&thread_mutex);
		fread(random_buffer[0],1,2048,fd);
		fread(random_buffer[1],1,2048,fd);
		fread(random_buffer[2],1,2048,fd);
		fread(random_buffer[3],1,2048,fd);
		pthread_mutex_unlock(&thread_mutex);
		for(i = 0; i < 4; i++){
			bin2alphabet((unsigned char*)random_buffer[i],2048);
		}
		for(j = 0; j < limit; j++ )	{
			for(i = 0; i < 4; i++)	{
				memcpy(current_minikey[i]+1,random_buffer[i]+j,INCREMENT_OFFSET);
				fprintf(stdout,"%s\n",current_minikey[i]);							
			}
		}
	}while(1);
	return NULL;
}

void bin2alphabet(unsigned char *buffer,int length)	{
	for(int i = 0; i < length; i++)	{
		buffer[i] = Ccoinbuffer[(int)buffer[i] % Ccoinbuffer_mod];
	}
}

bool increment_minikey_index(char *buffer,unsigned char *rawbuffer,int index)	{
	if(rawbuffer[index] < Ccoinbuffer_mod){
		rawbuffer[index]++;
		buffer[index] = Ccoinbuffer[rawbuffer[index]];
	}
	else	{
		rawbuffer[index] = 0x00;
		buffer[index] = Ccoinbuffer[0];
		if(index>0)	{
			return increment_minikey_index(buffer,rawbuffer,index-1);
		}
		else	{
			return false;
		}
	}
	return true;
}

#define BUFFMINIKEYCHECK23(buff,src) \
(buff)[ 0] = (uint32_t)src[ 0] << 24 | (uint32_t)src[ 1] << 16 | (uint32_t)src[ 2] << 8 | (uint32_t)src[ 3]; \
(buff)[ 1] = (uint32_t)src[ 4] << 24 | (uint32_t)src[ 5] << 16 | (uint32_t)src[ 6] << 8 | (uint32_t)src[ 7]; \
(buff)[ 2] = (uint32_t)src[ 8] << 24 | (uint32_t)src[ 9] << 16 | (uint32_t)src[10] << 8 | (uint32_t)src[11]; \
(buff)[ 3] = (uint32_t)src[12] << 24 | (uint32_t)src[13] << 16 | (uint32_t)src[14] << 8 | (uint32_t)src[15]; \
(buff)[ 4] = (uint32_t)src[16] << 24 | (uint32_t)src[17] << 16 | (uint32_t)src[18] << 8 | (uint32_t)src[19]; \
(buff)[ 5] = (uint32_t)src[20] << 24 | (uint32_t)src[21] << 16 | (uint32_t)src[22] << 8 | 0x80; \
(buff)[ 6] = 0; \
(buff)[ 7] = 0; \
(buff)[ 8] = 0; \
(buff)[ 9] = 0; \
(buff)[10] = 0; \
(buff)[11] = 0; \
(buff)[12] = 0; \
(buff)[13] = 0; \
(buff)[14] = 0; \
(buff)[15] = 0xB8;	//184 bits => 23 BYTES

void sha256sse_23(uint8_t *src0, uint8_t *src1, uint8_t *src2, uint8_t *src3, uint8_t *dst0, uint8_t *dst1, uint8_t *dst2, uint8_t *dst3)	{
  uint32_t b0[16];
  uint32_t b1[16];
  uint32_t b2[16];
  uint32_t b3[16];
  BUFFMINIKEYCHECK23(b0, src0);
  BUFFMINIKEYCHECK23(b1, src1);
  BUFFMINIKEYCHECK23(b2, src2);
  BUFFMINIKEYCHECK23(b3, src3);
  sha256sse_1B(b0, b1, b2, b3, dst0, dst1, dst2, dst3);
}

#define BUFFMINIKEYCHECK31(buff,src) \
(buff)[ 0] = (uint32_t)src[ 0] << 24 | (uint32_t)src[ 1] << 16 | (uint32_t)src[ 2] << 8 | (uint32_t)src[ 3]; \
(buff)[ 1] = (uint32_t)src[ 4] << 24 | (uint32_t)src[ 5] << 16 | (uint32_t)src[ 6] << 8 | (uint32_t)src[ 7]; \
(buff)[ 2] = (uint32_t)src[ 8] << 24 | (uint32_t)src[ 9] << 16 | (uint32_t)src[10] << 8 | (uint32_t)src[11]; \
(buff)[ 3] = (uint32_t)src[12] << 24 | (uint32_t)src[13] << 16 | (uint32_t)src[14] << 8 | (uint32_t)src[15]; \
(buff)[ 4] = (uint32_t)src[16] << 24 | (uint32_t)src[17] << 16 | (uint32_t)src[18] << 8 | (uint32_t)src[19]; \
(buff)[ 5] = (uint32_t)src[20] << 24 | (uint32_t)src[21] << 16 | (uint32_t)src[22] << 8 | (uint32_t)src[23]; \
(buff)[ 6] = (uint32_t)src[24] << 24 | (uint32_t)src[25] << 16 | (uint32_t)src[26] << 8 | (uint32_t)src[27]; \
(buff)[ 7] = (uint32_t)src[28] << 24 | (uint32_t)src[29] << 16 | (uint32_t)src[30] << 8 | 0x80; \
(buff)[ 8] = 0; \
(buff)[ 9] = 0; \
(buff)[10] = 0; \
(buff)[11] = 0; \
(buff)[12] = 0; \
(buff)[13] = 0; \
(buff)[14] = 0; \
(buff)[15] = 0xF8;	//248 bits => 31 BYTES

void sha256sse_31(uint8_t *src0, uint8_t *src1, uint8_t *src2, uint8_t *src3, uint8_t *dst0, uint8_t *dst1, uint8_t *dst2, uint8_t *dst3)	{
  uint32_t b0[16];
  uint32_t b1[16];
  uint32_t b2[16];
  uint32_t b3[16];
  BUFFMINIKEYCHECK31(b0, src0);
  BUFFMINIKEYCHECK31(b1, src1);
  BUFFMINIKEYCHECK31(b2, src2);
  BUFFMINIKEYCHECK31(b3, src3);
  sha256sse_1B(b0, b1, b2, b3, dst0, dst1, dst2, dst3);
}