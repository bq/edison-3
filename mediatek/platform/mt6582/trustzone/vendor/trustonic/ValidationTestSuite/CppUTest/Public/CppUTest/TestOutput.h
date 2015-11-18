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

#ifndef D_TestOutput_h
#define D_TestOutput_h
#include "CppUTest/PlatformSpecificFunctions.h"

#define CONSOLE_OUTPUT_TYPE_ECLIPSE // Also normal
#define CONSOLE_OUTPUT_TYPE_CSV 	// Comma separated value
///////////////////////////////////////////////////////////////////////////////
//
//  TestOutput.h
//
//  This is a minimal printer inteface.
//  We kept streams out too keep footprint small, and so the test
//  harness could be used with less capable compilers so more
//  platforms could use this test harness
//
///////////////////////////////////////////////////////////////////////////////

class Utest;
class TestFailure;
class TestResult;

class TestOutput
{
public:
	explicit TestOutput();
	explicit TestOutput(int consoleOutputType);
	virtual ~TestOutput();

	virtual void printTestsStarted(const char* buildTag,const char* test_suite_name);
	virtual void printTestsEnded(const TestResult& result);
	virtual void printCurrentTestStarted(const Utest& test);
	virtual void printCurrentTestEnded(const TestResult& res);
	virtual void printCurrentGroupStarted(const Utest& test);
	virtual void printCurrentGroupEnded(const TestResult& res);
	virtual void printResultInCsvFile(const TestResult& res, Utest* test, bool isPassed);
	virtual void printIgnoredInCsvFile(const Utest* ignoredTest);

	virtual void verbose();
	virtual void print(const char*)=0;
	virtual void print(long);
	virtual void printDouble(double);
	virtual void printHex(long);
	virtual void print(const TestFailure& failure);
	virtual void printTestRun(int number, int total);
	virtual void setProgressIndicator(const char*);
	virtual void setCsvReportingActive(bool);
	virtual bool isCsvReportingActive();
	virtual bool isXmlReportingActive();
	void setXmlReportingActive(bool value);

	virtual void flush();
	SimpleString getResultsPath() const;
    void setResultsPath(SimpleString path);
    void setPlatformName(SimpleString name);
    SimpleString getPlatformName() const;
    void setProductID();
    SimpleString getProductID() const;
    void GetTbaseVersionFromtlcInfo();
    SimpleString productID_;
	SimpleString tbaseInfos_;



private:

    void printTestResultInXmlFile(const TestResult& res, Utest* test, bool isPassed);
    void openResultsXmlFile(const char* buildTag, const char* test_suite_name);
    void openGroupTagInXmlFile(const Utest& test);
    void closeGroupTagInXmlFile();
    void closeResultsXmlFile();
    void openTestTagInXmlFile(const Utest& test);
    void closeTestTagInXmlFile();

	virtual void printProgressIndicator();
	void printFileAndLineForTestAndFailure(const TestFailure& failure);
	void printFileAndLineForFailure(const TestFailure& failure);
	void printFailureInTest(SimpleString testName);
	void printFailureMessage(SimpleString reason);
	void printEclipseErrorInFileOnLine(SimpleString testFile, int lineNumber);

	void savePreviousOutputFile();
	int returnLastFileSaveNumber();

	TestOutput(const TestOutput&);
	TestOutput& operator=(const TestOutput&);

	int dotCount_;
	bool csvReportingActive;
	bool xmlReportingActive;
	bool verbose_;
	bool resultsPathFlag;
	const char* progressIndication_;

	SimpleString previousSubject;
	TestFailure *currentTestFailed;
	PlatformSpecificFile xml_file;
	SimpleString resultsPath_;
	SimpleString platformName_;
	Utest *last_test_written;

};

TestOutput& operator<<(TestOutput&, const char*);
TestOutput& operator<<(TestOutput&, long);

///////////////////////////////////////////////////////////////////////////////
//
//  ConsoleTestOutput.h
//
//  Printf Based Solution
//
///////////////////////////////////////////////////////////////////////////////

class ConsoleTestOutput: public TestOutput
{
public:
	explicit ConsoleTestOutput()
	{
	}
	;
	virtual ~ConsoleTestOutput()
	{
	}
	;

	virtual void print(const char* s);
	virtual void flush();

private:
	ConsoleTestOutput(const ConsoleTestOutput&);
	ConsoleTestOutput& operator=(const ConsoleTestOutput&);
};

///////////////////////////////////////////////////////////////////////////////
//
//  StringBufferTestOutput.h
//
//  TestOutput for test purposes
//
///////////////////////////////////////////////////////////////////////////////


class StringBufferTestOutput: public TestOutput
{
public:
	explicit StringBufferTestOutput()
	{
	}
	;
	virtual ~StringBufferTestOutput()
	{
	}
	;

	void print(const char* s)
	{
		output += s;
	}

	void flush()
	{
		output = "";
	}

	const SimpleString& getOutput()
	{
		return output;
	}

private:
	SimpleString output;

	StringBufferTestOutput(const StringBufferTestOutput&);
	StringBufferTestOutput& operator=(const StringBufferTestOutput&);

};

#endif  // D_TestOutput_h
