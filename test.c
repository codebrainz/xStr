#include "xstr.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int strEqual(const char *s1, const char *s2)
{
	if (!s1 || !s2)
		return (s1 - s2);
	else
		return (strcmp(s1, s2) == 0);
}

#define assertEq(x, s) assert(strEqual((x)->str, s))
#define assertLen(x, n) assert((x)->len == (n))
#define assertCap(x) assert((x)->cap >= ((x)->len + 1))

static void printStr(const xStr *s)
{
	printf("Str: %s, len=%d, cap=%d\n", s->str, s->len, s->cap);
	fflush(stdout);
}

static void testInit(xStr *s)
{
	xStrInit(s, "abc");
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));
	assertCap(s);
}

static void testClear(xStr *s)
{
	xStrAssign(s, "abc");
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));
	assertCap(s);

	xStrClear(s);
	assertEq(s, "");
	assertLen(s, 0);
	assertCap(s);
}

static void testCompact(xStr *s)
{
	xStrClear(s);
	for (int i = 0; i < 100; i++)
		xStrAppend(s, "abc");
	assert(s->cap > (s->len + 1)); // oversized
	int oldLen = s->len;
	xStrCompact(s);
	assertLen(s, oldLen);
	assert(s->cap == (s->len + 1)); // exact fit
}

static void testReserve(xStr *s)
{
	xStrClear(s);
	xStrCompact(s);
	assertLen(s, 0);
	assert(s->cap == (s->len + 1));
	xStrReserve(s, 42);
	assert(s->cap == 43);
	for (int i = 0; i < s->len; i++)
		assert(s->str[i] == '\0');
	xStrReserve(s, 24);
	assert(s->cap == 43);
}

static void testResize(xStr *s)
{
	// resizing a 0 length string to zero length
	xStrClear(s);
	xStrResize(s, 0);
	assertEq(s, "");
	assertLen(s, 0);

	// truncation
	xStrAssign(s, "abc");
	xStrResize(s, 2);
	assertEq(s, "ab");
	assertLen(s, 2);
	xStrResize(s, 1);
	assertEq(s, "a");
	assertLen(s, 1);
	xStrResize(s, 0);
	assertEq(s, "");
	assertLen(s, 0);

	// filling with zeroes on right
	xStrAssign(s, "abc");
	xStrResize(s, 4);
	assertEq(s, "abc");
	assertLen(s, 4);
	assert(s->str[3] == '\0');

	// giving existing size does nothing
	xStrAssign(s, "abc");
	xStrResize(s, 3);
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));
}

static void testSwap(xStr *s)
{
	xStr s2;

	// two filled strings
	xStrInit(&s2, "abc");
	xStrAssign(s, "12345");
	xStrSwap(s, &s2);
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));
	assertEq(&s2, "12345");
	assertLen(&s2, strlen("12345"));

	// one str is empty
	xStrAssign(&s2, "abc");
	xStrClear(s);
	xStrSwap(&s2, s);
	assertEq(&s2, "");
	assertLen(&s2, 0);
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));

	xStrCleanup(&s2);
}

static void testAssign(xStr *s)
{
	xStrClear(s);

	xStrAssign(s, "abc");
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));
	assertCap(s);

	xStrAssignCh(s, 'a');
	assertEq(s, "a");
	assertLen(s, 1);
	assertCap(s);

	xStrAssignFmt(s, "abc%d", 123);
	assertEq(s, "abc123");
	assertLen(s, strlen("abc123"));
	assertCap(s);
}

static void testInsert(xStr *s)
{
	// insert into empty string at front
	xStrClear(s);
	xStrInsert(s, 0, "def");
	assertEq(s, "def");
	assertLen(s, strlen("def"));

	// insert into empty string at back
	xStrClear(s);
	xStrInsert(s, 0, "def");
	assertEq(s, "def");
	assertLen(s, strlen("def"));

	// insert abc at front
	xStrInsert(s, 0, "abc");
	assertEq(s, "abcdef");
	assertLen(s, strlen("abcdef"));

	// insert jkl at end
	xStrInsert(s, s->len, "jkl");
	assertEq(s, "abcdefjkl");
	assertLen(s, strlen("abcdefjkl"));

	// insert ghi before jkl
	xStrInsert(s, 6, "ghi");
	assertEq(s, "abcdefghijkl");
	assertLen(s, strlen("abcdefghijkl"));

	// inserting a char in the middle
	xStrClear(s);
	xStrInsert(s, 0, "ac");
	xStrInsertCh(s, 1, 'b');
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));

	// inserting a format into the middle
	xStrInsertFmt(s, 2, "1%d3", 2);
	assertEq(s, "ab123c");
	assertLen(s, strlen("ab123c"));
}

