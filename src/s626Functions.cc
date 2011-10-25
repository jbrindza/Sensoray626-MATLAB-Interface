/**
 *  Jordan Brindza
 *  brindza@seas.upenn.edu
 *  University of Pennsylvania
 *  December 2010
 */

#include "s626Thread.h"
#include "s626Functions.h"
#include "s626_util.h"

#include <map>
#include <string>
#include "mex.h"

typedef unsigned char uint8;
typedef unsigned int uint32;

// defined in encoderThread.cc
extern HBD board;
extern mxArray *fps;
extern mxArray *encoders;
extern mxArray *velFilterWeight;
extern mxArray *accelFilterWeight;
extern mxArray *encTicksPerRev;

int s626_functions_init() {
}

void s626_functions_cleanup() {
}

void mex_encoder(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  // return encoder array
  plhs[0] = encoders;
}

void mex_zero(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  if (nrhs < 1 || !mxIsNumeric(prhs[0])) {
    // zero all encoders
    zero_encoder_counter(board, CNTR_0A);
    zero_encoder_counter(board, CNTR_1A);
    zero_encoder_counter(board, CNTR_2A);
    zero_encoder_counter(board, CNTR_0B);
    zero_encoder_counter(board, CNTR_1B);
    zero_encoder_counter(board, CNTR_2B);
  } else {
    double *encIDs = mxGetPr(prhs[0]);
    int nencIDs = mxGetNumberOfElements(prhs[0]);

    for (int i = 0; i < nencIDs; i++) {
      int id = (int)(*(encIDs + i));
      switch (id) {
        case 1:
          zero_encoder_counter(board, CNTR_0A);
          break;
        case 2:
          zero_encoder_counter(board, CNTR_1A);
          break;
        case 3:
          zero_encoder_counter(board, CNTR_2A);
          break;
        case 4:
          zero_encoder_counter(board, CNTR_0B);
          break;
        case 5:
          zero_encoder_counter(board, CNTR_1B);
          break;
        case 6:
          zero_encoder_counter(board, CNTR_2B);
          break;
      }
    }
  }
}

void mex_dac(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  if (nrhs < 1 || !mxIsNumeric(prhs[0]))
    mexErrMsgTxt("Need input voltages");

  int nv = mxGetNumberOfElements(prhs[0]);

  int index = 0;

  if (nrhs >= 2)
    index = mxGetScalar(prhs[1]) - 1;

  double *voltages = mxGetPr(prhs[0]);
  for (int i = 0; i < nv; i++) {
    set_dac_voltage(board, ((index + i) % 4), *(voltages + i));
  }
}

void mex_set(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  static bool init = false;
  static std::map<std::string, void (*)(int nlhs, mxArray *plhs[],
    int nrhs, const mxArray *prhs[])> funcMap;
  
  if (!init) {
    funcMap["velFilterWeight"] = mex_set_velFilterWeight;
    funcMap["accelFilterWeight"] = mex_set_accelFilterWeight;
    funcMap["encTicksPerRev"] = mex_set_encTicksPerRev;
  }

  if ((nrhs < 1) || (!mxIsChar(prhs[0])))
    mexErrMsgTxt("Need to input string argument");
  std::string str(mxArrayToString(prhs[0]));

  std::map<std::string, void (*)(int nlhs, mxArray *plhs[],
    int nrhs, const mxArray *prhs[])>::iterator iFuncMap = funcMap.find(str);

  if (iFuncMap == funcMap.end())
    mexErrMsgTxt("Unknown function argument");

  (iFuncMap->second)(nlhs, plhs, nrhs-1, prhs+1);
}

void mex_set_velFilterWeight(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  if ((nrhs < 1) || (!mxIsNumeric(prhs[0])))
    mexErrMsgTxt("You must provide the filter weight");

  int nw = mxGetNumberOfElements(prhs[0]);

  int index = 0;
  if (nrhs >= 2)
    index = mxGetScalar(prhs[1]) - 1;

  double *weights = mxGetPr(prhs[0]);

  for (int i = 0; i < nw; i++) {
    *(mxGetPr(velFilterWeight) + ((index + i) % N_ENCODERS)) = *(weights+ i);
  }
}

void mex_set_accelFilterWeight(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  if ((nrhs < 1) || (!mxIsNumeric(prhs[0])))
    mexErrMsgTxt("You must provide the filter weight");
  
  int nw = mxGetNumberOfElements(prhs[0]);

  int index = 0;
  if (nrhs >= 2)
    index = mxGetScalar(prhs[1]) - 1;

  double *weights = mxGetPr(prhs[0]);

  for (int i = 0; i < nw; i++) {
    *(mxGetPr(accelFilterWeight) + ((index + i) % N_ENCODERS)) = *(weights+ i);
  }
}

void mex_set_encTicksPerRev(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  if ((nrhs < 1) || (!mxIsNumeric(prhs[0])))
    mexErrMsgTxt("You must provide the encoder ticks per revolution");

  int np = mxGetNumberOfElements(prhs[0]);

  int index = 0;
  if (nrhs >= 2)
    index = mxGetScalar(prhs[1]) - 1;

  double *ticks = mxGetPr(prhs[0]);

  for (int i = 0; i < np; i++) {
    *(mxGetPr(encTicksPerRev) + ((index + i) % N_ENCODERS)) = *(ticks + i);
  }
}

void mex_get(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  static bool init = false;
  static std::map<std::string, void (*)(int nlhs, mxArray *plhs[],
    int nrhs, const mxArray *prhs[])> funcMap;
  
  if (!init) {
    funcMap["velFilterWeight"] = mex_get_velFilterWeight;
    funcMap["accelFilterWeight"] = mex_get_accelFilterWeight;
    funcMap["encTicksPerRev"] = mex_get_encTicksPerRev;
    funcMap["fps"] = mex_get_fps;
  }

  if ((nrhs < 1) || (!mxIsChar(prhs[0])))
    mexErrMsgTxt("Need to input string argument");
  std::string str(mxArrayToString(prhs[0]));

  std::map<std::string, void (*)(int nlhs, mxArray *plhs[],
    int nrhs, const mxArray *prhs[])>::iterator iFuncMap = funcMap.find(str);

  if (iFuncMap == funcMap.end())
    mexErrMsgTxt("Unknown function argument");

  (iFuncMap->second)(nlhs, plhs, nrhs-1, prhs+1);
}

void mex_get_velFilterWeight(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  plhs[0] = velFilterWeight;
}

void mex_get_accelFilterWeight(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  plhs[0] = accelFilterWeight;
}

void mex_get_encTicksPerRev(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  plhs[0] = encTicksPerRev;
}

void mex_get_fps(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  plhs[0] = fps;
}

void mex_null(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  // Do nothing function
}

