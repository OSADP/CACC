#define	S_FUNCTION_NAME		thcal	/*‚rƒtƒ@ƒ“ƒNƒVƒ‡ƒ“–¼‚Ì’è‹`*/
#define	S_FUNCTION_LEVEL	2		/*ƒtƒ@ƒ“ƒNƒVƒ‡ƒ“ƒŒƒxƒ‹??‚Q*/


#include "simstruc.h"
#include "math.h"
//#include "size.h"

double	vt_in;
double	vt_buf;
double	vt_out;

double	vxg_in;
double sp_ini;


int first=0;
int counter=0;
double aux1 =0;
double aux2 =0;


static	void	mdlInitializeSizes( SimStruct *S )	/*ƒTƒCƒY?‰Šú‰»ŠÖ?”*/
{
	long	board_base;								/*ƒ{?[ƒhƒAƒhƒŒƒX—p•Ï?”*/

	ssSetNumSFcnParams( S, 0 );						/*ƒpƒ‰ƒ??[ƒ^ˆø?”‚Ì?Ý’è	*/
	ssSetNumContStates( S, 0 );						/**/
	ssSetNumDiscStates( S, 0 );						/**/
	ssSetNumInputPorts( S, 1 );						/*“ü—Í?”‚Ì?Ý’è	*/
	ssSetInputPortWidth( S, 0,  2 );				//‚R‚Â–Ú‚Ìˆø?”‚ªMatlab‚ÌMux‚ð’Ê‚¶‚Ä“ü—Í‚³‚ê‚é•Ï?”
													/*‚Q”Ô–Ú‚Ì“ü—Íƒf?[ƒ^‚Ì•?‚ð‚R—v‘f‚É‚Ì?Ý’è*/
	ssSetInputPortDirectFeedThrough( S, 0, 1 );		/*ƒ_ƒCƒŒƒNƒgƒtƒB?[ƒ‹ƒhƒXƒ‹?[‚Ì?Ý’è*/
	ssSetNumOutputPorts( S, 1 );					/*?o—Í?”‚Ì?Ý’è*/
	ssSetOutputPortWidth( S, 0, 1  );				//‚R‚Â–Ú‚Ìˆø?”‚ªMatlab?ã‚Å?o—Í‚³‚ê‚é•Ï?”‚Ì?”
													/*‚O”Ô–Ú‚Ì?o—Íƒf?[ƒ^‚Ì•?‚Ì?Ý’è*/
	ssSetNumSampleTimes( S, 1 );					/*ƒTƒ“ƒvƒ‹ƒ^ƒCƒ€?”‚Ì?Ý’è*/
	ssSetNumRWork( S, 0 );				/**/
	ssSetNumIWork( S, 0 );				/**/
	ssSetNumPWork( S, 0 );				/**/
	ssSetNumModes( S, 0 );				/**/
	ssSetNumNonsampledZCs( S, 0 );		/**/
	ssSetOptions(S,	0);					/**/

}

static	void	mdlInitializeSampleTimes( SimStruct *S )/*ƒTƒ“ƒvƒ‹ƒ^ƒCƒ€?‰Šú‰»ŠÖ?”*/
{
	ssSetSampleTime( S, 0, INHERITED_SAMPLE_TIME );						/*ƒTƒ“ƒvƒ‹ƒ^ƒCƒ€‚Ì?Ý’è?i‚P‚O‚O‚?‚“–ˆ‚ÉŽÀ?s‚·‚é?j*/
	ssSetOffsetTime( S, 0, 0.0 );						/*ƒIƒtƒZƒbƒgƒ^ƒCƒ€‚Ì?Ý’è*/
}






#define	MDL_INITIALIZE_CONDITIONS				/*’è‹`?F‚à‚µ’è‹`‚µ‚Ä‚¢‚È‚©‚Á‚½‚ç?iundef?j‰º‹L‚ð–³Œø*/
#if	defined( MDL_INITIALIZE_CONDITIONS )		/*‚à‚µ‚à?A’è‹`‚µ‚Ä‚ ‚Á‚½‚ç—LŒø‚É‚·‚é*/
static	void	mdlInitializeConditions( SimStruct *S )	
{				/*?‰Šú?Ý’èŠÖ?”?FƒTƒ“ƒvƒŠƒ“ƒOŽüŠú‚ªŠJŽn‚³‚ê‚é‘O‚Ì?‰Šú?Ý’è*/

}
#endif							/*‚à‚µ‚à?A’è‹`‚µ‚Ä‚ ‚Á‚½‚ç—LŒø‚É‚·‚é	*/

#define	MDL_START								/*’è‹`?F‚à‚µ’è‹`‚µ‚Ä‚¢‚È‚©‚Á‚½‚ç?iundef?j‰º‹L‚ð–³Œø*/
#if	defined( MDL_START )						/*‚à‚µ‚à?A’è‹`‚µ‚Ä‚ ‚Á‚½‚ç—LŒø‚É‚·‚é*/
	static	void	mdlStart( SimStruct *S )	/*ŠJŽnŽžŠÖ?”*/
	{											/*?‰Šú?Ý’èŠÖ?”?FƒTƒ“ƒvƒŠƒ“ƒOŽüŠú‚ªŠJŽn‚³‚ê‚é‘O‚Ì?‰Šú?Ý’è*/

	}
#endif							/*‚à‚µ‚à?A’è‹`‚µ‚Ä‚ ‚Á‚½‚ç—LŒø‚É‚·‚é*/

