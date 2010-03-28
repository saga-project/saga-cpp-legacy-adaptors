/*=============================================================================
Module          :ng_gen
Filename        :_stub_mmul2.c
RCS             :
        $Source:
        $Revision:
        $Author:
        $Data:Thu Jul  7 19:13:42 2005
        $Locker:
        $state:
=============================================================================*/
#include "ngEx.h"

#ifdef __cplusplus
extern "C"   {
#endif

extern void mmul(long n,double *A,double *B,double *C);

#ifdef __cplusplus
}
#endif

static char stub_description[] = "";


ngExpressionElement_t size0_1_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT, 0},
	{NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT, 0},
	{NG_EXPRESSION_VALUE_TYPE_OPCODE, 3},
	{NG_EXPRESSION_VALUE_TYPE_CONSTANT, 1},
	{NG_EXPRESSION_VALUE_TYPE_OPCODE, 1},
	{NG_EXPRESSION_VALUE_TYPE_CONSTANT, 1},
	{NG_EXPRESSION_VALUE_TYPE_OPCODE, 2},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};

ngExpressionElement_t start0_1_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_NONE, 0},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};

ngExpressionElement_t end0_1_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_NONE, 0},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};

ngExpressionElement_t step0_1_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_NONE, 0},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};


ngExpressionElement_t size0_2_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT, 0},
	{NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT, 0},
	{NG_EXPRESSION_VALUE_TYPE_OPCODE, 3},
	{NG_EXPRESSION_VALUE_TYPE_CONSTANT, 2},
	{NG_EXPRESSION_VALUE_TYPE_OPCODE, 1},
	{NG_EXPRESSION_VALUE_TYPE_CONSTANT, 3},
	{NG_EXPRESSION_VALUE_TYPE_OPCODE, 2},
	{NG_EXPRESSION_VALUE_TYPE_CONSTANT, 1},
	{NG_EXPRESSION_VALUE_TYPE_OPCODE, 1},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};

ngExpressionElement_t start0_2_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_NONE, 0},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};

ngExpressionElement_t end0_2_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_NONE, 0},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};

ngExpressionElement_t step0_2_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_NONE, 0},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};


ngExpressionElement_t size0_3_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT, 0},
	{NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT, 0},
	{NG_EXPRESSION_VALUE_TYPE_OPCODE, 3},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};

ngExpressionElement_t start0_3_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_NONE, 0},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};

ngExpressionElement_t end0_3_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_NONE, 0},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};

ngExpressionElement_t step0_3_0[] = {
	{NG_EXPRESSION_VALUE_TYPE_NONE, 0},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};


ngSubscriptInformation_t dims0_1[] = {
	{ size0_1_0, start0_1_0, end0_1_0, step0_1_0 },
};

ngSubscriptInformation_t dims0_2[] = {
	{ size0_2_0, start0_2_0, end0_2_0, step0_2_0 },
};

ngSubscriptInformation_t dims0_3[] = {
	{ size0_3_0, start0_3_0, end0_3_0, step0_3_0 },
};

ngArgumentInformation_t param_desc_0[] = {
	{ NG_ARGUMENT_IO_MODE_IN, NG_ARGUMENT_DATA_TYPE_LONG, 0, NULL,  NULL },
	{ NG_ARGUMENT_IO_MODE_IN, NG_ARGUMENT_DATA_TYPE_DOUBLE, 1, dims0_1,  NULL },
	{ NG_ARGUMENT_IO_MODE_IN, NG_ARGUMENT_DATA_TYPE_DOUBLE, 1, dims0_2,  NULL },
	{ NG_ARGUMENT_IO_MODE_OUT, NG_ARGUMENT_DATA_TYPE_DOUBLE, 1, dims0_3,  NULL },
};

static char method_description_0[] = "";

ngExpressionElement_t calc_order0[] = {
	{NG_EXPRESSION_VALUE_TYPE_NONE, 0},
	{NG_EXPRESSION_VALUE_TYPE_END, 0}
};

ngRemoteMethodInformation_t methods_desc[] = {
	{ "__default__", 0, calc_order0, 4, param_desc_0,
		0, /* server side shrink */
		method_description_0, 
	}, 
};

/** stub info declaration */

ngRemoteClassInformation_t remoteClassInfo = {
	"sample/mmul2",
	"1.0",
	stub_description, 
	(ngBackend_t)0, /* backend */
	(ngClassLanguage_t)0, /* lang */
	1, /* number of methods */
	methods_desc, 
};

/* Globals */
 int x,y,z; 
/* Status for obj */

/* Callback proxy */



/* Stub Main program */
int main(int argc, char ** argv){
	int error;
	int result;
	int methodID;
	int immediateExit;

	immediateExit = ngexStubAnalyzeArgumentWithExit(
		argc, argv, &remoteClassInfo, &error);
	if (immediateExit != 0) {
		exit(0);
	}


	result = ngexStubInitialize(argc, argv, &remoteClassInfo, &error);
	if (result != 1) {
		goto ng_stub_end;
	}

	while(1) {
		result = ngexStubGetRequest(&methodID, &error);
		if (result != NGEXI_INVOKE_METHOD) break;
		ngexStubCalculationStart(&error);
		switch (methodID) {
		case 0:
		{         /* __default__ */
			long n;
			double *A;
			double *B;
			double *C;
		ngexStubGetArgument(0,&n,&error);
		ngexStubGetArgument(1,&A,&error);
		ngexStubGetArgument(2,&B,&error);
		ngexStubGetArgument(3,&C,&error);
		mmul(n,A,B,C);
		}
		break;
		default:
			fprintf(stderr, "unknown method number");
		}
		result = ngexStubCalculationEnd(&error);
		if (result == 1){
		    continue;
		} else {
		    break;
		}
	}
ng_stub_end:
	ngexStubFinalize(&error);

	exit(0);
}
/*                     */
