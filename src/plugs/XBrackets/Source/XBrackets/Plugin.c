#include "base.h"
#include "AutoBrackets.h"
#include "SettingsDlg.h"
#include "resource.h"
#include <richedit.h>
#include "c_base/types.h"
#include "Plugin.h"

//#define _mad_verification_
#undef _mad_verification_

#ifdef _mad_verification_
  TCHAR str[256];
  void ShowMessage(const TCHAR* cszMessage, const TCHAR* cszTitle = _T(""))
  {
    MessageBox(NULL, cszMessage, cszTitle, MB_OK);
  }
#endif


#define AKEL_DLL_VERSION    AKELDLL
#define EXE_MIN_VERSION_3X  MAKE_IDENTIFIER(-1, -1, -1, -1)
#define EXE_MIN_VERSION_4X  MAKE_IDENTIFIER(4, 8, 4, 0)
#define PLUGIN_NAME         "XBrackets"


/*
// const
const TCHAR* cszPluginName = _T("XBrackets Plugin");
const TCHAR* cszUnloadPluginQuestion = _T("Plugin is active. Unload it?");
*/

// Functions prototypes
static LRESULT CALLBACK NewMainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK NewEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK NewFrameProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void EditParentMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void updatePluginData(PLUGINDATA* pd);
static void ReadOptions();
static void SaveOptions();

#if use_aen_paint
void OnAenPaint(AENPAINT* p);
#endif

// Global variables
HWND         g_hMainWnd = NULL;
BOOL         g_bOldWindows = FALSE;
BOOL         g_bOldRichEdit = FALSE; // TRUE means Rich Edit 2.0
int          g_nMDI = 0;
BOOL         g_bMDI = FALSE;
BOOL         g_bAkelEdit = FALSE;
BOOL         g_bInitialized = FALSE;
WNDPROCDATA* pMainProcData = NULL;
WNDPROCDATA* pEditProcData = NULL;
WNDPROCDATA* pFrameProcData = NULL;
HWND         g_hFocusedEditWnd = NULL;
LANGID       g_LangSystem = 0;

UINT         uBracketsHighlight = BRHLF_ENABLED | BRHLF_TEXT;
BOOL         bBracketsAutoComplete = TRUE;
BOOL         bBracketsHighlightVisibleArea = FALSE;
BOOL         bBracketsRightExistsOK = FALSE;
BOOL         bBracketsDoSingleQuote = FALSE;
BOOL         bBracketsDoTag = FALSE;
BOOL         bBracketsDoTag2 = FALSE;
BOOL         bBracketsDoTagIf = FALSE;
BOOL         bBracketsSkipEscaped = FALSE;
BOOL         bBracketsSkipComment1 = FALSE;
BOOL         bGoToMatchingBracketTriggered = FALSE;
BOOL         bOpeningNewDocument = FALSE;
BOOL         bDocumentJustOpened = FALSE;
HWND         hDocumentJustOpenedWnd = NULL;
COLORREF     bracketsColourHighlight[2] = { RGB(0xD0, 0x40, 0x40), RGB(0xFF, 0xFF, 0xFF) };
// [0] - text colour, [1] - background
// I also like RGB(0x10, 0x70, 0xC0); // ~ dark blue
// I also like RGB(0xD0, 0x40, 0x40); // ~ red

