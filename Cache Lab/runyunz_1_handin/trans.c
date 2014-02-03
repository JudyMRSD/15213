/* 
 * runyunz - Runyun Zhang
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"
#include "contracts.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES from 15-122 are included
 *     for your convenience. They can be removed if you like.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    /*
     * transpose_submit - transpose matrix of 32*32, 64*64, 67*61
     */

	int block = 8;
    int ii = 0, jj = 0, i = 0, j = 0, k = 0, l = 0;
    int temp = 0;

	REQUIRES(M > 0);
    REQUIRES(N > 0);

    if(M == 32 && N == 32)
    {
    	/* block: 8*8 */
    	block = 8;
    	for(ii = 0; ii < N/block; ii++)
    	{
    		for(jj = 0; jj < M/block; jj++)
    		{
        		for(i = 0; i < block; i++)
        		{
        			for(j = 0; j < block; j++)
        			{
        				/* avoid extra load for diagonal element */
        				if(ii==jj && i==j)
        				{
        					temp = A[i+ii*block][j+jj*block];
        				}
        				else
        				{
        					B[j+jj*block][i+ii*block] =
        							A[i+ii*block][j+jj*block];
        				}
        			}
        			/* load diagonal element back */
        			if(ii == jj)
        			{
        				B[i+ii*block][i+ii*block] = temp;
        			}
        		}
    		}
    	}
    }


    if(M == 64 && N == 64)
    {
    	/* 1st block: 8 * 8 */
    	block = 8;
    	for(ii = 0; ii < N / block; ii++)
    	{
    		for(jj = 0; jj < M / block; jj++)
    		{
				/* 2nd block: 4 * 4 */
    			for(i = 0; i < 2; i++)
				{
					for(j = 0; j < 2; j++)
					{
						for(k = 0; k < block/2; k++)
						{
							for(l = 0; l < block/2; l++)
							{
								/*
								 * for diagonal 1st block:
								 * avoid extra load for diagonal element
								 * in all 2nd blocks
								 */
								if(ii == jj && l == k)
								{
									temp = A[k+i*block/2+ii*block]
									         [l+j*block/2+jj*block];
								}
								else
								{
									B[l+j*block/2+jj*block]
									  [k+i*block/2+ii*block] =
											A[k+i*block/2+ii*block]
											  [l+j*block/2+jj*block];
								}
							}
							/* load diagonal element back */
							if(ii == jj)
							{
								B[k+j* block/2+jj*block]
								  [k+i*block/2+ii*block]
								                         = temp;
							}
						}
					}
				}
    		}
    	}
    }

    if(M == 61 && N == 67)
    {
    	/* blocking: 23 * 23 */
    	block = 23;
    	for(ii = 0; ii < N/block+1; ii++)
    	{
    		for(jj = 0; jj < M/block+1; jj++)
    		{
    			for(i = 0; i < block; i++)
    			{
        			for(j = 0; j < block; j++)
        			{
        				if(ii*block+i < N && jj*block+j < M)
        				{
        					B[jj*block+j][ii*block+i] = A[ii*block+i][jj*block+j];
        				}
        			}
    			}
    		}
    	}
    }

    ENSURES(is_transpose(M, N, A, B));
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */


/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

    ENSURES(is_transpose(M, N, A, B));
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
