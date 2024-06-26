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
 * THIS SOFTWARE IS PROVIDED BY THE EARLIER MENTIONED AUTHORS ''AS IS'' AND ANY
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

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockFailure.h"
#include "CppUTestExt/MockCheckedExpectedCall.h"
#include "CppUTestExt/MockExpectedCallsList.h"
#include "MockFailureReporterForTest.h"

TEST_GROUP(MockFailureTest)
{
    MockFailureReporter reporter;

    MockExpectedCallsList *list;
    MockCheckedExpectedCall* call1;
    MockCheckedExpectedCall* call2;
    MockCheckedExpectedCall* call3;
    MockCheckedExpectedCall* call4;
    MockCheckedExpectedCall* call5;

    void setup () CPPUTEST_OVERRIDE
    {
        list = new MockExpectedCallsList;
        call1 = new MockCheckedExpectedCall;
        call2 = new MockCheckedExpectedCall;
        call3 = new MockCheckedExpectedCall;
        call4 = new MockCheckedExpectedCall;
        call5 = new MockCheckedExpectedCall;
    }
    void teardown () CPPUTEST_OVERRIDE
    {
        delete list;
        delete call1;
        delete call2;
        delete call3;
        delete call4;
        delete call5;
        CHECK_NO_MOCK_FAILURE();
        MockFailureReporterForTest::clearReporter();
    }

    void addThreeCallsToList()
    {
        list->addExpectedCall(call1);
        list->addExpectedCall(call2);
        list->addExpectedCall(call3);
    }

    void addFiveCallsToList()
    {
        list->addExpectedCall(call1);
        list->addExpectedCall(call2);
        list->addExpectedCall(call3);
        list->addExpectedCall(call4);
        list->addExpectedCall(call5);
    }

    void checkUnexpectedNthCallMessage(unsigned int count, const char* expectedOrdinal)
    {
        MockExpectedCallsList callList;
        MockCheckedExpectedCall expectedCallSingle(1);
        MockCheckedExpectedCall expectedCallMulti(count-1);

        expectedCallSingle.withName("bar");
        expectedCallMulti.withName("bar");

        if (count > 1) {
            callList.addExpectedCall(&expectedCallSingle);
            expectedCallSingle.callWasMade(1);
        }

        if (count > 2) {
            callList.addExpectedCall(&expectedCallMulti);
            for (unsigned int i = 1; i < (count - 1); i++) {
                expectedCallMulti.callWasMade(i+1);
            }
        }

        MockUnexpectedCallHappenedFailure failure(UtestShell::getCurrent(), "bar", callList);

        SimpleString expectedMessage = StringFromFormat("Mock Failure: Unexpected additional (%s) call to function: bar\n\tEXPECTED", expectedOrdinal);
        STRCMP_CONTAINS(expectedMessage.asCharString(), failure.getMessage().asCharString());
    }
};

TEST(MockFailureTest, noErrorFailureSomethingGoneWrong)
{
    MockFailure failure(UtestShell::getCurrent());
    STRCMP_EQUAL("Test failed with MockFailure without an error! Something went seriously wrong.", failure.getMessage().asCharString());
}

TEST(MockFailureTest, unexpectedCallHappened)
{
    MockUnexpectedCallHappenedFailure failure(UtestShell::getCurrent(), "foobar", *list);
    STRCMP_EQUAL("Mock Failure: Unexpected call to function: foobar\n"
                 "\tEXPECTED calls that WERE NOT fulfilled:\n"
                 "\t\t<none>\n"
                 "\tEXPECTED calls that WERE fulfilled:\n"
                 "\t\t<none>", failure.getMessage().asCharString());
}

TEST(MockFailureTest, expectedCallDidNotHappen)
{
    call1->withName("foobar");
    call2->withName("world").withParameter("boo", 2).withParameter("hello", "world");
    call3->withName("haphaphap");
    call3->callWasMade(1);
    addThreeCallsToList();

    MockExpectedCallsDidntHappenFailure failure(UtestShell::getCurrent(), *list);
    STRCMP_EQUAL("Mock Failure: Expected call WAS NOT fulfilled.\n"
                 "\tEXPECTED calls that WERE NOT fulfilled:\n"
                 "\t\tfoobar -> no parameters (expected 1 call, called 0 times)\n"
                 "\t\tworld -> int boo: <2 (0x2)>, const char* hello: <world> (expected 1 call, called 0 times)\n"
                 "\tEXPECTED calls that WERE fulfilled:\n"
                 "\t\thaphaphap -> no parameters (expected 1 call, called 1 time)", failure.getMessage().asCharString());
}

TEST(MockFailureTest, MockUnexpectedNthAdditionalCallFailure)
{
    checkUnexpectedNthCallMessage(2, "2nd");
    checkUnexpectedNthCallMessage(3, "3rd");
    checkUnexpectedNthCallMessage(4, "4th");
    checkUnexpectedNthCallMessage(11, "11th");
    checkUnexpectedNthCallMessage(12, "12th");
    checkUnexpectedNthCallMessage(13, "13th");
    checkUnexpectedNthCallMessage(14, "14th");
    checkUnexpectedNthCallMessage(21, "21st");
    checkUnexpectedNthCallMessage(22, "22nd");
    checkUnexpectedNthCallMessage(23, "23rd");
}

