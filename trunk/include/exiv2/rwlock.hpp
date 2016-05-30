#ifndef RW_LOCK_HPP
#define RW_LOCK_HPP

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace Exiv2 {
    namespace Internal {
#ifdef _WIN32
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
                return TryAcquireSRWLockExclusive(&rwlock_);
            }

            void rdlock()
            {
                AcquireSRWLockShared(&rwlock_);
            }

            bool tryrdlock()
            {
                return TryAcquireSRWLockShared(&rwlock_);
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

            int wrlock()
            {
                return pthread_rwlock_wrlock(&rwlock_);
            }

            int trywrlock()
            {
                return pthread_rwlock_trywrlock(&rwlock_);
            }

            int rdlock()
            {
                return pthread_rwlock_rdlock(&rwlock_);
            }

            int tryrdlock()
            {
                return pthread_rwlock_tryrdlock(&rwlock_);
            }

            int unlock()
            {
                return pthread_rwlock_unlock(&rwlock_);
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
}

#endif // RW_LOCK_HPP