static void testPrepend(xStr *s)
{
	xStrAssign(s, "def");
	xStrPrepend(s, "abc");
	assertEq(s, "abcdef");
	assertLen(s, strlen("abcdef"));
	xStrPrependCh(s, '4');
	assertEq(s, "4abcdef");
	assertLen(s, strlen("4abcdef"));
	xStrPrependFmt(s, "1%d3", 2);
	assertEq(s, "1234abcdef");
	assertLen(s, strlen("1234abcdef"));
}

static void testAppend(xStr *s)
{
	xStrAssign(s, "abc");
	xStrAppend(s, "def");
	assertEq(s, "abcdef");
	assertLen(s, strlen("abcdef"));
	xStrAppendCh(s, '1');
	assertEq(s, "abcdef1");
	assertLen(s, strlen("abcdef1"));
	xStrAppendFmt(s, "2%d4", 3);
	assertEq(s, "abcdef1234");
	assertLen(s, strlen("abcdef1234"));
}

static void testErase(xStr *s)
{
	// erase at front
	xStrAssign(s, "abcdefghi");
	xStrErase(s, 0, 3);
	assertEq(s, "defghi");
	assertLen(s, strlen("defghi"));

	// erase at end
	xStrAssign(s, "abcdefghi");
	xStrErase(s, 6, 3);
	assertEq(s, "abcdef");
	assertLen(s, strlen("abcdef"));

	// erase at end, to end
	xStrAssign(s, "abcdefghi");
	xStrErase(s, 6, -1);
	assertEq(s, "abcdef");
	assertLen(s, strlen("abcdef"));

	// pos out of range does nothing
	xStrErase(s, 42, 1);
	assertEq(s, "abcdef");
	assertLen(s, strlen("abcdef"));

	// pos + len past str->len erases to end
	xStrErase(s, 2, 42);
	assertEq(s, "ab");
	assertLen(s, strlen("ab"));

	// from empty string
	xStrClear(s);
	xStrErase(s, 0, 1);
	assertEq(s, "");
	assertLen(s, 0);
}

static void testOverwrite(xStr *s)
{
	// overwrite in middle
	xStrAssign(s, "abcdefghi");
	xStrOverwrite(s, 3, 3, "123");
	assertEq(s, "abc123ghi");
	assertLen(s, strlen("abc123ghi"));

	// overwrite at front
	xStrOverwrite(s, 0, 3, "ABC");
	assertEq(s, "ABC123ghi");
	assertLen(s, strlen("ABC123ghi"));

	// overwrite at end
	xStrOverwrite(s, 6, 3, "GHI");
	assertEq(s, "ABC123GHI");
	assertLen(s, strlen("ABC123GHI"));

	// overwrite to end
	xStrOverwrite(s, 3, -1, "");
	assertEq(s, "ABC");
	assertLen(s, strlen("ABC"));

	// pos out of range does nothing
	xStrOverwrite(s, 42, 1, "abc");
	assertEq(s, "ABC");
	assertLen(s, strlen("ABC"));

	// pos + len past str->len erases to end
	xStrErase(s, 2, 42);
	assertEq(s, "AB");
	assertLen(s, strlen("AB"));

	// at index 0 in empty string, insert/assign
	xStrClear(s);
	xStrOverwrite(s, 0, 1, "abc");
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));

	// past index 0 in empty string, do nothing
	xStrClear(s);
	xStrOverwrite(s, 42, 1, "abc");
	assertEq(s, "");
	assertLen(s, 0);
}

