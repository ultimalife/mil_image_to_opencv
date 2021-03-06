/*************************************************************************************/
/*
 * File name: MDigProcess.cpp
 *
 * Synopsis:  This program shows the use of the MdigProcess() function to do perform
 *            real-time processing.
 *
 *            The user's processing code is written in a hook function that
 *            will be called for each frame grabbed (see ProcessingFunction()).
 *
 *      Note: The average processing time must be shorter than the grab time or
 *            some frames will be missed. Also, if the processing results are not
 *            displayed and the frame count is not drawn or printed, the
 *            CPU usage is reduced significantly.
 */
#include <mil.h>

#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

#pragma comment(lib,"opencv_core331.lib")
#pragma comment(lib,"opencv_imgcodecs331.lib")
#pragma comment(lib,"opencv_highgui331.lib")
 /* Number of images in the buffering grab queue.
	Generally, increasing this number gives better real-time grab.
  */
#define BUFFERING_SIZE_MAX 22

  /* User's processing function prototype. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId,void MPTYPE *HookDataPtr);

/* User's processing function hook data structure. */
typedef struct
{
	//MIL_ID  MilImageDisp;
	MIL_INT ProcessedImageCount;
	IplImage *pcvImage;
} HookDataStruct;

/* Main function. */
/* ---------------*/

int MosMain(void)
{
	MIL_ID MilApplication;
	MIL_ID MilSystem;
	MIL_ID MilDigitizer;
	//MIL_ID MilDisplay;
	//MIL_ID MilImageDisp;
	MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX] = { 0 };
	MIL_INT MilGrabBufferListSize;
	MIL_INT ProcessFrameCount = 0;
	MIL_INT NbFrames = 0, n = 0;
	MIL_INT numberOfDigitizer = 0;
	MIL_DOUBLE ProcessFrameRate = 0;
	HookDataStruct UserHookData;

	long lWidth, lHeight, lChannels;
	IplImage *pcvImage;

	/* Allocate defaults. */
	//MappAllocDefault(M_SETUP, &MilApplication, &MilSystem, &MilDisplay, &MilDigitizer, &MilImageDisp);

	MappAlloc(M_DEFAULT, &MilApplication);
	/* Allocate the grab buffers and clear them. */
	MappControl(M_ERROR, M_PRINT_DISABLE);

	MsysAlloc(M_SYSTEM_SETUP, M_DEF_SYSTEM_NUM, M_SETUP, &MilSystem);
	// Inquire the number of digitizers available on the system [CALL TO MIL]
	MsysInquire(MilSystem, M_DIGITIZER_NUM, &numberOfDigitizer);
	// edit your DCF file below
	MdigAlloc(MilSystem, M_DEV0, "d:/DCF/MV-BS20C.dcf", M_DEFAULT, &MilDigitizer);

	MdigControl(MilDigitizer, M_GRAB_TIMEOUT, M_INFINITE);
	MdigControl(MilDigitizer, M_CAMERALINK_CC1_SOURCE, M_GRAB_EXPOSURE + M_TIMER1);
	MdigControl(MilDigitizer, M_GRAB_MODE, M_ASYNCHRONOUS);

	lChannels=MdigInquire(MilDigitizer, M_SIZE_BAND, M_NULL);
	lWidth =MdigInquire(MilDigitizer, M_SIZE_X, M_NULL);
	lHeight =MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL);

	//cv
	pcvImage = cvCreateImage(cvSize((int)lWidth,(int)lHeight),IPL_DEPTH_8U,(int)lChannels);
	cvSetZero(pcvImage);

	for (MilGrabBufferListSize = 0; MilGrabBufferListSize < BUFFERING_SIZE_MAX; MilGrabBufferListSize++)
	{
		MbufAlloc2d(MilSystem,lWidth,lHeight,M_DEF_IMAGE_TYPE,M_IMAGE + M_GRAB + M_PROC,&MilGrabBufferList[MilGrabBufferListSize]);

		if (MilGrabBufferList[MilGrabBufferListSize])
		{
			MbufClear(MilGrabBufferList[MilGrabBufferListSize], 0xFF);
		}
		else
			break;
	}
	MappControl(M_ERROR, M_PRINT_ENABLE);
	/* Free buffers to leave space for possible temporary buffers. */
	for (n = 0; n < 2 && MilGrabBufferListSize; n++)
	{
		MilGrabBufferListSize--;
		MbufFree(MilGrabBufferList[MilGrabBufferListSize]);
	}

	/*MbufAlloc2d(MilSystem, lWidth, lHeight, M_DEF_IMAGE_TYPE, M_IMAGE + M_GRAB + M_PROC, &MilImageDisp);
	MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay);*/

	/* Print a message. */
	MosPrintf(MIL_TEXT("\nMULTIPLE BUFFERED PROCESSING.\n"));
	MosPrintf(MIL_TEXT("-----------------------------\n\n"));
	MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));

	/* Grab continuously on the display and wait for a key press. */
	/*MdigGrabContinuous(MilDigitizer, MilImageDisp);
	MosGetch();*/

	/* Halt continuous grab. */
	//MdigHalt(MilDigitizer);

	/* Initialize the User's processing function data structure. */
	//UserHookData.MilImageDisp = MilImageDisp;
	UserHookData.ProcessedImageCount = 0;
	UserHookData.pcvImage = pcvImage;
	/* Start the processing. The processing function is called for every frame grabbed. */
	MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,M_START, M_DEFAULT, ProcessingFunction, &UserHookData);


	/* NOTE: Now the main() is free to perform other tasks
	while the processing is executing. */
	/* -------------------------------------------------------------------------------- */


	/* Print a message and wait for a key press after a minimum number of frames. */
	MosPrintf(MIL_TEXT("Press <Enter> to stop.\n\n"));
	MosGetch();

	/* Stop the processing. */
	MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,M_STOP, M_DEFAULT, ProcessingFunction, &UserHookData);


	/* Print statistics. */
	MdigInquire(MilDigitizer, M_PROCESS_FRAME_COUNT, &ProcessFrameCount);
	MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE, &ProcessFrameRate);
	MosPrintf(MIL_TEXT("\n\n%ld frames grabbed at %.1f frames/sec (%.1f ms/frame).\n"),
		ProcessFrameCount, ProcessFrameRate, 1000.0 / ProcessFrameRate);
	MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
	MosGetch();

	/* Free the grab buffers. */
	while (MilGrabBufferListSize > 0)
		MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);

	/* Release defaults. */
	//MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);

	/*MdispFree(MilDisplay);
	MbufFree(MilImageDisp);*/
	MdigFree(MilDigitizer);
	MsysFree(MilSystem);
	MappFree(MilApplication);
	cvReleaseImage(&pcvImage);
	return 0;
}