COLORREF     g_CustomColoursHighlight[MAX_CUSTOM_COLOURS] = { RGB(0xD0,0x40,0x40), RGB(0x10,0x70,0xC0), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
COLORREF     g_CustomColoursHighlight_0[MAX_CUSTOM_COLOURS] = { 0 };

char         strHtmlFileExtsA[STR_FILEEXTS_SIZE] = "htm; xml; php\0";
wchar_t      strHtmlFileExtsW[STR_FILEEXTS_SIZE] = L"htm; xml; php\0";
wchar_t      strHtmlFileExtsW_0[STR_FILEEXTS_SIZE] = { 0 };
char         strComment1FileExtsA[STR_FILEEXTS_SIZE] = "cs; java; js; php\0";
wchar_t      strComment1FileExtsW[STR_FILEEXTS_SIZE] = L"cs; java; js; php\0";
wchar_t      strComment1FileExtsW_0[STR_FILEEXTS_SIZE] = { 0 };
wchar_t      strPluginFuncMainW[STR_PLUGINFUNC_SIZE] = { 0 };

#define      OPT_UNDEFINED_DWORD      0xFEDCBA98
#define      OPTF_AUTOCOMPLETE        0x0001
#define      OPTF_HIGHLIGHT_ENABLED   0x0002
#define      OPTF_VISIBLEAREA         0x0004
#define      OPTF_RIGHTBRACKETOK      0x0008
#define      OPTF_HIGHLIGHT_TEXT      0x0010
#define      OPTF_HIGHLIGHT_BKGND     0x0020
#define      OPTF_DOSINGLEQUOTE       0x0100
#define      OPTF_DOTAG               0x0200
#define      OPTF_DOTAGIF             0x0400
#define      OPTF_DOTAG2              0x0800
#define      OPTF_SKIPESCAPED         0x1000
#define      OPTF_SKIPCOMMENT1        0x2000

DWORD        opt_dwOptionsFlags0 = OPT_UNDEFINED_DWORD;
DWORD        opt_dwHighlightRGB0[2] = { OPT_UNDEFINED_DWORD, OPT_UNDEFINED_DWORD };

DWORD        g_dwOptions[OPT_DWORD_COUNT] = { 0 };
DWORD        g_dwOptions0[OPT_DWORD_COUNT] = { 0 };

wchar_t      opt_szUserBracketsW[MAX_USER_BRACKETS*4] = { 0x00AB, 0x00BB, L' ', 0x2018, 0x2019, L' ', 0x201C, 0x201D, 0 };
char         opt_szUserBracketsA[MAX_USER_BRACKETS*4] = { 0xAB, 0xBB, ' ', 0x91, 0x92, ' ', 0x93, 0x94, 0 };
wchar_t      opt_szUserBracketsW_0[MAX_USER_BRACKETS*4] = { 0 };
wchar_t      opt_szNextCharOkW[MAX_PREV_NEXT_CHAR_OK_SIZE] = L".,!?:;<)]}\"\'>/ \x00BB\x2019\x201D\0";
char         opt_szNextCharOkA[MAX_PREV_NEXT_CHAR_OK_SIZE] = ".,!?:;<)]}\"\'>/ \xBB\x92\x94\0";
wchar_t      opt_szNextCharOkW_0[MAX_PREV_NEXT_CHAR_OK_SIZE] = { 0 };
wchar_t      opt_szPrevCharOkW[MAX_PREV_NEXT_CHAR_OK_SIZE] = L"([{<= \x00AB\x2018\x201C\0";
char         opt_szPrevCharOkA[MAX_PREV_NEXT_CHAR_OK_SIZE] = "([{<= \xAB\x91\x93\0";
wchar_t      opt_szPrevCharOkW_0[MAX_PREV_NEXT_CHAR_OK_SIZE] = { 0 };

extern INT_X   CurrentBracketsIndexes[2];
extern HWND    hCurrentEditWnd;
extern HWND    hActualEditWnd;

#if use_aen_paint
extern UINT         nAenPaintWanted;
extern TEXTMETRICW  AenPaint_tmW;
extern TEXTMETRICA  AenPaint_tmA;
#endif

static const char* cszOptNamesA[OPT_TOTAL_COUNT] = {
  // dword options
  "autocomplete.all_autobr",
  "autocomplete.ovr_autobr",
  "autocomplete.sel_autobr",
  "highlight.hlt_bothbr",
  "highlight.hlt_style",
  "highlight.quote_detect_lines",
  "highlight.quote_max_lines",
  "highlight.br_max_lines",
  // other options
  "common.user_brpairs",
  "autobrackets.next_char_ok",
  "autobrackets.prev_char_ok",
  "OptionsFlags",
  "HighlightRGB",
  "HighlightBkRGB",
  "CustomRGB",
  "HtmlFileExts",
  "Comment1FileExts",
  "XBrackets"
};

static const wchar_t* cszOptNamesW[OPT_TOTAL_COUNT] = {
  // dword options
  L"autocomplete.all_autobr",
  L"autocomplete.ovr_autobr",
  L"autocomplete.sel_autobr",
  L"highlight.hlt_bothbr",
  L"highlight.hlt_style",
  L"highlight.quote_detect_lines",
  L"highlight.quote_max_lines",
  L"highlight.br_max_lines",
  // other options
  L"common.user_brpairs",
  L"autobrackets.next_char_ok",
  L"autobrackets.prev_char_ok",
  L"OptionsFlags",
  L"HighlightRGB",
  L"HighlightBkRGB",
  L"CustomRGB",
  L"HtmlFileExts",
  L"Comment1FileExts",
  L"XBrackets"
};

int IsBracketsHighlight(UINT uBrHglt)
{
  if (uBrHglt & BRHLF_ENABLED)
  {
    if ((uBrHglt & BRHLF_TEXT) || (uBrHglt & BRHLF_BKGND))
      return 1;
  }
  return 0;
}

static LANGID getAkelPadLang(PLUGINDATA* pd)
{
    /*if ( pd->bOldWindows )
    {
        if ( pd->szLangModule && pd->szLangModule[0] )
        {
            char szModule[5];
            
            lstrcpynA( szModule, pd->szLangModule, 4 );
            CharUpperA( szModule );
            if ( lstrcmpA(szModule, "ENG") == 0 )
                return LANG_ENGLISH;
            if ( lstrcmpA(szModule, "RUS") == 0 )
                return LANG_RUSSIAN;
            if ( lstrcmpA(szModule, "UKR") == 0 )
                return LANG_UKRAINIAN;
        }
    }
    else
    {
        if ( pd->wszLangModule && pd->wszLangModule[0] )
        {
            wchar_t wszModule[5];

            lstrcpynW( wszModule, pd->wszLangModule, 4 );
            CharUpperW( wszModule );
            if ( lstrcmpW(wszModule, L"ENG") == 0 )
                return LANG_ENGLISH;
            if ( lstrcmpW(wszModule, L"RUS") == 0 )
                return LANG_RUSSIAN;
            if ( lstrcmpW(wszModule, L"UKR") == 0 )
                return LANG_UKRAINIAN;
        }
    }*/

    return (pd->wLangModule ? pd->wLangModule : pd->wLangSystem);
}

// from "StrFunc.h"
static int xitoaA(INT_PTR nNumber, char *szStr)
{
  char szReverse[128];
  int a;
  int b=0;

  if (nNumber == 0)
  {
    if (szStr) szStr[b]='0';
    ++b;
  }
  else if (nNumber < 0)
  {
    if (szStr) szStr[b]='-';
    ++b;
    nNumber=0 - nNumber;
  }
  for (a=0; nNumber != 0; ++a)
  {
    szReverse[a]=(char)(nNumber % 10) + '0';
    nNumber=nNumber / 10;
  }
  if (!szStr) return a + b + 1;

  while (--a >= 0) szStr[b++]=szReverse[a];
  szStr[b]='\0';
  return b;
}

// from "StrFunc.h"
static int xitoaW(INT_PTR nNumber, wchar_t *wszStr)
{
  wchar_t wszReverse[128];
  int a;
  int b=0;

  if (nNumber == 0)
  {
    if (wszStr) wszStr[b]=L'0';
    ++b;
  }
  else if (nNumber < 0)
  {
    if (wszStr) wszStr[b]=L'-';
    ++b;
    nNumber=0 - nNumber;
  }
  for (a=0; nNumber != 0; ++a)
  {
    wszReverse[a]=(wchar_t)(nNumber % 10) + L'0';
    nNumber=nNumber / 10;
  }
  if (!wszStr) return a + b + 1;

  while (--a >= 0) wszStr[b++]=wszReverse[a];
  wszStr[b]=L'\0';
  return b;
}

// Identification
/* extern "C" */
void __declspec(dllexport) DllAkelPadID(PLUGINVERSION *pv)
{
  pv->dwAkelDllVersion  = AKEL_DLL_VERSION;
  pv->dwExeMinVersion3x = EXE_MIN_VERSION_3X;
  pv->dwExeMinVersion4x = EXE_MIN_VERSION_4X;
  pv->pPluginName = PLUGIN_NAME;
}

// Plugin extern function
/*extern "C"*/
void __declspec(dllexport) Main(PLUGINDATA *pd)
{
  pd->dwSupport |= PDS_SUPPORTALL;
  if ( pd->dwSupport & PDS_GETSUPPORT )
    return;

  updatePluginData(pd);

  if ( pd->bOldWindows )
    lstrcpyA( (LPSTR) strPluginFuncMainW, (LPCSTR) pd->pFunction );
  else
    lstrcpyW( strPluginFuncMainW, (LPCWSTR) pd->pFunction );

  // Is plugin already loaded?
  if (g_bInitialized)
  {
    /*
    if ( MessageBox(g_hMainWnd, cszUnloadPluginQuestion,
           cszPluginName, MB_YESNO | MB_ICONQUESTION) == IDYES )
    */
    {
      Uninitialize(FALSE);
      return;
    }
  }
  else
  {
    Initialize(pd);
  }

  // Stay in memory, and show as active
  pd->nUnload = UD_NONUNLOAD_ACTIVE;
}

/*extern "C"*/
void __declspec(dllexport) Settings(PLUGINDATA *pd)
{
  BOOL     prevInitialized;
  COLORREF prevColourHighlight[2];
  UINT     prevBracketsHighlight;
  BOOL     prevBracketsDoTag;
  BOOL     prevBracketsDoTagIf;
  BOOL     prevBracketsDoSingleQuote;
  BOOL     prevBracketsSkipEscaped;
  BOOL     prevBracketsSkipComment1;
  BOOL     bUpdateBracketsHighlight;
  INT_PTR  nRet;
    
  pd->dwSupport |= PDS_NOAUTOLOAD;
  if ( pd->dwSupport & PDS_GETSUPPORT )
    return;

  updatePluginData(pd);

  #ifdef _mad_verification_
    ShowMessage("Entering Settings...", "void Settings()");
  #endif

  prevInitialized = g_bInitialized;
  if (!g_bInitialized)
  {

    if ( pd->bOldWindows )
    {
      if ( !((LPCSTR) strPluginFuncMainW)[0] )
      {
        int i;

        lstrcpyA( (LPSTR) strPluginFuncMainW, (LPCSTR) pd->pFunction );
        for (i = 0; ((LPCSTR) strPluginFuncMainW)[i] != 0; i++)
        {
          if ( ((LPCSTR) strPluginFuncMainW)[i] == ':' )
          {
            i += 2;
            lstrcpyA( &((LPSTR) strPluginFuncMainW)[i], "Main" );
            break;
          }
        }
      }
    }
    else
    {
      if ( !strPluginFuncMainW[0] )
      {
        int i;
          
        lstrcpyW( strPluginFuncMainW, (LPCWSTR) pd->pFunction );
        for (i = 0; strPluginFuncMainW[i] != 0; i++)
        {
          if (strPluginFuncMainW[i] == L':')
          {
            i += 2;
            lstrcpyW( &strPluginFuncMainW[i], L"Main" );
            break;
          }
        }
      }
    }

    ReadOptions();
  }

  prevColourHighlight[0] = bracketsColourHighlight[0];
  prevColourHighlight[1] = bracketsColourHighlight[1];
  prevBracketsHighlight = uBracketsHighlight;
  prevBracketsDoTag = bBracketsDoTag;
  prevBracketsDoTagIf = bBracketsDoTagIf;
  prevBracketsDoSingleQuote = bBracketsDoSingleQuote;
  prevBracketsSkipEscaped = bBracketsSkipEscaped;
  prevBracketsSkipComment1 = bBracketsSkipComment1;
  bUpdateBracketsHighlight = FALSE;

  if (g_bOldWindows)
  {
    nRet = DialogBoxA(pd->hInstanceDLL, MAKEINTRESOURCEA(IDD_SETTINGS), 
      pd->hMainWnd, SettingsDlgProc);
  }
  else
  {
    nRet = DialogBoxW(pd->hInstanceDLL, MAKEINTRESOURCEW(IDD_SETTINGS), 
      pd->hMainWnd, SettingsDlgProc);
  }

  if (nRet /*&& !g_bInitialized*/)
  {
    SaveOptions();
  }

  if ((prevBracketsDoTag != bBracketsDoTag) ||
      (prevBracketsDoTagIf != bBracketsDoTagIf) ||
      (prevBracketsDoSingleQuote != bBracketsDoSingleQuote) ||
      (prevBracketsSkipEscaped != bBracketsSkipEscaped) ||
      (prevBracketsSkipComment1 != bBracketsSkipComment1))
  {
    bUpdateBracketsHighlight = TRUE;
  }

  if ((IsBracketsHighlight(prevBracketsHighlight) || bGoToMatchingBracketTriggered) && 
      (bUpdateBracketsHighlight || (prevBracketsHighlight != uBracketsHighlight)))
  {
    // 1. brackets were highlighted, now they are not
    // 2. uBracketsHighlight changed: BRHLF_TEXT or BRHLF_BKGND 
    RemoveAllHighlightInfo(TRUE);
    bGoToMatchingBracketTriggered = FALSE;
  }

  if (g_bInitialized)
  {
    if (IsBracketsHighlight(uBracketsHighlight))
    {
      bGoToMatchingBracketTriggered = FALSE;
      if ((prevBracketsHighlight != uBracketsHighlight) || 
          (!prevInitialized) || bUpdateBracketsHighlight)
      {
        MSGINFO msgi = { pd->hWndEdit, WM_PAINT, 0, 0 };

        OnEditGetActiveBrackets(&msgi, TRUE);
      }
      else if (/*(prevBracketsHighlight != uBracketsHighlight) ||*/
               (prevColourHighlight[0] != bracketsColourHighlight[0]) ||
               (prevColourHighlight[1] != bracketsColourHighlight[1]))
      {
        // brackets highlight colour has been changed
        OnEditHighlightActiveBrackets();
      }
    }
  }

  #ifdef _mad_verification_
    DWORD dwError = GetLastError();
    wsprintf(str,
      _T("DialogBox(0x%08X, ... IDD_SETTINGS ...) returned %ld\n" \
         "GetLastError() returned %lu"),
      pd->hInstanceDLL, nRet, dwError);
    ShowMessage(str, "void Settings()");
  #endif

  // Stay in memory, and show as non-active
  pd->nUnload = UD_NONUNLOAD_NONACTIVE;
}

enum eMatchingBracketAction {
  MBRA_GOTO = 1,
  MBRA_SELTO
};

static void DoMatchingBracketAction(const PLUGINDATA *pd, int action)
{
  if (g_bInitialized)
  {
    if ( (!IsBracketsHighlight(uBracketsHighlight)) || 
         (bBracketsHighlightVisibleArea && (CurrentBracketsIndexes[0] < 0)) )
    {
      BOOL    prevBracketsHighlightVisibleArea;
      MSGINFO msgi;
        
      // saving previous value
      prevBracketsHighlightVisibleArea = bBracketsHighlightVisibleArea;
      bBracketsHighlightVisibleArea = FALSE;

      bGoToMatchingBracketTriggered = TRUE;
      msgi.hWnd = pd->hWndEdit;
      msgi.uMsg = WM_PAINT;
      msgi.wParam = 0;
      msgi.lParam = 0;
      OnEditGetActiveBrackets(&msgi, FALSE);

      // restoring previous value
      bBracketsHighlightVisibleArea = prevBracketsHighlightVisibleArea;
    }
      
    if (CurrentBracketsIndexes[0] >= 0)
    {
      CHARRANGE_X cr = { -1, -1 };

      SendMessage(pd->hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr);
      if (cr.cpMin == cr.cpMax)
      {
        INT_X pos = -1;

        if (cr.cpMin == CurrentBracketsIndexes[0] + 1) // after left bracket
          pos = CurrentBracketsIndexes[1];
        else if (cr.cpMin == CurrentBracketsIndexes[1]) // before right bracket
          pos = CurrentBracketsIndexes[0] + 1;
        else if (cr.cpMin == CurrentBracketsIndexes[0]) // before left bracket
          pos = CurrentBracketsIndexes[1] + 1;
        else if (cr.cpMin == CurrentBracketsIndexes[1] + 1) // after right bracket
          pos = CurrentBracketsIndexes[0];

        if (pos >= 0)
        {
          if ( action == MBRA_GOTO )
          {
            cr.cpMin = pos;
            cr.cpMax = pos;
          }
          else if ( action == MBRA_SELTO )
          {
            /*if ( pos < cr.cpMin )
              cr.cpMin = pos;
            else*/
              cr.cpMax = pos;
          }
          SendMessage(pd->hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr);
        }
      }
    }
  }
}

static BOOL IsExtCallParamValid(LPARAM lParam, int nIndex)
{
  if ( *((INT_PTR *)lParam) >= (INT_PTR)((nIndex + 1) * sizeof(INT_PTR)) )
    return TRUE;
  return FALSE;
}

static INT_PTR GetExtCallParam(LPARAM lParam, int nIndex)
{
  if ( *((INT_PTR *)lParam) >= (INT_PTR)((nIndex + 1) * sizeof(INT_PTR)) )
    return *(((INT_PTR *)lParam) + nIndex);
  return 0;
}

static void funcGetCurrentBrackets(PLUGINDATA *pd)
{
  if ( IsExtCallParamValid(pd->lParam, 2) )
  {
    if ( pd->bOldWindows )
    {
      char* pszBuf = (char *) GetExtCallParam(pd->lParam, 2);
      if ( pszBuf )
      {
        int len;

        len = 0;
        len += xitoaA(CurrentBracketsIndexes[0], pszBuf);
        pszBuf[len] = ' ';
        ++len;
        len += xitoaA(CurrentBracketsIndexes[1], pszBuf + len);
        pszBuf[len] = ' ';
        ++len;
        lstrcpyA(pszBuf + len, getCurrentBracketsPairA());
      }
    }
    else
    {
      wchar_t* pszBuf = (wchar_t *) GetExtCallParam(pd->lParam, 2);
      if ( pszBuf )
      {
        int len;

        len = 0;
        len += xitoaW(CurrentBracketsIndexes[0], pszBuf);
        pszBuf[len] = L' ';
        ++len;
        len += xitoaW(CurrentBracketsIndexes[1], pszBuf + len);
        pszBuf[len] = L' ';
        ++len;
        lstrcpyW(pszBuf + len, getCurrentBracketsPairW());
      }
    }
  }
}

/*extern "C"*/
void __declspec(dllexport) GoToMatchingBracket(PLUGINDATA *pd)
{
  pd->dwSupport |= PDS_NOAUTOLOAD;
  if ( pd->dwSupport & PDS_GETSUPPORT )
    return;

  DoMatchingBracketAction(pd, MBRA_GOTO);

  pd->nUnload = UD_NONUNLOAD_NONACTIVE;
}

/*extern "C"*/
void __declspec(dllexport) SelToMatchingBracket(PLUGINDATA *pd)
{
  int isDefaultAction;

  pd->dwSupport |= PDS_NOAUTOLOAD;
  if ( pd->dwSupport & PDS_GETSUPPORT )
    return;

  isDefaultAction = 1;
  if ( pd->lParam )
  {
    if ( IsExtCallParamValid(pd->lParam, 1) )
    {
      switch ( GetExtCallParam(pd->lParam, 1) )
      {
        case 0:
          break;

        case 1:
          funcGetCurrentBrackets(pd);
          isDefaultAction = 0;
          break;

        default:
          isDefaultAction = 0;
          break;
      }
    }
  }

  if ( isDefaultAction )
    DoMatchingBracketAction(pd, MBRA_SELTO);

  pd->nUnload = UD_NONUNLOAD_NONACTIVE;
}

LRESULT CALLBACK NewEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  MSGINFO msgi;
  BOOL    bTextChangedLocal = FALSE;

  if ((uMsg == AEM_ADDCLONE) || (uMsg == AEM_DELCLONE))
  {
    LRESULT  lResult = 0;
      
    if (pEditProcData && pEditProcData->NextProc)
      lResult = pEditProcData->NextProc(hWnd, uMsg, wParam, lParam);

    bOpeningNewDocument = FALSE;

    return lResult;
  }
  
  if (uMsg == WM_SETFOCUS)
  {
    EDITINFO ei;
    LRESULT  lResult = 0;
      
    if (pEditProcData && pEditProcData->NextProc)
      lResult = pEditProcData->NextProc(hWnd, uMsg, wParam, lParam);

    ei.hWndEdit = NULL;
    SendMessage(g_hMainWnd, AKD_GETEDITINFO, (WPARAM) NULL, (LPARAM) &ei);

    if (bOpeningNewDocument || (ei.hWndEdit != g_hFocusedEditWnd))
    {
      // Sometimes there is no other way to clear this flag
      bOpeningNewDocument = FALSE;
        
      if (IsBracketsHighlight(uBracketsHighlight) || bGoToMatchingBracketTriggered)
      {
        // RemoveAllHighlightInfo with bRepaint=FALSE fixes strange bug 
        // under MustDie 9x when several MDI windows are repainted
        RemoveAllHighlightInfo(FALSE); 
        bGoToMatchingBracketTriggered = FALSE;
        
        if (IsBracketsHighlight(uBracketsHighlight))
        {
          if (ei.hWndEdit == hWnd)
          {
            MSGINFO msgi;

            msgi.hWnd = hWnd;
            msgi.uMsg = WM_SETFOCUS;
            msgi.wParam = 0;
            msgi.lParam = 0;

            OnEditGetActiveBrackets(&msgi, TRUE);
          }
        }
      }
      g_hFocusedEditWnd = ei.hWndEdit;
    }
    else
    {
      // Sometimes there is no other way to clear this flag
      bOpeningNewDocument = FALSE;
    }

    return lResult;
  }

  if (!g_bAkelEdit)
  {
    // Rich Edit control
    if (IsBracketsHighlight(uBracketsHighlight) || bGoToMatchingBracketTriggered)
    {
      if ( (uMsg == WM_CHAR && (/*wParam != VK_DELETE &&*/ wParam != VK_BACK)) || 
           (uMsg == WM_KEYDOWN && (wParam == VK_DELETE || wParam == VK_BACK)) ||
           (uMsg == EM_REPLACESEL) ||
           (uMsg == EM_STREAMIN) ||
           (uMsg == EM_PASTESPECIAL) ||
           (uMsg == WM_PASTE) ||
           (uMsg == WM_CUT) ||
           (uMsg == WM_CLEAR) ||
           (uMsg == EM_UNDO) ||
           (uMsg == EM_REDO)
         )
      {
        // before the text is changed: removing the highlighting
        RemoveAllHighlightInfo(TRUE);
        bGoToMatchingBracketTriggered = FALSE;
        bTextChangedLocal = TRUE;
      }
    }
  }
  
  if (bBracketsAutoComplete)
  {
    if (uMsg == WM_CHAR)
    {
      // local operations because only WM_CHAR is processed
      msgi.hWnd   = hWnd;
      msgi.uMsg   = uMsg;
      msgi.wParam = wParam;
      msgi.lParam = lParam;

      OnEditCharPressed(&msgi);

      // local operations because only WM_CHAR is processed
      //uMsg   = msgi.uMsg; // OnEditCharPressed() does not modify uMsg
      wParam = msgi.wParam;
      lParam = msgi.lParam;
    }
  }

  if (IsBracketsHighlight(uBracketsHighlight))
  {
    if (!g_bAkelEdit)
    {
      // Rich Edit control
      if (bTextChangedLocal)
      {
        LRESULT lResult = 0;

        if (pEditProcData && pEditProcData->NextProc)
          lResult = pEditProcData->NextProc(hWnd, uMsg, wParam, lParam);

        // after the text is changed: updating the highlighting
        msgi.hWnd   = hWnd;
        msgi.uMsg   = uMsg;
        msgi.wParam = wParam;
        msgi.lParam = lParam;

        OnEditGetActiveBrackets(&msgi, TRUE);

        return lResult;
      }
    }
      
    if ((uMsg == WM_PAINT) /* || (uMsg == WM_DISPLAYCHANGE) */ )
    {
      LRESULT lResult = 0;

      if (pEditProcData && pEditProcData->NextProc)
        lResult = pEditProcData->NextProc(hWnd, uMsg, wParam, lParam);

      if (bDocumentJustOpened && (hWnd == hDocumentJustOpenedWnd))
      {
        MSGINFO msgi = { hWnd, WM_PAINT, 0, 0 };
        OnEditGetActiveBrackets(&msgi, TRUE);
        bDocumentJustOpened = FALSE;
        hDocumentJustOpenedWnd = NULL;
      }
      else if (hWnd == g_hFocusedEditWnd)
      {
        OnEditHighlightActiveBrackets();
      }

      return lResult;
    }

  }

  if (pEditProcData && pEditProcData->NextProc)
    return pEditProcData->NextProc(hWnd, uMsg, wParam, lParam);
  else
    return 0;
}

