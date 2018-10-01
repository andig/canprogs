/*
 *
 *  Copyright (C) 2007-2014 Jürg Müller, CH-5524
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see http://www.gnu.org/licenses/ .
 */

#if defined(__PYTHON__)
  #include <Python.h>
#endif

#include <stdio.h>

#if defined(__WINDOWS__)
  #include <windows.h>
#endif

#if !defined(__NO_MULTITHREADS__)
  #if defined(__LINUX__)
    #include <signal.h>
    #include <unistd.h>
  #endif
#endif

#include "NTypes.h"

#include "NUtils.h"
#include "KThread.h"

using namespace NUtils;

#if defined(__WINDOWS__)
  TNativeInt gMainThreadId = GetCurrentThreadId();
  TNativeInt gProcessId = GetCurrentProcessId();
#endif
#if defined(__LINUX__)
  TNativeInt gMainThreadId = (TNativeInt) pthread_self();
  TNativeInt gProcessId = getpid();  // ist auch für Windows definiert (process.h)
#endif

#if !defined(__NO_MULTITHREADS__)

  #if defined(__WINDOWS__)
    const int Priorities[] = {
      THREAD_PRIORITY_IDLE, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL,
      THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL,
      THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_TIME_CRITICAL};

    TThreadPriorityEnum KThread::GetPriority(void)
    {
      int P;
      int i;
      TThreadPriorityEnum Result;

      P = GetThreadPriority(mHandle);
      CheckThreadErrorB(P != THREAD_PRIORITY_ERROR_RETURN);
      Result = tpNormal;
      for (i = 0; i <= (int) High(Priorities); i++)
        if (Priorities[i] == P) Result = TThreadPriorityEnum(i);
      return Result;
    }

    DWORD WINAPI ThreadProc(LPVOID aThread)
    {
      int Result = 0;
      KThread * This = (KThread *) aThread;
      try
      {
        if (!This->Terminated())
         This->Execute();
      }
      catch(...)
      {
      #if defined(__CONSOLE__)
        printf("exception caught in \"ThreadProc\"\n");
      #endif
      }
      This->mFinished = true;
      #if defined(__DEBUG__) && defined(__CONSOLE__)
        printf("stop thread 0x%x\n", (int)((KThread *)(aThread))->GetID());
      #endif

      ExitThread(Result);
      return Result;
    }

    void ShowMessage(const char * aMsgBuf)
    {
    #if defined(__CONSOLE__)
      printf("ART Error %s\n", aMsgBuf);
    #else
      #if defined(__WINDOWS__)
      // Wenn das Resultat 0 ist, dann steht nicht genügend Memory vom System zur
      // Verfügung. Dies ist beim Programmende der Fall, wenn das Softwarepaket als DLL
      // implementiert ist.
      MessageBoxA(NULL, aMsgBuf, "ART Error", MB_OK);
      #endif
    #endif
    }

  #endif

  #if defined(__LINUX__)
    void * KThread::ThreadProc(void * aThread)
    {
      void * Result = NULL;
      KThread * This = (KThread *) aThread;
      try
      {
        // Die virtuelle Methode "Execute" wird zu früh aufgerufen. Der Thread wird
        // ausgeführt, der Pointer zur Methode der übergeordneten Klasse wird jedoch
        // erst später gesetzt.
        // "mInitialSuspendDone" wird mit der Methode "Resume" auf true gesetzt.
        while (!This->mInitialSuspendDone && !This->Terminated())
          NUtils::SleepMs(1);
        if (!This->Terminated())
          This->Execute();
      }
      catch(...)
      {
      #if defined(__CONSOLE__)
        printf("exception caught in \"ThreadProc\"\n");
      #endif
      }
      This->mFinished = true;
      #if defined(__DEBUG__) && defined(__CONSOLE__)
        printf("stop thread 0x%x\n", (unsigned) (long)((KThread *)(aThread))->GetID());
      #endif

      // Directly call pthread_exit since EndThread will detach the thread causing
      // the pthread_join in KThread.WaitFor to fail.  Also, make sure the EndThreadProc
      // is called just like EndThread would do. EndThreadProc should ! return
      // && call pthread_exit itself.
      pthread_exit(&Result);
      return Result;
    }
  #endif

#endif

KThread::KThread()
{
  mInitialSuspendDone = false;
  mTerminated = false;
  mFinished = false;

  #if !defined(__NO_MULTITHREADS__)
    mThreadID = 0;
    #if defined(__WINDOWS__)
      mHandle = CreateThread(NULL, 0, ThreadProc, this, CREATE_SUSPENDED, &mThreadID);
      if (!mHandle)
      {
        char Buffer[64];

        sprintf(Buffer, "CreateThread error: %d", (int) GetLastError());
        ::ShowMessage(Buffer);
      }
    #endif

    #if defined(__LINUX__)
      int ErrCode;

      CheckThreadError(pthread_attr_init(&mThreadAttr));
      ErrCode = pthread_create(&mThreadID, NULL, ThreadProc, this);
      CheckThreadError(ErrCode);
    #endif

    #if defined(__DEBUG__) && defined(__CONSOLE__)
      printf("create thread 0x%x\n", (unsigned)(long) mThreadID);
    #endif
  #endif
}

