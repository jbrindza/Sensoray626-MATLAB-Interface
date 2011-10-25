/**
 *  Jordan Brindza
 *  brindza@seas.upenn.edu
 *  University of Pennsylvania
 *  December 2010
 */

#include "s626Thread.h"
#include "s626Functions.h"
#include "s626_util.h"
#include "haptics_util.h"
#include <string>
#include <map>
#include "mex.h"

void mexExit(void) {
  printf("Exiting S626...\n");
  s626_functions_cleanup();
  s626_thread_cleanup();
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  static bool init = false;
  static std::map<std::string, void (*)(int nlhs, mxArray *plhs[],
    int nrhs, const mxArray *prhs[])> funcMap;

  if (!init) {
    funcMap["encoder"] = mex_encoder;
    funcMap["zero"] = mex_zero;
    funcMap["dac"] = mex_dac;
    funcMap["set"] = mex_set;
    funcMap["get"] = mex_get;

    s626_functions_init();
    if (s626_thread_init() < 0)
      mexErrMsgTxt("Error Initializing S626 Thread");
    usleep(300000);

    mexAtExit(mexExit);
    init = true;
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

