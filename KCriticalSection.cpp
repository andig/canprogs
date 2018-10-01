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

#if !defined(__NO_MULTITHREADS__)
  #if defined(__WINDOWS__)
    #include <windows.h>
  #else
    #include <pthread.h>
    #include <sys/sem.h>
  #endif
#endif

#include "NTypes.h"

#include "KCriticalSection.h"

KCriticalSection::KCriticalSection()
{
  mSection = 0;
  #if !defined(__NO_MULTITHREADS__)
    #if defined(__WINDOWS__)
      mSection = new CRITICAL_SECTION;
      InitializeCriticalSection((CRITICAL_SECTION *) mSection);
    #else
      bool AttrOk = false;
      pthread_mutexattr_t Attribute;

      mSection = new pthread_mutex_t;
      if (!pthread_mutexattr_init(&Attribute))
      {
        AttrOk = pthread_mutexattr_settype(&Attribute, PTHREAD_MUTEX_RECURSIVE /* 1 */) == 0;
        pthread_mutex_init((pthread_mutex_t *) mSection, AttrOk ? &Attribute : NULL);

        pthread_mutexattr_destroy(&Attribute);
      } else
        pthread_mutex_init((pthread_mutex_t *) mSection, NULL);
    #endif
  #endif
}

KCriticalSection::~KCriticalSection()
{
  #if !defined(__NO_MULTITHREADS__)
    #if defined(__WINDOWS__)
      DeleteCriticalSection((CRITICAL_SECTION *) mSection);
      delete (CRITICAL_SECTION *) mSection;
    #else
      pthread_mutex_destroy((pthread_mutex_t *) mSection);
      delete (pthread_mutex_t *) mSection;
    #endif
  #endif
  mSection = NULL;
}

bool KCriticalSection::Acquire()
{
  if (!mSection)
    return false;

  #if !defined(__NO_MULTITHREADS__)
    #if defined(__WINDOWS__)
      EnterCriticalSection((CRITICAL_SECTION *) mSection);
    #else
      pthread_mutex_lock((pthread_mutex_t *) mSection);
    #endif
  #endif
  return true;
}

bool KCriticalSection::TryEnter()
{
  if (mSection)
  {
  #if !defined(__NO_MULTITHREADS__)
    #if defined(__WINDOWS__)
      #if(_WIN32_WINNT >= 0x0400)
        // TryEnterCriticalSection ist unter Windows 95 nicht verfï¿½gbar
        // Bei einem erfolgreichen TryEnterCriticalSection muss danach die
        // CriticalSection mittels LeaveCriticalSection wieder freigegeben werden.
        return TryEnterCriticalSection((CRITICAL_SECTION *) mSection) != 0;
      #else
        Acquire();
        return true;
      #endif
    #else
      return pthread_mutex_trylock((pthread_mutex_t *) mSection) == 0;
    #endif
  #endif
  }
  return false;
}

void KCriticalSection::Release()
{
  if (!mSection)
    return;

  #if !defined(__NO_MULTITHREADS__)
    #if defined(__WINDOWS__)
      LeaveCriticalSection((CRITICAL_SECTION *) mSection);
    #else
      pthread_mutex_unlock((pthread_mutex_t *) mSection);
    #endif
  #endif
}

