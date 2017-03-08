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

#ifndef _MSC_VER
#include <pthread.h>
#endif

namespace Exiv2 {
#ifdef _MSC_VER
// Visual Studio 2013 and later use SRWLOCK
#if _MSC_VER >= 1800
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
        // Visual Studio 2005,8,10,12 use CRITICAL_SECTION
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
        // UNIX systems (including MinGW and Cygwin)
        class RWLock
        {
        public:
            RWLock(const pthread_rwlockattr_t *attr = 0)
            {
                pthread_rwlock_init(&rwlock_, attr);
            }

            ~RWLock()
            {
                pthread_rwlock_destroy(&rwlock_);
            }

            void wrlock()
            {
                pthread_rwlock_wrlock(&rwlock_);
            }

            bool trywrlock()
            {
                return 0 == pthread_rwlock_trywrlock(&rwlock_);
            }

            void rdlock()
            {
                pthread_rwlock_rdlock(&rwlock_);
            }

            bool tryrdlock()
            {
                return 0 == pthread_rwlock_tryrdlock(&rwlock_);
            }

            void unlock()
            {
                pthread_rwlock_unlock(&rwlock_);
            }

            void rdunlock() { unlock(); }
            void wrunlock() { unlock(); }

        private:
            pthread_rwlock_t rwlock_;
        };
#endif

        class ScopedReadLock
        {
        public:
            ScopedReadLock(RWLock &rwlock):
                rwlock_(rwlock)
            {
                rwlock_.rdlock();
            }

            ~ScopedReadLock() { rwlock_.rdunlock(); }

        private:
            RWLock &rwlock_;
        };

        class ScopedWriteLock
        {
        public:
            ScopedWriteLock(RWLock &rwlock):
                rwlock_(rwlock)
            {
                rwlock_.wrlock();
            }

            ~ScopedWriteLock() { rwlock_.wrunlock(); }

        private:
            RWLock &rwlock_;
        };
}

#endif // RW_LOCK_HPP