TEST(MockFailureTest, MockUnexpectedInputParameterFailure)
{
    call1->withName("foo").withParameter("boo", 2);
    call2->withName("foo").withParameter("boo", 3.3);
    call3->withName("unrelated");
    addThreeCallsToList();

    MockNamedValue actualParameter("bar");
    actualParameter.setValue(2);

    MockUnexpectedInputParameterFailure failure(UtestShell::getCurrent(), "foo", actualParameter, *list);
    STRCMP_EQUAL("Mock Failure: Unexpected parameter name to function \"foo\": bar\n"
                 "\tEXPECTED calls that WERE NOT fulfilled related to function: foo\n"
                 "\t\tfoo -> int boo: <2 (0x2)> (expected 1 call, called 0 times)\n"
                 "\t\tfoo -> double boo: <3.3> (expected 1 call, called 0 times)\n"
                 "\tEXPECTED calls that WERE fulfilled related to function: foo\n"
                 "\t\t<none>\n"
                 "\tACTUAL unexpected parameter passed to function: foo\n"
                 "\t\tint bar: <2 (0x2)>", failure.getMessage().asCharString());
}

TEST(MockFailureTest, MockUnexpectedOutputParameterFailure)
{
    int out1;
    int out2;
    call1->withName("foo").withOutputParameterReturning("boo", &out1, sizeof(out1));
    call2->withName("foo").withOutputParameterReturning("boo", &out2, sizeof(out2));
    call3->withName("unrelated");
    addThreeCallsToList();

    MockNamedValue actualParameter("bar");
    actualParameter.setValue((void *)0x123);

    MockUnexpectedOutputParameterFailure failure(UtestShell::getCurrent(), "foo", actualParameter, *list);
    STRCMP_EQUAL("Mock Failure: Unexpected output parameter name to function \"foo\": bar\n"
                 "\tEXPECTED calls that WERE NOT fulfilled related to function: foo\n"
                 "\t\tfoo -> const void* boo: <output> (expected 1 call, called 0 times)\n"
                 "\t\tfoo -> const void* boo: <output> (expected 1 call, called 0 times)\n"
                 "\tEXPECTED calls that WERE fulfilled related to function: foo\n"
                 "\t\t<none>\n"
                 "\tACTUAL unexpected output parameter passed to function: foo\n"
                 "\t\tvoid* bar", failure.getMessage().asCharString());
}

TEST(MockFailureTest, MockUnexpectedUnmodifiedOutputParameterFailure)
{
    int out1;
    call1->withName("foo").withOutputParameterReturning("boo", &out1, sizeof(out1));
    call2->withName("foo").withUnmodifiedOutputParameter("boo");
    call3->withName("unrelated");
    addThreeCallsToList();

    MockNamedValue actualParameter("bar");
    actualParameter.setValue((void *)0x123);

    MockUnexpectedOutputParameterFailure failure(UtestShell::getCurrent(), "foo", actualParameter, *list);
    STRCMP_EQUAL("Mock Failure: Unexpected output parameter name to function \"foo\": bar\n"
                 "\tEXPECTED calls that WERE NOT fulfilled related to function: foo\n"
                 "\t\tfoo -> const void* boo: <output> (expected 1 call, called 0 times)\n"
                 "\t\tfoo -> const void* boo: <output> (expected 1 call, called 0 times)\n"
                 "\tEXPECTED calls that WERE fulfilled related to function: foo\n"
                 "\t\t<none>\n"
                 "\tACTUAL unexpected output parameter passed to function: foo\n"
                 "\t\tvoid* bar", failure.getMessage().asCharString());
}

TEST(MockFailureTest, MockUnexpectedParameterValueFailure)
{
    call1->withName("foo").withParameter("boo", 2);
    call2->withName("foo").withParameter("boo", 10);
    call3->withName("unrelated");
    addThreeCallsToList();

    MockNamedValue actualParameter("boo");
    actualParameter.setValue(20);

    MockUnexpectedInputParameterFailure failure(UtestShell::getCurrent(), "foo", actualParameter, *list);
    STRCMP_EQUAL("Mock Failure: Unexpected parameter value to parameter \"boo\" to function \"foo\": <20 (0x14)>\n"
                 "\tEXPECTED calls that WERE NOT fulfilled related to function: foo\n"
                 "\t\tfoo -> int boo: <2 (0x2)> (expected 1 call, called 0 times)\n"
                 "\t\tfoo -> int boo: <10 (0xa)> (expected 1 call, called 0 times)\n"
                 "\tEXPECTED calls that WERE fulfilled related to function: foo\n"
                 "\t\t<none>\n"
                 "\tACTUAL unexpected parameter passed to function: foo\n"
                 "\t\tint boo: <20 (0x14)>", failure.getMessage().asCharString());
}

