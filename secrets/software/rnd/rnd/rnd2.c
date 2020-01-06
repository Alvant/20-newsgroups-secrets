/* 
 * rnd2.c --
 *
 *   Pseudo-random character generator.  Based on algorithm
 *   used in the BSD Unix (tm) rand.c stdlib module.
 *   Provides 16 types of PRNG (all using polynomials of
 *   the form X**a + X**b + 1).
 *
 * Copyright 1993 Peter K. Boucher
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.
 */

#define DEG_0 383
#define SEP_0 108

#define DEG_1 521
#define SEP_1 168

#define DEG_2 601
#define SEP_2 202

#define DEG_3 647
#define SEP_3 150

#define DEG_4 607
#define SEP_4 147

#define DEG_5 751
#define SEP_5 310

#define DEG_6 953
#define SEP_6 224

#define DEG_7 977
#define SEP_7 480

#define DEG_8 937
#define SEP_8 316

#define DEG_9 1031
#define SEP_9 116

#define DEG_10 1151
#define SEP_10 90

#define DEG_11 1289
#define SEP_11 242

#define DEG_12 1327
#define SEP_12 466

#define DEG_13 1321
#define SEP_13 52

#define DEG_14 1361
#define SEP_14 56

#define DEG_15 1409
#define SEP_15 194

static  long		state2[ DEG_15 ];
static  long		*fptr2;
static  long		*rptr2;
static  long		*end_ptr2;

unsigned char
rnd2()
{
	register unsigned long	i;
	register long	*frnt=fptr2, *rear=rptr2;

	i = *frnt + *rear;
	*frnt = i;
	if(  ++frnt  >=  end_ptr2  )  {
	    frnt = state2;
	    ++rear;
	} else  {
	    if(  ++rear  >=  end_ptr2  )  rear = state2;
	}
	fptr2=frnt;
	rptr2=rear;
	return( (unsigned char)(i >> 6) );
}

void set_rnd_seed2( seed, type )
    register unsigned long seed;
    unsigned int  type;

{
	register unsigned long	i, t = type&15;
	register unsigned long	degree;

	rptr2 = state2;
	switch( t ) {
            case 0:
		degree = DEG_0;
		end_ptr2 = state2 + DEG_0;
		fptr2 = state2 + SEP_0;
		break;
            case 1:
		degree = DEG_2;
		end_ptr2 = state2 + DEG_1;
		fptr2 = state2 + SEP_1;
		break;
            case 2:
		degree = DEG_2;
		end_ptr2 = state2 + DEG_2;
		fptr2 = state2 + SEP_2;
		break;
            case 3:
		degree = DEG_3;
		end_ptr2 = state2 + DEG_3;
		fptr2 = state2 + SEP_3;
		break;
            case 4:
		degree = DEG_4;
		end_ptr2 = state2 + DEG_4;
		fptr2 = state2 + SEP_4;
		break;
            case 5:
		degree = DEG_5;
		end_ptr2 = state2 + DEG_5;
		fptr2 = state2 + SEP_5;
		break;
            case 6:
		degree = DEG_6;
		end_ptr2 = state2 + DEG_6;
		fptr2 = state2 + SEP_6;
		break;
            case 7:
		degree = DEG_7;
		end_ptr2 = state2 + DEG_7;
		fptr2 = state2 + SEP_7;
		break;
            case 8:
		degree = DEG_8;
		end_ptr2 = state2 + DEG_8;
		fptr2 = state2 + SEP_8;
		break;
            case 9:
		degree = DEG_9;
		end_ptr2 = state2 + DEG_9;
		fptr2 = state2 + SEP_9;
		break;
            case 10:
		degree = DEG_10;
		end_ptr2 = state2 + DEG_10;
		fptr2 = state2 + SEP_10;
		break;
            case 11:
		degree = DEG_11;
		end_ptr2 = state2 + DEG_11;
		fptr2 = state2 + SEP_11;
		break;
            case 12:
		degree = DEG_12;
		end_ptr2 = state2 + DEG_12;
		fptr2 = state2 + SEP_12;
		break;
            case 13:
		degree = DEG_13;
		end_ptr2 = state2 + DEG_13;
		fptr2 = state2 + SEP_13;
		break;
            case 14:
		degree = DEG_14;
		end_ptr2 = state2 + DEG_14;
		fptr2 = state2 + SEP_14;
		break;
            case 15:
		degree = DEG_15;
		end_ptr2 = state2 + DEG_15;
		fptr2 = state2 + SEP_15;
		break;
	}
	state2[ 0 ] = seed;
	for( i = 1; i < degree; i++ )  {
	    seed = (state2[i] = 1103515245*seed + 12345);
	}
	degree = (10*degree) + (seed%degree);
	for( i = 0; i < degree; i++ )  rnd2();
}

