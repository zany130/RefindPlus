// Independent geometry oracle uses wide signed arithmetic throughout.
typedef __int128 Wide;
static Wide smaller(Wide a, Wide b) { return a < b ? a : b; }
static Wide larger(Wide a, Wide b) { return a > b ? a : b; }
static uint32_t seed = 261 + WORD_BITS;
static uint32_t random_word(void) {
    seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
    return seed;
}
static void layout_cases(void) {
    const UINTN sizes[][2] = {{640,480},{800,600},{1024,768},{1280,720},{1920,1080},
        {2560,1440},{3840,2160},{1,1},{64,32},{4096,64},{64,4096}};
    const INTN edge[] = {WORD_MIN,-100000,-100,-40,-16,-10,-9,-6,-1,0,1,6,10,16,40,100,100000,WORD_MAX};
    for (unsigned i = 0; i < 500000; ++i) {
        unsigned size = random_word() % (sizeof sizes/sizeof *sizes);
        ScreenW = sizes[size][0]; ScreenH = sizes[size][1];
        TileSizes[0] = 1 + random_word()%640; TileSizes[1] = 1 + random_word()%320;
        font = 4 + random_word()%64;
        GlobalConfig.RowIconsMove = (INTN)(random_word()%3)-1;
        GlobalConfig.RowIconsTune = edge[random_word()%(sizeof edge/sizeof *edge)];
        GlobalConfig.RowToolsTune = i%2 ? edge[random_word()%(sizeof edge/sizeof *edge)] : (int)(random_word()%1025)-512;
        GlobalConfig.ToolsRowAbove = random_word()%2;
        GlobalConfig.HideUIFlags = random_word()%2;
        IconRowCount = random_word()%10; ToolRowCount = random_word()%8;
        Wide y = (Wide)(ScreenH/2) - TileSizes[0]/2;
        if (GlobalConfig.RowIconsMove > 0) y += (Wide)(TileSizes[0]/2) - ScreenH/4;
        if (GlobalConfig.RowIconsMove < 0) y += ScreenH/4;
        y = smaller(ScreenH, larger(0, y + GlobalConfig.RowIconsTune));
        IconRowPosY = ComputeRow0PosY(TRUE);
        assert(IconRowPosY == y);
        Wide tune = GlobalConfig.RowToolsTune;
        Wide gap = larger(6, 16 + tune);
        Wide space = y - TileSizes[1];
        BOOLEAN above = GlobalConfig.ToolsRowAbove && IconRowCount && ToolRowCount &&
            y < ScreenH && space >= smaller(16,gap);
        Wide bottom = y + TileSizes[0];
        Wide expected = bottom + 16;
        if (above) expected = larger(0, space - gap);
        else if (ToolRowCount) {
            Wide room = (Wide)ScreenH - bottom - TileSizes[1] - 16 - 2*(font+4)
                - (GlobalConfig.HideUIFlags ? 0 : 3*font);
            expected = bottom + smaller(gap, larger(16, room));
        }
        assert(PlaceToolsAbove() == above);
        UINTN actual = ComputeToolRowPosY();
        assert(actual == expected);
        if (IconRowCount && ToolRowCount) {
            assert(above ? (Wide)actual+TileSizes[1]+6 <= y : actual >= bottom+6);
        }
        Wide occupied = IconRowCount ? (Wide)IconRowCount*(TileSizes[0]+8)-8 : 0;
        assert(RowStartX(IconRowCount,TileSizes[0]) == larger(0, (ScreenW-occupied)/2));
    }
}

static void parse_line(const CHAR16 *line) {
    REFIT_FILE file={line};
    CHAR16 **tokens;
    UINTN count=ReadTokenLine(&file,&tokens);
    parse(tokens,count);
    for (UINTN i=0; i<count; ++i) free(tokens[i]);
    free(tokens);
}

static void parser_cases(void) {
    CHAR16 *tokens[] = {L"ROW_TOOLS_TUNE",L"40",L"extra"};
    parse(tokens,2); assert(GlobalConfig.RowToolsTune == 40);
    CHAR16 *bad[] = {L"",L"+",L"-",L"+-1",L"--2",L"1.2",L"12px",L"abc",L" 6"};
    for (unsigned i=0; i<sizeof bad/sizeof *bad; ++i) {
        tokens[1]=bad[i]; parse(tokens,2); assert(GlobalConfig.RowToolsTune==40);
    }
    tokens[1]=L"-6"; parse(tokens,1); parse(tokens,3); assert(GlobalConfig.RowToolsTune==40);
    parse(tokens,2); assert(GlobalConfig.RowToolsTune==-6);
    CHAR16 digits[8192]; digits[0]=L'-';
    for (unsigned i=1; i<8191; ++i) digits[i]=L'9';
    digits[8191]=0; tokens[1]=digits;
    parse(tokens,2); assert(GlobalConfig.RowToolsTune==WORD_MIN);
    tokens[1]=digits+1; parse(tokens,2); assert(GlobalConfig.RowToolsTune==WORD_MAX);
    digits[8190]=L'x'; parse(tokens,2); assert(GlobalConfig.RowToolsTune==WORD_MAX);
    tokens[0]=L"row_icons_tune"; tokens[1]=L"+20"; parse(tokens,2); assert(GlobalConfig.RowIconsTune==-20);
    tokens[0]=L"icon_row_tune"; tokens[1]=L"-20"; parse(tokens,2); assert(GlobalConfig.RowIconsTune==20);
    tokens[0]=L"row_icons_move"; parse(tokens,2); assert(GlobalConfig.RowIconsMove==-20);
    tokens[0]=L"icon_row_move"; tokens[1]=L"+10"; parse(tokens,2); assert(GlobalConfig.RowIconsMove==10);
    tokens[0]=L"row_tools_position"; tokens[1]=L"above"; parse(tokens,2); assert(GlobalConfig.ToolsRowAbove);
    tokens[1]=L"invalid"; parse(tokens,2); assert(GlobalConfig.ToolsRowAbove);
    tokens[1]=L"BELOW"; parse(tokens,2); assert(!GlobalConfig.ToolsRowAbove);
    parse_line(WORD_BITS==32 ? L"row_tools_tune -2147483648" : L"row_tools_tune -9223372036854775808");
    assert(GlobalConfig.RowToolsTune==WORD_MIN);
    parse_line(WORD_BITS==32 ? L"row_tools_tune 2147483647" : L"row_tools_tune 9223372036854775807");
    assert(GlobalConfig.RowToolsTune==WORD_MAX);
    parse_line(L"   row_tools_position = above # theme override");
    parse_line(L"\trow_tools_tune  +16");
    parse_line(L"row_tools_tune");
    parse_line(L"row_tools_tune 100000 invalid");
    parse_line(L"row_tools_tune 1.5");
    assert(GlobalConfig.RowToolsTune==16 && GlobalConfig.ToolsRowAbove);
    parse_line(L"row_tools_tune \"-10\"");
    assert(GlobalConfig.RowToolsTune==-10);
}

