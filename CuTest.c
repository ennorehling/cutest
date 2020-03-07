#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "CuTest.h"

/*-------------------------------------------------------------------------*
 * CuStr
 *-------------------------------------------------------------------------*/

char* CuStrAlloc(int size)
{
	char* newStr = (char*) malloc( sizeof(char) * (size) );
	return newStr;
}

char* CuStrCopy(const char* old)
{
	int len = strlen(old);
	char* newStr = CuStrAlloc(len + 1);
	strcpy(newStr, old);
	return newStr;
}

/*-------------------------------------------------------------------------*
 * CuString
 *-------------------------------------------------------------------------*/

void CuStringInit(CuString* str)
{
	str->length = 0;
	str->size = STRING_MAX;
	str->buffer = (char*) malloc(sizeof(char) * str->size);
	str->buffer[0] = '\0';
}

CuString* CuStringNew(void)
{
	CuString* str = (CuString*) malloc(sizeof(CuString));
	str->length = 0;
	str->size = STRING_MAX;
	str->buffer = (char*) malloc(sizeof(char) * str->size);
	str->buffer[0] = '\0';
	return str;
}

void CuStringDelete(CuString *str)
{
        if (!str) return;
        free(str->buffer);
        free(str);
}

void CuStringResize(CuString* str, int newSize)
{
	str->buffer = (char*) realloc(str->buffer, sizeof(char) * newSize);
	str->size = newSize;
}

void CuStringAppend(CuString* str, const char* text)
{
	int length;

	if (text == NULL) {
		text = "NULL";
	}

	length = strlen(text);
	if (str->length + length + 1 >= str->size)
		CuStringResize(str, str->length + length + 1 + STRING_INC);
	str->length += length;
	strcat(str->buffer, text);
}

void CuStringAppendChar(CuString* str, char ch)
{
	char text[2];
	text[0] = ch;
	text[1] = '\0';
	CuStringAppend(str, text);
}

void CuStringAppendFormat(CuString* str, const char* format, ...)
{
	va_list argp;
	char buf[HUGE_STRING_LEN];
	va_start(argp, format);
	vsprintf(buf, format, argp);
	va_end(argp);
	CuStringAppend(str, buf);
}

void CuStringInsert(CuString* str, const char* text, int pos)
{
	int length = strlen(text);
	if (pos > str->length)
		pos = str->length;
	if (str->length + length + 1 >= str->size)
		CuStringResize(str, str->length + length + 1 + STRING_INC);
	memmove(str->buffer + pos + length, str->buffer + pos, (str->length - pos) + 1);
	str->length += length;
	memcpy(str->buffer + pos, text, length);
}

/*-------------------------------------------------------------------------*
 * CuTest
 *-------------------------------------------------------------------------*/

void CuTestInit(CuTest* t, const char* name, TestFunction function)
{
	t->name = CuStrCopy(name);
	t->failed = 0;
	t->ran = 0;
        t->message = NULL;
	t->function = function;
	t->jumpBuf = NULL;
}

CuTest* CuTestNew(const char* name, TestFunction function)
{
	CuTest* tc = CU_ALLOC(CuTest);
	CuTestInit(tc, name, function);
	return tc;
}

void CuTestDelete(CuTest *t)
{
        if (!t) return;
        CuStringDelete(t->message);
        free(t->name);
        free(t);
}

static void TestFunctionRun(CuTest *tc, TestFunction function) {
	jmp_buf buf;
	tc->jumpBuf = &buf;
	if (setjmp(buf) == 0)
	{
		tc->ran = 1;
		function(tc);
	}
	tc->jumpBuf = 0;
}

void CuTestRun(CuTest* tc)
{
	TestFunctionRun(tc, tc->function);
}

static void CuFailInternal(CuTest* tc, const char* file, int line, CuString* string)
{
	char buf[HUGE_STRING_LEN];

	sprintf(buf, "%s:%d: ", file, line);
	CuStringInsert(string, buf, 0);

	tc->failed = 1;
	if (NULL != tc->message) {
		CuStringAppend(tc->message, "\n");
	}
	else {
		tc->message = CuStringNew();
	}
	CuStringAppend(tc->message, string->buffer);
	if (tc->jumpBuf != 0) longjmp(*(tc->jumpBuf), 0);
}

void CuFail_Line(CuTest* tc, const char* file, int line, const char* message2, const char* message)
{
	CuString string;

	CuStringInit(&string);
	if (message2 != NULL) 
	{
		CuStringAppend(&string, message2);
		CuStringAppend(&string, ": ");
	}
	CuStringAppend(&string, message);
	CuFailInternal(tc, file, line, &string);
}

void CuAssert_Line(CuTest* tc, const char* file, int line, const char* message, int condition)
{
	if (condition) return;
	CuFail_Line(tc, file, line, NULL, message);
}

void CuAssertStrEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message, 
	const char* expected, const char* actual)
{
	CuString string;
	if ((expected == NULL && actual == NULL) ||
	    (expected != NULL && actual != NULL &&
	     strcmp(expected, actual) == 0))
	{
		return;
	}

	CuStringInit(&string);
	if (message != NULL) 
	{
		CuStringAppend(&string, message);
		CuStringAppend(&string, ": ");
	}
	CuStringAppend(&string, "expected <");
	CuStringAppend(&string, expected);
	CuStringAppend(&string, "> but was <");
	CuStringAppend(&string, actual);
	CuStringAppend(&string, ">");
	CuFailInternal(tc, file, line, &string);
}

void CuAssertIntEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message, 
	int expected, int actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%d> but was <%d>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}

void CuAssertDblEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message, 
	double expected, double actual, double delta)
{
	char buf[STRING_MAX];
	if (fabs(expected - actual) <= delta) return;
	sprintf(buf, "expected <%f> but was <%f>", expected, actual); 

	CuFail_Line(tc, file, line, message, buf);
}

void CuAssertPtrEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message, 
	void* expected, void* actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected pointer <0x%p> but was <0x%p>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}

void *CuTestContextGet(CuTest *tc) {
	return tc->context;
}

void CuTestContextSet(CuTest *tc, void *context) {
	tc->context = context;
}

/*-------------------------------------------------------------------------*
 * CuSuite
 *-------------------------------------------------------------------------*/
static void EmptySetup(CuTest *tc) {
	(void)tc;
}

static void EmptyTeardown(CuTest *tc) {
	(void)tc;
}

static const CuTestFrame EmptyFrame = {
	.setup = EmptySetup,
	.teardown = EmptyTeardown,
};

void CuSuiteInit(CuSuite* testSuite)
{
	CuSuiteInitWithFrame(testSuite, &EmptyFrame, NULL);
}

void CuSuiteInitWithFrame(CuSuite* testSuite, const CuTestFrame *frame, void *frameContext)
{
	testSuite->count = 0;
	testSuite->failCount = 0;
	memset(testSuite->list, 0, sizeof(testSuite->list));
	testSuite->frame = frame;
	testSuite->frameContext = frameContext;
	testSuite->next = NULL;
}

CuSuite* CuSuiteNew(void)
{
	return CuSuiteNewWithFrame(&EmptyFrame, NULL);
}

CuSuite* CuSuiteNewWithFrame(const CuTestFrame *frame, void *frameContext) {
	CuSuite* testSuite = CU_ALLOC(CuSuite);
	CuSuiteInitWithFrame(testSuite, frame, frameContext);
	return testSuite;
}

void CuSuiteDelete(CuSuite *testSuite)
{
        unsigned int n;
        for (n=0; n < MAX_TEST_CASES; n++)
        {
                if (testSuite->list[n])
                {
                        CuTestDelete(testSuite->list[n]);
                }
        }
        free(testSuite);

}

void CuSuiteAdd(CuSuite* testSuite, CuTest *testCase)
{
	assert(testSuite->count < MAX_TEST_CASES);
	testSuite->list[testSuite->count] = testCase;
	testSuite->count++;
}

void CuSuiteAddSuite(CuSuite* testSuite, CuSuite* testSuite2)
{
	CuSuite *cursor = testSuite;
	while (NULL != cursor->next) {
		cursor = cursor->next;
	}
	cursor->next = testSuite2;
	testSuite2->next = NULL;
}

void CuSuiteRun(CuSuite* testSuite)
{
	while (NULL != testSuite) {
		const CuTestFrame * const frame = testSuite->frame;
		int i;

		for (i = 0 ; i < testSuite->count ; ++i)
		{
			CuTest* testCase = testSuite->list[i];
			testCase->context = testSuite->frameContext;

			TestFunctionRun(testCase, frame->setup);
			if (!testCase->failed) {
				CuTestRun(testCase);
				TestFunctionRun(testCase, frame->teardown);
			}
			testSuite->frameContext = testCase->context;
			if (testCase->failed) { testSuite->failCount += 1; }
		}

		testSuite = testSuite->next;
	}
}

void CuSuiteSummary(CuSuite* testSuite, CuString* summary)
{
	while (NULL != testSuite) {
		int i;
		for (i = 0 ; i < testSuite->count ; ++i)
		{
			CuTest* testCase = testSuite->list[i];
			CuStringAppend(summary, testCase->failed ? "F" : ".");
		}
		testSuite = testSuite->next;
	}
	CuStringAppend(summary, "\n\n");
}

void CuSuiteDetails(CuSuite* testSuite, CuString* details)
{
	int failCount = 0;
	int testCount = 0;
	CuString epilogue;
	const CuSuite *cursor = testSuite;

	CuStringInit(&epilogue);

	while (NULL != cursor) {
		testCount += cursor->count;

		int i;
		for (i = 0 ; i < cursor->count ; ++i)
		{
			CuTest* testCase = cursor->list[i];
			if (testCase->failed)
			{
				failCount++;
				CuStringAppendFormat(&epilogue, "%d) %s: %s\n",
						failCount, testCase->name, testCase->message->buffer);
			}
		}

		cursor = cursor->next;
	}

	{
		int passCount = testCount - failCount;

		if (failCount == 0)
		{
			const char* testWord = passCount == 1 ? "test" : "tests";
			CuStringAppendFormat(details, "OK (%d %s)\n", passCount, testWord);
		}
		else
		{
			if (failCount == 1)
				CuStringAppend(details, "There was 1 failure:\n");
			else
				CuStringAppendFormat(details, "There were %d failures:\n", failCount);

			CuStringAppend(details, epilogue.buffer);

			CuStringAppend(details, "\n!!!FAILURES!!!\n");
			CuStringAppendFormat(details, "Runs: %d ",   testCount);
			CuStringAppendFormat(details, "Passes: %d ", passCount);
			CuStringAppendFormat(details, "Fails: %d\n",  failCount);
		}
	}
}