LRESULT CALLBACK NewMainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  /*
  if (uMsg == AKDN_MAIN_ONSTART_FINISH)
  {
    ReadOptions();
  }
  else
  */
  if (uMsg == AKDN_MAIN_ONFINISH)
  {
    LRESULT lResult = 0;

    if (pMainProcData && pMainProcData->NextProc)
      lResult = pMainProcData->NextProc(hWnd, uMsg, wParam, lParam);

    SaveOptions();
    Uninitialize(TRUE);

    return lResult;
  }

  if (IsBracketsHighlight(uBracketsHighlight) || bGoToMatchingBracketTriggered)
  {
    if ((uMsg == AKDN_OPENDOCUMENT_START) ||
        (uMsg == AKDN_EDIT_ONSTART) ||
        (uMsg == WM_COMMAND && LOWORD(wParam) == IDM_FILE_NEW))
    {
      // AKDN_EDIT_ONSTART - required to set bOpeningNewDocument before EN_SELCHANGE
      // WM_COMMAND, IDM_FILE_NEW - required for Sessions plugin
      bOpeningNewDocument = TRUE;
    }
    else if (uMsg == AKDN_OPENDOCUMENT_FINISH)
    {
      LRESULT lResult = 0;

      if (pMainProcData && pMainProcData->NextProc)
        lResult = pMainProcData->NextProc(hWnd, uMsg, wParam, lParam);

      bOpeningNewDocument = FALSE;
      bDocumentJustOpened = TRUE;
      hDocumentJustOpenedWnd = (HWND) wParam;
      /*if (IsBracketsHighlight(uBracketsHighlight))
      {
        MSGINFO msgi = { (HWND) wParam, WM_PAINT, 0, 0 };
        OnEditGetActiveBrackets(&msgi, TRUE);
      }*/

      return lResult;
    }
    else
      EditParentMessages(hWnd, uMsg, wParam, lParam);
  }

