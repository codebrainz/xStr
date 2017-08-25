#include "xstr.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WS_CHARS " \t\n\r\v\f"
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void xStrInit(xStr *str, const char *init)
{
	xStrInitLen(str, init, -1);
}

void xStrInitLen(xStr *str, const char *init, int len)
{
	str->len = 0;
	str->cap = 1;
	str->str = malloc(1);
	str->str[0] = '\0';
	if (init)
		xStrAppendLen(str, init, len);
}

void xStrCleanup(xStr *str)
{
	if (str)
		free(str->str);
}

xStr *xStrNew(const char *init)
{
	return xStrNewLen(init, -1);
}

xStr *xStrNewLen(const char *init, int len)
{
	xStr *str = malloc(sizeof(xStr));
	if (str)
		xStrInitLen(str, init, len);
	return str;
}

void xStrDelete(xStr *str)
{
	if (str) {
		xStrCleanup(str);
		free(str);
	}
}

void xStrClear(xStr *str)
{
	if (str->str)
		str->str[0] = '\0';
	str->len = 0;
}

void xStrCompact(xStr *str)
{
	int ncap = str->len + 1;
	if (ncap != str->cap) {
		void *tmp = realloc(str->str, ncap);
		if (!tmp)
			return;
		str->str = tmp;
		str->cap = ncap;
	}
}

void xStrReserve(xStr *str, int n)
{
	int ncap = n + 1;
	if (ncap > str->cap) {
		void *tmp = realloc(str->str, ncap);
		if (!tmp)
			return;
		str->str = tmp;
		str->cap = ncap;
	}
}

void xStrResize(xStr *str, int len)
{
	if (len < 0)
		return;
	if (len != str->len) {
		if (len < str->len) {
			str->str[len] = '\0';
			str->len = len;
		} else {
			xStrReserve(str, len);
			memset(str->str + str->len, 0, (len + 1) - str->len);
			str->len = len;
		}
	}
}

void xStrSwap(xStr *str, xStr *other)
{
	if (!str || !other)
		return;
	xStr tmp;
	tmp = *str;
	*str = *other;
	*other = tmp;
}

void xStrAssign(xStr *str, const char *s)
{
	xStrAssignLen(str, s, -1);
}

void xStrAssignLen(xStr *str, const char *s, int len)
{
	xStrClear(str);
	if (!s || len == 0)
		return;
	xStrAppendLen(str, s, len);
}

void xStrAssignCh(xStr *str, char ch)
{
	char buf[2] = { ch, '\0' };
	xStrAssignLen(str, buf, 1);
}

void xStrAssignFmt(xStr *str, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	xStrAssignFmtV(str, fmt, ap);
	va_end(ap);
}

void xStrAssignFmtV(xStr *str, const char *fmt, va_list ap)
{
	va_list args;
	xStrClear(str);
	va_copy(args, ap);
	xStrAppendFmtV(str, fmt, ap);
	va_end(args);
}

static int xStrEnsureCap(xStr *str, int cap)
{
	if (cap > str->cap) {
		int ncap = str->cap * 2;
		ncap = MAX(ncap, cap);
		void *tmp = realloc(str->str, ncap);
		if (!tmp)
			return 0;
		str->str = tmp;
		str->cap = ncap;
	}
	return 1;
}

void xStrInsert(xStr *str, int pos, const char *s)
{
	xStrInsertLen(str, pos, s, -1);
}

void xStrInsertLen(xStr *str, int pos, const char *s, int len)
{
	if (!s || pos < 0 || pos > str->len)
		return;
	if (len < 0)
		len = strlen(s);
	int nlen = str->len + len;
	if (!xStrEnsureCap(str, nlen + 1))
		return;
	if (pos < str->len)
		memmove(str->str + pos + len, str->str + pos, (str->len - pos) + 1);
	memcpy(str->str + pos, s, len);
	str->len += len;
	str->str[str->len] = '\0';
}

void xStrInsertCh(xStr *str, int pos, char ch)
{
	char buf[2] = { ch, 0 };
	xStrInsertLen(str, pos, buf, 1);
}

void xStrInsertFmt(xStr *str, int pos, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	xStrInsertFmtV(str, pos, fmt, ap);
	va_end(ap);
}

static char *strFormat(int *len, const char *fmt, va_list ap)
{
	int size = 0;
	char *p = NULL;
	va_list args;

	va_copy(args, ap);
	// cppcheck false positive for p being NULL, see C11 draft 7.21.6.5/12
	size = vsnprintf(p /* cppcheck-suppress nullPointer */, size, fmt, args);
	va_end(args);

	if (size < 0)
		return NULL;

	size++;
	p = malloc(size);
	if (p == NULL)
		return NULL;

	va_copy(args, ap);
	size = vsnprintf(p, size, fmt, args);
	va_end(args);

	if (size < 0) {
		free(p);
		return NULL;
	}

	if (len)
		*len = size;

	return p;
}

