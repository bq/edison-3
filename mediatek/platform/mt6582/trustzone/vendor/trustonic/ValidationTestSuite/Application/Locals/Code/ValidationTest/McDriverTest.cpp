/**
 * @addtogroup TEST_MCDRVAPI
 *
 * <h2>Introduction</h2>
 * Tests checking the basic functionality of the MobiCore driver interface.
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */
#include "CppUTest/CommandLineTestRunner.h"
#include "../buildTag.h"
#include <stdio.h>

int main(int ac, char** av)
{
    printf("%s\n",MOBICORE_COMPONENT_BUILD_TAG);
    return RUN_ALL_TESTS(ac, av);
}

/** @} */
