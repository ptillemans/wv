#include <string.h>
#include <stdio.h>
#include "wv.h"

void wvGetDOPTYPOGRAPHY(DOPTYPOGRAPHY *dopt,FILE *fd)
	{
	int i;
	U16 temp16=read_16ubit(fd);
	
	dopt->fKerningPunct = temp16&0x0001;
	dopt->iJustification = (temp16&0x0006)>>1;
	dopt->iLevelOfKinsoku = (temp16&0x0018)>>3;
	dopt->f2on1 = (temp16&0x0020)>>5;
	dopt->reserved = (temp16&0xFFC0)>>6;

	dopt->cchFollowingPunct = read_16ubit(fd);
	dopt->cchLeadingPunct = read_16ubit(fd);

	for (i=0;i<101;i++)
		dopt->rgxchFPunct[i] = read_16ubit(fd);

	for (i=0;i<51;i++)
		dopt->rgxchLPunct[i] = read_16ubit(fd);
	}