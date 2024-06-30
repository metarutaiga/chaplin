

//    
//  $Workfile: atiddhsl.h $
//
//  Description:
//
//  Trade secret of ATI Technologies, Inc.
//  Copyright 1999-2001, ATI Technologies, Inc., (unpublished)
//
//  All rights reserved.
//  The year included in the foregoing notice is the year of creation of the work.
//
//
 
#ifndef _ATIDDHSL_H
#define _ATIDDHSL_H

typedef void* LPD3DHAL_DP2COMMAND;
typedef void* LPD3DHAL_DP2CREATEPIXELSHADER;

#define D3DSI_GETREGNUM(token)  (token & D3DSP_REGNUM_MASK)
#define D3DSI_GETOPCODE(command) (command & D3DSI_OPCODE_MASK)
#define D3DSI_GETWRITEMASK(token) (token & D3DSP_WRITEMASK_ALL)
#define D3DVS_GETSWIZZLECOMP(source, component)  (source >> ((component << 1) + 16) & 0x3)
#define D3DVS_GETSWIZZLE(token)  (token & D3DVS_SWIZZLE_MASK)
#define D3DVS_GETSRCMODIFIER(token) (token & D3DSP_SRCMOD_MASK)
#define D3DVS_GETADDRESSMODE(token) (token & D3DVS_ADDRESSMODE_MASK)

#if(DIRECT3D_VERSION < 0x0900)

#define D3DSI_GETREGTYPE(token) ((D3DSHADER_PARAM_REGISTER_TYPE)(token & D3DSP_REGTYPE_MASK))

#else

#define D3DSI_GETREGTYPE(token) ((D3DSHADER_PARAM_REGISTER_TYPE)(((token & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT) | \
                                 ((token & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2)))

#endif

//********************************** HSLDPF and HSLASSERT *********************
//How to use:
//  HSLDPF( DWORD level, char* pDbgMsg, ...);
//  There are ten debug message settings which control the output of HSLDPF.
//  The debug message output is enabled by registry settings.  During driver
//  initalization the registry settings are read from HKEY_LOCAL_MACHINE,
//  "SOFTWARE\ATI Technologies\Driver\Debug"
//  Here are the levels and corresponding registry settings:
//
//  Level                     Resistry String                  Value
//
//  E_ASSERTION,            *No string Value*,      Always print Assertions
//                                              ( Causes int 3 debug interupt )
//  E_HAL_ENTRY_EXIT,       "HalEntryExit",         1 Enable, 0 Disable/Default
//  E_GENERAL_ENTRY_EXIT,   "GeneralEntryExit",     1 Enable, 0 Disable/Default
//  E_INFO_MESSAGE,         "InfoMessage",          1 Enable, 0 Disable/Default
//  E_ERROR_MESSAGE,        "ErrorMessage",         1 Enable, 0 Disable/Default
//  E_DATA_DUMP,            "DataDump",             1 Enable, 0 Disable/Default
//  E_DEBUG1,               "Debug1",               1 Enable, 0 Disable/Default
//  E_DEBUG2,               "Debug2",               1 Enable, 0 Disable/Default
//  E_BLT_DATA,             "BltData",              1 Enable, 0 Disable/Default
//  E_SURFACE_DATA,         "SurfaceData",          1 Enable, 0 Disable/Default
//  E_STATE_DATA,           "StateData",            1 Enable, 0 Disable/Default
//  E_PIXELSHADER_DATA      "PixelShaderData"       1 Enable, 0 Disable/Default
//  E_VERTEXSHADER_DATA     "VertexShaderData"      1 Enable, 0 Disable/Default
//  E_SHADER_DUMP           "ShaderDump"            1 Enable, 0 Disable/Default
//  E_VIDMEM_DATA           "VidMemData"            1 Enable, 0 Disable/Default
//  E_PIXELSHADER_DUMP      "PixelShaderDump"       1 Enable, 0 Disable/Default
//
//Usage - Debug Level, Message / format string and Varible argument list
//Example:
//  HSLDPF( E_INFO_MESSAGE, "We are being asked to fill a ZBuffer" );
//  HSLDPF( E_DATA_DUMP, "Fill value = %08x", dvFillDepth );
//
//  HSLASSERT( int Expression ); Use in the place of the assert standard run
//  time library function. Evaluates an expression and when the result is
//  FALSE, prints the file and line number of the failed asseration.
//
//Example:
//  HSLASSERT( ( lpD3DDev->SurfHandleList[i].dwNumATITexCntxAllocated == 0 ) );
//  See also: Direct Draw and D3D HAL InterfaceRev 0.4

