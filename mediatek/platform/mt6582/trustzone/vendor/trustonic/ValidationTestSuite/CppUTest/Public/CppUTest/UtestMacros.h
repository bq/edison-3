/*
 * Copyright (c) 2007, Michael Feathers, James Grenning and Bas Vodde
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE EARLIER MENTIONED AUTHORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef D_UTestMacros_h
#define D_UTestMacros_h

/*! \brief Define a group of tests
 *
 * All tests in a TEST_GROUP share the same setup
 * and teardown.  setup is run before the opening
 * curly brace of the test group and teardown is
 * called after the closing curly brace of the test group.
 *
 */

#ifdef WIN32
#define OS_DEPENDENT_TYPEOF(a) decltype(a)
#else
#define OS_DEPENDENT_TYPEOF(a) typeof(a)
#endif


#define TEST_GROUP_BASE(testGroup, baseclass) \
  int externTestGroup##testGroup = 0; \
  struct TEST_GROUP_##CppUTestGroup##testGroup : public baseclass \

#define  TIMEOUT(value) \
   void init_timeout(){timeout_=value;}

#define TEST_BASE(testBaseClass) \
  struct testBaseClass : public Utest

#define TEST_GROUP(testGroup, ...) \
  TEST_GROUP_BASE(testGroup, Utest)

#define TEST_SCENARIO   TEST_GROUP

#define TEST_GROUP_INT(testGroup) \
  int externTestGroup##testGroup = 0;

#define TEST_GROUP_DECLARE(testGroup) \
	struct TEST_GROUP_##CppUTestGroup##testGroup : public Utest

#define TEST_GROUP_BACKWARD(testGroup) \
	TEST_GROUP_##CppUTestGroup##testGroup

#define TEST_SETUP() \
  virtual void setup()

#define TEST_TEARDOWN() \
  virtual void teardown()

/************* following macro must be adapted for windows ***********/
#ifdef WIN32

#define TEST_intern(subject, testGroup, testName, ...) \
  class TEST_##subject##_##testGroup##_##testName##_Test : public TEST_GROUP_##CppUTestGroup##testGroup \
{ public: TEST_##subject##_##testGroup##_##testName##_Test () : TEST_GROUP_##CppUTestGroup##testGroup () { init_timeout();} \
       void testBody(); } \
    TEST_##subject##_##testGroup##_##testName##_Instance; \
  TestInstaller TEST_##subject##_##testGroup##_##testName##_Installer(&TEST_##subject##_##testGroup##_##testName##_Instance, #subject, #testGroup, #testName, 0, __FILE__,__LINE__); \
	void TEST_##subject##_##testGroup##_##testName##_Test::testBody()

#else

#define TEST_intern(subject, testGroup, testName, ...) \
  class TEST_##subject##_##testGroup##_##testName##_Test : public TEST_GROUP_##CppUTestGroup##testGroup \
{ public: TEST_##subject##_##testGroup##_##testName##_Test () : TEST_GROUP_##CppUTestGroup##testGroup () { init_timeout();} \
       void testBody(); } \
    TEST_##subject##_##testGroup##_##testName##_Instance; \
  TestInstaller TEST_##subject##_##testGroup##_##testName##_Installer(&TEST_##subject##_##testGroup##_##testName##_Instance, #subject, #testGroup, #testName, __VA_ARGS__+0, __FILE__,__LINE__); \
	void TEST_##subject##_##testGroup##_##testName##_Test::testBody()

#endif

#define TESTPARAM getTestParameter()
#define TESTCASE(subject,scenario,name,...) TEST_intern(subject,scenario,name,__VA_ARGS__)
#define TEST(testGroup,testName) TEST_intern(TEST,testGroup,testName,0)

#define IGNORE_TEST(subject, testGroup, testName, ...)\
  class IGNORE##subject##_##testGroup##_##testName##_Test : public TEST_GROUP_##CppUTestGroup##testGroup \
{ public: IGNORE##subject##_##testGroup##_##testName##_Test () : TEST_GROUP_##CppUTestGroup##testGroup () {} \
    virtual void run (TestResult& result_parameter) { \
    	result_parameter.countIgnored(); } \
    virtual const char* getProgressIndicator() const {return "!";} \
  protected:  virtual SimpleString getMacroName() const \
      { return "IGNORING  "; } \
  public: void testBodyThatNeverRuns (); } \
    TEST_##subject##_##testGroup##_##testName##_Instance; \
    void IGNORE##subject##_##testGroup##_##testName##_Test::testBodyThatNeverRuns ()

  /*TestInstaller TEST_##subject##_##testGroup##testName##_Installer(&TEST_##subject##_##testGroup##_##testName##_Instance, #subject, #testGroup, #testName, __VA_ARGS__+0, __FILE__,__LINE__); \*/


