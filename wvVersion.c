#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "config.h"
#include "wv.h"
/*
Released under GPL, written by Caolan.McNamara@ul.ie.

Copyright (C) 1998,1999 
	Caolan McNamara

Real Life: Caolan McNamara           *  Doing: MSc in HCI
Work: Caolan.McNamara@ul.ie          *  Phone: +353-86-8790257
URL: http://skynet.csn.ul.ie/~caolan *  Sig: an oblique strategy
How would you have done it?
*/

/*
returns 1 for not an ole doc
2 ole but not word doc
-1 for an error of some unknown kind
0 on success
*/

void usage( void )
	{
	printf("Usage: wvVersion filename.doc\n");
	exit(-1);
	}

int main(int argc,char **argv)
	{
	wvStream *input;
	int ret;
	wvParseStruct ps;

	wvInitError();

	if (argc < 2) 
		usage();

	input = fopen(argv[1],"rb");
	if (!input)
		{
		fprintf(stderr,"Failed to open %s: %s\n",argv[1],strerror(errno));
		return(-1);
		}

	ret = wvInitParser(&ps,input);

	if (ret == -1)
		{
		fprintf(stderr,"%s couldn't be opened as any known word document\n",argv[1]);
		return(-1);
		}

	ret = wvQuerySupported(&ps.fib,NULL);

	printf("Version: ");
	switch(ret&0x7fff)
		{
		case WORD8:
			printf("word8");
			break;
		case WORD7:
			printf("word7");
			break;
		case WORD6:
			printf("word6");
			break;
		case WORD5:
			printf("word5");
			break;
		case WORD2:
			printf("word2 (maybe)");
			break;
		}

	printf(",Encrypted: ");
	if (ret & 0x8000)
		printf("Yes\n");
	else
		printf("No\n");

	if (ret)
		{
		/*
		wvError(("startup error\n"));
		*/
		wvOLEFree();
		return(-1);
		}
	wvOLEFree();
	return(ret);
	}
