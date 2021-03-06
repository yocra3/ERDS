/*
**		Authors: Mingfu Zhu & David B. Goldstein
**		Organization: Center for Human Genome Variation, Duke School of Medicine
**		Modified from Tapas Kanungo, "UMDHMM: Hidden Markov Model Toolkit," in "Extended Finite
**		State Models of Language," A. Kornai (editor), Cambridge University Press, 1999.																	    
*/


#include <math.h>
#include "hmm.h"
#include "nrutil.h"
#define VITHUGE  100000000000.0

void ViterbiLog(HMM *phmm, int T, float *O, float *L, int *H, double **delta, int **psi,
        int *q, double *pprob, float *logp)
{
        int     i, j;   /* state indices */
        int     t;      /* time index */
 
        int     maxvalind;
        double  maxval, val, poissonp;
		double  **biot;
		double  **cist;

	/* 0. Preprocessing */

	for (i = 1; i <= phmm->N; i++) 
		phmm->pi[i] = log(phmm->pi[i]);
	for (i = 1; i <= phmm->N; i++) 
		for (j = 1; j <= phmm->N; j++) {
			phmm->A[i][j] = log(phmm->A[i][j]);
		}

	biot = dmatrix(1, phmm->N, 1, T);
	cist = dmatrix(1, phmm->N, 1, T);
	for (i = 1; i <= phmm->N - 1; i++) 
		for (t = 1; t <= T; t++) {
			poissonp=poisson(L[t],O[t],(i-1));
			if(poissonp==1e-300 & O[t]>L[t]*(phmm->N - 1) &L[t]>0 & i == phmm->N - 1){
				poissonp=1e-100;
			}
			biot[i][t] = log(poissonp);
			cist[i][t] = log(hetp(i,H[t],phmm->N));
		}
	for (t = 1; t <= T; t++) {
		if(O[t]==-1){
			biot[phmm->N][t] = 10;
			cist[phmm->N][t] = 0;
		}
		else{
			biot[phmm->N][t] = -VITHUGE;
			cist[phmm->N][t] = 0;
		}
	}

        /* 1. Initialization  */
 
        for (i = 1; i <= phmm->N; i++) {
                delta[1][i] = phmm->pi[i] + biot[i][1] + cist[i][1];
                psi[1][i] = 0;
        }
 
        /* 2. Recursion */
 
        for (t = 2; t <= T; t++) {
                for (j = 1; j <= phmm->N; j++) {
                        maxval = -VITHUGE;
                        maxvalind = 1;
                        for (i = 1; i <= phmm->N; i++) {
                                val = delta[t-1][i] + (phmm->A[i][j]);
                                if (val > maxval) {
                                        maxval = val;
                                        maxvalind = i;
                                }
                        }
 
                        delta[t][j] = maxval + biot[j][t] + cist[j][t]; 
                        psi[t][j] = maxvalind;
                }
        }
 
        /* 3. Termination */
 
        *pprob = -VITHUGE;
        q[T] = 1;
        for (i = 1; i <= phmm->N; i++) {
                if (delta[T][i] > *pprob) {
                        *pprob = delta[T][i];
                        q[T] = i;
                }
        }
 
 
	/* 4. Path (state sequence) backtracking */

	for (t = T - 1; t >= 1; t--)
		q[t] = psi[t+1][q[t+1]];

	/* find zero copy-number difference state */
	int normal = 3;
	/* write Viterbi and log odds sequences */

	logp[1]=phmm->pi[q[1]] - phmm->pi[normal]+ biot[q[1]][1] - biot[normal][1]+ cist[q[1]][1]-cist[normal][1];
	
	for (t = 2; t <= T; t++){
		logp[t]=1*(phmm->A[q[t - 1]][q[t]] - phmm->A[q[t - 1]][normal]) + biot[q[t]][t] - biot[normal][t]+ cist[q[t]][t]-cist[normal][t];
	}
	
	free_dmatrix(biot, 1, phmm->N, 1, T);
	free_dmatrix(cist, 1, phmm->N, 1, T);
}
 