#if use_aen_paint
  if (uMsg == AKDN_EDIT_ONSTART)
  {
    HWND hEditWnd;
    UINT uMask;
    LRESULT lResult = 0;

    if (pMainProcData && pMainProcData->NextProc)
      lResult = pMainProcData->NextProc(hWnd, uMsg, wParam, lParam);

    hEditWnd = (HWND) wParam;
    uMask = (UINT) SendMessage(hEditWnd, AEM_GETEVENTMASK, 0, 0);
    if (!(uMask & AENM_PAINT))
    {
      uMask |= AENM_PAINT;
      SendMessage(hEditWnd, AEM_SETEVENTMASK, 0, uMask);
    }

    return lResult;
  }
#endif

  if (pMainProcData && pMainProcData->NextProc)
    return pMainProcData->NextProc(hWnd, uMsg, wParam, lParam);
  else
    return 0;
}

LRESULT CALLBACK NewFrameProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (IsBracketsHighlight(uBracketsHighlight) || bGoToMatchingBracketTriggered)
  {
    /*
    if (uMsg == WM_MDIACTIVATE)
    {
      LRESULT lResult = 0;

      if (pFrameProcData && pFrameProcData->NextProc)
        lResult = pFrameProcData->NextProc(hWnd, uMsg, wParam, lParam);

      OnActiveDocumentChanged( GetFocus() );

      return lResult;
    }
    */
  
    EditParentMessages(hWnd, uMsg, wParam, lParam);
  }

  if (pFrameProcData && pFrameProcData->NextProc)
    return pFrameProcData->NextProc(hWnd, uMsg, wParam, lParam);
  else
    return 0;
}

void EditParentMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_NOTIFY)
  {
    if (wParam == ID_EDIT)
    {
      if (((NMHDR *)lParam)->code == EN_SELCHANGE)
      {
        if (g_hFocusedEditWnd == ((NMHDR *)lParam)->hwndFrom)
        {
          MSGINFO msgi;

          msgi.hWnd = ((NMHDR *)lParam)->hwndFrom;
          msgi.uMsg = WM_PAINT;
          msgi.wParam = 0;
          msgi.lParam = 0;
          
          if (IsBracketsHighlight(uBracketsHighlight) && !bOpeningNewDocument)
          {
            OnEditGetActiveBrackets(&msgi, TRUE);
          }
        }
      }
      else if (g_bAkelEdit)
      {
        // AkelEdit control
        switch (((NMHDR *)lParam)->code)
        {
          case AEN_TEXTCHANGING:
          {
            // before the text is changed: removing the highlighting
            RemoveAllHighlightInfo(TRUE);
            bGoToMatchingBracketTriggered = FALSE;
            break;
          }
          case AEN_TEXTCHANGED:
          {
            // after the text is changed: updating the highlighting
            if (IsBracketsHighlight(uBracketsHighlight))
            {
              MSGINFO msgi;

              msgi.hWnd = ((NMHDR *)lParam)->hwndFrom;
              msgi.uMsg = WM_PAINT;
              msgi.wParam = 0;
              msgi.lParam = 0;

              OnEditGetActiveBrackets(&msgi, TRUE);
            }
            break;
          }
#if use_aen_paint
          case AEN_PAINT:
          {
            if (nAenPaintWanted & 0x01)  // 0x01 - process AEN_PAINT
            {
              OnAenPaint( (AENPAINT *) lParam );
            }
            break;
          }
#endif
        }
      }
    }
  }
  else if (uMsg == AKDN_FRAME_ACTIVATE)
  {
    if ((g_nMDI == WMD_PMDI) && !(wParam & FWA_NOVISUPDATE))
    {
      if (IsBracketsHighlight(uBracketsHighlight))
      {
        MSGINFO    msgi;
        FRAMEDATA* lpFrame;

        lpFrame = (FRAMEDATA *) lParam;

        // PMDI mode
        g_hFocusedEditWnd = lpFrame->ei.hWndEdit;
        hCurrentEditWnd = NULL;
        hActualEditWnd = NULL;
        RemoveAllHighlightInfo(FALSE);

        msgi.hWnd = lpFrame->ei.hWndEdit;
        msgi.uMsg = WM_PAINT;
        msgi.wParam = 0;
        msgi.lParam = 0;

        OnEditGetActiveBrackets(&msgi, TRUE);

        // PMDI mode
        bOpeningNewDocument = FALSE;
      }
    }
  }
  //return 0;
}

#if use_aen_paint
void OnAenPaint(AENPAINT* p)
{
  if (p->dwType == AEPNT_DRAWLINE)
  {
    if (g_bOldWindows)
    {
      if (GetTextMetricsA(p->hDC, &AenPaint_tmA))
        nAenPaintWanted |= 0x02;  // 0x02 - AEN_PAINT processed successfully
    }
    else
    {
      if (GetTextMetricsW(p->hDC, &AenPaint_tmW))
        nAenPaintWanted |= 0x02;  // 0x02 - AEN_PAINT processed successfully
    }
  }
}
#endif

void updatePluginData(PLUGINDATA* pd)
{
  g_hMainWnd = pd->hMainWnd;
  g_nMDI = pd->nMDI;
  g_bMDI = (pd->nMDI == WMD_MDI);
  g_bOldWindows = pd->bOldWindows;
  g_bOldRichEdit = pd->bOldRichEdit;
  g_bAkelEdit = pd->bAkelEdit;
  g_LangSystem = getAkelPadLang(pd);
}

