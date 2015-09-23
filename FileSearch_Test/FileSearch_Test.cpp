// FileSearch_Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <gtest/gtest.h>

#ifdef _DEBUG
#include <vld.h> // Visual Leak Detector, just comment this line if you don't have it installed
#endif

int _tmain(int argc, _TCHAR* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    int res = RUN_ALL_TESTS();

    return res;
}

