#ifndef XSTR_H
#define XSTR_H

#include <stdarg.h>

#ifdef __GNUC__
#define XSTR_PRINTF(nFmt, nVa) __attribute__((format(printf, nFmt, nVa)))
#define XSTR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define XSTR_PRINTF(nFmt, nVa)
#define XSTR_WARN_UNUSED_RESULT
#endif

typedef struct {
	int len, cap;
	char *str;
} xStr;

void xStrInit(xStr *str, const char *init);
void xStrInitLen(xStr *str, const char *init, int len);
void xStrCleanup(xStr *str);
xStr *xStrNew(const char *init) XSTR_WARN_UNUSED_RESULT;
xStr *xStrNewLen(const char *init, int len) XSTR_WARN_UNUSED_RESULT;
void xStrDelete(xStr *str);

void xStrClear(xStr *str);
void xStrCompact(xStr *str);
void xStrReserve(xStr *str, int n);
void xStrResize(xStr *str, int len);
void xStrSwap(xStr *str, xStr *other);

void xStrAssign(xStr *str, const char *s);
void xStrAssignLen(xStr *str, const char *s, int len);
void xStrAssignCh(xStr *str, char ch);
void xStrAssignFmt(xStr *str, const char *fmt, ...) XSTR_PRINTF(2, 3);
void xStrAssignFmtV(xStr *str, const char *fmt, va_list ap);

void xStrInsert(xStr *str, int pos, const char *s);
void xStrInsertLen(xStr *str, int pos, const char *s, int len);
void xStrInsertCh(xStr *str, int pos, char ch);
void xStrInsertFmt(xStr *str, int pos, const char *fmt, ...) XSTR_PRINTF(3, 4);
void xStrInsertFmtV(xStr *str, int pos, const char *fmt, va_list ap);

void xStrPrepend(xStr *str, const char *s);
void xStrPrependLen(xStr *str, const char *s, int len);
void xStrPrependCh(xStr *str, char ch);
void xStrPrependFmt(xStr *str, const char *fmt, ...) XSTR_PRINTF(2, 3);
void xStrPrependFmtV(xStr *str, const char *fmt, va_list ap);

void xStrAppend(xStr *str, const char *s);
void xStrAppendLen(xStr *str, const char *s, int len);
void xStrAppendCh(xStr *str, char ch);
void xStrAppendFmt(xStr *str, const char *fmt, ...) XSTR_PRINTF(2, 3);
void xStrAppendFmtV(xStr *str, const char *fmt, va_list ap);

void xStrErase(xStr *str, int pos, int len);

void xStrOverwrite(xStr *str, int pos, int len, const char *s);
void xStrOverwriteLen(xStr *str, int pos, int len, const char *s, int slen);
void xStrOverwriteCh(xStr *str, int pos, int len, char ch);
void xStrOverwriteFmt(xStr *str, int pos, int len, const char *fmt, ...) XSTR_PRINTF(4, 5);
void xStrOverwriteFmtV(xStr *str, int pos, int len, const char *fmt, va_list ap);

void xStrReplace(xStr *str, const char *needle, const char *repl, int maxReplace);

void xStrStripFront(xStr *str, const char *chrs);
void xStrStripBack(xStr *str, const char *chrs);
void xStrStrip(xStr *str, const char *chrs);

int xStrCompare(const xStr *str1, const xStr *str2);
int xStrCaseCompare(const xStr *str1, const xStr *str2);
int xStrEqual(const xStr *str1, const xStr *str2);

void xStrToUpper(xStr *str);
void xStrToLower(xStr *str);

int xStrFirstIndexOf(const xStr *str, const char *s);
int xStrFirstIndexOfCh(const xStr *str, char c);
int xStrLastIndexOf(const xStr *str, const char *s);
int xStrLastIndexOfCh(const xStr *str, char c);

void xStrLeftJustify(xStr *str, int len, char fill);
void xStrRightJustify(xStr *str, int len, char fill);
void xStrCenter(xStr *str, int len, char fill);

int xStrStartsWith(const xStr *str, const char *s);
int xStrEndsWith(const xStr *str, const char *s);

#endif // XSTR_H
