﻿#include "pch.h"
#include "Thunks/api-ms-win-core-synch.hpp"


namespace api_ms_win_core_synch
{
    TEST_CLASS(WaitOnAddress)
    {
        AwaysNullGuard Guard;

    public:
        WaitOnAddress()
        {
            Guard |= YY::Thunks::aways_null_try_get_WaitOnAddress;
            Guard |= YY::Thunks::aways_null_try_get_WakeByAddressSingle;
        }

        TEST_METHOD(结果本身不同)
        {
            //本身不同时，数据应该立即返回

            ULONG TargetValue = 0x2;
            ULONG UndesiredValue = 0;

            auto bRet = ::WaitOnAddress(&TargetValue, &UndesiredValue, sizeof(UndesiredValue), 500);

            Assert::IsTrue(bRet);
        }

        TEST_METHOD(结果本身相同)
        {
            ULONG TargetValue = 0x2;
            ULONG UndesiredValue = 0x2;

            auto bRet = ::WaitOnAddress(&TargetValue, &UndesiredValue, sizeof(UndesiredValue), 500);

            Assert::IsFalse(bRet);
        }
    };


    TEST_CLASS(WakeByAddressSingle)
    {
        AwaysNullGuard Guard;

    public:
        WakeByAddressSingle()
        {
            Guard |= YY::Thunks::aways_null_try_get_WaitOnAddress;
            Guard |= YY::Thunks::aways_null_try_get_WakeByAddressSingle;
        }

        TEST_METHOD(只唤醒了一个线程)
        {
            
            struct MyData
            {
                ULONG TargetValue;
                volatile long RunCount;
            };

            MyData Data = { 0x2,0 };


            HANDLE hThreadHandles[100];

            for (auto& hThreadHandle : hThreadHandles)
            {
                hThreadHandle = (HANDLE)_beginthreadex(nullptr, 0, [](void* pMyData) -> unsigned
                    {
                        auto& Data = *(MyData*)pMyData;

                        ULONG UndesiredValue = 0x2;
                        auto bRet = ::WaitOnAddress(&Data.TargetValue, &UndesiredValue, sizeof(UndesiredValue), INFINITE);

                        Assert::IsTrue(bRet);

                        InterlockedIncrement(&Data.RunCount);

                        return 0;
                    }, & Data, 0, nullptr);

                Assert::IsNotNull(hThreadHandle);
            }

            Sleep(200);

            Assert::AreEqual(0l, (long)Data.RunCount);

            Data.TargetValue = 0;
            ::WakeByAddressSingle(&Data.TargetValue);

            Sleep(200);

            Assert::AreEqual(1l, (long)Data.RunCount);


            for (int i = 0; i != 99; ++i)
            {
                ::WakeByAddressSingle(&Data.TargetValue);
            }

            for (auto hThreadHandle : hThreadHandles)
            {
                Assert::AreEqual(WaitForSingleObject(hThreadHandle, 500), (DWORD)WAIT_OBJECT_0);
            }

            Assert::AreEqual(100l, (long)Data.RunCount);

        }
    };


    TEST_CLASS(WakeByAddressAll)
    {
        AwaysNullGuard Guard;

    public:
        WakeByAddressAll()
        {
            Guard |= YY::Thunks::aways_null_try_get_WaitOnAddress;
            Guard |= YY::Thunks::aways_null_try_get_WakeByAddressAll;
        }

        TEST_METHOD(唤醒所有线程)
        {
            struct MyData
            {
                ULONG TargetValue;
                volatile long RunCount;
            };

            MyData Data = { 0x2,0 };


            HANDLE hThreadHandles[100];

            for (auto& hThreadHandle : hThreadHandles)
            {
                hThreadHandle = (HANDLE)_beginthreadex(nullptr, 0, [](void* pMyData) -> unsigned
                    {
                        auto& Data = *(MyData*)pMyData;

                        ULONG UndesiredValue = 0x2;
                        auto bRet = ::WaitOnAddress(&Data.TargetValue, &UndesiredValue, sizeof(UndesiredValue), INFINITE);

                        Assert::IsTrue(bRet);

                        InterlockedIncrement(&Data.RunCount);

                        return 0;
                    }, &Data, 0, nullptr);

                Assert::IsNotNull(hThreadHandle);
            }

            Sleep(200);

            Assert::AreEqual(0l, (long)Data.RunCount);

            Data.TargetValue = 0;
            ::WakeByAddressAll(&Data.TargetValue);

            for (auto hThreadHandle : hThreadHandles)
            {
                Assert::AreEqual(WaitForSingleObject(hThreadHandle, 500), (DWORD)WAIT_OBJECT_0);
            }

            Assert::AreEqual(100l, (long)Data.RunCount);


        }
    };

