/* 
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

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32) {
        int i, j, m;
        int t1, t2, t3, t4, t5, t6, t7, t8;
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (m = i; m < i + 8; ++m) {
                    t1 = A[m][j];
                    t2 = A[m][j+1];
                    t3 = A[m][j+2];
                    t4 = A[m][j+3];
                    t5 = A[m][j+4];
                    t6 = A[m][j+5];
                    t7 = A[m][j+6];			
                    t8 = A[m][j+7];
                    B[j][m] = t1;
                    B[j+1][m] = t2;
                    B[j+2][m] = t3;
                    B[j+3][m] = t4;
                    B[j+4][m] = t5;
                    B[j+5][m] = t6;
                    B[j+6][m] = t7;
                    B[j+7][m] = t8;
                }
            }
        }
    } else if (M == 64) {
        int i, j, x, y;
        int t1, t2, t3, t4, t5, t6, t7, t8;
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (x = i; x < i + 4; ++x)
                {
                    t1 = A[x][j]; 
                    t2 = A[x][j+1]; 
                    t3 = A[x][j+2]; 
                    t4 = A[x][j+3];
                    t5 = A[x][j+4]; 
                    t6 = A[x][j+5]; 
                    t7 = A[x][j+6]; 
                    t8 = A[x][j+7];
                    
                    B[j][x] = t1; 
                    B[j+1][x] = t2; 
                    B[j+2][x] = t3; 
                    B[j+3][x] = t4;
                    B[j][x+4] = t5; 
                    B[j+1][x+4] = t6; 
                    B[j+2][x+4] = t7; 
                    B[j+3][x+4] = t8;
                }

                for (y = j; y < j + 4; ++y)
                {
                    t1 = A[i+4][y]; 
                    t2 = A[i+5][y]; 
                    t3 = A[i+6][y]; 
                    t4 = A[i+7][y];
                    t5 = B[y][i+4]; 
                    t6 = B[y][i+5]; 
                    t7 = B[y][i+6]; 
                    t8 = B[y][i+7];
                    
                    B[y][i+4] = t1; 
                    B[y][i+5] = t2; 
                    B[y][i+6] = t3; 
                    B[y][i+7] = t4;
                    B[y+4][i] = t5; 
                    B[y+4][i+1] = t6; 
                    B[y+4][i+2] = t7; 
                    B[y+4][i+3] = t8;
                }

                for (x = i + 4; x < i + 8; ++x)
                {
                    t1 = A[x][j+4]; 
                    t2 = A[x][j+5]; 
                    t3 = A[x][j+6]; 
                    t4 = A[x][j+7];
                    B[j+4][x] = t1; 
                    B[j+5][x] = t2; 
                    B[j+6][x] = t3; 
                    B[j+7][x] = t4;
                }
            }
        } 
    } else if (M == 61) {
        int i, j;
        int n = N / 8 * 8;
		int m = M / 8 * 8;
        int t1, t2, t3, t4, t5, t6, t7, t8;
        for (j = 0; j < m; j += 8) {
            for (i = 0; i < n; ++i) {
				t1 = A[i][j];
				t2 = A[i][j+1];
				t3 = A[i][j+2];
				t4 = A[i][j+3];
				t5 = A[i][j+4];
				t6 = A[i][j+5];
				t7 = A[i][j+6];
				t8 = A[i][j+7];
				
				B[j][i] = t1;
				B[j+1][i] = t2;
				B[j+2][i] = t3;
				B[j+3][i] = t4;
				B[j+4][i] = t5;
				B[j+5][i] = t6;
				B[j+6][i] = t7;
				B[j+7][i] = t8;
			}
        }
			
		for (i = n; i < N; ++i) {
            for (j = m; j < M; ++j) {
				t1 = A[i][j];
				B[j][i] = t1;
			}
        }

		for (i = 0; i < N; ++i) {
			for (j = m; j < M; ++j) {
				t1 = A[i][j];
				B[j][i] = t1;
			}
        }


		for (i = n; i < N; ++i) {
			for (j = 0; j < M; ++j) {
				t1 = A[i][j];
				B[j][i] = t1;
			}
        }
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

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

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

