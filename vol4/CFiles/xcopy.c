/* ************************************************************************
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ************************************************************************/


//#define DEBUG_COPY

#include <stdio.h>
#include <string.h>
#include "../HFiles/clBLAS.h"
#include "../HFiles/devinfo.h"
#include "../HFiles/clblas-internal.h"
#include "../HFiles/solution_seq.h"


clblasStatus
doCopy(
	CLBlasKargs *kargs,
    size_t N,
    const cl_mem X,
    size_t offx,
    int incx,
    cl_mem Y,
    size_t offy,
    int incy,
    cl_uint numCommandQueues,
    cl_command_queue *commandQueues,
    cl_uint numEventsInWaitList,
    const cl_event *eventWaitList,
    cl_event *events)
	{
		cl_int err;
		ListHead seq;
        clblasStatus retCode = clblasSuccess;

		if (!clblasInitialized) {
        return clblasNotInitialized;
		}

		/* Validate arguments */

        retCode = checkMemObjects(X, Y, X, false, X_VEC_ERRSET, Y_VEC_ERRSET, X_VEC_ERRSET );
		if (retCode) {
			#ifdef DEBUG_COPY
			printf("Invalid mem object..\n");
			#endif
            return retCode;
		}

		// Check wheather enough memory was allocated

		if ((retCode = checkVectorSizes(kargs->dtype, N, X, offx, incx, X_VEC_ERRSET))) {
			#ifdef DEBUG_COPY
			printf("Invalid Size for X\n");
			#endif
            return retCode;
		}
		if ((retCode = checkVectorSizes(kargs->dtype, N, Y, offy, incy, Y_VEC_ERRSET))) {
			#ifdef DEBUG_COPY
			printf("Invalid Size for Y\n");
			#endif
            return retCode;
		}
		///////////////////////////////////////////////////////////////

		if ((commandQueues == NULL) || (numCommandQueues == 0))
		{
			return clblasInvalidValue;
		}

		/* numCommandQueues will be hardcoded to 1 as of now. No multi-gpu support */
		numCommandQueues = 1;
		if (commandQueues[0] == NULL)
		{
			return clblasInvalidCommandQueue;
		}

		if ((numEventsInWaitList !=0) && (eventWaitList == NULL))
		{
			return clblasInvalidEventWaitList;
		}

		kargs->N = N;
		kargs->A = X;
		kargs->offBX = offx;
		kargs->ldb.Vector = incx;	// Will be using this as incx
		kargs->B = Y;
		kargs->offCY = offy;
		kargs->ldc.Vector = incy;	// Will be using this as incy

		#ifdef DEBUG_COPY
		printf("Calling makeSolutionSeq from DoCopy: COPY\n");
		#endif

		listInitHead(&seq);
		err = makeSolutionSeq(CLBLAS_COPY, kargs, numCommandQueues, commandQueues,
        					        numEventsInWaitList, eventWaitList, events, &seq);
		if (err == CL_SUCCESS) {
       		err = executeSolutionSeq(&seq);
		}

		freeSolutionSeq(&seq);

		return (clblasStatus)err;
	}





clblasStatus
clblasScopy(
    size_t N,
    const cl_mem X,
    size_t offx,
    int incx,
    cl_mem Y,
    size_t offy,
    int incy,
    cl_uint numCommandQueues,
    cl_command_queue *commandQueues,
    cl_uint numEventsInWaitList,
    const cl_event *eventWaitList,
    cl_event *events)
	{
		CLBlasKargs kargs;

		#ifdef DEBUG_COPY
		printf("\nSCOPY Called\n");
		#endif

		memset(&kargs, 0, sizeof(kargs));
		kargs.dtype = TYPE_FLOAT;

		return doCopy(&kargs, N, X, offx, incx, Y, offy, incy,
						numCommandQueues, commandQueues, numEventsInWaitList, eventWaitList, events);
	}

clblasStatus
clblasDcopy(
    size_t N,
    const cl_mem X,
    size_t offx,
    int incx,
    cl_mem Y,
    size_t offy,
    int incy,
    cl_uint numCommandQueues,
    cl_command_queue *commandQueues,
    cl_uint numEventsInWaitList,
    const cl_event *eventWaitList,
    cl_event *events)
	{
		CLBlasKargs kargs;

		#ifdef DEBUG_COPY
		printf("\nDCOPY Called\n");
		#endif

		memset(&kargs, 0, sizeof(kargs));
		kargs.dtype = TYPE_DOUBLE;

		return doCopy(&kargs, N, X, offx, incx, Y, offy, incy,
						numCommandQueues, commandQueues, numEventsInWaitList, eventWaitList, events);
	}

clblasStatus
clblasCcopy(
    size_t N,
    const cl_mem X,
    size_t offx,
    int incx,
    cl_mem Y,
    size_t offy,
    int  incy,
    cl_uint numCommandQueues,
    cl_command_queue *commandQueues,
    cl_uint numEventsInWaitList,
    const cl_event *eventWaitList,
    cl_event *events)
	{
		CLBlasKargs kargs;

		#ifdef DEBUG_COPY
		printf("\nCCOPY Called\n");
		#endif

		memset(&kargs, 0, sizeof(kargs));
		kargs.dtype = TYPE_COMPLEX_FLOAT;

		return doCopy(&kargs, N, X, offx, incx, Y, offy, incy,
						numCommandQueues, commandQueues, numEventsInWaitList, eventWaitList, events);
	}

clblasStatus
clblasZcopy(
    size_t N,
    const cl_mem X,
    size_t offx,
    int incx,
    cl_mem Y,
    size_t offy,
    int incy,
    cl_uint numCommandQueues,
    cl_command_queue *commandQueues,
    cl_uint numEventsInWaitList,
    const cl_event *eventWaitList,
    cl_event *events)
	{
		CLBlasKargs kargs;

		#ifdef DEBUG_COPY
		printf("\nZCOPY Called\n");
		#endif

		memset(&kargs, 0, sizeof(kargs));
		kargs.dtype = TYPE_COMPLEX_DOUBLE;

		return doCopy(&kargs, N, X, offx, incx, Y, offy, incy,
						numCommandQueues, commandQueues, numEventsInWaitList, eventWaitList, events);
	}