void Initialize(PLUGINDATA* pd)
{
  #ifdef _mad_verification_
    ShowMessage("Entering Initialize...", "void Initialize()");
  #endif

  #ifdef _mad_verification_
    wsprintf(str,
      _T("hMainWnd = 0x%08X\n" \
         "bMDI = %lu\n" \
         "bOldWindows = %lu\n" \
         "bOldRichEdit = %lu"),
      g_hMainWnd, g_bMDI, g_bOldWindows, g_bOldRichEdit);
    ShowMessage(str, "void Initialize()");
  #endif

  if (!g_bInitialized)
  {
    ReadOptions();

    // SubClass main window
    pMainProcData = NULL;
    SendMessage( g_hMainWnd, AKD_SETMAINPROC,
      (WPARAM) NewMainProc, (LPARAM) &pMainProcData );

    // SubClass edit window
    pEditProcData = NULL;
    SendMessage( g_hMainWnd, AKD_SETEDITPROC,
      (WPARAM) NewEditProc, (LPARAM) &pEditProcData );

    if (g_bMDI)
    {
      // SubClass frame window
      pFrameProcData = NULL;
      SendMessage( g_hMainWnd, AKD_SETFRAMEPROC,
        (WPARAM) NewFrameProc, (LPARAM) &pFrameProcData );
    }

    g_hFocusedEditWnd = pd->hWndEdit;

    if (IsBracketsHighlight(uBracketsHighlight))
    {
      MSGINFO msgi = { pd->hWndEdit, WM_PAINT, 0, 0 };

      OnEditGetActiveBrackets(&msgi, TRUE);
    }

    #ifdef _mad_verification_
      wsprintf(str,
        _T("bInitialized = FALSE\n" \
           "pMainProcData = 0x%08X\n" \
           "pEditProcData = 0x%08X"),
        pMainProcData, pEditProcData);
      ShowMessage(str, "void Initialize()");
      } else {
      ShowMessage("bInitialized = TRUE", "void Initialize()");
    #endif
  }

  g_bInitialized = TRUE;
}

void Uninitialize(BOOL bIsExiting )
{
  if (g_bInitialized)
  {
    g_bInitialized = FALSE;

    g_hFocusedEditWnd = NULL;

    if (!bIsExiting)
    {
      if (IsBracketsHighlight(uBracketsHighlight) || bGoToMatchingBracketTriggered)
      {
        RemoveAllHighlightInfo(TRUE);
        bGoToMatchingBracketTriggered = FALSE;
      }
    }

    if (pMainProcData)
    {
      // Remove subclass (main window)
      SendMessage( g_hMainWnd, AKD_SETMAINPROC,
        (WPARAM) NULL, (LPARAM) &pMainProcData );
      pMainProcData = NULL;
    }

    if (pEditProcData)
    {
      // Remove subclass (edit window)
      SendMessage( g_hMainWnd, AKD_SETEDITPROC,
        (WPARAM) NULL, (LPARAM) &pEditProcData );
      pEditProcData = NULL;
    }

    if (g_bMDI && pFrameProcData)
    {
      // Remove subclass (frame window)
      SendMessage( g_hMainWnd, AKD_SETFRAMEPROC,
        (WPARAM) NULL, (LPARAM) &pFrameProcData );
      pFrameProcData = NULL;
    }

    if (!bIsExiting)
    {
      SaveOptions();

      AutoBrackets_Uninitialize();
    }
  }
}

static const char* getOldOptionNameA(const char* pszNameA)
{
  while ( (*pszNameA) && (*pszNameA != '.') )
  {
    ++pszNameA;
  }
  if (*pszNameA == '.')
    ++pszNameA;
  return pszNameA;
}

static const wchar_t* getOldOptionNameW(const wchar_t* pszNameW)
{
  while ( (*pszNameW) && (*pszNameW != L'.') )
  {
    ++pszNameW;
  }
  if (*pszNameW == L'.')
    ++pszNameW;
  return pszNameW;
}

static BOOL readOptionA(HANDLE hOptions, const char* pszOptionNameA, 
                 void* pData, DWORD dwDataSize, DWORD dwOptionType)
{
  DWORD dwSize;
  PLUGINOPTIONA poA;

  poA.pOptionName = pszOptionNameA;
  poA.lpData = (BYTE*) pData;
  poA.dwData = dwDataSize;
  poA.dwType = dwOptionType;
  dwSize = (DWORD) SendMessageA(g_hMainWnd, AKD_OPTIONA, (WPARAM) hOptions, (LPARAM) &poA);
  if (dwSize == 0)
  {
    pszOptionNameA = getOldOptionNameA(pszOptionNameA);
    if (pszOptionNameA[0])
    {
      poA.pOptionName = pszOptionNameA;
      poA.lpData = (BYTE*) pData;
      poA.dwData = dwDataSize;
      poA.dwType = dwOptionType;
      dwSize = (DWORD) SendMessageA(g_hMainWnd, AKD_OPTIONA, (WPARAM) hOptions, (LPARAM) &poA);
    }
  }
  return (dwSize > 0);
}

static BOOL readOptionW(HANDLE hOptions, const wchar_t* pszOptionNameW, 
                 void* pData, DWORD dwDataSize, DWORD dwOptionType)
{
  DWORD dwSize;
  PLUGINOPTIONW poW;

  poW.pOptionName = pszOptionNameW;
  poW.lpData = (BYTE*) pData;
  poW.dwData = dwDataSize;
  poW.dwType = dwOptionType;
  dwSize = (DWORD) SendMessageW(g_hMainWnd, AKD_OPTIONW, (WPARAM) hOptions, (LPARAM) &poW);
  if (dwSize == 0)
  {
    pszOptionNameW = getOldOptionNameW(pszOptionNameW);
    if (pszOptionNameW[0])
    {
      poW.pOptionName = pszOptionNameW;
      poW.lpData = (BYTE*) pData;
      poW.dwData = dwDataSize;
      poW.dwType = dwOptionType;
      dwSize = (DWORD) SendMessageW(g_hMainWnd, AKD_OPTIONW, (WPARAM) hOptions, (LPARAM) &poW);
    }
  }
  return (dwSize > 0);
}

static BOOL readOptionBinaryA(HANDLE hOptions, const char* pszOptionNameA, void* pBuf, DWORD dwBufSize)
{
  return readOptionA(hOptions, pszOptionNameA, pBuf, dwBufSize, PO_BINARY);
}

static BOOL readOptionBinaryW(HANDLE hOptions, const wchar_t* pszOptionNameW, void* pBuf, DWORD dwBufSize)
{
  return readOptionW(hOptions, pszOptionNameW, pBuf, dwBufSize, PO_BINARY);
}

static BOOL readOptionDwordA(HANDLE hOptions, const char* pszOptionNameA, DWORD* pdwValue)
{
  return readOptionA(hOptions, pszOptionNameA, pdwValue, sizeof(DWORD), PO_DWORD);
}

static BOOL readOptionDwordW(HANDLE hOptions, const wchar_t* pszOptionNameW, DWORD* pdwValue)
{
  return readOptionW(hOptions, pszOptionNameW, pdwValue, sizeof(DWORD), PO_DWORD);
}

static BOOL readOptionStrA(HANDLE hOptions, const char* pszOptionNameA, char* pszStrA, DWORD dwStrSize)
{
  return readOptionA(hOptions, pszOptionNameA, pszStrA, dwStrSize*sizeof(char), PO_STRING);
}

static BOOL readOptionStrW(HANDLE hOptions, const wchar_t* pszOptionNameW, wchar_t* pszStrW, DWORD dwStrSize)
{
  return readOptionW(hOptions, pszOptionNameW, pszStrW, dwStrSize*sizeof(wchar_t), PO_STRING);
}

