/********************************************************************************/
/* 
 * File name: MDigGrab.cpp 
 *
 * Synopsis:  This program demonstrates how to grab from a camera in
 *            continuous and monoshot mode.
 */
#include <mil.h> 
#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

#pragma comment(lib,"opencv_core331.lib")
#pragma comment(lib,"opencv_imgcodecs331.lib")
#pragma comment(lib,"opencv_highgui331.lib")

struct TestStruct
{
	MIL_ID MilImage;
	IplImage *pcvImage;
};

MIL_INT MFTYPE FrameEndHookHandler(MIL_INT HookType, MIL_ID EventId, void MPTYPE* UserDataPtr);

int MosMain(void)
{ 
   MIL_ID MilApplication,  /* Application identifier.  */
          MilSystem,       /* System identifier.       */
          MilDisplay,      /* Display identifier.      */
          MilDigitizer,    /* Digitizer identifier.    */ 
          MilImage;        /* Image buffer identifier. */
   MIL_INT numberOfDigitizer = 0;
   long nWidth, nHeight, nChannels;
   IplImage *pcvImage;
   TestStruct stTest;
   /* Allocate defaults. */
   //MappAllocDefault(M_SETUP, &MilApplication, &MilSystem, &MilDisplay, &MilDigitizer, &MilImage);

   MappAlloc(M_DEFAULT, &MilApplication);
   /* Allocate the grab buffers and clear them. */
   MappControl(M_ERROR, M_PRINT_DISABLE);

   MsysAlloc(M_SYSTEM_SETUP, M_DEF_SYSTEM_NUM, M_SETUP, &MilSystem);
   // Inquire the number of digitizers available on the system [CALL TO MIL]
   MsysInquire(MilSystem, M_DIGITIZER_NUM, &numberOfDigitizer);
   MdigAlloc(MilSystem, M_DEV0, "d:/DCF/MV-BS20C.dcf", M_DEFAULT, &MilDigitizer);

   MdigControl(MilDigitizer, M_GRAB_TIMEOUT, M_INFINITE);
   MdigControl(MilDigitizer, M_CAMERALINK_CC1_SOURCE, M_GRAB_EXPOSURE + M_TIMER1);
   MdigControl(MilDigitizer, M_GRAB_MODE, M_ASYNCHRONOUS);

   nChannels = MdigInquire(MilDigitizer, M_SIZE_BAND, M_NULL);
   nWidth = MdigInquire(MilDigitizer, M_SIZE_X, M_NULL);
   nHeight = MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL);

   MbufAlloc2d(MilSystem, nWidth, nHeight, M_DEF_IMAGE_TYPE, M_IMAGE + M_GRAB + M_PROC+ M_DISP, &MilImage);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay);

   //open cv display
   pcvImage=cvCreateImage(cvSize((int)nWidth, (int)nHeight), IPL_DEPTH_8U, (int)nChannels);
   cvSetZero(pcvImage);

   stTest.MilImage = MilImage;
   stTest.pcvImage = pcvImage;
   //mil call back
   MdigHookFunction(MilDigitizer, M_GRAB_FRAME_END, FrameEndHookHandler, (void*)&stTest);
   /* Grab continuously. */
   MdigGrabContinuous(MilDigitizer, MilImage);

   /* When a key is pressed, halt. */
   MosPrintf(MIL_TEXT("\nDIGITIZER ACQUISITION:\n"));
   MosPrintf(MIL_TEXT("----------------------\n\n"));
   MosPrintf(MIL_TEXT("Continuous image grab in progress.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to stop.\n\n"));
   MosGetch();

   /* Stop continuous grab. */
   MdigHalt(MilDigitizer);
   //mil call back
   MdigHookFunction(MilDigitizer, M_GRAB_FRAME_END + M_UNHOOK, FrameEndHookHandler, (void*)&stTest);

   /* Pause to show the result. */
   MosPrintf(MIL_TEXT("Continuous grab stopped.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to do a single image grab.\n\n"));
   MosGetch();

   /* Monoshot grab. */
   MdigGrab(MilDigitizer, MilImage);

   /* Pause to show the result. */
   MosPrintf(MIL_TEXT("Displaying the grabbed image.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Free defaults. */
   //MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImage);

   MdispFree(MilDisplay);
   MbufFree(MilImage);
   MdigFree(MilDigitizer);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   cvReleaseImage(&pcvImage);
   return 0;
}

MIL_INT MFTYPE FrameEndHookHandler(MIL_INT HookType, MIL_ID EventId, void MPTYPE* UserDataPtr)
{
	TestStruct *testStruct = (TestStruct *)UserDataPtr;
	IplImage *pcvImage = testStruct->pcvImage;
	MIL_ID MilImage = testStruct->MilImage;

	MbufGet(MilImage, pcvImage->imageData);
	cvNamedWindow("showimg", CV_WINDOW_NORMAL);
	cvShowImage("showimg", pcvImage);
	cvWaitKey(10);
	return M_NULL;
}
