/*************************************************************************
 * FILE:
 *   eth_bit_encoder_sfcn.c
 *
 * DESCRIPTION:
 *
 * REMARKS:
 *
 * AUTHOR(S):
 *     L. Juhász 
 *
 *
 * Copyright (c) 1997-2004 dSPACE GmbH, GERMANY
 *
 * 
 * Description: 8 bit inputs to one byte output
 * 
 * Inputs: 
 *
 *		1)	Bit7 
 *		2)	Bit6
 *		...
 *		8)	Bit0
 *
 * Outputs:
 *		1) Byte == Bit7*128+Bit6*64+....+Bit0*1
 *
 *
 **************************************************************************/


/**************************************************************************\
*
*  constant, macro, and type definitions
*
\**************************************************************************/
#define S_FUNCTION_NAME  eth_bit_encoder_sfcn
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"
#include <math.h>

#ifndef MATLAB_MEX_FILE
 #include <brtenv.h>
 #include <dstypes.h>
#endif


/* need to include simstruc.h for the definition of the SimStruct and
   its associated macro definitions */

#define MDL_SET_WORK_WIDTHS
#define NUM_SAMPLE_TIMES        (1)
#define OUTPUT_WIDTH            (1) //( (uint32_T ) (mxGetPr(ssGetSFcnParam(S, 0))[0]) )
#define INPUT_WIDTH             (8) 


/**************************************************************************\
*
* FUNCTION:
*   initializes the size array
*
* SYNTAX:
*   static void mdlInitializeSizes(SimStruct* _tl_S)
*
* DESCRIPTION:
*   The sizes array is used by Simulink to determine the S-function blocks'
*   characteristics (number of inputs, outputs, states, etc.).
*
\**************************************************************************/
static void mdlInitializeSizes(SimStruct* S)
{
  int i=0;
  ssSetNumSFcnParams(S,0);          /* number of expected parameters */
  if(ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
  {
    /* return if number of expected != number of actual parameters */
    return;
  }

  /* set number of ports */
  if(!ssSetNumInputPorts (S,INPUT_WIDTH))   return;
  if(!ssSetNumOutputPorts(S,OUTPUT_WIDTH)) return;

 for (i=0;i<INPUT_WIDTH;i++) 
 {
  /* input port width(s) and feedthrough */
  ssSetInputPortWidth(S,i,1);
  ssSetInputPortDirectFeedThrough(S,i,1);
  /* any numeric datytype is supported */
  // ssSetInputPortDataType(S,i,DYNAMICALLY_TYPED);
}



  /* output port width(s) */
  for (i=0;i<OUTPUT_WIDTH;i++) 
  {
  ssSetOutputPortWidth(S,i,1);
  // ssSetOutputPortReusable(S,i,0);
  };

  /* set remaining fields of sizes array */
  ssSetNumSampleTimes(   S,1);                   /* number of sample times                */
  ssSetNumIWork(         S,DYNAMICALLY_SIZED);   /* number of integer work vector elements */
  ssSetNumPWork(         S,DYNAMICALLY_SIZED);   /* number of pointer work vector elements*/
  ssSetNumDWork(         S,DYNAMICALLY_SIZED);   /* number of datatype work vector elements*/
} /* mdlInitializeSizes */





/**************************************************************************\
*
* FUNCTION:
*   initializes DWork vectors
*
* SYNTAX:
*   static void mdlSetWorkWidths(SimStruct* _tl_S)
*
* DESCRIPTION:
*   The DWork vector is used for work and state variables.
*
\**************************************************************************/
static void mdlSetWorkWidths(SimStruct* S)
{
  ssSetNumIWork(         S,0);   /* number of integer work vector elements */
  ssSetNumPWork(         S,0);   /* number of pointer work vector elements */
  ssSetNumDWork(         S,0);   /* number of datatype work vector elements */

} /* mdlSetWorkWidths */


/**************************************************************************\
*
* FUNCTION:
*   initializes the sample time
*
* SYNTAX:
*   static void  mdlInitializeSampleTimes(SimStruct* _tl_S)
*
* DESCRIPTION:
*   Initializes the sample time.
*
\**************************************************************************/
static void mdlInitializeSampleTimes(SimStruct* _tl_S)
{
 
} /* mdlInitializeSampleTimes */


/**************************************************************************\
*
* FUNCTION:
*   start
*
* SYNTAX:
*   static void mdlStart(SimStruct* _tl_S)
*
* DESCRIPTION:
*   Parameter checks as well as work variable and log object initializations
*   are performed here.
*
\**************************************************************************/
static void mdlStart(SimStruct* _tl_S)
{


} /* mdlStart */


/**************************************************************************\
*
* FUNCTION:
*   calculates outputs
*
* SYNTAX:
*   static void mdlOutputs(SimStruct* _tl_S, int_T tid)
*
* DESCRIPTION:
*   The output ports are calculated with the user-supplied code module.
*
\**************************************************************************/
static void mdlOutputs(SimStruct* S, int_T tid)
{
 int i;	       
 unsigned char bit[INPUT_WIDTH];
 unsigned char u8out=0;
 InputRealPtrsType   uPtrs[INPUT_WIDTH]  ;



 for (i=0;i<INPUT_WIDTH;i++) 
 {
  uPtrs[i]  = ssGetInputPortRealSignalPtrs(S,i);
 }



  for (i=0;i<INPUT_WIDTH;i++) 
 {
   bit[i]=(unsigned char) (*uPtrs[i][0]);
   if (bit[i]>1) bit[i]=1;
   // else if (bit[i]<0) bit [i]=0;
   u8out=(((unsigned char) bit[i] << (INPUT_WIDTH-1-i))| u8out);
}
   

  /* end of custom code for output evaluation */

  /* rewrite evaluated output(s) */
  ssGetOutputPortRealSignal(S,0)[0]=u8out;

} /* mdlOutputs */



/**************************************************************************\
*
* FUNCTION:
*   called when simulation is terminated
*
* SYNTAX:
*   static void mdlTerminate(SimStruct* _tl_S)
*
* DESCRIPTION:
*   The mdlTerminate function is called when the simulation is being
*   terminated.
*
\**************************************************************************/
static void mdlTerminate(SimStruct* _tl_S)
{


} /* mdlTerminate */




#ifdef   MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