void ReadOptions()
{
  DWORD dwValue;
  int   i;

  strHtmlFileExtsW_0[0] = 0;
  strComment1FileExtsW_0[0] = 0;
  opt_szNextCharOkW_0[0] = 0;
  opt_szPrevCharOkW_0[0] = 0;
  opt_szUserBracketsW_0[0] = 0;

  for (i = 0; i < OPT_DWORD_COUNT; i++)
  {
    g_dwOptions[i] = OPT_UNDEFINED_DWORD;
    g_dwOptions0[i] = OPT_UNDEFINED_DWORD;
  }

  if (g_bOldWindows)
  {
    HANDLE hOptions;

    hOptions = (HANDLE) SendMessage(g_hMainWnd,
      AKD_BEGINOPTIONS, POB_READ, (LPARAM) cszOptNamesA[OPT_XBRACKETS]);

    if (hOptions)
    {
      dwValue = OPT_UNDEFINED_DWORD;
      readOptionDwordA(hOptions, cszOptNamesA[OPT_OPTIONSFLAGS], &dwValue);
      opt_dwOptionsFlags0 = dwValue;  // opt_dwOptionsFlags0 value is set

      for (i = 0; i < 2; i++)
      {
        dwValue = OPT_UNDEFINED_DWORD;
        readOptionBinaryA(hOptions, 
          cszOptNamesA[(i == 0) ? OPT_HIGHLIGHTRGB : OPT_HIGHLIGHTBKRGB], 
            &dwValue, sizeof(DWORD));
        opt_dwHighlightRGB0[i] = dwValue;
        if (opt_dwHighlightRGB0[i] != OPT_UNDEFINED_DWORD)
        {
          /*c_base::*/byte_t btColour[4];

          btColour[0] = GetRValue( (COLORREF) opt_dwHighlightRGB0[i] );
          btColour[1] = GetGValue( (COLORREF) opt_dwHighlightRGB0[i] );
          btColour[2] = GetBValue( (COLORREF) opt_dwHighlightRGB0[i] );
          bracketsColourHighlight[i] = RGB(btColour[0], btColour[1], btColour[2]);
        }
      }

      readOptionBinaryA(hOptions, cszOptNamesA[OPT_CUSTOMRGB], 
        g_CustomColoursHighlight, MAX_CUSTOM_COLOURS*sizeof(COLORREF));
      
      readOptionStrA(hOptions, cszOptNamesA[OPT_HTMLFILEEXTS], 
        strHtmlFileExtsA, STR_FILEEXTS_SIZE - 1);
      CharLowerA(strHtmlFileExtsA);
      lstrcpyA( (LPSTR) strHtmlFileExtsW_0, strHtmlFileExtsA );

      readOptionStrA(hOptions, cszOptNamesA[OPT_COMMENT1FILEEXTS], 
        strComment1FileExtsA, STR_FILEEXTS_SIZE - 1);
      CharLowerA(strComment1FileExtsA);
      lstrcpyA( (LPSTR) strComment1FileExtsW_0, strComment1FileExtsA );

      readOptionStrA(hOptions, cszOptNamesA[OPT_COMMON_USER_BRPAIRS],
        (char *) opt_szUserBracketsW_0, MAX_USER_BRACKETS*4 - 1);

      readOptionStrA(hOptions, cszOptNamesA[OPT_AUTOBRACKETS_NEXT_CHAR_OK], 
        (char *) opt_szNextCharOkW_0, MAX_PREV_NEXT_CHAR_OK_SIZE - 1);

      readOptionStrA(hOptions, cszOptNamesA[OPT_AUTOBRACKETS_PREV_CHAR_OK],
        (char *) opt_szPrevCharOkW_0, MAX_PREV_NEXT_CHAR_OK_SIZE - 1);
      
      for (i = 0; i < OPT_DWORD_COUNT; i++)
      {
        g_dwOptions[i] = OPT_UNDEFINED_DWORD;
        readOptionDwordA(hOptions, cszOptNamesA[i], &g_dwOptions[i]);
        g_dwOptions0[i] = g_dwOptions[i];
      }

      SendMessage(g_hMainWnd, AKD_ENDOPTIONS, (WPARAM) hOptions, 0);
    }

  }
  else
  {
    HANDLE hOptions;

    hOptions = (HANDLE) SendMessage(g_hMainWnd,
      AKD_BEGINOPTIONS, POB_READ, (LPARAM) cszOptNamesW[OPT_XBRACKETS]);

    if (hOptions)
    {
      dwValue = OPT_UNDEFINED_DWORD;
      readOptionDwordW(hOptions, cszOptNamesW[OPT_OPTIONSFLAGS], &dwValue);
      opt_dwOptionsFlags0 = dwValue;  // opt_dwOptionsFlags0 value is set

      for (i = 0; i < 2; i++)
      {
        dwValue = OPT_UNDEFINED_DWORD;
        readOptionBinaryW(hOptions,
          cszOptNamesW[(i == 0) ? OPT_HIGHLIGHTRGB : OPT_HIGHLIGHTBKRGB],
            &dwValue, sizeof(DWORD));
        opt_dwHighlightRGB0[i] = dwValue;
        if (opt_dwHighlightRGB0[i] != OPT_UNDEFINED_DWORD)
        {
          /*c_base::*/byte_t btColour[4];

          btColour[0] = GetRValue( (COLORREF) opt_dwHighlightRGB0[i] );
          btColour[1] = GetGValue( (COLORREF) opt_dwHighlightRGB0[i] );
          btColour[2] = GetBValue( (COLORREF) opt_dwHighlightRGB0[i] );
          bracketsColourHighlight[i] = RGB(btColour[0], btColour[1], btColour[2]);
        }
      }

      readOptionBinaryW(hOptions, cszOptNamesW[OPT_CUSTOMRGB], 
        g_CustomColoursHighlight, MAX_CUSTOM_COLOURS*sizeof(COLORREF));

      readOptionStrW(hOptions, cszOptNamesW[OPT_HTMLFILEEXTS],
        strHtmlFileExtsW, STR_FILEEXTS_SIZE - 1);
      CharLowerW(strHtmlFileExtsW);
      lstrcpyW(strHtmlFileExtsW_0, strHtmlFileExtsW);

      readOptionStrW(hOptions, cszOptNamesW[OPT_COMMENT1FILEEXTS],
        strComment1FileExtsW, STR_FILEEXTS_SIZE - 1);
      CharLowerW(strComment1FileExtsW);
      lstrcpyW(strComment1FileExtsW_0, strComment1FileExtsW);

      readOptionStrW(hOptions, cszOptNamesW[OPT_COMMON_USER_BRPAIRS],
        opt_szUserBracketsW_0, MAX_USER_BRACKETS*4 - 1);

      readOptionStrW(hOptions, cszOptNamesW[OPT_AUTOBRACKETS_NEXT_CHAR_OK],
        opt_szNextCharOkW_0, MAX_PREV_NEXT_CHAR_OK_SIZE - 1);

      readOptionStrW(hOptions, cszOptNamesW[OPT_AUTOBRACKETS_PREV_CHAR_OK],
        opt_szPrevCharOkW_0, MAX_PREV_NEXT_CHAR_OK_SIZE - 1);

      for (i = 0; i < OPT_DWORD_COUNT; i++)
      {
        g_dwOptions[i] = OPT_UNDEFINED_DWORD;
        readOptionDwordW(hOptions, cszOptNamesW[i], &g_dwOptions[i]);
        g_dwOptions0[i] = g_dwOptions[i];
      }

      SendMessage(g_hMainWnd, AKD_ENDOPTIONS, (WPARAM) hOptions, 0);
    }

  }

  for (i = 0; i < MAX_CUSTOM_COLOURS; i++)
  {
    g_CustomColoursHighlight_0[i] = g_CustomColoursHighlight[i];
  }

  // opt_dwOptionsFlags0 value has been set before
  if (opt_dwOptionsFlags0 != OPT_UNDEFINED_DWORD)
  {
    bBracketsAutoComplete =
      ((opt_dwOptionsFlags0 & OPTF_AUTOCOMPLETE) == OPTF_AUTOCOMPLETE);
    uBracketsHighlight = 0x00;
    if ((opt_dwOptionsFlags0 & OPTF_HIGHLIGHT_ENABLED) == OPTF_HIGHLIGHT_ENABLED)
      uBracketsHighlight |= BRHLF_ENABLED;
    if ((opt_dwOptionsFlags0 & OPTF_HIGHLIGHT_TEXT) == OPTF_HIGHLIGHT_TEXT)
      uBracketsHighlight |= BRHLF_TEXT;
    if ((opt_dwOptionsFlags0 & OPTF_HIGHLIGHT_BKGND) == OPTF_HIGHLIGHT_BKGND)
      uBracketsHighlight |= BRHLF_BKGND;
    bBracketsHighlightVisibleArea =
      ((opt_dwOptionsFlags0 & OPTF_VISIBLEAREA) == OPTF_VISIBLEAREA);
    bBracketsRightExistsOK =
      ((opt_dwOptionsFlags0 & OPTF_RIGHTBRACKETOK) == OPTF_RIGHTBRACKETOK);
    bBracketsDoSingleQuote =
      ((opt_dwOptionsFlags0 & OPTF_DOSINGLEQUOTE) == OPTF_DOSINGLEQUOTE);
    bBracketsDoTag =
      ((opt_dwOptionsFlags0 & OPTF_DOTAG) == OPTF_DOTAG);
    bBracketsDoTag2 =
      ((opt_dwOptionsFlags0 & OPTF_DOTAG2) == OPTF_DOTAG2);
    bBracketsDoTagIf =
      ((opt_dwOptionsFlags0 & OPTF_DOTAGIF) == OPTF_DOTAGIF);
    bBracketsSkipEscaped =
      ((opt_dwOptionsFlags0 & OPTF_SKIPESCAPED) == OPTF_SKIPESCAPED);
    bBracketsSkipComment1 = 
      ((opt_dwOptionsFlags0 & OPTF_SKIPCOMMENT1) == OPTF_SKIPCOMMENT1);
  }

  if (opt_szNextCharOkW_0[0] != 0)
  {
    if (g_bOldWindows)
    {
      lstrcpyA(opt_szNextCharOkA, (LPCSTR)opt_szNextCharOkW_0);
      MultiByteToWideChar(CP_ACP, 0, opt_szNextCharOkA, -1, 
        opt_szNextCharOkW, MAX_PREV_NEXT_CHAR_OK_SIZE - 1);
    }
    else
    {
      lstrcpyW(opt_szNextCharOkW, opt_szNextCharOkW_0);
    }
  }
  setNextCharOkW(opt_szNextCharOkW);

  if (opt_szPrevCharOkW_0[0] != 0)
  {
    if (g_bOldWindows)
    {
      lstrcpyA(opt_szPrevCharOkA, (LPCSTR)opt_szPrevCharOkW_0);
      MultiByteToWideChar(CP_ACP, 0, opt_szPrevCharOkA, -1, 
        opt_szPrevCharOkW, MAX_PREV_NEXT_CHAR_OK_SIZE - 1);
    }
    else
    {
      lstrcpyW(opt_szPrevCharOkW, opt_szPrevCharOkW_0);
    }
  }
  setPrevCharOkW(opt_szPrevCharOkW);
  
  if (opt_szUserBracketsW_0[0] != 0)
  {
    if (g_bOldWindows)
    {
      lstrcpyA(opt_szUserBracketsA, (LPCSTR)opt_szUserBracketsW_0);
      MultiByteToWideChar(CP_ACP, 0, opt_szUserBracketsA, -1, 
        opt_szUserBracketsW, MAX_USER_BRACKETS*4 - 1);
    }
    else
    {
      lstrcpyW(opt_szUserBracketsW, opt_szUserBracketsW_0);
    }
  }

  if (g_bOldWindows)
  {
    setUserBracketsA(opt_szUserBracketsA);
    setUserBracketsW(opt_szUserBracketsW);
  }
  else
  {
    setUserBracketsW(opt_szUserBracketsW);
  }

  if (g_dwOptions[OPT_DWORD_AUTOCOMPLETE_ALL_AUTOBR] == OPT_UNDEFINED_DWORD)
    g_dwOptions[OPT_DWORD_AUTOCOMPLETE_ALL_AUTOBR] = 0;
  if (g_dwOptions[OPT_DWORD_AUTOCOMPLETE_OVR_AUTOBR] == OPT_UNDEFINED_DWORD)
    g_dwOptions[OPT_DWORD_AUTOCOMPLETE_OVR_AUTOBR] = 0;
  if (g_dwOptions[OPT_DWORD_AUTOCOMPLETE_SEL_AUTOBR] == OPT_UNDEFINED_DWORD)
    g_dwOptions[OPT_DWORD_AUTOCOMPLETE_SEL_AUTOBR] = 0;
  if (g_dwOptions[OPT_DWORD_HIGHLIGHT_HLT_BOTHBR] == OPT_UNDEFINED_DWORD)
    g_dwOptions[OPT_DWORD_HIGHLIGHT_HLT_BOTHBR] = 1;
  if (g_dwOptions[OPT_DWORD_HIGHLIGHT_HLT_STYLE] == OPT_UNDEFINED_DWORD)
    g_dwOptions[OPT_DWORD_HIGHLIGHT_HLT_STYLE] = XBR_HSF_BOLDFONT | XBR_HSF_REDRAWCODER;
  if (g_dwOptions[OPT_DWORD_HIGHLIGHT_QUOTE_DETECT_LINES] == OPT_UNDEFINED_DWORD)
    g_dwOptions[OPT_DWORD_HIGHLIGHT_QUOTE_DETECT_LINES] = DEFAULT_MAX_DETECT_DUPPAIR_LINES;
  if (g_dwOptions[OPT_DWORD_HIGHLIGHT_QUOTE_MAX_LINES] == OPT_UNDEFINED_DWORD)
    g_dwOptions[OPT_DWORD_HIGHLIGHT_QUOTE_MAX_LINES] = DEFAULT_MAX_KNOWN_DUPPAIR_LINES;
  if (g_dwOptions[OPT_DWORD_HIGHLIGHT_BR_MAX_LINES] == OPT_UNDEFINED_DWORD)
    g_dwOptions[OPT_DWORD_HIGHLIGHT_BR_MAX_LINES] = DEFAULT_MAX_BR_LINES;

  if (g_dwOptions[OPT_DWORD_HIGHLIGHT_QUOTE_DETECT_LINES] > g_dwOptions[OPT_DWORD_HIGHLIGHT_QUOTE_MAX_LINES])
  {
    g_dwOptions[OPT_DWORD_HIGHLIGHT_QUOTE_DETECT_LINES] = g_dwOptions[OPT_DWORD_HIGHLIGHT_QUOTE_MAX_LINES];
    // we do not want this value to be overwritten in the config file/registry
    g_dwOptions0[OPT_DWORD_HIGHLIGHT_QUOTE_DETECT_LINES] = g_dwOptions[OPT_DWORD_HIGHLIGHT_QUOTE_MAX_LINES];
  }
}

