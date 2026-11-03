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
#define MY_FREE_POOL(p) do { free(p); (p)=NULL; } while (0)
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

typedef void *EFI_EVENT;
typedef int EFI_STATUS;
static EFI_EVENT *WaitList;
static UINTN WaitListLength, pointers, generation, event_index, events_seen;
static UINTN pdCount(void) { return pointers; }
static EFI_EVENT pdWaitEvent(UINTN i) { return (void *)(uintptr_t)(2+generation+i); }
static struct { EFI_EVENT WaitForKey; } keyboard;
static struct { __typeof__(keyboard) *ConIn; } system_table={&keyboard}, *gST=&system_table;
#define EFI_ERROR(status) ((status)!=0)
#define EVT_TIMER 0
#define TimerRelative 0
#define INPUT_KEY 0
#define INPUT_POINTER 1
#define INPUT_TIMEOUT 2
#define INPUT_TIMER_ERROR 3
#define REFIT_CALL_1_WRAPPER(f,a) f(a)
#define REFIT_CALL_3_WRAPPER(f,a,b,c) f(a,b,c)
#define REFIT_CALL_5_WRAPPER(f,a,b,c,d,e) f(a,b,c,d,e)
#define RefitStall(n) ((void)(n))
static EFI_STATUS create(int a,int b,void *c,void *d,EFI_EVENT *e) {
    (void)a;(void)b;(void)c;(void)d;*e=(EFI_EVENT)(uintptr_t)1;return 0;
}
static EFI_STATUS timer(EFI_EVENT e,int mode,UINTN ticks) {
    (void)e;(void)mode;(void)ticks;return 0;
}
static EFI_STATUS wait_events(UINTN count,EFI_EVENT *events,UINTN *index) {
    assert(events[0]==keyboard.WaitForKey && event_index<count);
    for (UINTN i=0;i<pointers;++i) assert(events[1+i]==pdWaitEvent(i));
    if (count==pointers+2) assert(events[count-1]==(EFI_EVENT)(uintptr_t)1);
    events_seen=count;*index=event_index;return 0;
}
static EFI_STATUS close_event(EFI_EVENT e) { (void)e;return 0; }
static struct {
    __typeof__(create) *CreateEvent; __typeof__(timer) *SetTimer;
    __typeof__(wait_events) *WaitForEvent; __typeof__(close_event) *CloseEvent;
} services={create,timer,wait_events,close_event}, *gBS=&services;