KThread::~KThread()
{
  #if !defined(__NO_MULTITHREADS__)
    if (mThreadID && !mFinished)
    {
      Terminate();
      WaitFor();
    }

    #if defined(__WINDOWS__)
      if (mHandle)
        CloseHandle(mHandle);
    #endif

    #if defined(__LINUX__)
      // This final check is to ensure that even if the thread was never waited on
      // its resources will be freed.
      if (mThreadID)
        pthread_detach(mThreadID);
      pthread_attr_destroy(&mThreadAttr);
    #endif
  #endif
}

void KThread::CheckThreadError(int aErrCode)
{
  #if !defined(__NO_MULTITHREADS__)
    if (aErrCode)
    {
    #if defined(__CONSOLE__)
      printf("Thread error %d\n", aErrCode);
    #endif
    }
  #endif
}

void KThread::CheckThreadErrorB(bool aSuccess)
{
  #if !defined(__NO_MULTITHREADS__)
    #if defined(__WINDOWS__)
    if (!aSuccess)
      CheckThreadError(GetLastError());
    #endif
  #endif
}

bool KThread::Finished()
{
  if (!this)
    return true;

  return mFinished;
}

void KThread::Resume()
{
  if (mTerminated)
    return;

  #if !defined(__NO_MULTITHREADS__)
    #if defined(__WINDOWS__)
      int SuspendCount;
      SuspendCount = ResumeThread(mHandle);
      CheckThreadErrorB(SuspendCount >= 0);
      mInitialSuspendDone = true;
    #endif
    #if defined(__LINUX__)
      /* About Suspend and Resume. POSIX does not support suspending/resuming a thread.
      Suspending a thread is considerd dangerous since it is not guaranteed where the
      thread would be suspend. It might be holding a lock, mutex or it might be inside
      a critical section.  In order to simulate it in Linux we've used signals. To
      suspend, a thread SIGSTOP is sent and to resume, SIGCONT is sent. Note that this
      is Linux only i.e. according to POSIX if a thread receives SIGSTOP then the
      entire process is stopped. However Linux doesn't entirely exhibit the POSIX-mandated
      behaviour. If and when it fully complies with the POSIX standard then suspend
      and resume won't work. */
      if (!mInitialSuspendDone)
      {
        mInitialSuspendDone = true;
      } else
        CheckThreadError(pthread_kill(mThreadID, SIGCONT));
    #endif
  #endif
}

void KThread::SetPriority(TThreadPriorityEnum aValue)
{
  #if !defined(__NO_MULTITHREADS__)
    #if defined(__WINDOWS__)
      CheckThreadErrorB(SetThreadPriority(mHandle, Priorities[aValue]) != 0);
    #endif
    #if defined(__LINUX__)
      switch (aValue)
      {
      case tpHigher:
        mThreadParam.sched_priority = 50;
        break;

      case tpHighest:
        mThreadParam.sched_priority = 50;
        break;

      case tpTimeCritical:
        mThreadParam.sched_priority = 50;
        break;

      default:
        mThreadParam.sched_priority = 0;
        break;
      }
      CheckThreadError(pthread_attr_setschedpolicy(&mThreadAttr, SCHED_RR));
      CheckThreadError(pthread_attr_setschedparam(&mThreadAttr, &mThreadParam));
    #endif
  #endif
}

void KThread::Terminate()
{
  #if !defined(__NO_MULTITHREADS__)
    if (this) mTerminated = true;
  #endif
}

bool KThread::Terminated()
{
  #if !defined(__NO_MULTITHREADS__)
    return this ? mTerminated : true;
  #else
    return false;
  #endif
}

#if !defined(__NO_MULTITHREADS__)
  #if defined(__WINDOWS__)
  unsigned long KThread::WaitFor(void)
  {
    HANDLE H;
//    unsigned long WaitResult;
//    MSG Msg;
    unsigned long Result;

    if (!mInitialSuspendDone)
      return false;

    H = mHandle;
 /*   if (false && (TNativeInt) GetCurrentThreadId() == gMainThreadId)
    {
      WaitResult = 0;
      do
      {
        // This prevents a potential deadlock if the background thread
        //  does a SendMessage to the foreground thread
        if (WaitResult == WAIT_OBJECT_0 + 1)
          PeekMessage(&Msg, 0, 0, 0, PM_NOREMOVE);
        SleepMs(1);
        WaitResult = MsgWaitForMultipleObjects(1, &H, false, 0, QS_SENDMESSAGE);
        if (WaitResult == WAIT_FAILED)
        {
          char Buffer[64];

          sprintf(Buffer, "Thread wait error: %d", GetLastError());
          ShowMessage(Buffer);
        }
      } while (WaitResult == WAIT_OBJECT_0 && !mFinished);
    } else    */
      WaitForSingleObject(H, INFINITE);
    CheckThreadError(GetExitCodeThread(H, &Result));

    return Result;
  }
  #endif
  #if defined(__LINUX__)
  pthread_t KThread::WaitFor(void)
  {
    void * X;
    pthread_t ID;
    pthread_t Result = 0;

    ID = mThreadID;
    if ((void *)pthread_self() == (void *)gMainThreadId)
      while (!mFinished)
      {
        NUtils::SleepMs(1);
      }
    mThreadID = 0;
    X = &Result;
    CheckThreadError(pthread_join(ID, &X));
    return Result;
  }
  #endif
#endif

