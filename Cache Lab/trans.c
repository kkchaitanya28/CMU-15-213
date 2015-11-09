/*Ram Verma, ramv 
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
  int i, j, k, l;
  int v0, v1, v2, v3, v4, v5, v6, v7;
  switch (M)
  {
    case 32:
    //each block can hold 8 integers
    //since it can hold 32 bytes(2^b(=5))
    //and one cache can hold 5 sets, or 8x8 integer block
    //so we have 4 by 4 blocks
    //just use temporary variables to combat conflict misses
    //conflict misses give 348 misses in total
    for (i=0; i<4; i++)//row blocks
    {
      for (j=0; j<4; j++)//col blocks
      {
        for (k=0; k<8; k++)
        {
          v0 = A[i * 8 + k][j * 8 + 0];
          v1 = A[i * 8 + k][j * 8 + 1];
          v2 = A[i * 8 + k][j * 8 + 2];
          v3 = A[i * 8 + k][j * 8 + 3];
          v4 = A[i * 8 + k][j * 8 + 4];
          v5 = A[i * 8 + k][j * 8 + 5];
          v6 = A[i * 8 + k][j * 8 + 6];
          v7 = A[i * 8 + k][j * 8 + 7];
          
          B[j * 8 + 0][i * 8 + k] = v0;
          B[j * 8 + 1][i * 8 + k] = v1;
          B[j * 8 + 2][i * 8 + k] = v2;
          B[j * 8 + 3][i * 8 + k] = v3;
          B[j * 8 + 4][i * 8 + k] = v4;
          B[j * 8 + 5][i * 8 + k] = v5;
          B[j * 8 + 6][i * 8 + k] = v6;
          B[j * 8 + 7][i * 8 + k] = v7;
        }
      }
    } 
    break;
    case 64:
    //topleft=A1, topright=A2, bottomleft=3, bottomright=4.
    //Trying have 4x4 blocks in the 64 8x8 blocks
    //doesnt work, give like 1.8k misses.
    //So within 4x4 block, need to do something.
    //Go in blocks of 4, but this gives 1467 misses
    //with variables for A2.
    //Need to take care of diagonal again to get less
    //then 1300.
    for (i=0; i<8; i++) 
    {
      for (j=0; j<8; j++) 
      {
        if (i!=j)//not on diagonal
        { 
          for (k=0; k<4; k++) 
          {
	        for (l=0; l<4; l++) 
	        {
	          B[j * 8 + l][i * 8 + k] = A[i * 8 + k][j * 8 + l];
	          //transpose A1
	        }
	      }
		  v0 = A[i * 8 + 0][j * 8 + 4];
		  v1 = A[i * 8 + 0][j * 8 + 5];
		  v2 = A[i * 8 + 0][j * 8 + 6];
		  v3 = A[i * 8 + 0][j * 8 + 7];
		  v4 = A[i * 8 + 1][j * 8 + 4];
		  v5 = A[i * 8 + 1][j * 8 + 5];
		  v6 = A[i * 8 + 1][j * 8 + 6];
		  v7 = A[i * 8 + 1][j * 8 + 7];
		  //first two line of A2 stored into variables
	
	      for (k=4; k<8; k++) 
	      {
	        for (l=0; l<4; l++) 
	        {
	          B[j * 8 + l][i * 8 + k] = A[i * 8 + k][j * 8 + l];
	          //transpose A3
	        }
	      }
	      for (k=4; k<8; k++) 
	      {
	        for (l=4; l<8; l++) 
	        {
	          B[j * 8 + l][i * 8 + k] = A[i * 8 + k][j * 8 + l];
	          //transpose A4
	        }
	      }
		  B[j * 8 + 4][i * 8 + 0] = v0;
		  B[j * 8 + 5][i * 8 + 0] = v1;
		  B[j * 8 + 6][i * 8 + 0] = v2;
		  B[j * 8 + 7][i * 8 + 0] = v3;
		  B[j * 8 + 4][i * 8 + 1] = v4;
		  B[j * 8 + 5][i * 8 + 1] = v5;
		  B[j * 8 + 6][i * 8 + 1] = v6;
		  B[j * 8 + 7][i * 8 + 1] = v7;
		  //transpose A2 first two lines
		  for (k=2; k<4; k++) 
		  {
		    for (l=4; l<8; l++) 
		    {
		      B[j * 8 + l][i * 8 + k] = A[i * 8 + k][j * 8 + l];
		      //transpose last two lines of A2
		    }
		  }
	  }
        else//on diagonal
        { 
          //go left down then right down; A1->A3->A2->A4
          //2 rows by 2 rows since 8 variables to be used
          for (l=0; l<2; l++) 
          {
            for (k=0; k<2; k++) 
            {
		      v0 = A[i * 8 + k * 4 + 0][j * 8 + l * 4 + 0];
			  v1 = A[i * 8 + k * 4 + 0][j * 8 + l * 4 + 1];
			  v2 = A[i * 8 + k * 4 + 0][j * 8 + l * 4 + 2];
			  v3 = A[i * 8 + k * 4 + 0][j * 8 + l * 4 + 3];
			  v4 = A[i * 8 + k * 4 + 1][j * 8 + l * 4 + 0];
			  v5 = A[i * 8 + k * 4 + 1][j * 8 + l * 4 + 1];
			  v6 = A[i * 8 + k * 4 + 1][j * 8 + l * 4 + 2];
			  v7 = A[i * 8 + k * 4 + 1][j * 8 + l * 4 + 3];
		  
			  B[j * 8 + l * 4 + 0][i * 8 + k * 4 + 0] = v0;
			  B[j * 8 + l * 4 + 1][i * 8 + k * 4 + 0] = v1;
			  B[j * 8 + l * 4 + 2][i * 8 + k * 4 + 0] = v2;
			  B[j * 8 + l * 4 + 3][i * 8 + k * 4 + 0] = v3;
			  B[j * 8 + l * 4 + 0][i * 8 + k * 4 + 1] = v4;
			  B[j * 8 + l * 4 + 1][i * 8 + k * 4 + 1] = v5;
			  B[j * 8 + l * 4 + 2][i * 8 + k * 4 + 1] = v6;
			  B[j * 8 + l * 4 + 3][i * 8 + k * 4 + 1] = v7;
			
			  v0 = A[i * 8 + k * 4 + 2][j * 8 + l * 4 + 0];
			  v1 = A[i * 8 + k * 4 + 2][j * 8 + l * 4 + 1];
			  v2 = A[i * 8 + k * 4 + 2][j * 8 + l * 4 + 2];
			  v3 = A[i * 8 + k * 4 + 2][j * 8 + l * 4 + 3];
			  v4 = A[i * 8 + k * 4 + 3][j * 8 + l * 4 + 0];
			  v5 = A[i * 8 + k * 4 + 3][j * 8 + l * 4 + 1];
			  v6 = A[i * 8 + k * 4 + 3][j * 8 + l * 4 + 2];
			  v7 = A[i * 8 + k * 4 + 3][j * 8 + l * 4 + 3];
			
			  B[j * 8 + l * 4 + 0][i * 8 + k * 4 + 2] = v0;
			  B[j * 8 + l * 4 + 1][i * 8 + k * 4 + 2] = v1;
			  B[j * 8 + l * 4 + 2][i * 8 + k * 4 + 2] = v2;
			  B[j * 8 + l * 4 + 3][i * 8 + k * 4 + 2] = v3;
			  B[j * 8 + l * 4 + 0][i * 8 + k * 4 + 3] = v4;
			  B[j * 8 + l * 4 + 1][i * 8 + k * 4 + 3] = v5;
			  B[j * 8 + l * 4 + 2][i * 8 + k * 4 + 3] = v6;
			  B[j * 8 + l * 4 + 3][i * 8 + k * 4 + 3] = v7;
            } 
          }
        }
      }
    } 
    break;
    case 61:
    //similar to 32x32 idea, use blocks of 8
    for (i=0; i<N; i +=8)//use the mod by 8 idea
    {
    for (j=0; j<M; j+=8)
      {
        for(k=j; (k<(j+8)) && (k<M); k++)//dont exceed rows of B
        //increase by every block dependent on M
          {
            for(l=i; (l<(i+8)) && (l<N); l++)//dont exceed cols of B
            //similarly, increase in block by N
              {
                B[k][l] = A[l][k];
              }
          }
      }
  }
    break;
  }  
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
    //registerTransFunction(trans, trans_desc); 


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


