// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2017 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */

#ifndef RW_LOCK_HPP
#define RW_LOCK_HPP

#ifdef _MSC_VER
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace Exiv2 {
#ifdef _MSC_VER
/*
Visual Studio 2013 and later use SRWLOCK, however don't use Vista/7+ features
when targeting XP.

_USING_V110_SDK71_ is defined when Platform Toolset is set to target XP (and
thus uses Windows 7.1 SDK).

_ATL_XP_TARGETING can be used if you want to target XP but also want to use
a newer SDK, such as 8.
*/
#if _MSC_VER >= 1800 \
    && !(defined(_USING_V110_SDK71_) || defined(_ATL_XP_TARGETING))
        /*!
         @brief Class to provide a Read-Write Lock
        */
        class RWLock
        {
        public:
            RWLock()
            {
                InitializeSRWLock(&rwlock_);
            }

            ~RWLock()
            {
                // do not explicitly destroy
            }

            void wrlock()
            {
                AcquireSRWLockExclusive(&rwlock_);
            }

            bool trywrlock()
            {
                return 0 != TryAcquireSRWLockExclusive(&rwlock_);
            }

            void rdlock()
            {
                AcquireSRWLockShared(&rwlock_);
            }

            bool tryrdlock()
            {
                return 0 != TryAcquireSRWLockShared(&rwlock_);
            }

            void rdunlock()
            {
                ReleaseSRWLockShared(&rwlock_);
            }

            void wrunlock()
            {
                ReleaseSRWLockExclusive(&rwlock_);
            }

        private:
            SRWLOCK rwlock_;
        };
#else
        /*!
         @brief Class to provide a Read-Write Lock
        */
        // Visual Studio 2005,8,10,12 and XP targets use CRITICAL_SECTION
        class RWLock
        {
        public:
            RWLock()
            {
                InitializeCriticalSection(&lock_);
            }

            ~RWLock()
            {
                DeleteCriticalSection(&lock_);
            }

            void wrlock() { enter(); }

            bool trywrlock() { return tryenter(); }

            void rdlock() { enter(); }

            bool tryrdlock() { return tryenter(); }

            void rdunlock() { leave(); }

            void wrunlock() { leave(); }

        private:
            void enter()
            {
                EnterCriticalSection(&lock_);
            }

            void leave()
            {
                LeaveCriticalSection(&lock_);
            }

            bool tryenter()
            {
#if defined(MSDEV_2003) || defined(MSDEV_2005)
                EnterCriticalSection(&lock_);
                return true;
#else
                return 0 != TryEnterCriticalSection(&lock_);
#endif
            }

        private:
            CRITICAL_SECTION lock_;
        };
#endif

#else
        /*!
         @brief Class to provide a Read-Write Lock
        */
        // UNIX systems (including MinGW and Cygwin)
        class RWLock
        {
        public:
        	//! constructor (acquires the lock)
            RWLock(const pthread_rwlockattr_t *attr = 0)
            {
                pthread_rwlock_init(&rwlock_, attr);
            }

        	//! constructor (releases lock)
            ~RWLock()
            {
                pthread_rwlock_destroy(&rwlock_);
            }

        	//! acquire rw lock
            void wrlock()
            {
                pthread_rwlock_wrlock(&rwlock_);
            }

            //! test to see if the rw lock can be acquired
            bool trywrlock()
            {
                return 0 == pthread_rwlock_trywrlock(&rwlock_);
            }

            //! acquire rd lock
            void rdlock()
            {
                pthread_rwlock_rdlock(&rwlock_);
            }

            //! test to see if the rd lock can be acquired
            bool tryrdlock()
            {
                return 0 == pthread_rwlock_tryrdlock(&rwlock_);
            }

            //! release rw lock
            void unlock()
            {
                pthread_rwlock_unlock(&rwlock_);
            }

            //! unlock rd lock
            void rdunlock() { unlock(); }

            //! unlock rw lock
            void wrunlock() { unlock(); }

        private:
        	//! the lock itself
            pthread_rwlock_t rwlock_;
        };
#endif

        /*!
         @brief Class to provide a ScopedReadLock.
         The lock is applied by the constructor and released by the destructor.
        */
        class ScopedReadLock
        {
        public:
            //! constructor - locks the object
            ScopedReadLock(RWLock &rwlock):
                rwlock_(rwlock)
            {
                rwlock_.rdlock();
            }

            //! destructor - unlocks the object used in constructor
            ~ScopedReadLock() { rwlock_.rdunlock(); }

        private:
        	//! object locked by the constructor (and released by destructor)
            RWLock &rwlock_;
        };

        /*!
         @brief Class to provide a ScopedWriteLock.
         The lock is applied by the constructor and released by the destructor.
        */
        class ScopedWriteLock
        {
        public:
            //! constructor - locks the object
            ScopedWriteLock(RWLock &rwlock):
                rwlock_(rwlock)
            {
                rwlock_.wrlock();
            }

            //! destructor - unlocks the object used in constructor
            ~ScopedWriteLock() { rwlock_.wrunlock(); }

        private:
        	//! object locked by the constructor (and released by destructor)
            RWLock &rwlock_;
        };
}

#endif // RW_LOCK_HPP