static	void	mdlOutputs( SimStruct *S, int_T tid )	/*?o—ÍŠÖ?”*/
{
	InputRealPtrsType	uPtrs =	ssGetInputPortRealSignalPtrs( S, 0 );
	real_T			*y = ssGetOutputPortRealSignal( S, 0 );	
	

	vt_in = *uPtrs[0];
	vxg_in = *uPtrs[1];
    
 if (vxg_in == 1)
 {   
    if (first == 0)
    {
        sp_ini = 4.0; // vt_in;   
        first = 1;
    }
    
    if (counter < 3001)
    {
        vt_out = sp_ini; 
        counter = counter + 1;
    }    
    if (counter > 3000 && counter < 3501)
    {
        vt_out = sp_ini-1*(aux1/500.0); 
        counter = counter + 1;
        aux1=aux1+1;
    }     
    
    if (counter > 3500 && counter < 4501)
    {
        vt_out = sp_ini-1; 
        counter = counter + 1;
        aux1=0;
        aux2=0;
    }    
    
    if (counter > 4500 && counter < 5001)
    {
        vt_out = sp_ini-1+3*(aux2/500.0); 
        counter = counter + 1;
        aux2=aux2+1;
    }   
    
    if (counter > 5000 && counter < 6001)
    {
        vt_out = sp_ini+2; 
        counter = counter + 1;
        aux1=0;
        aux2=0;
    }    
    
    if (counter > 6000 && counter < 6501)
    {
        vt_out = sp_ini+2-4*(aux1/500.0); 
        counter = counter + 1;
        aux1=aux1+1;
    }     
    
    if (counter > 6500 && counter < 8501)
    {
        vt_out = sp_ini-2; 
        counter = counter + 1;
        aux1=0;
        aux2=0;
    }    
    
    if (counter > 8500 && counter < 9001)
    {
        vt_out = sp_ini-2+2*(aux2/500.0); 
        counter = counter + 1;
        aux2=aux2+1;
    }   
    
    if (counter > 9000 && counter < 10001)
    {
        vt_out = sp_ini; 
        counter = counter + 1;
        aux1=0;
        aux2=0;
    } 
    
    if (counter > 10000 && counter < 10501)
    {
        vt_out = sp_ini+1*(aux1/500.0); 
        counter = counter + 1;
        aux1=aux1+1;
    }     
    
    if (counter > 10500 && counter < 11501)
    {
        vt_out = sp_ini+1; 
        counter = counter + 1;
        aux1=0;
        aux2=0;
    }    
    
    if (counter > 11500 && counter < 12001)
    {
        vt_out = sp_ini+1-2*(aux2/500.0); 
        counter = counter + 1;
        aux2=aux2+1;
    }   
       
    if (counter > 12000 && counter < 13001)
    {
        vt_out = sp_ini-1; 
        counter = counter + 1;
        aux1=0;
        aux2=0;
    } 
    
    if (counter > 13000 && counter < 13501)
    {
        vt_out = sp_ini-1-1.5*(aux1/500.0); 
        counter = counter + 1;
        aux1=aux1+1;
    }     
    
    
    if (counter > 13500 && counter < 14501)
    {
        vt_out = sp_ini-2.5; 
        counter = counter + 1;
        aux1=0;
        aux2=0;
    }    
    
    if (counter > 14500 && counter < 15001)
    {
        vt_out = sp_ini-2.5-1.5*(aux2/500.0); 
        counter = counter + 1;
        aux2=aux2+1;
    }   
    
    if (counter > 15000)
    {
        vt_out = sp_ini-4; 
        counter = counter + 1;
        aux1=0;
        aux2=0;
    }    
 }
 if (vxg_in == 0)
 {   
     counter = 0;
     aux1 = 0;
     aux2 = 0;
 }
     
    
    

	y[0] = vt_out;
	


}

#undef	MDL_UPDATE						/*’è‹`*/
#if	defined( MDL_UPDATE )					/*‚à‚µ‚à?A’è‹`‚µ‚Ä‚ ‚Á‚½‚ç—LŒø‚É‚·‚é*/
	static	void	mdlUpdate( SimStruct *S, int_T tid )	/*?X?VŠÖ?”*/
	{
	}
#endif								/*‚à‚µ‚à?A’è‹`‚µ‚Ä‚ ‚Á‚½‚ç—LŒø‚É‚·‚é*/



#undef	MDL_DERIVATIVES						/**/
#if	defined( MDL_DERIVATIVES )				/*‚à‚µ‚à?A’è‹`‚µ‚Ä‚ ‚Á‚½‚ç—LŒø‚É‚·‚é*/
	static	void	mdlDerivatives( SimStruct *S )		/*”÷•ª—pŠÖ?”*/
	{
	}
#endif								/*‚à‚µ‚à?A’è‹`‚µ‚Ä‚ ‚Á‚½‚ç—LŒø‚É‚·‚é*/



static	void	mdlTerminate( SimStruct *S )			/*?I’[ŠÖ?”*/
{
}

#ifdef	MATLAB_MEX_FILE						/*‚r‚h‚l‚t‚k‚h‚m‚j‚ðŽg—p‚·‚é?ê?‡‚Í?A	*/
	#include	"simulink.c"				/*ƒCƒ“ƒNƒ‹?[ƒh‚·‚é			*/
#else								/*‚`‚•‚”‚?‚‚‚?‚˜‚ðŽg—p‚·‚é?ê?‡‚Í?A	*/
	#include	"cg_sfun.h"				/*ƒCƒ“ƒNƒ‹?[ƒh‚·‚é			*/
#endif