void xStrInsertFmtV(xStr *str, int pos, const char *fmt, va_list ap)
{
	int len = 0;
	char *tmp = strFormat(&len, fmt, ap);
	if (tmp != NULL) {
		xStrInsertLen(str, pos, tmp, len);
		free(tmp);
	}
}

void xStrPrepend(xStr *str, const char *s)
{
	xStrPrependLen(str, s, -1);
}

void xStrPrependLen(xStr *str, const char *s, int len)
{
	xStrInsertLen(str, 0, s, len);
}

void xStrPrependCh(xStr *str, char ch)
{
	xStrInsertCh(str, 0, ch);
}

void xStrPrependFmt(xStr *str, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	xStrPrependFmtV(str, fmt, ap);
	va_end(ap);
}

void xStrPrependFmtV(xStr *str, const char *fmt, va_list ap)
{
	va_list args;
	va_copy(args, ap);
	xStrInsertFmtV(str, 0, fmt, args);
	va_end(args);
}

void xStrAppend(xStr *str, const char *s)
{
	xStrAppendLen(str, s, -1);
}

void xStrAppendLen(xStr *str, const char *s, int len)
{
	xStrInsertLen(str, str->len, s, len);
}

void xStrAppendCh(xStr *str, char ch)
{
	xStrInsertCh(str, str->len, ch);
}

void xStrAppendFmt(xStr *str, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	xStrAppendFmtV(str, fmt, ap);
	va_end(ap);
}

void xStrAppendFmtV(xStr *str, const char *fmt, va_list ap)
{
	va_list args;
	va_copy(args, ap);
	xStrInsertFmtV(str, str->len, fmt, args);
	va_end(args);
}

void xStrErase(xStr *str, int pos, int len)
{
	if (pos < 0 || pos >= str->len || len == 0 || len < -1)
		return;
	if (len == -1)
		len = str->len - pos;
	int end;
	if ((pos + len) > str->len)
		end = str->len;
	else
		end = pos + len;
	if (end == str->len) {
		str->str[pos] = '\0';
		str->len = pos;
	} else {
		memmove(str->str + pos, str->str + end, str->len - (end - pos));
		str->len -= len;
		str->str[str->len] = '\0';
	}
}

void xStrOverwrite(xStr *str, int pos, int len, const char *s)
{
	xStrOverwriteLen(str, pos, len, s, -1);
}

void xStrOverwriteLen(xStr *str, int pos, int len, const char *s, int slen)
{
	if (slen < 0)
		slen = strlen(s);
	xStrErase(str, pos, len);
	if (s && slen > 0)
		xStrInsertLen(str, pos, s, slen);
}

void xStrOverwriteCh(xStr *str, int pos, int len, char ch)
{
	char buf[2] = { ch, 0 };
	xStrOverwriteLen(str, pos, len, buf, -1);
}

void xStrOverwriteFmt(xStr *str, int pos, int len, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	xStrOverwriteFmtV(str, pos, len, fmt, ap);
	va_end(ap);
}

void xStrOverwriteFmtV(xStr *str, int pos, int len, const char *fmt,
	va_list ap)
{
	va_list args;
	va_copy(args, ap);
	int slen = 0;
	char *tmp = strFormat(&slen, fmt, args);
	va_end(args);
	xStrOverwriteLen(str, pos, len, tmp, slen);
	free(tmp);
}

static int strFirstIndexOf(const char *s, const char *find)
{
	const char *found = strstr(s, find);
	if (!found)
		return -1;
	return (found - s);
}

void xStrReplace(xStr *str, const char *needle, const char *repl,
	int maxReplace)
{
	if (!needle || !repl || maxReplace < 0 || str->len == 0)
		return;

	int pos = 0;
	int numReplaced = 0;
	const int needleLen = strlen(needle);
	const int replLen = strlen(repl);

	while (pos < str->len && (maxReplace < 1 || numReplaced < maxReplace)) {
		int idx = strFirstIndexOf(str->str + pos, needle);
		if (idx >= 0) {
			pos += idx;
			xStrOverwriteLen(str, pos, needleLen, repl, replLen);
			numReplaced++;
		} else {
			break;
		}
	}
}

void xStrStripFront(xStr *str, const char *chrs)
{
	if (!chrs)
		chrs = WS_CHARS;
	const char *start;
	for (start = str->str; *start && (strchr(chrs, *start) != NULL); start++)
		;
	int len = strlen(start);
	memmove(str->str, start, len + 1);
	str->len = len;
}