TEST(MockFailureTest, MockExpectedParameterDidntHappenFailure)
{
    call1->withName("foo").withParameter("bar", 2).withParameter("boo", "str");
    call1->inputParameterWasPassed("bar");
    call2->withName("foo").withParameter("bar", 10).withParameter("boo", "bleh");
    call2->callWasMade(1);
    call2->inputParameterWasPassed("bar");
    call2->inputParameterWasPassed("boo");
    call3->withName("foo").withParameter("bar", 2).withParameter("boo", "blah").withParameter("baa", 0u);
    call3->inputParameterWasPassed("bar");
    call4->withName("foo").withParameter("bar", 20);
    call5->withName("unrelated");
    addFiveCallsToList();

    MockExpectedCallsList matchingCalls;
    matchingCalls.addExpectedCall(call1);
    matchingCalls.addExpectedCall(call3);

    MockExpectedParameterDidntHappenFailure failure(UtestShell::getCurrent(), "foo", *list, matchingCalls);
    STRCMP_EQUAL("Mock Failure: Expected parameter for function \"foo\" did not happen.\n"
                 "\tEXPECTED calls with MISSING parameters related to function: foo\n"
                 "\t\tfoo -> int bar: <2 (0x2)>, const char* boo: <str> (expected 1 call, called 0 times)\n"
                 "\t\t\tMISSING parameters: const char* boo\n"
                 "\t\tfoo -> int bar: <2 (0x2)>, const char* boo: <blah>, unsigned int baa: <0 (0x0)> (expected 1 call, called 0 times)\n"
                 "\t\t\tMISSING parameters: const char* boo, unsigned int baa\n"
                 "\tEXPECTED calls that WERE NOT fulfilled related to function: foo\n"
                 "\t\tfoo -> int bar: <2 (0x2)>, const char* boo: <str> (expected 1 call, called 0 times)\n"
                 "\t\tfoo -> int bar: <2 (0x2)>, const char* boo: <blah>, unsigned int baa: <0 (0x0)> (expected 1 call, called 0 times)\n"
                 "\t\tfoo -> int bar: <20 (0x14)> (expected 1 call, called 0 times)\n"
                 "\tEXPECTED calls that WERE fulfilled related to function: foo\n"
                 "\t\tfoo -> int bar: <10 (0xa)>, const char* boo: <bleh> (expected 1 call, called 1 time)",
                 failure.getMessage().asCharString());
}

TEST(MockFailureTest, MockNoWayToCompareCustomTypeFailure)
{
    MockNoWayToCompareCustomTypeFailure failure(UtestShell::getCurrent(), "myType");
    STRCMP_EQUAL("MockFailure: No way to compare type <myType>. Please install a MockNamedValueComparator.", failure.getMessage().asCharString());
}

TEST(MockFailureTest, MockUnexpectedObjectFailure)
{
    call1->withName("foo").onObject((void*) 0x02);
    call2->withName("foo").onObject((void*) 0x03);
    call2->callWasMade(1);
    call2->wasPassedToObject();
    call3->withName("unrelated");
    addThreeCallsToList();

    MockUnexpectedObjectFailure failure(UtestShell::getCurrent(), "foo", (void*)0x1, *list);
    STRCMP_EQUAL(StringFromFormat (
                 "MockFailure: Function called on an unexpected object: foo\n"
                 "\tActual object for call has address: <%p>\n"
                 "\tEXPECTED calls that WERE NOT fulfilled related to function: foo\n"
                 "\t\t(object address: %p)::foo -> no parameters (expected 1 call, called 0 times)\n"
                 "\tEXPECTED calls that WERE fulfilled related to function: foo\n"
                 "\t\t(object address: %p)::foo -> no parameters (expected 1 call, called 1 time)",
                 (void*) 0x01, (void*) 0x02, (void*) 0x03).asCharString(), failure.getMessage().asCharString());
}

TEST(MockFailureTest, MockExpectedObjectDidntHappenFailure)
{
    call1->withName("foo").onObject((void*) 0x02);
    call2->withName("foo").onObject((void*) 0x03);
    call2->callWasMade(1);
    call2->wasPassedToObject();
    call3->withName("unrelated");
    addThreeCallsToList();

    MockExpectedObjectDidntHappenFailure failure(UtestShell::getCurrent(), "foo", *list);
    STRCMP_EQUAL(StringFromFormat(
                 "Mock Failure: Expected call on object for function \"foo\" but it did not happen.\n"
                 "\tEXPECTED calls that WERE NOT fulfilled related to function: foo\n"
                 "\t\t(object address: %p)::foo -> no parameters (expected 1 call, called 0 times)\n"
                 "\tEXPECTED calls that WERE fulfilled related to function: foo\n"
                 "\t\t(object address: %p)::foo -> no parameters (expected 1 call, called 1 time)",
                 (void*) 0x2, (void*) 0x3).asCharString(), failure.getMessage().asCharString());
}
