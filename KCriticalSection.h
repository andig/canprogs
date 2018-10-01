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

#ifndef KCriticalSection_H

  #define KCriticalSection_H

  class KCriticalSection
  {
    private:
      void * mSection;

    public:
      KCriticalSection();
      ~KCriticalSection();

      bool Acquire();
      bool TryEnter();
      void Release();
  };

#endif