void xStrStripBack(xStr *str, const char *chrs)
{
	if (!chrs)
		chrs = WS_CHARS;
	int len = str->len;
	while (len--) {
		if (strchr(chrs, str->str[len]) != NULL)
			str->str[len] = '\0';
		else
			break;
	}
	str->len = strlen(str->str);
}

void xStrStrip(xStr *str, const char *chrs)
{
	xStrStripFront(str, chrs);
	xStrStripBack(str, chrs);
}

int xStrCompare(const xStr *str1, const xStr *str2)
{
	if (!str1 || !str2)
		return (str1 - str2);
	else if (!str1->str || !str2->str)
		return (str1->str - str2->str);
	else
		return strcmp(str1->str, str2->str);
}

static int strCaseCompare(char *s1, char *s2)
{
	if (s1 == s2)
		return 0;
	const unsigned char *p1 = (unsigned char *)s1;
	const unsigned char *p2 = (unsigned char *)s2;
	unsigned char c1, c2;
	do {
		c1 = tolower(*p1), c2 = tolower(*p2);
		if (!c1)
			break;
		p1++, p2++;
	} while (c1 == c2);
#if UCHAR_MAX <= INT_MAX
	return c1 - c2;
#else
	return (c1 > c2 ? 1 : (c1 < c2 ? -1 : 0));
#endif
}

int xStrCaseCompare(const xStr *str1, const xStr *str2)
{
	if (!str1 || !str2)
		return (str1 - str2);
	else if (!str1->str || !str2->str)
		return (str1->str - str2->str);
	else
		return strCaseCompare(str1->str, str2->str);
}

int xStrEqual(const xStr *str1, const xStr *str2)
{
	return (xStrCompare(str1, str2) == 0);
}

void xStrToUpper(xStr *str)
{
	for (int i = 0; i < str->len; i++)
		str->str[i] = toupper(str->str[i]);
}

void xStrToLower(xStr *str)
{
	for (int i = 0; i < str->len; i++)
		str->str[i] = tolower(str->str[i]);
}

int xStrFirstIndexOf(const xStr *str, const char *s)
{
	if (!s || s[0] == '\0')
		return -1;
	return strFirstIndexOf(str->str, s);
}

int xStrFirstIndexOfCh(const xStr *str, char c)
{
	char buf[2] = { c, 0 };
	return xStrFirstIndexOf(str, buf);
}

int xStrLastIndexOf(const xStr *str, const char *s)
{
	if (!s || s[0] == '\0')
		return -1;
	int len = strlen(s);
	const char *off = (str->str + str->len) - len;
	while (off >= str->str) {
		const char *found = strstr(off, s);
		if (found)
			return (found - str->str);
		off--;
	}
	return -1;
}

int xStrLastIndexOfCh(const xStr *str, char c)
{
	char buf[2] = { c, 0 };
	return xStrLastIndexOf(str, buf);
}

void xStrLeftJustify(xStr *str, int len, char fill)
{
	if (len <= str->len)
		return;
	const int diff = len - str->len;
	for (int i = 0; i < diff; i++)
		xStrAppendCh(str, fill);
}

void xStrRightJustify(xStr *str, int len, char fill)
{
	if (len <= str->len)
		return;
	const int diff = len - str->len;
	for (int i = 0; i < diff; i++)
		xStrPrependCh(str, fill);
}

void xStrCenter(xStr *str, int len, char fill)
{
	if (len <= str->len)
		return;

	xStr tmp;
	xStrInit(&tmp, "");
	xStrReserve(&tmp, len);

	const int x = (len / 2) - (str->len / 2);
	for (int i = 0; i < x; i++)
		xStrAppendCh(&tmp, fill);
	for (int i = 0; i < str->len; i++)
		xStrAppendCh(&tmp, str->str[i]);
	for (int i = x + str->len; i < len; i++)
		xStrAppendCh(&tmp, fill);

	xStrSwap(&tmp, str);
	xStrCleanup(&tmp);
}

int xStrStartsWith(const xStr *str, const char *s)
{
	if (!s || s[0] == '\0')
		return 0;
	const char *found = strstr(str->str, s);
	if (!found)
		return 0;
	return (found == str->str);
}

int xStrEndsWith(const xStr *str, const char *s)
{
	if (!s || s[0] == '\0')
		return 0;
	int slen = strlen(s);
	if (slen > str->len)
		return 0;
	const char *last = str->str + (str->len - slen);
	return (strcmp(last, s) == 0);
}
