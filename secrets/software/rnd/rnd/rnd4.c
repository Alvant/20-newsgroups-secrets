/* 
 * rnd4.c --
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

#define DEG_0 401
#define SEP_0 170

#define DEG_1 433
#define SEP_1 118

#define DEG_2 487
#define SEP_2 94

#define DEG_3 673
#define SEP_3 252

#define DEG_4 641
#define SEP_4 36

#define DEG_5 809
#define SEP_5 92

#define DEG_6 857
#define SEP_6 402

#define DEG_7 967
#define SEP_7 210

#define DEG_8 1033
#define SEP_8 498

#define DEG_9 1063
#define SEP_9 208

#define DEG_10 1129
#define SEP_10 208

#define DEG_11 1153
#define SEP_11 268

#define DEG_12 1223
#define SEP_12 588

#define DEG_13 1249
#define SEP_13 237

#define DEG_14 1367
#define SEP_14 134

#define DEG_15 1423
#define SEP_15 228

static  long		state4[ DEG_15 ];
static  long		*fptr4;
static  long		*rptr4;
static  long		*end_ptr4;

unsigned char
rnd4()
{
	register unsigned long	i;
	register long	*frnt=fptr4, *rear=rptr4;

	i = *frnt + *rear;
	*frnt = i;
	if(  ++frnt  >=  end_ptr4  )  {
	    frnt = state4;
	    ++rear;
	} else  {
	    if(  ++rear  >=  end_ptr4  )  rear = state4;
	}
	fptr4=frnt;
	rptr4=rear;
	return( (unsigned char)(i >> 23) );
}

void set_rnd_seed4( seed, type )
    register unsigned long seed;
    unsigned int  type;

{
	register unsigned long	i, t = type&15;
	register unsigned long	degree;

	rptr4 = state4;
	switch( t ) {
            case 0:
		degree = DEG_0;
		end_ptr4 = state4 + DEG_0;
		fptr4 = state4 + SEP_0;
		break;
            case 1:
		degree = DEG_4;
		end_ptr4 = state4 + DEG_1;
		fptr4 = state4 + SEP_1;
		break;
            case 2:
		degree = DEG_2;
		end_ptr4 = state4 + DEG_2;
		fptr4 = state4 + SEP_2;
		break;
            case 3:
		degree = DEG_3;
		end_ptr4 = state4 + DEG_3;
		fptr4 = state4 + SEP_3;
		break;
            case 4:
		degree = DEG_4;
		end_ptr4 = state4 + DEG_4;
		fptr4 = state4 + SEP_4;
		break;
            case 5:
		degree = DEG_5;
		end_ptr4 = state4 + DEG_5;
		fptr4 = state4 + SEP_5;
		break;
            case 6:
		degree = DEG_6;
		end_ptr4 = state4 + DEG_6;
		fptr4 = state4 + SEP_6;
		break;
            case 7:
		degree = DEG_7;
		end_ptr4 = state4 + DEG_7;
		fptr4 = state4 + SEP_7;
		break;
            case 8:
		degree = DEG_8;
		end_ptr4 = state4 + DEG_8;
		fptr4 = state4 + SEP_8;
		break;
            case 9:
		degree = DEG_9;
		end_ptr4 = state4 + DEG_9;
		fptr4 = state4 + SEP_9;
		break;
            case 10:
		degree = DEG_10;
		end_ptr4 = state4 + DEG_10;
		fptr4 = state4 + SEP_10;
		break;
            case 11:
		degree = DEG_11;
		end_ptr4 = state4 + DEG_11;
		fptr4 = state4 + SEP_11;
		break;
            case 12:
		degree = DEG_12;
		end_ptr4 = state4 + DEG_12;
		fptr4 = state4 + SEP_12;
		break;
            case 13:
		degree = DEG_13;
		end_ptr4 = state4 + DEG_13;
		fptr4 = state4 + SEP_13;
		break;
            case 14:
		degree = DEG_14;
		end_ptr4 = state4 + DEG_14;
		fptr4 = state4 + SEP_14;
		break;
            case 15:
		degree = DEG_15;
		end_ptr4 = state4 + DEG_15;
		fptr4 = state4 + SEP_15;
		break;
	}
	state4[ 0 ] = seed;
	for( i = 1; i < degree; i++ )  {
	    seed = (state4[i] = 1103515245*seed + 12345);
	}
	degree = (10*degree) + (seed%degree);
	for( i = 0; i < degree; i++ )  rnd4();
}

