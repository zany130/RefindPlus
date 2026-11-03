// Host services used by the extracted production functions. WORD_BITS models
// EFI native integer widths; the host process and allocator remain native.
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#if WORD_BITS == 32
typedef int32_t INTN;
typedef uint32_t UINTN;
#define WORD_MAX INT32_MAX
#else
typedef int64_t INTN;
typedef uint64_t UINTN;
#define WORD_MAX INT64_MAX
#endif
#define WORD_MIN (-WORD_MAX - 1)
typedef int64_t INT64;
typedef wchar_t CHAR16;
typedef int BOOLEAN;
#define TRUE 1
#define FALSE 0
#define IN
#define OUT
#define VOID void
#define AllocatePool malloc
#define MY_FREE_POOL(p) free(p)
#define HIDEUI_FLAG_HINTS 1
static BOOLEAN AllowGraphicsMode;
static UINTN ScreenW, ScreenH, ConHeight, TileSizes[2], IconRowCount, IconRowItems;
static UINTN ToolRowCount, IconRowPosY, IconRowPosX, ToolRowPosY, ToolRowPosX, font;
static UINTN egGetFontHeight(void) { return font; }
static UINTN TextLineHeight(void) { return font + 4; }
static struct { INTN RowIconsMove, RowIconsTune, RowToolsTune; BOOLEAN ToolsRowAbove; UINTN HideUIFlags; } GlobalConfig;
typedef struct { UINTN Row; } Entry;
typedef struct { UINTN EntryCount; Entry **Entries; } REFIT_MENU_SCREEN;
static BOOLEAN MyStriCmp(const CHAR16 *left, const CHAR16 *right) {
    while (*left && (*left | 32) == (*right | 32)) { ++left; ++right; }
    return *left == *right;
}
typedef struct { UINTN Width, Height; BOOLEAN HasAlpha; unsigned *PixelData; } EG_IMAGE;
typedef unsigned EG_PIXEL;
typedef struct { const CHAR16 *line; } REFIT_FILE;
static UINTN StrSize(const CHAR16 *s) {
    UINTN len=0;
    while (s[len]) ++len;
    return (len+1)*sizeof(CHAR16);
}
static CHAR16 *StrDuplicate(const CHAR16 *s) {
    size_t bytes=StrSize(s);
    CHAR16 *copy=malloc(bytes);
    assert(copy);
    return memcpy(copy,s,bytes);
}
static void StrCpyS(CHAR16 *dest, UINTN capacity, const CHAR16 *src) {
    assert(StrSize(src)<=capacity*sizeof(CHAR16));
    memcpy(dest,src,StrSize(src));
}
static CHAR16 *ReadLine(REFIT_FILE *file) {
    if (!file->line) return NULL;
    CHAR16 *line=StrDuplicate(file->line);
    file->line=NULL;
    return line;
}
static void AddListElement(void ***list, UINTN *count, void *element) {
    ++*count;
    *list=realloc(*list,*count*sizeof(void *));
    assert(*list);
    (*list)[*count-1]=element;
}