    TEST_CLASS(TryAcquireSRWLockExclusive)
    {
        AwaysNullGuard Guard;

    public:
        TryAcquireSRWLockExclusive()
        {
            Guard |= YY::Thunks::aways_null_try_get_TryAcquireSRWLockExclusive;
            Guard |= YY::Thunks::aways_null_try_get_AcquireSRWLockExclusive;
            Guard |= YY::Thunks::aways_null_try_get_ReleaseSRWLockExclusive;
        }

        TEST_METHOD(首次肯定成功)
        {
            SRWLOCK _SRWLock = {};
            auto _bRet = ::TryAcquireSRWLockExclusive(&_SRWLock);

            Assert::AreEqual(BOOLEAN(1), _bRet);
        }

        TEST_METHOD(如果其他线程占用，那么应该失败)
        {
            SRWLOCK _SRWLock = {};
            auto _bRet = ::TryAcquireSRWLockExclusive(&_SRWLock);

            Assert::AreEqual(BOOLEAN(1), _bRet);

            auto _hThreadHandle = (HANDLE)_beginthreadex(nullptr, 0,
                [](void* pMyData) -> unsigned
                {
                    return ::TryAcquireSRWLockExclusive((SRWLOCK*)pMyData);
                },
                & _SRWLock,
                0,
                nullptr);

            Assert::IsNotNull(_hThreadHandle);

            auto _nRet = WaitForSingleObject(_hThreadHandle, 5 * 1000);
            ::ReleaseSRWLockExclusive(&_SRWLock);

            Assert::AreEqual((DWORD)WAIT_OBJECT_0, _nRet);

            DWORD _uCode = -1;
            GetExitCodeThread(_hThreadHandle, &_uCode);
            CloseHandle(_hThreadHandle);

            Assert::AreEqual(_uCode, (DWORD)0u);
        }

        TEST_METHOD(锁定后其他线程会等待)
        {
            SRWLOCK _SRWLock = {};
            auto _bRet = ::TryAcquireSRWLockExclusive(&_SRWLock);

            auto _hThreadHandle = (HANDLE)_beginthreadex(nullptr, 0,
                [](void* pMyData) -> unsigned
                {
                    const auto _uStart = GetTickCount64();
                    ::AcquireSRWLockExclusive((SRWLOCK*)pMyData);

                    return static_cast<DWORD>(GetTickCount64() - _uStart);
                },
                &_SRWLock,
                0,
                nullptr);
            Assert::IsNotNull(_hThreadHandle);

            Sleep(500);
            ::ReleaseSRWLockExclusive(&_SRWLock);

            auto _nRet = WaitForSingleObject(_hThreadHandle, 5 * 1000);

            Assert::AreEqual((DWORD)WAIT_OBJECT_0, _nRet);

            DWORD _uCode = -1;
            GetExitCodeThread(_hThreadHandle, &_uCode);
            CloseHandle(_hThreadHandle);

            Assert::IsTrue(_uCode >= 400 && _uCode <= 800);
        }
    };

    TEST_CLASS(SynchronizationBarrier)
    {
        AwaysNullGuard Guard;

    public:
        SynchronizationBarrier()
        {
            // 海好蓝发现 EnterSynchronizationBarrier的返回值存在一些问题。
            Guard |= YY::Thunks::aways_null_try_get_InitializeSynchronizationBarrier;
            Guard |= YY::Thunks::aways_null_try_get_EnterSynchronizationBarrier;
            Guard |= YY::Thunks::aways_null_try_get_DeleteSynchronizationBarrier;
        }

        TEST_METHOD(创建销毁Barrier)
        {
            SYNCHRONIZATION_BARRIER _Barrier;
            Assert::IsTrue(::InitializeSynchronizationBarrier(&_Barrier, 2, 0));
            Assert::IsTrue(::DeleteSynchronizationBarrier(&_Barrier));
        }

        TEST_METHOD(EnterSynchronizationBarrier阻塞验证)
        {
            {
                SYNCHRONIZATION_BARRIER _Barrier;
                Assert::IsTrue(::InitializeSynchronizationBarrier(&_Barrier, 3, 0));

                struct ThreadInfo
                {
                    HANDLE hThread = nullptr;
                    SYNCHRONIZATION_BARRIER* pBarrier = nullptr;
                    BOOL bRet = -1;
                };

                ThreadInfo _Threads[2];

                for (auto& _Thread : _Threads)
                {
                    _Thread.pBarrier = &_Barrier;
                    _Thread.hThread = CreateThread(nullptr, 0, [](void* _pUserData) ->DWORD
                        {
                            ThreadInfo& _Thread = *(ThreadInfo*)_pUserData;

                            _Thread.bRet = ::EnterSynchronizationBarrier(_Thread.pBarrier, 0);

                            return 0;
                        }, & _Thread, 0, nullptr);
                }

                // 3次进入前，一定会被阻塞
                Assert::AreEqual(WaitForSingleObject(_Threads[0].hThread, 500), (DWORD)WAIT_TIMEOUT);

                Assert::IsTrue(::EnterSynchronizationBarrier(&_Barrier, 0));

                for (auto& _Thread : _Threads)
                {
                    Assert::AreEqual(WaitForSingleObject(_Threads[0].hThread, 100), (DWORD)WAIT_OBJECT_0);
                    Assert::AreEqual(_Thread.bRet, FALSE);
                }

                Assert::IsTrue(::DeleteSynchronizationBarrier(&_Barrier));
            }
        }
    };
}