static void testReplace(xStr *s)
{
	// replace 1
	xStrAssign(s, "abc123def123ghi");
	xStrReplace(s, "123", "XYZ", 1);
	assertEq(s, "abcXYZdef123ghi");
	assertLen(s, strlen("abcXYZdef123ghi"));

	// replace all
	xStrAssign(s, "abc123def123ghi");
	xStrReplace(s, "123", "XYZ", 0);
	assertEq(s, "abcXYZdefXYZghi");
	assertLen(s, strlen("abcXYZdefXYZghi"));

	// replace in empty string
	xStrClear(s);
	xStrReplace(s, "123", "XYZ", 0);
	assertEq(s, "");
	assertLen(s, 0);
}

static void testStripFront(xStr *s)
{
	xStrAssign(s, "abcdef");
	xStrStripFront(s, "bac");
	assertEq(s, "def");
	assertLen(s, strlen("def"));

	xStrAssign(s, " \t\n\r\v\fabc");
	xStrStripFront(s, NULL);
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));

	xStrClear(s);
	xStrStripFront(s, NULL);
	assertEq(s, "");
	assertLen(s, 0);
}

static void testStripBack(xStr *s)
{
	xStrAssign(s, "abcdef");
	xStrStripBack(s, "fde");
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));

	xStrAssign(s, "abc\f\v\r\n\t ");
	xStrStripBack(s, NULL);
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));

	xStrClear(s);
	xStrStripBack(s, NULL);
	assertEq(s, "");
	assertLen(s, 0);
}

static void testStrip(xStr *s)
{
	xStrAssign(s, "abcdef");
	xStrStrip(s, "abef");
	assertEq(s, "cd");
	assertLen(s, strlen("cd"));

	xStrAssign(s, "\r\t \n\f\vabc\f\v\r\n\t ");
	xStrStrip(s, NULL);
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));

	xStrClear(s);
	xStrStrip(s, NULL);
	assertEq(s, "");
	assertLen(s, 0);
}

static void testCompare(xStr *s)
{
	xStr s2;
	xStrInit(&s2, "def");

	xStrAssign(s, "def");
	assert(xStrCompare(s, &s2) == 0);

	xStrAssign(&s2, "ghi");
	assert(xStrCompare(s, &s2) < 0);

	xStrAssign(&s2, "abc");
	assert(xStrCompare(s, &s2) > 0);

	assert(xStrCompare(NULL, NULL) == 0);
	assert(xStrCompare(&s2, NULL) < 0);
	assert(xStrCompare(NULL, &s2) > 0);

	xStrCleanup(&s2);
}

static void testCaseCompare(xStr *s)
{
	xStr s2;
	xStrInit(&s2, "dEf");

	xStrAssign(s, "DeF");
	assert(xStrCaseCompare(s, &s2) == 0);

	xStrAssign(&s2, "Ghi");
	assert(xStrCaseCompare(s, &s2) < 0);

	xStrAssign(&s2, "abC");
	assert(xStrCaseCompare(s, &s2) > 0);

	assert(xStrCaseCompare(NULL, NULL) == 0);
	assert(xStrCaseCompare(&s2, NULL) < 0);
	assert(xStrCaseCompare(NULL, &s2) > 0);

	xStrCleanup(&s2);
}

static void testEqual(xStr *s)
{
	xStr s2;
	xStrInit(&s2, "abc");

	xStrAssign(s, "abc");
	assert(xStrEqual(s, &s2));

	xStrAssign(s, "def");
	assert(!xStrEqual(s, &s2));

	assert(!xStrEqual(&s2, NULL));
	assert(!xStrEqual(NULL, &s2));

	xStrClear(s);
	xStrClear(&s2);
	assert(xStrEqual(s, &s2));
	assert(xStrEqual(NULL, NULL));

	xStrCleanup(&s2);
}

static void testToUpper(xStr *s)
{
	xStrAssign(s, "a1b2C;");
	xStrToUpper(s);
	assertEq(s, "A1B2C;");
	xStrClear(s);
	xStrToUpper(s);
	assertEq(s, "");
}

static void testToLower(xStr *s)
{
	xStrAssign(s, "A1B2c;");
	xStrToLower(s);
	assertEq(s, "a1b2c;");
	xStrClear(s);
	xStrToLower(s);
	assertEq(s, "");
}

static void testFirstIndexOf(xStr *s)
{
	xStrAssign(s, "abcabc");
	assert(xStrFirstIndexOf(s, "b") == 1);
	assert(xStrFirstIndexOfCh(s, 'c') == 2);
	assert(xStrFirstIndexOf(s, "z") == -1);
	assert(xStrFirstIndexOf(s, "") == -1);
}

