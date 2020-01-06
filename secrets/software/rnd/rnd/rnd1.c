/* 
 * rnd1.c --
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

#define DEG_0 367
#define SEP_0 171

#define DEG_1 431
#define SEP_1 200

#define DEG_2 503
#define SEP_2 248

#define DEG_3 463
#define SEP_3 214

#define DEG_4 409
#define SEP_4 87

#define DEG_5 617
#define SEP_5 200

#define DEG_6 719
#define SEP_6 150

#define DEG_7 631
#define SEP_7 307

#define DEG_8 823
#define SEP_8 280

#define DEG_9 887
#define SEP_9 336

#define DEG_10 1039
#define SEP_10 418

#define DEG_11 1097
#define SEP_11 86

#define DEG_12 1279
#define SEP_12 216

#define DEG_13 1193
#define SEP_13 173

#define DEG_14 1217
#define SEP_14 393

#define DEG_15 1399
#define SEP_15 220

static  long		state1[ DEG_15 ];
static  long		*fptr1;
static  long		*rptr1;
static  long		*end_ptr1;

unsigned char
rnd1()
{
	register unsigned long	i;
	register long	*frnt=fptr1, *rear=rptr1;

	i = *frnt + *rear;
	*frnt = i;
	if(  ++frnt  >=  end_ptr1  )  {
	    frnt = state1;
	    ++rear;
	} else  {
	    if(  ++rear  >=  end_ptr1  )  rear = state1;
	}
	fptr1=frnt;
	rptr1=rear;
	return( (unsigned char)(i >> 10) );
}

void set_rnd_seed1( seed, type )
    register unsigned long seed;
    unsigned int  type;

{
	register unsigned long	i, t = type&15;
	register unsigned long	degree;

	rptr1 = state1;
	switch( t ) {
            case 0:
		degree = DEG_0;
		end_ptr1 = state1 + DEG_0;
		fptr1 = state1 + SEP_0;
		break;
            case 1:
		degree = DEG_1;
		end_ptr1 = state1 + DEG_1;
		fptr1 = state1 + SEP_1;
		break;
            case 2:
		degree = DEG_2;
		end_ptr1 = state1 + DEG_2;
		fptr1 = state1 + SEP_2;
		break;
            case 3:
		degree = DEG_3;
		end_ptr1 = state1 + DEG_3;
		fptr1 = state1 + SEP_3;
		break;
            case 4:
		degree = DEG_4;
		end_ptr1 = state1 + DEG_4;
		fptr1 = state1 + SEP_4;
		break;
            case 5:
		degree = DEG_5;
		end_ptr1 = state1 + DEG_5;
		fptr1 = state1 + SEP_5;
		break;
            case 6:
		degree = DEG_6;
		end_ptr1 = state1 + DEG_6;
		fptr1 = state1 + SEP_6;
		break;
            case 7:
		degree = DEG_7;
		end_ptr1 = state1 + DEG_7;
		fptr1 = state1 + SEP_7;
		break;
            case 8:
		degree = DEG_8;
		end_ptr1 = state1 + DEG_8;
		fptr1 = state1 + SEP_8;
		break;
            case 9:
		degree = DEG_9;
		end_ptr1 = state1 + DEG_9;
		fptr1 = state1 + SEP_9;
		break;
            case 10:
		degree = DEG_10;
		end_ptr1 = state1 + DEG_10;
		fptr1 = state1 + SEP_10;
		break;
            case 11:
		degree = DEG_11;
		end_ptr1 = state1 + DEG_11;
		fptr1 = state1 + SEP_11;
		break;
            case 12:
		degree = DEG_12;
		end_ptr1 = state1 + DEG_12;
		fptr1 = state1 + SEP_12;
		break;
            case 13:
		degree = DEG_13;
		end_ptr1 = state1 + DEG_13;
		fptr1 = state1 + SEP_13;
		break;
            case 14:
		degree = DEG_14;
		end_ptr1 = state1 + DEG_14;
		fptr1 = state1 + SEP_14;
		break;
            case 15:
		degree = DEG_15;
		end_ptr1 = state1 + DEG_15;
		fptr1 = state1 + SEP_15;
		break;
	}
	state1[ 0 ] = seed;
	for( i = 1; i < degree; i++ )  {
	    seed = (state1[i] = 1103515245*seed + 12345);
	}
	degree = (10*degree) + (seed%degree);
	for( i = 0; i < degree; i++ )  rnd1();
}