void SaveOptions()
{
  DWORD dwNewOptionsFlags;
  DWORD dwNewHighlightRGB[2];
  int   nCmpCustomColours;
  int   nStrCmpHtmlExts;
  int   nStrCmpComment1Exts;
  int   nStrCmpNextCharOk;
  int   nStrCmpPrevCharOk;
  int   nStrCmpUsrBrPairs;
  int   i;

  dwNewHighlightRGB[0] = (DWORD) bracketsColourHighlight[0];
  dwNewHighlightRGB[1] = (DWORD) bracketsColourHighlight[1];

  dwNewOptionsFlags = 0;
  if (bBracketsAutoComplete)
    dwNewOptionsFlags |= OPTF_AUTOCOMPLETE;
  if (uBracketsHighlight & BRHLF_ENABLED)
    dwNewOptionsFlags |= OPTF_HIGHLIGHT_ENABLED;
  if (uBracketsHighlight & BRHLF_TEXT)
    dwNewOptionsFlags |= OPTF_HIGHLIGHT_TEXT;
  if (uBracketsHighlight & BRHLF_BKGND)
    dwNewOptionsFlags |= OPTF_HIGHLIGHT_BKGND;
  if (bBracketsHighlightVisibleArea)
    dwNewOptionsFlags |= OPTF_VISIBLEAREA;
  if (bBracketsRightExistsOK)
    dwNewOptionsFlags |= OPTF_RIGHTBRACKETOK;
  if (bBracketsDoSingleQuote)
    dwNewOptionsFlags |= OPTF_DOSINGLEQUOTE;
  if (bBracketsDoTag)
    dwNewOptionsFlags |= OPTF_DOTAG;
  if (bBracketsDoTag2)
    dwNewOptionsFlags |= OPTF_DOTAG2;
  if (bBracketsDoTagIf)
    dwNewOptionsFlags |= OPTF_DOTAGIF;
  if (bBracketsSkipEscaped)
    dwNewOptionsFlags |= OPTF_SKIPESCAPED;
  if (bBracketsSkipComment1) 
    dwNewOptionsFlags |= OPTF_SKIPCOMMENT1;

  if (g_bOldWindows)
  {
    nStrCmpHtmlExts = lstrcmpiA( (LPCSTR) strHtmlFileExtsW_0, strHtmlFileExtsA );
    nStrCmpComment1Exts = lstrcmpiA( (LPCSTR) strComment1FileExtsW_0, strComment1FileExtsA );
    nStrCmpNextCharOk = lstrcmpA( (LPCSTR) opt_szNextCharOkW_0, opt_szNextCharOkA );
    nStrCmpPrevCharOk = lstrcmpA( (LPCSTR) opt_szPrevCharOkW_0, opt_szPrevCharOkA );
    nStrCmpUsrBrPairs = lstrcmpA( (LPCSTR) opt_szUserBracketsW_0, opt_szUserBracketsA );
  }
  else
  {
    nStrCmpHtmlExts = lstrcmpiW( strHtmlFileExtsW_0, strHtmlFileExtsW );
    nStrCmpComment1Exts = lstrcmpiW( strComment1FileExtsW_0, strComment1FileExtsW );
    nStrCmpNextCharOk = lstrcmpW( opt_szNextCharOkW_0, opt_szNextCharOkW );
    nStrCmpPrevCharOk = lstrcmpW( opt_szPrevCharOkW_0, opt_szPrevCharOkW );
    nStrCmpUsrBrPairs = lstrcmpW( opt_szUserBracketsW_0, opt_szUserBracketsW );
  }

  nCmpCustomColours = 0;
  for (i = 0; i < MAX_CUSTOM_COLOURS; i++)
  {
    if (g_CustomColoursHighlight_0[i] != g_CustomColoursHighlight[i])
    {
      nCmpCustomColours = 1;
      break;
    }
  }

  for (i = 0; i < OPT_DWORD_COUNT; i++)
  {
    if (g_dwOptions[i] != g_dwOptions0[i])
      break;
  }

  if ((i < OPT_DWORD_COUNT) ||
      (dwNewOptionsFlags != opt_dwOptionsFlags0) ||
      (dwNewHighlightRGB[0] != opt_dwHighlightRGB0[0]) ||
      (dwNewHighlightRGB[1] != opt_dwHighlightRGB0[1]) ||
      (nCmpCustomColours != 0) ||
      (nStrCmpHtmlExts != 0) ||
      (nStrCmpComment1Exts != 0) ||
      (nStrCmpNextCharOk != 0) ||
      (nStrCmpPrevCharOk != 0) ||
      (nStrCmpUsrBrPairs != 0))
  {
    if (g_bOldWindows)
    {
      HANDLE        hOptions;
      PLUGINOPTIONA poA;

      hOptions = (HANDLE) SendMessage(g_hMainWnd,
        AKD_BEGINOPTIONS, POB_SAVE, (LPARAM) cszOptNamesA[OPT_XBRACKETS]);

      if (hOptions)
      {
        if (dwNewOptionsFlags != opt_dwOptionsFlags0)
        {
          poA.pOptionName = cszOptNamesA[OPT_OPTIONSFLAGS];
          poA.lpData = (BYTE*) &dwNewOptionsFlags;
          poA.dwData = sizeof(DWORD);
          poA.dwType = PO_DWORD;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poA);
          opt_dwOptionsFlags0 = dwNewOptionsFlags;
        }
        for (i = 0; i < 2; i++)
        {
          if (dwNewHighlightRGB[i] != opt_dwHighlightRGB0[i])
          {
            poA.pOptionName = cszOptNamesA[(i == 0) ? OPT_HIGHLIGHTRGB : OPT_HIGHLIGHTBKRGB];
            poA.lpData = (BYTE*) &dwNewHighlightRGB[i];
            poA.dwData = sizeof(DWORD);
            poA.dwType = PO_BINARY;
            SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poA);
            opt_dwHighlightRGB0[i] = dwNewHighlightRGB[i];
          }
        }
        if (nCmpCustomColours != 0)
        {
          poA.pOptionName = cszOptNamesA[OPT_CUSTOMRGB];
          poA.lpData = (BYTE*) g_CustomColoursHighlight;
          poA.dwData = MAX_CUSTOM_COLOURS*sizeof(COLORREF);
          poA.dwType = PO_BINARY;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poA);
          for (i = 0; i < MAX_CUSTOM_COLOURS; i++)
          {
            g_CustomColoursHighlight_0[i] = g_CustomColoursHighlight[i];
          }
        }
        if (nStrCmpHtmlExts != 0)
        {
          poA.pOptionName = cszOptNamesA[OPT_HTMLFILEEXTS];
          poA.lpData = (BYTE*) strHtmlFileExtsA;
          poA.dwData = (lstrlenA(strHtmlFileExtsA) + 1)*sizeof(char);
          poA.dwType = PO_STRING;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poA);
          lstrcpyA( (LPSTR) strHtmlFileExtsW_0, strHtmlFileExtsA );
        }
        if (nStrCmpComment1Exts != 0)
        {
          poA.pOptionName = cszOptNamesA[OPT_COMMENT1FILEEXTS];
          poA.lpData = (BYTE*) strComment1FileExtsA;
          poA.dwData = (lstrlenA(strComment1FileExtsA) + 1)*sizeof(char);
          poA.dwType = PO_STRING;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poA);
          lstrcpyA( (LPSTR) strComment1FileExtsW_0, strComment1FileExtsA );
        }
        if (nStrCmpUsrBrPairs != 0)
        {
          poA.pOptionName = cszOptNamesA[OPT_COMMON_USER_BRPAIRS];
          poA.lpData = (BYTE*) opt_szUserBracketsA;
          poA.dwData = (lstrlenA(opt_szUserBracketsA) + 1)*sizeof(char);
          poA.dwType = PO_STRING;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poA);
          lstrcpyA( (LPSTR) opt_szUserBracketsW_0, opt_szUserBracketsA );
        }
        if (nStrCmpNextCharOk != 0)
        {
          poA.pOptionName = cszOptNamesA[OPT_AUTOBRACKETS_NEXT_CHAR_OK];
          poA.lpData = (BYTE*) opt_szNextCharOkA;
          poA.dwData = (lstrlenA(opt_szNextCharOkA) + 1)*sizeof(char);
          poA.dwType = PO_STRING;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poA);
          lstrcpyA( (LPSTR) opt_szNextCharOkW_0, opt_szNextCharOkA );
        }
        if (nStrCmpPrevCharOk != 0)
        {
          poA.pOptionName = cszOptNamesA[OPT_AUTOBRACKETS_PREV_CHAR_OK];
          poA.lpData = (BYTE*) opt_szPrevCharOkA;
          poA.dwData = (lstrlenA(opt_szPrevCharOkA) + 1)*sizeof(char);
          poA.dwType = PO_STRING;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poA);
          lstrcpyA( (LPSTR) opt_szPrevCharOkW_0, opt_szPrevCharOkA );
        }
        for (i = 0; i < OPT_DWORD_COUNT; i++)
        {
          if (g_dwOptions[i] != g_dwOptions0[i])
          {
            poA.pOptionName = cszOptNamesA[i];
            poA.lpData = (BYTE*) &g_dwOptions[i];
            poA.dwData = sizeof(DWORD);
            poA.dwType = PO_DWORD;
            SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poA);
            g_dwOptions0[i] = g_dwOptions[i];
          }
        }
        SendMessage(g_hMainWnd, AKD_ENDOPTIONS, (WPARAM) hOptions, 0);
      }

    }
    else
    {
      HANDLE        hOptions;
      PLUGINOPTIONW poW;

      hOptions = (HANDLE) SendMessage(g_hMainWnd,
        AKD_BEGINOPTIONS, POB_SAVE, (LPARAM) cszOptNamesW[OPT_XBRACKETS]);

      if (hOptions)
      {
        if (dwNewOptionsFlags != opt_dwOptionsFlags0)
        {
          poW.pOptionName = cszOptNamesW[OPT_OPTIONSFLAGS];
          poW.lpData = (BYTE*) &dwNewOptionsFlags;
          poW.dwData = sizeof(DWORD);
          poW.dwType = PO_DWORD;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poW);
          opt_dwOptionsFlags0 = dwNewOptionsFlags;
        }
        for (i = 0; i < 2; i++)
        {
          if (dwNewHighlightRGB[i] != opt_dwHighlightRGB0[i])
          {
            poW.pOptionName = cszOptNamesW[(i == 0) ? OPT_HIGHLIGHTRGB : OPT_HIGHLIGHTBKRGB];
            poW.lpData = (BYTE*) &dwNewHighlightRGB[i];
            poW.dwData = sizeof(DWORD);
            poW.dwType = PO_BINARY;
            SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poW);
            opt_dwHighlightRGB0[i] = dwNewHighlightRGB[i];
          }
        }
        if (nCmpCustomColours != 0)
        {
          poW.pOptionName = cszOptNamesW[OPT_CUSTOMRGB];
          poW.lpData = (BYTE*) g_CustomColoursHighlight;
          poW.dwData = MAX_CUSTOM_COLOURS*sizeof(COLORREF);
          poW.dwType = PO_BINARY;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poW);
          for (i = 0; i < MAX_CUSTOM_COLOURS; i++)
          {
            g_CustomColoursHighlight_0[i] = g_CustomColoursHighlight[i];
          }
        }
        if (nStrCmpHtmlExts != 0)
        {
          poW.pOptionName = cszOptNamesW[OPT_HTMLFILEEXTS];
          poW.lpData = (BYTE*) strHtmlFileExtsW;
          poW.dwData = (lstrlenW(strHtmlFileExtsW) + 1)*sizeof(wchar_t);
          poW.dwType = PO_STRING;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poW);
          lstrcpyW(strHtmlFileExtsW_0, strHtmlFileExtsW);
        }
        if (nStrCmpComment1Exts != 0)
        {
          poW.pOptionName = cszOptNamesW[OPT_COMMENT1FILEEXTS];
          poW.lpData = (BYTE*) strComment1FileExtsW;
          poW.dwData = (lstrlenW(strComment1FileExtsW) + 1)*sizeof(wchar_t);
          poW.dwType = PO_STRING;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poW);
          lstrcpyW(strComment1FileExtsW_0, strComment1FileExtsW);
        }
        if (nStrCmpUsrBrPairs != 0)
        {
          poW.pOptionName = cszOptNamesW[OPT_COMMON_USER_BRPAIRS];
          poW.lpData = (BYTE*) opt_szUserBracketsW;
          poW.dwData = (lstrlenW(opt_szUserBracketsW) + 1)*sizeof(wchar_t);
          poW.dwType = PO_STRING;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poW);
          lstrcpyW(opt_szUserBracketsW_0, opt_szUserBracketsW);
        }
        if (nStrCmpNextCharOk != 0)
        {
          poW.pOptionName = cszOptNamesW[OPT_AUTOBRACKETS_NEXT_CHAR_OK];
          poW.lpData = (BYTE*) opt_szNextCharOkW;
          poW.dwData = (lstrlenW(opt_szNextCharOkW) + 1)*sizeof(wchar_t);
          poW.dwType = PO_STRING;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poW);
          lstrcpyW(opt_szNextCharOkW_0, opt_szNextCharOkW);
        }
        if (nStrCmpPrevCharOk != 0)
        {
          poW.pOptionName = cszOptNamesW[OPT_AUTOBRACKETS_PREV_CHAR_OK];
          poW.lpData = (BYTE*) opt_szPrevCharOkW;
          poW.dwData = (lstrlenW(opt_szPrevCharOkW) + 1)*sizeof(wchar_t);
          poW.dwType = PO_STRING;
          SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poW);
          lstrcpyW(opt_szPrevCharOkW_0, opt_szPrevCharOkW);
        }
        for (i = 0; i < OPT_DWORD_COUNT; i++)
        {
          if (g_dwOptions[i] != g_dwOptions0[i])
          {
            poW.pOptionName = cszOptNamesW[i];
            poW.lpData = (BYTE*) &g_dwOptions[i];
            poW.dwData = sizeof(DWORD);
            poW.dwType = PO_DWORD;
            SendMessage(g_hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poW);
            g_dwOptions0[i] = g_dwOptions[i];
          }
        }
        SendMessage(g_hMainWnd, AKD_ENDOPTIONS, (WPARAM) hOptions, 0);
      }

    }
  }

}

// Entry point
/*extern "C"*/
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  if (fdwReason == DLL_PROCESS_ATTACH)
  {
  }
  else if (fdwReason == DLL_THREAD_ATTACH)
  {
  }
  else if (fdwReason == DLL_THREAD_DETACH)
  {
  }
  else if (fdwReason == DLL_PROCESS_DETACH)
  {
  }
  return TRUE;
}