static void input_cases(void) {
    Entry entries[40]; Entry *list[40];
    ScreenH=768; TileSizes[0]=144; TileSizes[1]=64; AllowGraphicsMode=TRUE;
    GlobalConfig.RowToolsTune=0; GlobalConfig.RowIconsMove=0; GlobalConfig.RowIconsTune=0;
    for (int width=1; width<=1024; width=width*2) {
        ScreenW=width;
        for (int loaders=0; loaders<=20; loaders+= loaders < 2 ? 1 : 6)
        for (int tools=0; tools<=10; tools+= tools < 2 ? 1 : 4)
        for (int above=0; above<=1; ++above) {
            if (!loaders && !tools) continue;
            GlobalConfig.ToolsRowAbove=above;
            for (int i=0; i<loaders+tools; ++i) {
                entries[i].Row=i>=loaders; list[i]=&entries[i];
            }
            REFIT_MENU_SCREEN screen={loaders+tools,list};
            SCROLL_STATE state={0}; state.ScrollMode=SCROLL_MODE_ICONS;
            InitScroll(&state,screen.EntryCount,0); GetStateInfo(&screen,&state); IdentifyRows(&state,&screen);
            assert(state.MaxVisible>=1 && IconRowItems==(UINTN)loaders);
            for (int i=0; i<loaders+tools; ++i) {
                SCROLL_STATE current=state; current.CurrentSelection=i;
                if (i<loaders) AdjustScrollState(&current);
                for (int key=0;key<=1;++key) {
                    SCROLL_STATE next=current; UpdateScroll(&next,key);
                    assert(next.CurrentSelection>=0 && next.CurrentSelection < loaders+tools);
                    if (!loaders) assert(next.CurrentSelection==i);
                    else if (!tools) {
                        // Upstream treats the last loader as InitialRow1 in a
                        // loader-only menu. Preserve its first/last shortcuts.
                        int expected = key==SCROLL_LINE_DOWN ? loaders-1 :
                            (i==loaders-1 ? current.FirstVisible : i);
                        assert(next.CurrentSelection==expected);
                    }
                    else {
                        BOOLEAN towardTools = (key == (above ? SCROLL_LINE_UP : SCROLL_LINE_DOWN));
                        assert((next.CurrentSelection >= loaders) == towardTools);
                    }
                }
                UINTN row=entries[i].Row, tile=TileSizes[row];
                UINTN x=row ? ToolRowPosX+(i-loaders)*(tile+8) : IconRowPosX+(i-current.FirstVisible)*(tile+8);
                UINTN y=row ? ToolRowPosY : IconRowPosY;
                if (x+tile<=ScreenW) {
                    assert(FindMainMenuItem(&screen,&current,x,y)==(UINTN)i);
                    assert(FindMainMenuItem(&screen,&current,x+tile-1,y+tile-1)==(UINTN)i);
                    assert(FindMainMenuItem(&screen,&current,x,y+tile)!=(UINTN)i);
                    assert(FindMainMenuItem(&screen,&current,x+tile,y)!=(UINTN)i);
                }
            }
        }
    }
}
int main(void) {
    assert(sizeof(CHAR16)==2);
    layout_cases(); parser_cases(); input_cases();
    assert(!egCreateImage((UINTN)-1, 2, FALSE));
    assert(!egCreateImage(0, 10, FALSE));
    assert(AddPixels((UINTN)-2, 6)==(UINTN)-1);
    unsigned pixels[]={1,2,3,4}; EG_IMAGE original={2,2,FALSE,pixels};
    assert(!egCropImage(&original,0,(UINTN)-1,1,2));
    assert(!egCropImage(&original,(UINTN)-1,0,2,1));
    EG_IMAGE *one=egCropImage(&original,1,1,1,1);
    assert(one && one->PixelData[0]==4); free(one->PixelData); free(one);
    printf("PASS: %d-bit EFI integers; 500000 randomized layouts, parser, crop, navigation and pointer cases\n", WORD_BITS);
}
