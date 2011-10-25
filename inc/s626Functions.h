/**
 *  Jordan Brindza
 *  brindza@seas.upenn.edu
 *  University of Pennsylvania
 *  December 2010
 */

#ifndef encoderFunctions_h_DEFINED
#define encoderFunctions_h_DEFINED

#include "mex.h"

#ifdef __cplusplus
extern "C" {
#endif

int s626_functions_init();
void s626_functions_cleanup();

void mex_encoder(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
void mex_zero(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
void mex_dac(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
void mex_set(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
void mex_set_velFilterWeight(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 
void mex_set_accelFilterWeight(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 
void mex_set_encTicksPerRev(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 
void mex_get(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
void mex_get_velFilterWeight(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 
void mex_get_accelFilterWeight(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 
void mex_get_encTicksPerRev(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 
void mex_get_fps(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 

void mex_null(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);

#ifdef __cplusplus
}
#endif

#endif // naoCamThread_h_DEFINED