#define IGNORE_TESTCASE(subject,scenario,name,...) IGNORE_TEST(subject, scenario,name,__VA_ARGS__)

#define IMPORT_TEST_GROUP(testGroup) \
  extern int externTestGroup##testGroup;\
  int* p##testGroup = &externTestGroup##testGroup

//Check any boolean condition

#define CHECK(condition)\
  CHECK_LOCATION(condition, #condition, __FILE__, __LINE__)

#define CHECK_LOCATION(condition, conditionString, file, line)\
  { Utest::getCurrent()->assertTrue(condition, conditionString, file, line); }

//This check needs the operator!=(), and a StringFrom(YourType) function
#define CHECK_EQUAL(expected,actual)\
    CHECK_EQUAL_CODE(expected,actual,0)

#define CHECK_EQUAL_CODE(expected,actual,actualStringCode)\
  CHECK_EQUAL_LOCATION_STRING(expected, actual, #expected, #actual, actualStringCode, __FILE__, __LINE__)

#define CHECK_EQUAL_LOCATION_STRING(expected,actual, expectedString, actualString, actualStringCode, file, line)\
    do { \
    OS_DEPENDENT_TYPEOF(actual) result = (actual); \
  if ((expected) != result)\
  {\
	 { \
      Utest::getTestResult()->countCheck();\
  	   CheckEqualFailure _f(Utest::getCurrent(), file, line, expectedString, actualString, actualStringCode, StringFrom(expected), StringFrom(result)); \
      Utest::getTestResult()->addFailure(_f);\
    } \
    Utest::getCurrent()->exitCurrentTest(); \
  }\
  else {\
	 Utest::getTestResult()->countCheck(); \
  } \
    } while(0);

#define CHECK_EQUAL_LOCATION(expected,actual, file, line)\
  if ((expected) != (actual))\
  {\
     { \
      Utest::getTestResult()->countCheck();\
       CheckEqualFailure _f(Utest::getCurrent(), file, line, StringFrom(expected), StringFrom(actual)); \
      Utest::getTestResult()->addFailure(_f);\
    } \
    Utest::getCurrent()->exitCurrentTest(); \
  }\
  else\
     Utest::getTestResult()->countCheck();

//This check checks for char* string equality using strcmp.
//This makes up for the fact that CHECK_EQUAL only compares the pointers to char*'s
#define STRCMP_EQUAL(expected,actual)\
  STRCMP_EQUAL_LOCATION(expected, actual, __FILE__, __LINE__)

#define STRCMP_EQUAL_LOCATION(expected,actual, file, line)\
  { Utest::getCurrent()->assertCstrEqual(expected, actual, file, line); }

#define STRCMP_NOCASE_EQUAL(expected,actual)\
  STRCMP_NOCASE_EQUAL_LOCATION(expected, actual, __FILE__, __LINE__)

#define STRCMP_NOCASE_EQUAL_LOCATION(expected,actual, file, line)\
  { Utest::getCurrent()->assertCstrNoCaseEqual(expected, actual, file, line); }

#define STRCMP_CONTAINS(expected,actual)\
  STRCMP_CONTAINS_LOCATION(expected, actual, __FILE__, __LINE__)

#define STRCMP_CONTAINS_LOCATION(expected,actual, file, line)\
  { Utest::getCurrent()->assertCstrContains(expected, actual, file, line); }

#define STRCMP_NOCASE_CONTAINS(expected,actual)\
  STRCMP_NOCASE_CONTAINS_LOCATION(expected, actual, __FILE__, __LINE__)

#define STRCMP_NOCASE_CONTAINS_LOCATION(expected,actual, file, line)\
  { Utest::getCurrent()->assertCstrNoCaseContains(expected, actual, file, line); }

//Check two long integers for equality
#define LONGS_EQUAL(expected,actual)\
        LONGS_EQUAL_CODE(expected,actual,0)

#define LONGS_EQUAL_CODE(expected,actual,actualStringCode)\
  LONGS_EQUAL_LOCATION_STRING(expected,actual, #expected, #actual, actualStringCode, __FILE__, __LINE__)

#define LONGS_EQUAL_LOCATION_STRING(expected,actual,expectedString, actualString, actualStringCode, file,line)\
  { Utest::getCurrent()->assertLongsEqual(expected, actual, expectedString, actualString, actualStringCode, file, line); }

#define LONGS_EQUAL_LOCATION(expected,actual,file,line)\
  { Utest::getCurrent()->assertLongsEqual(expected, actual,  file, line); }

#define BYTES_EQUAL(expected, actual)\
    LONGS_EQUAL((expected) & 0xff,(actual) & 0xff)

#define POINTERS_EQUAL(expected, actual)\
    POINTERS_EQUAL_LOCATION((expected),(actual), __FILE__, __LINE__)

#define POINTERS_EQUAL_LOCATION(expected,actual,file,line)\
  { Utest::getCurrent()->assertPointersEqual(expected, actual,  file, line); }

//Check two doubles for equality within a tolerance threshold
#define DOUBLES_EQUAL(expected,actual,threshold)\
  DOUBLES_EQUAL_LOCATION(expected,actual,threshold,__FILE__,__LINE__)

#define DOUBLES_EQUAL_LOCATION(expected,actual,threshold,file,line)\
  { Utest::getCurrent()->assertDoublesEqual(expected, actual, threshold,  file, line); }

//Fail if you get to this macro
//The macro FAIL may already be taken, so allow FAIL_TEST too
#ifndef FAIL
#define FAIL(text)\
  FAIL_LOCATION(text, __FILE__,__LINE__)

#define FAIL_LOCATION(text, file, line)\
  { Utest::getCurrent()->fail(text,  file, line); Utest::getCurrent()->exitCurrentTest(); }
#endif

#define FAIL_TEST(text)\
  FAIL_TEST_LOCATION(text, __FILE__,__LINE__)

#define FAIL_TEST_LOCATION(text, file,line)\
  { Utest::getCurrent()->fail(text, file, line); Utest::getCurrent()->exitCurrentTest(); }

#define UT_PRINT_LOCATION(text, file, line) \
   { Utest::getCurrent()->print(text, file, line); }

#define UT_PRINT(text) \
   UT_PRINT_LOCATION(text, __FILE__, __LINE__)

#define UT_CRASH() { UT_PRINT("Going to crash here\n"); Utest* ptr = (Utest*) 0x0; ptr->countTests(); }

#ifdef MY_BUILD_NUMBER
/* TRICK: detect if MY_BUILD_NUMBER is defined but empty */
#if 0 == MY_BUILD_NUMBER-0
#undef  MY_BUILD_NUMBER
#define MY_BUILD_NUMBER 00000
#endif
#else
/* MY_BUILD_NUMBER is not defined */
#define MY_BUILD_NUMBER 00000
#endif


#ifdef MY_BUILD_ID
/* TRICK: detect if MY_BUILD_ID is defined but empty */
#if 0 == MY_BUILD_ID-0
#undef  MY_BUILD_ID
#define MY_BUILD_ID 00000
#endif
#else
/* MY_BUILD_ID is not defined */
#define MY_BUILD_ID 00000
#endif


#ifndef TEST_SUITE_NAME
#define TEST_SUITE_NAME UNDEFINED
#endif

#define __STRINGIFY(X) #X
#define __STRINGIFY2(X) __STRINGIFY(X)

#define TEST_SUITE_NAME_STRING __STRINGIFY2(TEST_SUITE_NAME)

#define TEST_SUITE_VERSION_STRING __STRINGIFY2(MY_BUILD_NUMBER) "." __STRINGIFY2(MY_BUILD_ID)

#ifndef TIMEOUT_LONG_VALUE
#define TIMEOUT_LONG_VALUE 600
#endif

#ifndef TIMEOUT_MEDIUM_VALUE
#define TIMEOUT_MEDIUM_VALUE 180
#endif

#ifndef TIMEOUT_SHORT_VALUE
#define TIMEOUT_SHORT_VALUE 30
#endif

#ifndef TIMEOUT_DEFAULT_VALUE
#define TIMEOUT_DEFAULT_VALUE 60
#endif


#define RUN_ALL_TESTS(ac, av) CommandLineTestRunner::RunAllTests(ac, av,(char*)MOBICORE_COMPONENT_BUILD_TAG,(char*)TEST_SUITE_NAME_STRING,TIMEOUT_DEFAULT_VALUE, TIMEOUT_SHORT_VALUE, TIMEOUT_MEDIUM_VALUE,TIMEOUT_LONG_VALUE)

#endif /*D_UTestMacros_h*/
