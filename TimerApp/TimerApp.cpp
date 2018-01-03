#include "stdafx.h"
#include <windows.h>
#include <iostream>



WORD gloTransTime = 300; 
DWORD gloTransTimerId = 100;

BOOL gloIsTransTimeElapsed = FALSE; 

BOOL gloCanResetTimer = TRUE; 

HANDLE glohKernelEvent = NULL; 

CRITICAL_SECTION gloCriticalSection; 

HANDLE glohTimerThread = NULL; 

HANDLE glohWaitableTimer = NULL; 

DWORD gloThreadID = 0;

DWORD Timer_HandleTimerElapse();
DWORD Timer_SetDefaultTimerValues();
DWORD Timer_TerminateAndClearSession();
DWORD Timer_ThreadTimerProc();

DWORD Timer_InitSyncObjsAndTimerThread()
{
	DWORD	RetChk = 0;

	Timer_SetDefaultTimerValues();

	InitializeCriticalSection(&gloCriticalSection);


	if (NULL == glohTimerThread)
		glohTimerThread = CreateThread(NULL,                 // Security Attributes
			0,                       // Default Stack size
			(PTHREAD_START_ROUTINE)Timer_ThreadTimerProc, // Thread Function
			NULL,                    // Parameter to thread
			CREATE_SUSPENDED,       // Thread Created but still not started yet
			&gloThreadID// Thread identifier
		);

	if (NULL == glohKernelEvent)
		glohKernelEvent = CreateEvent(NULL,    // Security Attributes
			TRUE,    // Auto Reset event object
			FALSE,   // Initially Non-Signaled
			(LPCWSTR)"EmvKernelTimerThreadEvent"     // Name for Event object
		);


	if (NULL == glohWaitableTimer)
		glohWaitableTimer =
		CreateWaitableTimer(NULL,       // Security Attributes
			FALSE,      // Synchronization timer. 
			(LPCWSTR)"EMVKernelWaitableTimer" // The name of the timer object. 
		);

	return 0;

}

DWORD Timer_SetDefaultTimerValues()
{
	DWORD RetChk = 0;

	gloTransTime = 60; 

	return 0;
}

DWORD Timer_ThreadTimerProc()
{
	__int64  qwDueTime;
	LARGE_INTEGER liDueTime;

	HANDLE hMultiObjHandleArray[] = { glohKernelEvent, 		glohWaitableTimer };

	DWORD RetChk = 0;

	printf("Thread Started\n");

	if (gloTransTime <= 0)
		gloTransTime = gloTransTime; 

	qwDueTime = 0 - ((__int64)gloTransTime * (__int64)10000000);
	liDueTime.LowPart = (DWORD)(qwDueTime & 0xFFFFFFFF);
	liDueTime.HighPart = (LONG)(qwDueTime >> 32);

	while (TRUE) /* Start a Infinite loop. */
	{
		printf("Inside While\n");
		
		DWORD RetVal =
			WaitForMultipleObjects(2,       
				hMultiObjHandleArray, 
				FALSE, 
				INFINITE 
			);

		switch (RetVal - WAIT_OBJECT_0)
		{
		case 0: 
			printf("Object is Kernel Event\n");
			if (TRUE == gloCanResetTimer)
			{
				printf("Resetting the timer\n");
				SetWaitableTimer(glohWaitableTimer, /* Handle to the timer object. */
					&liDueTime, /* Time after which the state of the timer is to be set to signaled */
					0, /*Period of the timer, 0 : Timer will be signaled ONCE. */
					NULL, /* completion routine */
					NULL, /* Arguments for the completion routine. */
					0 /* Do not restore system in suspended power conservation mode after time elapsed. */
				);


				printf("Timer is reset\n");
				ResetEvent(glohKernelEvent);
			}
			break;
		case 1: 
			printf("Object is WaitableObject\n");

			Timer_HandleTimerElapse();
			break;

		}
	}
}

DWORD Timer_HandleTimerElapse()
{
	DWORD RetChk = 0;

	printf("Timer Elapsed\n");

	EnterCriticalSection(&gloCriticalSection);
	gloIsTransTimeElapsed = TRUE;
	gloCanResetTimer = FALSE;
	LeaveCriticalSection(&gloCriticalSection);

	
	Timer_TerminateAndClearSession();

	if (NULL != glohKernelEvent)
		ResetEvent(glohKernelEvent);

	if (NULL != glohWaitableTimer)
	{
		
		CancelWaitableTimer(glohWaitableTimer);
	}

	return 0;
}

DWORD Timer_TerminateAndClearSession()
{
	DWORD RetChk = 0;

	printf("EveryThing is cleared\n");

	return 0;
}

void Initialization()
{
	Timer_InitSyncObjsAndTimerThread();
	CancelWaitableTimer(glohWaitableTimer);
	ResetEvent(glohKernelEvent); 
	ResumeThread(glohTimerThread);
}

void ChipConnect()
{
	EnterCriticalSection(&gloCriticalSection);
	gloCanResetTimer = TRUE;
	SetEvent(glohKernelEvent);
	LeaveCriticalSection(&gloCriticalSection);
}

void DoStuff()
{
	int t = 1;
	bool firstloop = false;
	while (true)
	{
		
		printf("Doing Stuff - %d\n",t);
		Sleep(10000);
		
		if (t % 4==0)
		{
			printf("API exection\n");
			gloCanResetTimer = TRUE;
			SetEvent(glohKernelEvent);
		}
		else
		{
			gloCanResetTimer = TRUE;
			SetEvent(glohKernelEvent);
		}
		t++;
		/*if (t % 15 == 0)
		{
			if (firstloop) 
			{
				printf("API exection\n");
				gloCanResetTimer = TRUE;
				SetEvent(glohKernelEvent);
			}
			else
			{
				gloCanResetTimer = FALSE;
				ResetEvent(glohKernelEvent);
			}
			firstloop = false;
		}*/
	}
}



void main()
{
	Initialization();
	Sleep(5000);
	ChipConnect();
	DoStuff();
}