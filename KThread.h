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

#ifndef KThread_H

  #define KThread_H

  #if defined(__LINUX__)
    #include <pthread.h>
  #endif
  #include "NTypes.h"

  enum TThreadPriorityEnum
  {
    tpIdle,
    tpLowest,
    tpLower,
    tpNormal,
    tpHigher,
    tpHighest,
    tpTimeCritical
  };

  #if defined(__MAC__) && !defined(__LINUX__)
    typedef _opaque_pthread_t *  pthread_t;
  #endif

  class KThread
  {
    private:

      // Der Thread wird zum Terminieren aufgefordert.
      bool mTerminated;
      // Flag, damit der Thread richtig gestartet werden kann.
      bool mInitialSuspendDone;

      #if !defined(__NO_MULTITHREADS__)
        #if defined(__WINDOWS__)
          void * mHandle;
          unsigned long mThreadID;
        #else
          // ** mThreadID is not THandle in Linux **
          // Handle zum Thread.
          pthread_t mThreadID;
        #endif
        #if defined(__LINUX__)
          // ThreadProc wird nach dem Kreieren eines Threads aufgerufen.
          static void * ThreadProc(void * aThread);
        #endif
      #endif

    protected:
      #if !defined(__NO_MULTITHREADS__)
        #if defined(__LINUX__)
          // Attribute zum Thread.
          pthread_attr_t mThreadAttr;
          // Damit die Threadpriorität erhöht werden kann.
          sched_param mThreadParam;
        #endif
      #endif

      // Wenn ein Fehler vorliegt, wird eine Meldung ausgegeben.
      void CheckThreadError(int aErrCode);

      // Bei einem Fehler wird er ermittelt und angezeigt (nur für Windows).
      void CheckThreadErrorB(bool aSuccess);

    public:

      // Wird nach dem Ende von "Execute" gesetzt.
      bool mFinished;

      #if !defined(__NO_MULTITHREADS__)
        #if defined(__WINDOWS__)
          // Wartet auf den Thread, bis er terminiert.
          unsigned long WaitFor(void);
          // Liest die Priorität.
          TThreadPriorityEnum GetPriority(void);
          unsigned long GetID(void) { return (unsigned long)(mHandle); };
        #endif
        #if defined(__LINUX__)
          // Wartet auf den Thread, bis er terminiert.
          pthread_t WaitFor(void);
          pthread_t GetID(void) { return mThreadID; };
        #endif
      #endif

      KThread();

      virtual ~KThread();

      // Execute enthält die Aufgabe, die der Thread zu erfüllen hat. Der Thread
      // ist solange am Leben, wie Execute "läuft".
      virtual void Execute()=0;

      bool Finished();

      // Starten von Execute.
      void Resume();

      // Unter Linux muss neben der Priorität auch die Policy gesetzt werden.
      // Setzt eine neue Priorität.
      void SetPriority(TThreadPriorityEnum aValue);

      void Terminate();

      bool Terminated();

  };


  extern TNativeInt gMainThreadId;
  extern TNativeInt gProcessId;

#endif