/* User's processing function called every time a grab buffer is modified. */
/* -----------------------------------------------------------------------*/

/* Local defines. */
#define STRING_LENGTH_MAX  40
#define STRING_POS_X       20
#define STRING_POS_Y       20

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId,void MPTYPE *HookDataPtr)
{
	HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
	MIL_ID ModifiedBufferId;
	MIL_INT GrabbedBufferIndex;
	MIL_TEXT_CHAR Text[STRING_LENGTH_MAX] = { MIL_TEXT('\0'), };
	IplImage *pcvImage=UserHookDataPtr->pcvImage;

	/* Retrieve the MIL_ID of the grabbed buffer. */
	MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);
	MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_INDEX, &GrabbedBufferIndex);

	/* Print and draw the frame count. */
	UserHookDataPtr->ProcessedImageCount++;
	MosPrintf(MIL_TEXT("Processing frame #%d.\r"), UserHookDataPtr->ProcessedImageCount);
	MosSprintf(Text, STRING_LENGTH_MAX, MIL_TEXT("frm=%ld BufInd=%02d"),UserHookDataPtr->ProcessedImageCount,GrabbedBufferIndex);
	MgraText(M_DEFAULT, ModifiedBufferId, STRING_POS_X, STRING_POS_Y, Text);

	/* Perform the processing and update the display. */
	//MimArith(ModifiedBufferId, M_NULL, UserHookDataPtr->MilImageDisp, M_NOT);

	MbufGet(ModifiedBufferId, pcvImage->imageData);
	cvShowImage("image show", pcvImage);
	cvWaitKey(10);
	return 0;
}
