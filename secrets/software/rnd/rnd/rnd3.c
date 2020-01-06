/* 
 * rnd3.c --
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

#define DEG_0 479
#define SEP_0 224

#define DEG_1 439
#define SEP_1 156

#define DEG_2 569
#define SEP_2 210

#define DEG_3 353
#define SEP_3 173

#define DEG_4 727
#define SEP_4 180

#define DEG_5 769
#define SEP_5 216

#define DEG_6 881
#define SEP_6 236

#define DEG_7 839
#define SEP_7 54

#define DEG_8 991
#define SEP_8 472

#define DEG_9 1009
#define SEP_9 148

#define DEG_10 1103
#define SEP_10 164

#define DEG_11 1201
#define SEP_11 388

#define DEG_12 1297
#define SEP_12 198

#define DEG_13 1327
#define SEP_13 372

#define DEG_14 1409
#define SEP_14 464

#define DEG_15 1489
#define SEP_15 252

static  long		state3[ DEG_15 ];
static  long		*fptr3;
static  long		*rptr3;
static  long		*end_ptr3;

unsigned char
rnd3()
{
	register unsigned long	i;
	register long	*frnt=fptr3, *rear=rptr3;

	i = *frnt + *rear;
	*frnt = i;
	if(  ++frnt  >=  end_ptr3  )  {
	    frnt = state3;
	    ++rear;
	} else  {
	    if(  ++rear  >=  end_ptr3  )  rear = state3;
	}
	fptr3=frnt;
	rptr3=rear;
	return( (unsigned char)(i >> 7) );
}

void set_rnd_seed3( seed, type )
    register unsigned long seed;
    unsigned int  type;

{
	register unsigned long	i, t = type&15;
	register unsigned long	degree;

	rptr3 = state3;
	switch( t ) {
            case 0:
		degree = DEG_0;
		end_ptr3 = state3 + DEG_0;
		fptr3 = state3 + SEP_0;
		break;
            case 1:
		degree = DEG_3;
		end_ptr3 = state3 + DEG_1;
		fptr3 = state3 + SEP_1;
		break;
            case 2:
		degree = DEG_2;
		end_ptr3 = state3 + DEG_2;
		fptr3 = state3 + SEP_2;
		break;
            case 3:
		degree = DEG_3;
		end_ptr3 = state3 + DEG_3;
		fptr3 = state3 + SEP_3;
		break;
            case 4:
		degree = DEG_4;
		end_ptr3 = state3 + DEG_4;
		fptr3 = state3 + SEP_4;
		break;
            case 5:
		degree = DEG_5;
		end_ptr3 = state3 + DEG_5;
		fptr3 = state3 + SEP_5;
		break;
            case 6:
		degree = DEG_6;
		end_ptr3 = state3 + DEG_6;
		fptr3 = state3 + SEP_6;
		break;
            case 7:
		degree = DEG_7;
		end_ptr3 = state3 + DEG_7;
		fptr3 = state3 + SEP_7;
		break;
            case 8:
		degree = DEG_8;
		end_ptr3 = state3 + DEG_8;
		fptr3 = state3 + SEP_8;
		break;
            case 9:
		degree = DEG_9;
		end_ptr3 = state3 + DEG_9;
		fptr3 = state3 + SEP_9;
		break;
            case 10:
		degree = DEG_10;
		end_ptr3 = state3 + DEG_10;
		fptr3 = state3 + SEP_10;
		break;
            case 11:
		degree = DEG_11;
		end_ptr3 = state3 + DEG_11;
		fptr3 = state3 + SEP_11;
		break;
            case 12:
		degree = DEG_12;
		end_ptr3 = state3 + DEG_12;
		fptr3 = state3 + SEP_12;
		break;
            case 13:
		degree = DEG_13;
		end_ptr3 = state3 + DEG_13;
		fptr3 = state3 + SEP_13;
		break;
            case 14:
		degree = DEG_14;
		end_ptr3 = state3 + DEG_14;
		fptr3 = state3 + SEP_14;
		break;
            case 15:
		degree = DEG_15;
		end_ptr3 = state3 + DEG_15;
		fptr3 = state3 + SEP_15;
		break;
	}
	state3[ 0 ] = seed;
	for( i = 1; i < degree; i++ )  {
	    seed = (state3[i] = 1103515245*seed + 12345);
	}
	degree = (10*degree) + (seed%degree);
	for( i = 0; i < degree; i++ )  rnd3();
}