static void testLastIndexOf(xStr *s)
{
	xStrAssign(s, "abcabc");
	assert(xStrLastIndexOf(s, "b") == 4);
	assert(xStrLastIndexOfCh(s, 'c') == 5);
	assert(xStrLastIndexOf(s, "z") == -1);
	assert(xStrLastIndexOf(s, "") == -1);
}

static void testLeftJustify(xStr *s)
{
	// filling same size does nothing
	xStrAssign(s, "abc");
	xStrLeftJustify(s, 3, ' ');
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));

	// filling 1 extra with char '1'
	xStrLeftJustify(s, 4, '1');
	assertEq(s, "abc1");
	assertLen(s, strlen("abc1"));
	assert(s->str[3] == '1');

	// filling with a few spaces
	xStrAssign(s, "abc");
	xStrLeftJustify(s, 6, ' ');
	assertEq(s, "abc   ");
	assertLen(s, strlen("abc   "));

	// filling an empty string with 3 z's
	xStrClear(s);
	xStrLeftJustify(s, 3, 'z');
	assertEq(s, "zzz");
	assertLen(s, strlen("zzz"));
}

static void testRightJustify(xStr *s)
{
	// filling same size does nothing
	xStrAssign(s, "abc");
	xStrRightJustify(s, 3, ' ');
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));

	// filling 1 extra with char '1'
	xStrRightJustify(s, 4, '1');
	assertEq(s, "1abc");
	assertLen(s, strlen("1abc"));
	assert(s->str[0] == '1');

	// filling with a few spaces
	xStrAssign(s, "abc");
	xStrRightJustify(s, 6, ' ');
	assertEq(s, "   abc");
	assertLen(s, strlen("   abc"));

	// filling an empty string with 3 z's
	xStrClear(s);
	xStrRightJustify(s, 3, 'z');
	assertEq(s, "zzz");
	assertLen(s, strlen("zzz"));
}

static void testCenter(xStr *s)
{
	// filling same size does nothing
	xStrAssign(s, "abc");
	xStrCenter(s, 3, ' ');
	assertEq(s, "abc");
	assertLen(s, strlen("abc"));

	// equal padding on both sides
	xStrAssign(s, "abc");
	xStrCenter(s, 9, '1');
	assertEq(s, "111abc111");
	assertLen(s, strlen("111abc111"));

	// unequal padding leans right
	xStrAssign(s, "abc");
	xStrCenter(s, 8, ' ');
	assertEq(s, "   abc  ");
	assertLen(s, strlen("   abc  "));
}

static void testStartsWith(xStr *s)
{
	xStrAssign(s, "abc123def");
	assert(xStrStartsWith(s, "abc"));
	assert(!xStrStartsWith(s, "xyz"));
	assert(!xStrStartsWith(s, "abc123defghi"));

	xStrClear(s);
	assert(!xStrStartsWith(s, "abc"));
	assert(!xStrStartsWith(s, ""));
}

static void testEndsWith(xStr *s)
{
	xStrAssign(s, "abc123def");
	assert(xStrEndsWith(s, "def"));
	assert(!xStrEndsWith(s, "xyz"));
	assert(!xStrEndsWith(s, "abc123defghi"));

	xStrClear(s);
	assert(!xStrEndsWith(s, "def"));
	assert(!xStrEndsWith(s, ""));
}

int main()
{
	xStr s;

	testInit(&s);

	testClear(&s);
	testCompact(&s);
	testReserve(&s);
	testResize(&s);
	testSwap(&s);
	testAssign(&s);
	testInsert(&s);
	testPrepend(&s);
	testAppend(&s);
	testErase(&s);
	testOverwrite(&s);
	testReplace(&s);
	testStripFront(&s);
	testStripBack(&s);
	testStrip(&s);
	testCompare(&s);
	testCaseCompare(&s);
	testEqual(&s);
	testToUpper(&s);
	testToLower(&s);
	testFirstIndexOf(&s);
	testLastIndexOf(&s);
	testLeftJustify(&s);
	testRightJustify(&s);
	testCenter(&s);
	testStartsWith(&s);
	testEndsWith(&s);

	xStrCleanup(&s);

	(void)printStr;

	return 0;
}