typedef enum
{
    E_ASSERTION             = 0,
    E_HAL_ENTRY_EXIT        = 1,
    E_GENERAL_ENTRY_EXIT    = 2,
    E_INFO_MESSAGE          = 3,
    E_ERROR_MESSAGE         = 4,
    E_DATA_DUMP             = 5,
    E_DEBUG1                = 6,
    E_DEBUG2                = 7,
    E_BLT_DATA              = 8,
    E_SURFACE_DATA          = 9,
    E_STATE_DATA            = 10,
    E_PIXELSHADER_DATA      = 11,
    E_VERTEXSHADER_DATA     = 12,
    E_SHADER_DUMP           = 13,
    E_VIDMEM_DATA           = 14,
    E_PIXELSHADER_DUMP      = 15,
    E_LDDM_CREATE_RES       = 16,
    E_LDDM_LOCK             = 17,
    E_DEBUGLEVEL_TOO_BIG    = 18
} EDDHSLDEBUGLEVEL;


#ifndef   _X86_
#if defined (i386) || defined (_i386_)
#define _X86_
#endif /* i386 || _i386_ */
#endif /* _X86_ */


#if defined  (_X86_) && !defined (_WIN64)
#define HSL_STOP_IN_DEBUGGER { __asm int 3 }
#define HSL_NOP              { __asm nop   }
#else  /* _X86_ */
#if DEBUG
#define HSL_STOP_IN_DEBUGGER    DebugBreak()
#else
#define HSL_STOP_IN_DEBUGGER
#endif
#define HSL_NOP
#endif /* _X86_ */

VOID
APIENTRY
vDdHslDebugPrint(
    __in  EDDHSLDEBUGLEVEL eDbgLevel,
    __in  char* pDbgMsg,
    ...
    );

#ifdef DEBUG

    // Master control for HSL Debug output.  If set to false, then no HSL Output
    // will occur even if the corresponding status flag is set in HSLDebugTable.
    // It defaults to TRUE since a function will have to explicitly request it to
    // be disabled.
    extern BOOL bHSLDebugOutputOn;

    #define HSLDPF vDdHslDebugPrint

    #define HSLDPF_ENABLE()  bHSLDebugOutputOn = TRUE;
    #define HSLDPF_DISABLE() bHSLDebugOutputOn = FALSE;

    #if defined(_PREFAST_) || defined(_PREFIX_)
        __declspec(noreturn) void  die(__in  char* pDbgMsg);  // noreturn function

        #define HSLASSERT( exp )  ( void ) ( ( exp ) || \
                ( vDdHslDebugPrint( E_ASSERTION, "Assertion failed: %s (%s:%u)", \
                                    #exp, __FILE__, __LINE__ ), 0 ), die("no return") )
    #else
        #define HSLASSERT( exp )  ( void ) ( ( exp ) || \
                ( vDdHslDebugPrint( E_ASSERTION, "Assertion failed: %s (%s:%u)", \
                                    #exp, __FILE__, __LINE__ ), 0 ) )
    #endif
#else
    #define HSLDPF 1 ? ( void )0 : ( void )

    #define HSLDPF_ENABLE()
    #define HSLDPF_DISABLE()

    #define HSLASSERT( exp ) ( ( void ) 0 )
#endif

#endif //_ATIDDHSL_H

