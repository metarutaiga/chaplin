

//  Workfile: r200pixelshader.c
//
//  Description: DX8 PixelShader Support
//
//  Trade secret of ATI Technologies, Inc.
//  Copyright 2000, ATI Technologies, Inc., (unpublished)
//
//  All rights reserved.
//  The year included in the foregoing notice is the year of creation of the work.
//
//

#include "inc/pixelshader.h"

#define PS_SWIZZLE_RGAA \
    ( (0 << (D3DSP_SWIZZLE_SHIFT + 0)) | \
      (1 << (D3DSP_SWIZZLE_SHIFT + 2)) | \
      (3 << (D3DSP_SWIZZLE_SHIFT + 4)) | \
      (3 << (D3DSP_SWIZZLE_SHIFT + 6)) )

const char* const DBGSTR_PS_BLEND_OPERATION[8] =
{
    "PS_ADD",
    "PS_SUBTRACT",
    "PS_CND0",
    "PS_LERP",
    "PS_DOT3",
    "PS_DOT4",
    "PS_CND",
    "PS_DOT2ADD",
};

void Rad2InitPixelShader( LPRAD2PIXELSHADER lpPShader )
{
    memset( lpPShader, 0, sizeof(RAD2PIXELSHADER) );

    lpPShader->iState.intRGBTerminalIns = -1;
    lpPShader->iState.intAlphaTerminalIns = -1;
    lpPShader->dwTexCoordDepthMode[0] = 0x2000;
    lpPShader->dwTexCoordDepthMode[1] = 0x2000;
    lpPShader->dwTexCoordDepthMode[2] = 0x2000;
    lpPShader->dwTexCoordDepthMode[3] = 0x2000;
    lpPShader->dwTexCoordDepthMode[4] = 0x2000;
    lpPShader->dwTexCoordDepthMode[5] = 0x2000;
    lpPShader->bumpping_d3dstage = -1;
}

static DWORD OutputScale( DWORD dwDestReg )
{
    DWORD scale = 0;

    HSLDPF( E_GENERAL_ENTRY_EXIT, "OuputScale: Entry" );
    switch ( (dwDestReg & D3DSP_DSTSHIFT_MASK) >> D3DSP_DSTSHIFT_SHIFT )
    {
    case 0x0:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Scale = 1X" );
        scale = PP_PIXSHADER_IX_C1__OUTPUT_SCALE__1X;
        break;
    case 0x1:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Scale = 2X" );
        scale = PP_PIXSHADER_IX_C1__OUTPUT_SCALE__2X;
        break;
    case 0x2:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Scale = 4X" );
        scale = PP_PIXSHADER_IX_C1__OUTPUT_SCALE__4X;
        break;
    case 0x3:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Scale = 8X" );
        scale = PP_PIXSHADER_IX_C1__OUTPUT_SCALE__8X;
        break;
    case 0xD:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Scale = 1/8X" );
        scale = PP_PIXSHADER_IX_C1__OUTPUT_SCALE__OPVER8X;
        break;
    case 0xE:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Scale = 1/4X" );
        scale = PP_PIXSHADER_IX_C1__OUTPUT_SCALE__OVER4X;
        break;
    case 0xF:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Scale = 1/2X" );
        scale = PP_PIXSHADER_IX_C1__OUTPUT_SCALE__OVER2X;
        break;
    default:
        HSLDPF( E_ERROR_MESSAGE, "OutputScale: error parsing dest register scaling modifier" );
        break;
    }
    HSLDPF( E_GENERAL_ENTRY_EXIT, "OuputScale: Exit" );

    return scale;
}

static DWORD OutputClamp( DWORD dwDestReg )
{
    DWORD clamp = 0;

    HSLDPF( E_GENERAL_ENTRY_EXIT, "OuputClamp: Entry" );
    switch ( dwDestReg & D3DSP_DSTMOD_MASK )
    {
    case D3DSPDM_NONE:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Clamp = +/- 8.0" );
        clamp = E_PS_OUTPUT_CLAMP_CLAMP << PP_PIXSHADER_I0_C1__OUTPUT_CLAMP__SHIFT;
        break;
    case D3DSPDM_SATURATE:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Clamp = 0.0 to +1.0" );
        clamp = E_PS_OUTPUT_CLAMP_SAT << PP_PIXSHADER_I0_C1__OUTPUT_CLAMP__SHIFT;
        break;
    case ATI_D3DSPDM_WRAP:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Clamp = R200 Wrap" );
        clamp = E_PS_OUTPUT_CLAMP_WRAP << PP_PIXSHADER_I0_C1__OUTPUT_CLAMP__SHIFT;
        break;
    default:
        HSLDPF( E_ERROR_MESSAGE, "OutputClamp: error parsing dest register generic modifier" );
        clamp = PP_PIXSHADER_IX_C1__OUTPUT_CLAMP__CLAMP;
        break;
    }
    HSLDPF( E_GENERAL_ENTRY_EXIT, "OuputClamp: Exit" );

    return clamp;
}

static DWORD OutputSelect( DWORD dwDestReg, LPD3DPSREGISTERS lpD3DPSRegs )
{
    DWORD d3dRegNum = D3DSI_GETREGNUM( dwDestReg );
    DWORD d3dRegType = D3DSI_GETREGTYPE( dwDestReg );
    DWORD select = 0;

    HSLDPF( E_GENERAL_ENTRY_EXIT, "OutputSelect: Entry" );
    switch ( d3dRegType )
    {
    case D3DSPR_TEMP:
        HSLASSERT( d3dRegNum == 0 || d3dRegNum == 1 );
        select = lpD3DPSRegs->dwTempRegs[d3dRegNum] + 1;
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Select: r%d=%d", d3dRegNum, select );
        break;
    case D3DSPR_TEXTURE:
        HSLASSERT( d3dRegNum <= 5 );
        select = lpD3DPSRegs->dwTexRegs[d3dRegNum] + 1;
        HSLDPF( E_PIXELSHADER_DATA, "\tOutput Select: t%d=%d", d3dRegNum, select );
        break;
    default:
        HSLDPF( E_ERROR_MESSAGE, "OutputSelect: Error processing output dest register type" );
        break;
    }
    HSLDPF( E_GENERAL_ENTRY_EXIT, "OutputSelect: Exit" );

    return select << PP_PIXSHADER_I0_C1__OUTPUT_SELECT__SHIFT;
}

static DWORD OutputMask( DWORD dwDestReg )
{
    DWORD mask = 0;

    HSLDPF( E_GENERAL_ENTRY_EXIT, "OutputMask: Entry" );
    if ( (dwDestReg & PS_WRITEMASK_RGB) != PS_WRITEMASK_RGB )
    {
        if ( (dwDestReg & PS_WRITEMASK_R) == 0 )
            mask |= PP_PIXSHADER_IX_C1__OUTPUT_MASK__GB;
        if ( (dwDestReg & PS_WRITEMASK_G) == 0 )
            mask |= PP_PIXSHADER_IX_C1__OUTPUT_MASK__RB;
        if ( (dwDestReg & PS_WRITEMASK_B) == 0 )
            mask |= PP_PIXSHADER_IX_C1__OUTPUT_MASK__RG;
        HSLDPF( E_PIXELSHADER_DATA, "\tOutputMask:  Masking RGB output.  Will write only %c%c%c",
                (mask & PP_PIXSHADER_IX_C1__OUTPUT_MASK__GB) ? '_' : 'R',
                (mask & PP_PIXSHADER_IX_C1__OUTPUT_MASK__RB) ? '_' : 'G',
                (mask & PP_PIXSHADER_IX_C1__OUTPUT_MASK__RG) ? '_' : 'B' );
    }
    HSLDPF( E_GENERAL_ENTRY_EXIT, "OutputMask: Exit" );

    return mask;
}

static DWORD OutputRotate( DWORD dwSwizzle )
{
    DWORD rotate = 0;

    HSLDPF( E_GENERAL_ENTRY_EXIT, "OutputRotate: Entry" );
    switch ( dwSwizzle & D3DSP_SWIZZLE_MASK )
    {
    case D3DSP_NOSWIZZLE:
        rotate = 0;
        break;
    case PS_SWIZZLE_AARG:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutputRotate:  AARG swizzle detected." );
        rotate = 1 << PP_PIXSHADER_I0_C1__OUTPUT_ROTATE__SHIFT;
        break;
    case PS_SWIZZLE_AGBA:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutputRotate:  AGBA swizzle detected." );
        rotate = 2 << PP_PIXSHADER_I0_C1__OUTPUT_ROTATE__SHIFT;
        break;
    case PS_SWIZZLE_ARGA:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutputRotate:  ARGA swizzle detected." );
        rotate = 3 << PP_PIXSHADER_I0_C1__OUTPUT_ROTATE__SHIFT;
        break;
    default:
        HSLDPF( E_PIXELSHADER_DATA, "\tOutputRotate: ERROR unsupported output swizzle (0x%x)", dwSwizzle & D3DSP_SWIZZLE_MASK );
        rotate = 0;
        break;
    }
    HSLDPF( E_GENERAL_ENTRY_EXIT, "OutputRotate: Exit" );

    return rotate;
}

static DWORD InputModifiers( DWORD dwSrc )
{
    DWORD modifiers = 0;

    HSLDPF( E_GENERAL_ENTRY_EXIT, "InputModifiers: Entry" );
    switch ( dwSrc & D3DSP_SRCMOD_MASK )
    {
    case D3DSPSM_NONE:
        HSLDPF( E_PIXELSHADER_DATA, "\tInputModifier:  None." );
        modifiers = 0;
        break;
    case D3DSPSM_NEG:
        HSLDPF( E_PIXELSHADER_DATA, "\tInputModifier:  Negation." );
        modifiers = PP_PIXSHADER_IX_C0__COLOR_NEG_ARG_A__NEG;
        break;
    case D3DSPSM_BIAS:
        HSLDPF( E_PIXELSHADER_DATA, "\tInputModifier:  Bias." );
        modifiers = PP_PIXSHADER_IX_C0__COLOR_BIAS_ARG_A__BIAS;
        break;
    case D3DSPSM_BIASNEG:
        HSLDPF( E_PIXELSHADER_DATA, "\tInputModifier:  BiasNeg." );
        modifiers = PP_PIXSHADER_IX_C0__COLOR_BIAS_ARG_A__BIAS | PP_PIXSHADER_IX_C0__COLOR_NEG_ARG_A__NEG;
        break;
    case D3DSPSM_SIGN:
        HSLDPF( E_PIXELSHADER_DATA, "\tInputModifier:  Sign." );
        modifiers = PP_PIXSHADER_IX_C0__COLOR_BIAS_ARG_A__BIAS | PP_PIXSHADER_IX_C0__COLOR_SCALE_ARG_A__SCALE;
        break;
    case D3DSPSM_SIGNNEG:
        HSLDPF( E_PIXELSHADER_DATA, "\tInputModifier:  SignNeg." );
        modifiers = PP_PIXSHADER_IX_C0__COLOR_BIAS_ARG_A__BIAS | PP_PIXSHADER_IX_C0__COLOR_SCALE_ARG_A__SCALE | PP_PIXSHADER_IX_C0__COLOR_NEG_ARG_A__NEG;
        break;
    case D3DSPSM_COMP:
        HSLDPF( E_PIXELSHADER_DATA, "\tInputModifier:  Complement." );
        modifiers = PP_PIXSHADER_IX_C0__COLOR_COMP_ARG_A__COMP;
        break;
    case D3DSPSM_X2:
        HSLDPF( E_PIXELSHADER_DATA, "\tInputModifier:  X2." );
        modifiers = PP_PIXSHADER_IX_C0__COLOR_SCALE_ARG_A__SCALE;
        break;
    case D3DSPSM_X2NEG:
        HSLDPF( E_PIXELSHADER_DATA, "\tInputModifier:  X2Neg." );
        modifiers = PP_PIXSHADER_IX_C0__COLOR_SCALE_ARG_A__SCALE | PP_PIXSHADER_IX_C0__COLOR_NEG_ARG_A__NEG;
        break;
    case D3DSPSM_DZ:
        HSLDPF( E_PIXELSHADER_DATA, "\tInputModifier:  DZ." );
        modifiers = 0;
        break;
    case D3DSPSM_DW:
        HSLDPF( E_PIXELSHADER_DATA, "\tInputModifier:  DW." );
        modifiers = 0;
        break;
    }
    HSLDPF( E_GENERAL_ENTRY_EXIT, "InputModifiers: Exit" );

    return modifiers;
}

static DWORD MapD3DInputRegisterToHW( LPD3DPSREGISTERS lpD3DPSRegs, DWORD input )
{
    DWORD d3dRegNum = D3DSI_GETREGNUM( input );
    DWORD d3dRegType = D3DSI_GETREGTYPE( input );
    DWORD map = 0;

    HSLDPF( E_GENERAL_ENTRY_EXIT, "MapD3DInputRegisterToHW: Entry" );
    switch ( d3dRegType )
    {
    case D3DSPR_TEMP:
        HSLASSERT( d3dRegNum == 0 || d3dRegNum == 1 );
        map = lpD3DPSRegs->dwTempRegs[d3dRegNum] * 2 + 10;
        HSLDPF( E_PIXELSHADER_DATA, "\tRegister Select: r%d=%d", d3dRegNum, map );
        break;
    case D3DSPR_INPUT:
        HSLASSERT( d3dRegNum == 0 || d3dRegNum == 1 );
        map = d3dRegNum * 2 + 4;
        HSLDPF( E_PIXELSHADER_DATA, "\tRegister Select: v%d=%d", d3dRegNum, map );
        break;
    case D3DSPR_TEXTURE:
        HSLASSERT( d3dRegNum <= 5 );
        map = lpD3DPSRegs->dwTexRegs[d3dRegNum] * 2 + 10;
        HSLDPF( E_PIXELSHADER_DATA, "\tRegister Select: t%d=%d", d3dRegNum, map );
        break;
    default:
        HSLDPF( E_ERROR_MESSAGE, "MapD3DInputRegisterToHW: unknown register type" );
        break;
    }
    HSLDPF( E_GENERAL_ENTRY_EXIT, "MapD3DInputRegisterToHW: Exit" );

    return map;
}

static void InputSwizzle( DWORD dwSrc, DWORD* lpReplicateRGB, DWORD* lpReplicateAlpha, DWORD* lpReplicateBlue )
{
    HSLDPF( E_GENERAL_ENTRY_EXIT, "InputSwizzle: Entry" );

    *lpReplicateRGB = 0;
    *lpReplicateAlpha = 0;
    *lpReplicateBlue = 0;

    switch ( dwSrc & D3DSP_SWIZZLE_MASK )
    {
    case D3DSP_REPLICATERED:
        HSLDPF( E_PIXELSHADER_DATA, "\tInput Swizzle: RRRR" );
        *lpReplicateRGB = 1;
        break;
    case D3DSP_REPLICATEGREEN:
        HSLDPF( E_PIXELSHADER_DATA, "\tInput Swizzle: GGGG" );
        *lpReplicateRGB = 2;
        break;
    case D3DSP_REPLICATEBLUE:
        HSLDPF( E_PIXELSHADER_DATA, "\tInput Swizzle: BBBB" );
        *lpReplicateRGB = 3;
        *lpReplicateBlue = 1;
        break;
    case D3DSP_NOSWIZZLE:
        HSLDPF( E_PIXELSHADER_DATA, "\tInput Swizzle: None" );
        break;
    case D3DSP_REPLICATEALPHA:
        HSLDPF( E_PIXELSHADER_DATA, "\tInput Swizzle: Replicate Alpha" );
        *lpReplicateAlpha = 1;
        break;
    default:
        HSLDPF( E_ERROR_MESSAGE, "InputSwizzle: ERROR.  unhandled src operand swizzle (0x%x) encountered.", dwSrc & D3DSP_SWIZZLE_MASK );
        break;
    }
    HSLDPF( E_GENERAL_ENTRY_EXIT, "InputSwizzle: Exit" );
}

static void SetPSColorArg( struct _ATID3DCNTX * pCtxt,
                           LPRAD2PIXELSHADER lpPS, DWORD dwNum, DWORD dwArg,
                           DWORD* lpFactorsUsed, LPPSINSTRUCTIONGROUP lpPSInstrGrp,
                           LPD3DPSREGISTERS lpD3DPSRegs, BOOL bIsLegacyShader )
{
    DWORD d3dRegNum = D3DSI_GETREGNUM( dwArg );
    DWORD d3dRegType = D3DSI_GETREGTYPE( dwArg );
    DWORD dwReplicateRGB = 0;
    DWORD dwReplicateAlpha = 0;
    DWORD dwReplicateBlue = 0;
    DWORD dwArgument = 0;
    DWORD dwInputMods = 0;

    HSLDPF( E_GENERAL_ENTRY_EXIT, "SetPSColorArg: Entry" );

    dwInputMods = InputModifiers( dwArg );
    InputSwizzle( dwArg, &dwReplicateRGB, &dwReplicateAlpha, &dwReplicateBlue );
    if ( d3dRegType == D3DSPR_CONST )
    {
        dwArgument = 8;
        switch ( d3dRegNum )
        {
        case ATI_PS_BLACK_REGNUM:
            if ( dwReplicateAlpha )
            {
                if ( dwInputMods & PP_PIXSHADER_IX_C0__COLOR_COMP_ARG_A__COMP )
                    dwInputMods &= ~PP_PIXSHADER_IX_C0__COLOR_COMP_ARG_A__COMP;
                else
                    dwInputMods |= PP_PIXSHADER_IX_C0__COLOR_COMP_ARG_A__COMP;
            }
            dwReplicateRGB = 0;
            dwReplicateAlpha = 0;
            dwArgument = 0;
            break;
        case ATI_PS_ZERO_REGNUM:
            dwReplicateRGB = 0;
            dwReplicateAlpha = 0;
            dwArgument = 0;
            break;
        case ATI_PS_MINUSONE_REGNUM:
            HSLASSERT( dwInputMods == 0 );
            dwReplicateRGB = 0;
            dwReplicateAlpha = 0;
            dwArgument = 0;
            dwInputMods = PP_PIXSHADER_IX_C0__COLOR_COMP_ARG_A__COMP | PP_PIXSHADER_IX_C0__COLOR_NEG_ARG_A__NEG;
            break;
        default:
            DWORD dwFactor = d3dRegNum;
            if ( d3dRegNum == ATI_PS_ROTMAT0_REGNUM )
                dwFactor = 8;
            if ( d3dRegNum == ATI_PS_ROTMAT1_REGNUM )
                dwFactor = 9;
            if ( *lpFactorsUsed )
            {
                HSLASSERT( *lpFactorsUsed < 2 );
                dwArgument = 26;
                dwFactor <<= 4;
            }
            *lpFactorsUsed += 1;
            lpPSInstrGrp->C1 |= dwFactor;
            // TODO
            if ( d3dRegNum != ATI_PS_ROTMAT0_REGNUM && d3dRegNum != ATI_PS_ROTMAT1_REGNUM && bIsLegacyShader == FALSE )
            {
                if ( dwInputMods & PP_PIXSHADER_IX_C0__COLOR_SCALE_ARG_A__SCALE )
                    HSLDPF( E_ERROR_MESSAGE, "SetPSColorArg: Unsupported Constant Modifier: SCALE" );
                if ( dwInputMods & PP_PIXSHADER_IX_C0__COLOR_BIAS_ARG_A__BIAS )
                    HSLDPF( E_ERROR_MESSAGE, "SetPSColorArg: Unsupported Constant Modifier: BIAS" );
                if ( dwInputMods & PP_PIXSHADER_IX_C0__COLOR_COMP_ARG_A__COMP )
                    dwInputMods |= PP_PIXSHADER_IX_C0__COLOR_SCALE_ARG_A__SCALE;
                else
                    dwInputMods |= PP_PIXSHADER_IX_C0__COLOR_BIAS_ARG_A__BIAS | PP_PIXSHADER_IX_C0__COLOR_SCALE_ARG_A__SCALE;
            }
            break;
        }
    }
    else
    {
        dwArgument = MapD3DInputRegisterToHW( lpD3DPSRegs, dwArg );
    }

    dwArgument += dwReplicateAlpha;

    switch ( dwNum )
    {
    case 0:
        lpPSInstrGrp->C0 |= ( dwInputMods | ( dwArgument ) * 1 ) << 0;
        lpPSInstrGrp->C1 |= dwReplicateRGB << PP_PIXSHADER_I0_C1__REPLICATE_ARG_A__SHIFT;
        if ( lpPS->iState.eCurrPhase == PS_BLENDING_PASS )
            PS_SetColorArgumentOverride( lpPS, lpPS->dwInstrCount[PS_BLENDING_PASS], dwArgument, 0x00010000, 0x000F001F );
        break;
    case 1:
        lpPSInstrGrp->C0 |= ( dwInputMods | ( dwArgument ) * 2 ) << 4;
        lpPSInstrGrp->C1 |= dwReplicateRGB << PP_PIXSHADER_I0_C1__REPLICATE_ARG_B__SHIFT;
        if ( lpPS->iState.eCurrPhase == PS_BLENDING_PASS )
            PS_SetColorArgumentOverride( lpPS, lpPS->dwInstrCount[PS_BLENDING_PASS], dwArgument, 0x00100000, 0x00F003E0 );
        break;
    case 2:
        lpPSInstrGrp->C0 |= ( dwInputMods | ( dwArgument ) * 4 ) << 8;
        lpPSInstrGrp->C1 |= dwReplicateRGB << PP_PIXSHADER_I0_C1__REPLICATE_ARG_C__SHIFT;
        if ( lpPS->iState.eCurrPhase == PS_BLENDING_PASS )
            PS_SetColorArgumentOverride( lpPS, lpPS->dwInstrCount[PS_BLENDING_PASS], dwArgument, 0x01000000, 0x0F007C00 );
        break;
    }

    HSLDPF( E_GENERAL_ENTRY_EXIT, "SetPSColorArg: Exit" );
}

static void SetPSAlphaArg( struct _ATID3DCNTX * pCtxt,
                           LPRAD2PIXELSHADER lpPS, DWORD dwNum, DWORD dwArg,
                           DWORD* lpFactorsUsed, LPPSINSTRUCTIONGROUP lpPSInstrGrp,
                           LPD3DPSREGISTERS lpD3DPSRegs, BOOL bIsLegacyShader )
{
    DWORD d3dRegNum = D3DSI_GETREGNUM( dwArg );
    DWORD d3dRegType = D3DSI_GETREGTYPE( dwArg );
    DWORD dwReplicateRGB = 0;
    DWORD dwReplicateAlpha = 0;
    DWORD dwReplicateBlue = 0;
    DWORD dwArgument = 0;
    DWORD dwInputMods = 0;

    HSLDPF( E_GENERAL_ENTRY_EXIT, "SetPSAlphaArg: Entry" );

    dwInputMods = InputModifiers( dwArg );
    InputSwizzle( dwArg, &dwReplicateRGB, &dwReplicateAlpha, &dwReplicateBlue );
    if ( d3dRegType == D3DSPR_CONST )
    {
        dwArgument = 8;
        switch ( d3dRegNum )
        {
        case ATI_PS_BLACK_REGNUM:
            if ( dwInputMods & PP_PIXSHADER_IX_A0__ALPHA_COMP_ARG_A__COMP )
                dwInputMods &= ~PP_PIXSHADER_IX_A0__ALPHA_COMP_ARG_A__COMP;
            else
                dwInputMods |= PP_PIXSHADER_IX_A0__ALPHA_COMP_ARG_A__COMP;
            dwReplicateRGB = 0;
            dwReplicateBlue = 0;
            dwArgument = 0;
            break;
        case ATI_PS_ZERO_REGNUM:
            dwReplicateRGB = 0;
            dwReplicateBlue = 0;
            dwArgument = 0;
            break;
        case ATI_PS_MINUSONE_REGNUM:
            HSLASSERT( dwInputMods == 0 );
            dwReplicateRGB = 0;
            dwReplicateBlue = 0;
            dwArgument = 0;
            dwInputMods = PP_PIXSHADER_IX_A0__ALPHA_COMP_ARG_A__COMP | PP_PIXSHADER_IX_A0__ALPHA_NEG_ARG_A__NEG;
            break;
        default:
            DWORD dwFactor = d3dRegNum;
            if ( d3dRegNum == ATI_PS_ROTMAT0_REGNUM )
                dwFactor = 8;
            if ( d3dRegNum == ATI_PS_ROTMAT1_REGNUM )
                dwFactor = 9;
            if ( *lpFactorsUsed )
            {
                HSLASSERT( *lpFactorsUsed < 2 );
                dwArgument = 26;
                dwFactor <<= 4;
            }
            *lpFactorsUsed += 1;
            lpPSInstrGrp->A1 |= dwFactor;
            // TODO
            if ( d3dRegNum != ATI_PS_ROTMAT0_REGNUM && d3dRegNum != ATI_PS_ROTMAT1_REGNUM && bIsLegacyShader == FALSE )
            {
                if ( dwInputMods & PP_PIXSHADER_IX_A0__ALPHA_SCALE_ARG_A__SCALE )
                    HSLDPF( E_ERROR_MESSAGE, "SetPSAlphaArg: Unsupported Constant Modifier: SCALE" );
                if ( dwInputMods & PP_PIXSHADER_IX_A0__ALPHA_BIAS_ARG_A__BIAS )
                    HSLDPF( E_ERROR_MESSAGE, "SetPSAlphaArg: Unsupported Constant Modifier: BIAS" );
                if ( dwInputMods & PP_PIXSHADER_IX_A0__ALPHA_COMP_ARG_A__COMP )
                    dwInputMods |= PP_PIXSHADER_IX_A0__ALPHA_SCALE_ARG_A__SCALE;
                else
                    dwInputMods |= PP_PIXSHADER_IX_A0__ALPHA_BIAS_ARG_A__BIAS | PP_PIXSHADER_IX_A0__ALPHA_SCALE_ARG_A__SCALE;
            }
            break;
        }
    }
    else
    {
        dwArgument = MapD3DInputRegisterToHW( lpD3DPSRegs, dwArg );
    }

    dwArgument += dwReplicateBlue;
    if ( dwReplicateBlue )
        dwReplicateRGB = 0;

    switch ( dwNum )
    {
    case 0:
        lpPSInstrGrp->A0 |= ( dwInputMods | ( dwArgument ) * 1 ) << 0;
        lpPSInstrGrp->A1 |= dwReplicateRGB << PP_PIXSHADER_I0_A1__REPLICATE_ARG_A__SHIFT;
        if ( lpPS->iState.eCurrPhase == PS_BLENDING_PASS )
            PS_SetAlphaArgumentOverride( lpPS, lpPS->dwInstrCount[PS_BLENDING_PASS], dwArgument, 0x00010000, 0x000F001F );
        break;
    case 1:
        lpPSInstrGrp->A0 |= ( dwInputMods | ( dwArgument ) * 2 ) << 4;
        lpPSInstrGrp->A1 |= dwReplicateRGB << PP_PIXSHADER_I0_A1__REPLICATE_ARG_B__SHIFT;
        if ( lpPS->iState.eCurrPhase == PS_BLENDING_PASS )
            PS_SetAlphaArgumentOverride( lpPS, lpPS->dwInstrCount[PS_BLENDING_PASS], dwArgument, 0x00100000, 0x00F003E0 );
        break;
    case 2:
        lpPSInstrGrp->A0 |= ( dwInputMods | ( dwArgument ) * 4 ) << 8;
        lpPSInstrGrp->A1 |= dwReplicateRGB << PP_PIXSHADER_I0_A1__REPLICATE_ARG_C__SHIFT;
        if ( lpPS->iState.eCurrPhase == PS_BLENDING_PASS )
            PS_SetAlphaArgumentOverride( lpPS, lpPS->dwInstrCount[PS_BLENDING_PASS], dwArgument, 0x01000000, 0x0F007C00 );
        break;
    }

    HSLDPF( E_GENERAL_ENTRY_EXIT, "SetPSAlphaArg: Exit" );
}

void set_PS_Instruction( struct _ATID3DCNTX * pCtxt,
                         LPRAD2PIXELSHADER lpPS,
                         LPPSINSTRUCTIONGROUP lpPSInstrGrp, DWORD opType,
                         PS_BLEND_CNTL psBlendCntl, DWORD dwDestReg,
                         DWORD dwDestRotate, DWORD dwArgA, DWORD dwArgB,
                         DWORD dwArgC, LPD3DPSREGISTERS lpD3DPSRegs,
                         BOOL bIsLegacyShader )
{
    DWORD dwFactorsUsed = 0;
    HSLDPF( E_GENERAL_ENTRY_EXIT, "set_PS_Instruction: Entry" );
    if ( lpPSInstrGrp )
    {
        HSLDPF( E_PIXELSHADER_DATA, "set_PS_Instruction: Blend Operation=%s", DBGSTR_PS_BLEND_OPERATION[psBlendCntl >> PP_PIXSHADER_I0_C0__BLEND_CTL__SHIFT] );
        DWORD scale = OutputScale( dwDestReg );
        DWORD clamp = OutputClamp( dwDestReg ) | scale;
        DWORD select = OutputSelect( dwDestReg, lpD3DPSRegs ) | clamp;
        if ( opType & PS_RBG_INSTRUCTION )
        {
            HSLDPF( E_PIXELSHADER_DATA, "set_PS_Instruction: Color Instruction" );
            DWORD mask = OutputMask( dwDestReg ) | select;
            DWORD rotate = OutputRotate( dwDestRotate ) | mask;
            lpPSInstrGrp->C0 = psBlendCntl;
            lpPSInstrGrp->C1 = rotate;
            HSLDPF( E_PIXELSHADER_DATA, "set_PS_Instruction: Color ArgA" );
            SetPSColorArg( pCtxt, lpPS, 0, dwArgA, &dwFactorsUsed, lpPSInstrGrp, lpD3DPSRegs, bIsLegacyShader );
            HSLDPF( E_PIXELSHADER_DATA, "set_PS_Instruction: Color ArgB" );
            SetPSColorArg( pCtxt, lpPS, 1, dwArgB, &dwFactorsUsed, lpPSInstrGrp, lpD3DPSRegs, bIsLegacyShader );
            if ( psBlendCntl != PSBLEND_DOT3 && psBlendCntl != PSBLEND_DOT4 )
            {
                HSLDPF( E_PIXELSHADER_DATA, "set_PS_Instruction: Color ArgC" );
                SetPSColorArg( pCtxt, lpPS, 2, dwArgC, &dwFactorsUsed, lpPSInstrGrp, lpD3DPSRegs, bIsLegacyShader );
            }
        }
        dwFactorsUsed = 0;
        if ( opType & PS_ALPHA_INSTRUCTION )
        {
            HSLDPF( E_PIXELSHADER_DATA, "set_PS_Instruction: Alpha Instruction" );
            select = select & 0xFC8FFFFF;
            if ( psBlendCntl == PSBLEND_DOT3 || psBlendCntl == PSBLEND_DOT4 || psBlendCntl == PSBLEND_DOT2ADD )
            {
                lpPSInstrGrp->A0 = 0;
                lpPSInstrGrp->A1 = 1 << PP_PIXSHADER_I0_A1__DOT_ALPHA__SHIFT;
            }
            else
            {
                lpPSInstrGrp->A0 = psBlendCntl;
                lpPSInstrGrp->A1 = 0;
            }
            lpPSInstrGrp->A1 |= select;
            HSLDPF( E_PIXELSHADER_DATA, "set_PS_Instruction: Alpha ArgA" );
            SetPSAlphaArg( pCtxt, lpPS, 0, dwArgA, &dwFactorsUsed, lpPSInstrGrp, lpD3DPSRegs, bIsLegacyShader );
            HSLDPF( E_PIXELSHADER_DATA, "set_PS_Instruction: Alpha ArgB" );
            SetPSAlphaArg( pCtxt, lpPS, 1, dwArgB, &dwFactorsUsed, lpPSInstrGrp, lpD3DPSRegs, bIsLegacyShader );
            if ( psBlendCntl != PSBLEND_DOT3 && psBlendCntl != PSBLEND_DOT4 )
            {
                HSLDPF( E_PIXELSHADER_DATA, "set_PS_Instruction: Alpha ArgC" );
                SetPSAlphaArg( pCtxt, lpPS, 2, dwArgC, &dwFactorsUsed, lpPSInstrGrp, lpD3DPSRegs, bIsLegacyShader );
            }
        }
        HSLDPF( E_GENERAL_ENTRY_EXIT, "set_PS_Instruction: Entry" );
    }
}

void set_PS_enable( LPRAD2PIXELSHADER lpPS, E_PS_SHADER_PASS pass, DWORD dwInstrIndx, BOOL value )
{
    HSLDPF( E_PIXELSHADER_DATA, " set_PS_enable: pass=%d  value=%d", pass, value );
    if ( pass == PS_BLENDING_PASS )
    {
        switch ( dwInstrIndx )
        {
        case 0:
            set_PP_CNTL_tex_blend_0_enable( &lpPS->PpCntl_reg, value );
            break;
        case 1:
            set_PP_CNTL_tex_blend_1_enable( &lpPS->PpCntl_reg, value );
            break;
        case 2:
            set_PP_CNTL_tex_blend_2_enable( &lpPS->PpCntl_reg, value );
            break;
        case 3:
            set_PP_CNTL_tex_blend_3_enable( &lpPS->PpCntl_reg, value );
            break;
        case 4:
            set_PP_CNTL_tex_blend_4_enable( &lpPS->PpCntl_reg, value );
            break;
        case 5:
            set_PP_CNTL_tex_blend_5_enable( &lpPS->PpCntl_reg, value );
            break;
        case 6:
            set_PP_CNTL_tex_blend_6_enable( &lpPS->PpCntl_reg, value );
            break;
        case 7:
            set_PP_CNTL_tex_blend_7_enable( &lpPS->PpCntl_reg, value );
            break;
        default:
            HSLDPF( E_ERROR_MESSAGE, "set_PS_enable: BS index out of range" );
            break;
        }
    }
    else
    {
        HSLASSERT( pass == PS_ADDRESS_PASS );
        switch ( dwInstrIndx )
        {
        case 0:
            set_PP_CNTL_X_tex_blend_a0_enable( &lpPS->PpCntlx_reg, value );
            break;
        case 1:
            set_PP_CNTL_X_tex_blend_a1_enable( &lpPS->PpCntlx_reg, value );
            break;
        case 2:
            set_PP_CNTL_X_tex_blend_a2_enable( &lpPS->PpCntlx_reg, value );
            break;
        case 3:
            set_PP_CNTL_X_tex_blend_a3_enable( &lpPS->PpCntlx_reg, value );
            break;
        case 4:
            set_PP_CNTL_X_tex_blend_a4_enable( &lpPS->PpCntlx_reg, value );
            break;
        case 5:
            set_PP_CNTL_X_tex_blend_a5_enable( &lpPS->PpCntlx_reg, value );
            break;
        case 6:
            set_PP_CNTL_X_tex_blend_a6_enable( &lpPS->PpCntlx_reg, value );
            break;
        case 7:
            set_PP_CNTL_X_tex_blend_a7_enable( &lpPS->PpCntlx_reg, value );
            break;
        default:
            HSLDPF( E_ERROR_MESSAGE, "set_PS_enable: AS index out of range" );
            break;
        }
    }
}

DWORD PS_MapD3DTexRegisterToHW( LPRAD2PIXELSHADER lpPS, LPD3DPSREGISTERS lpD3DPSRegisters, DWORD input )
{
    DWORD d3dRegNum = D3DSI_GETREGNUM( input );
    DWORD d3dRegType = D3DSI_GETREGTYPE( input );
    DWORD map = 0;

    switch ( d3dRegType )
    {
    case D3DSPR_TEMP:
        HSLASSERT( d3dRegNum <= 1 );
        map = lpD3DPSRegisters->dwTempRegs[d3dRegNum];
        break;
    case D3DSPR_TEXTURE:
        HSLASSERT( d3dRegNum <= 5 );
        map = lpD3DPSRegisters->dwTexRegs[d3dRegNum];
        break;
    default:
        HSLDPF( E_ERROR_MESSAGE, "PS_MapD3DTexRegisterToHW: invalid register type" );
        break;
    }

    return map;
}

void ASTexBypassEnable( LPRAD2PIXELSHADER lpPS, DWORD unit )
{
    HSLASSERT( lpPS != NULL );
    HSLASSERT( unit < ATI_RAD2_NUM_TEX_UNITS );

    switch ( unit )
    {
    case 0:
        set_PP_TXMULTI_CTL_0_lookup_disable_1( &lpPS->PpTxMultiCntl0_reg, 1 );
        break;
    case 1:
        set_PP_TXMULTI_CTL_1_lookup_disable_1( &lpPS->PpTxMultiCntl1_reg, 1 );
        break;
    case 2:
        set_PP_TXMULTI_CTL_2_lookup_disable_1( &lpPS->PpTxMultiCntl2_reg, 1 );
        break;
    case 3:
        set_PP_TXMULTI_CTL_3_lookup_disable_1( &lpPS->PpTxMultiCntl3_reg, 1 );
        break;
    case 4:
        set_PP_TXMULTI_CTL_4_lookup_disable_1( &lpPS->PpTxMultiCntl4_reg, 1 );
        break;
    case 5:
        set_PP_TXMULTI_CTL_5_lookup_disable_1( &lpPS->PpTxMultiCntl5_reg, 1 );
        break;
    }
}

void LoopbackAddress( LPRAD2PIXELSHADER lpPS, DWORD AS_Unit, DWORD BS_Unit )
{
    HSLASSERT( AS_Unit <= 5 );
    HSLASSERT( BS_Unit <= 5 );
    HSLASSERT( lpPS != NULL );

    set_PP_CNTL_multi_pass_enable( &lpPS->PpCntl_reg, TRUE );
    set_PP_CNTL_X_indirect_regs( &lpPS->PpCntlx_reg, (1 << AS_Unit) | lpPS->PpCntlx_reg.bitfields.INDIRECT_REGS );
    switch ( BS_Unit )
    {
    case 0:
        set_PP_TXMULTI_CTL_0_st_route_2( &lpPS->PpTxMultiCntl0_reg, AS_Unit + 2 );
        break;
    case 1:
        set_PP_TXMULTI_CTL_1_st_route_2( &lpPS->PpTxMultiCntl1_reg, AS_Unit + 2 );
        break;
    case 2:
        set_PP_TXMULTI_CTL_2_st_route_2( &lpPS->PpTxMultiCntl2_reg, AS_Unit + 2 );
        break;
    case 3:
        set_PP_TXMULTI_CTL_3_st_route_2( &lpPS->PpTxMultiCntl3_reg, AS_Unit + 2 );
        break;
    case 4:
        set_PP_TXMULTI_CTL_4_st_route_2( &lpPS->PpTxMultiCntl4_reg, AS_Unit + 2 );
        break;
    case 5:
        set_PP_TXMULTI_CTL_5_st_route_2( &lpPS->PpTxMultiCntl5_reg, AS_Unit + 2 );
        break;
    }
    lpPS->TxModes[PS_BLENDING_PASS][BS_Unit].modes |= 0x41;
}

PS_RESULT Rad2CompileBumpmapInst( struct _ATID3DCNTX *pCtxt,
                                  LPRAD2PIXELSHADER lpPS,
                                  LPD3DPSREGISTERS  lpD3DPSRegs,
                                  DWORD dwInst, DWORD dwDest,
                                  DWORD dwSrc0 )
{
    DWORD dest = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwDest );
    DWORD src0 = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwSrc0 );
    DWORD tssDest = lpPS->dwHWRegToTSSMap[dest];

    HSLASSERT( D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXBEM_LEGACY ||
               D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXBEML_LEGACY ||
               D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXBEM ||
               D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXBEML );
    dwDest = D3DCONVERT_DESTCLAMP( dwDest, ATI_D3DSPDM_WRAP );
    if ( D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXBEM_LEGACY || D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXBEML_LEGACY )
    {
        tssDest--;
    }
    lpPS->bumpping_d3dstage = tssDest;

    set_TxModes( lpPS, src0, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE );
    if ( lpPS->bReuseTexRegs[D3DSI_GETREGNUM(dwSrc0)] == FALSE )
        set_TxModes( lpPS, src0, PS_BLENDING_PASS, ATI_PS_TXMODE_DISABLE );
    set_TxModes( lpPS, dest, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS );
    set_TxModes( lpPS, dest, PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_COORDTYPE_FORCE );

    PS_Compile_DOT2ADD( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION,
                        PS_MOD_WRITEMASK_R(dwDest),
                        D3DSP_NOSWIZZLE,
                        dwSrc0,
                        D3DPSPS(CONST, ATI_PS_ROTMAT0_REGNUM),
                        PS_MOD_SWIZZLE_RRRR(D3DPDTOS(dwDest)),
                        lpD3DPSRegs );
    if ( D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXBEML_LEGACY || D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXBEML )
    {
        DWORD luma = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, D3DPSPS(TEMP, 1) );
        lpPS->dwUseFlags |= ATI_PS_USE_LUMINANCE;
        lpPS->dwLuminanceTexUnit = luma;

        set_TxModes( lpPS, luma, PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FORCE | ATI_PS_TXMODE_COORDTYPE_3D );
        set_PS_enable( lpPS, PS_ADDRESS_PASS, lpPS->dwInstrCount[PS_ADDRESS_PASS], TRUE );
        set_PS_Instruction( pCtxt, lpPS, &(lpPS->psInstructions[PS_ADDRESS_PASS][lpPS->dwInstrCount[PS_ADDRESS_PASS]]),
                            PS_RBG_INSTRUCTION, PSBLEND_DOT2ADD,
                            PS_MOD_WRITEMASK_GB(dwDest), PS_SWIZZLE_ARGA,
                            dwSrc0,
                            D3DPSPS(CONST, ATI_PS_ROTMAT1_REGNUM),
                            PS_MOD_SWIZZLE_GGGG(D3DPDTOS(dwDest)),
                            lpD3DPSRegs,
                            lpPS->bPixelShaderIsLegacy );
        set_PS_Instruction( pCtxt, lpPS, &(lpPS->psInstructions[PS_ADDRESS_PASS][lpPS->dwInstrCount[PS_ADDRESS_PASS]]),
                            PS_ALPHA_INSTRUCTION, PSBLEND_ADD,
                            D3DCONVERT_DESTCLAMP(dwDest, D3DSPDM_SATURATE), D3DSP_NOSWIZZLE,
                            PS_MOD_SWIZZLE_BBBB(dwSrc0),
                            D3DPSPS(CONST, ATI_PS_ROTMAT0_REGNUM),
                            D3DPSPS(CONST, ATI_PS_ROTMAT1_REGNUM),
                            lpD3DPSRegs,
                            lpPS->bPixelShaderIsLegacy );
        lpPS->dwInstrCount[PS_ADDRESS_PASS]++;
        LoopbackAddress( lpPS, dest, luma );

        PS_Compile_MUL( pCtxt, lpPS, PS_BLENDING_PASS, PS_RBG_INSTRUCTION,
                        PS_MOD_WRITEMASK_RGB(dwDest), D3DSP_NOSWIZZLE,
                        PS_MOD_SWIZZLE_BBBB(D3DPSPS(CONST, 1)),
                        dwDest,
                        lpD3DPSRegs );
    }
    else
    {
        PS_Compile_DOT2ADD( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION,
                            PS_MOD_WRITEMASK_GB(dwDest), D3DSP_NOSWIZZLE,
                            dwSrc0,
                            D3DPSPS(CONST, ATI_PS_ROTMAT1_REGNUM),
                            PS_MOD_SWIZZLE_GGGG(D3DPDTOS(dwDest)),
                            lpD3DPSRegs );
    }
    LoopbackAddress( lpPS, dest, dest );

    return PS_OK;
}

PS_RESULT Rad2CompileTexReg2Lookup( struct _ATID3DCNTX * pCtxt,
                                    LPRAD2PIXELSHADER lpPS,
                                    LPD3DPSREGISTERS  lpD3DPSRegs,
                                    DWORD dwInst, DWORD dwDest, DWORD dwSrc0,
                                    DWORD dwOutputRotate )
{
    DWORD dest = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwDest );
    DWORD src0 = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwSrc0 );

    set_TxModes( lpPS, src0, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE );
    if ( lpPS->bReuseTexRegs[D3DSI_GETREGNUM(dwSrc0)] == FALSE )
        set_TxModes( lpPS, src0, PS_BLENDING_PASS, ATI_PS_TXMODE_DISABLE );
    set_TxModes( lpPS, dest, PS_ADDRESS_PASS, ATI_PS_TXMODE_DISABLE );

    if ( D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXREG2RGB )
    {
        PS_Compile_MOV( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RGBA_INSTRUCTION, dwDest, dwOutputRotate, dwSrc0, lpD3DPSRegs );
        set_TxModes( lpPS, dest, PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE );
    }
    else
    {
        // TODO
        HSLASSERT( dwInst == D3DSIO_TEXREG2AR || dwInst == D3DSIO_TEXREG2GB );
        set_TxModes( lpPS, dest, PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_COORDTYPE_FORCE );
        PS_Compile_MOV( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RGBA_INSTRUCTION, PS_MOD_WRITEMASK_RG(dwDest), dwOutputRotate, dwSrc0, lpD3DPSRegs );
    }
    LoopbackAddress( lpPS, dest, dest );

    return PS_OK;
}

PS_RESULT Rad2CompileTexDP3Inst( struct _ATID3DCNTX * pCtxt,
                                 LPRAD2PIXELSHADER lpPS,
                                 LPD3DPSREGISTERS  lpD3DPSRegs,
                                 DWORD dwInst, DWORD dwDest,
                                 DWORD dwSrc0)
{
    DWORD dest = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwDest );
    DWORD src0 = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwSrc0 );

    set_TxModes( lpPS, src0, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE );
    if ( lpPS->bReuseTexRegs[D3DSI_GETREGNUM(dwSrc0)] == FALSE )
        set_TxModes( lpPS, src0, PS_BLENDING_PASS, ATI_PS_TXMODE_DISABLE );
    set_TxModes( lpPS, dest, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FVF );

    if ( D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXDP3TEX )
    {
        set_TxModes( lpPS, dest, PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_COORDTYPE_FORCE | ATI_PS_TXMODE_COORDTYPE_1D );
    }
    else
    {
        // TODO
        HSLASSERT( dwInst == D3DSIO_TEXDP3 );
        set_TxModes( lpPS, dest, PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FORCE | ATI_PS_TXMODE_COORDTYPE_3D );
    }
    PS_Compile_DP3( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RGBA_INSTRUCTION, dwDest, D3DSP_NOSWIZZLE, dwSrc0, D3DPDTOS(dwDest), lpD3DPSRegs );
    LoopbackAddress( lpPS, dest, dest );

    return PS_OK;
}

PS_RESULT Rad2CompileTexm3x2Inst( struct _ATID3DCNTX *pCtxt,
                                  LPRAD2PIXELSHADER lpPS,
                                  LPD3DPSREGISTERS  lpD3DPSRegs,
                                  DWORD dwInst, DWORD dwDest,
                                  DWORD dwSrc0, DWORD dwSrc1,
                                  DWORD dwSrc0Row1,
                                  DWORD dwSrc0Row2)
{
    DWORD dest = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwDest );
    DWORD src0 = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwSrc0 );
    DWORD src1 = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwSrc1 );
    dwDest = D3DCONVERT_DESTCLAMP( dwDest, ATI_D3DSPDM_WRAP );

    PS_Compile_DP3( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION, PS_MOD_WRITEMASK_G(dwDest), D3DSP_NOSWIZZLE, dwSrc0Row2, D3DPDTOS(dwDest), lpD3DPSRegs );
    PS_Compile_DP3( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION, PS_MOD_WRITEMASK_R(dwDest), D3DSP_NOSWIZZLE, dwSrc0Row1, dwSrc1, lpD3DPSRegs );

    set_TxModes( lpPS, src0, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE );
    if ( lpPS->bReuseTexRegs[D3DSI_GETREGNUM(dwSrc0)] == FALSE )
        set_TxModes( lpPS, src0, PS_BLENDING_PASS, ATI_PS_TXMODE_DISABLE );
    set_TxModes( lpPS, src1, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FVF );
    set_TxModes( lpPS, src1, PS_BLENDING_PASS, ATI_PS_TXMODE_DISABLE );

    if ( D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXM3x2TEX )
    {
        set_TxModes( lpPS, dest, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FVF );
        set_TxModes( lpPS, dest, PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE );
    }
    else
    {
        HSLASSERT( D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXM3x2DEPTH );
        lpPS->dwUseFlags |= ATI_PS_USE_TEXDEPTH;
        set_TxModes( lpPS, dest, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FVF );
        set_TxModes( lpPS, dest, PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FORCE | ATI_PS_TXMODE_COORDTYPE_DEPTH );
        PS_Compile_CMP( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION,
                        PS_MOD_WRITEMASK_R(D3DPSPS(TEMP, 1)), D3DSP_NOSWIZZLE,
                        PS_MOD_SWIZZLE_GGGG(D3DPDTOS(dwDest)),
                        D3DPSCREATESRC(CONST, 9, D3DSPSM_COMP, D3DSP_NOSWIZZLE),
                        D3DPDTOS(dwDest),
                        lpD3DPSRegs );
        PS_Compile_CMP( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION,
                        PS_MOD_WRITEMASK_R(dwDest), D3DSP_NOSWIZZLE,
                        PS_MOD_SWIZZLE_GGGG(D3DPDTOS(dwDest) | D3DSPSM_NEG),
                        PS_MOD_SWIZZLE_RRRR(D3DPSPS(TEMP, 1)),
                        D3DPDTOS(dwDest),
                        lpD3DPSRegs );
    }
    LoopbackAddress( lpPS, dest, dest );

    return PS_OK;
}

PS_RESULT Rad2CompileTexm3x3texInst( struct _ATID3DCNTX *pCtxt,
                                     LPRAD2PIXELSHADER lpPS,
                                     LPD3DPSREGISTERS  lpD3DPSRegs,
                                     DWORD dwInst, DWORD dwDest,
                                     DWORD dwSrc0, DWORD dwSrc1,
                                     DWORD dwSrc2, DWORD dwSrc0Row1,
                                     DWORD dwSrc0Row2, DWORD dwSrc0Row3)
{
    DWORD dest = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwDest );
    DWORD src0 = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwSrc0 );
    DWORD src1 = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwSrc1 );
    DWORD src2 = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwSrc2 );
    dwDest = D3DCONVERT_DESTCLAMP( dwDest, ATI_D3DSPDM_WRAP );

    set_TxModes( lpPS, src0, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE );
    if ( lpPS->bReuseTexRegs[D3DSI_GETREGNUM(dwSrc0)] == FALSE )
        set_TxModes( lpPS, src0, PS_BLENDING_PASS, ATI_PS_TXMODE_DISABLE );
    set_TxModes( lpPS, src1, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FVF );
    set_TxModes( lpPS, src1, PS_BLENDING_PASS, ATI_PS_TXMODE_DISABLE );
    set_TxModes( lpPS, src2, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FVF );
    set_TxModes( lpPS, src2, PS_BLENDING_PASS, ATI_PS_TXMODE_DISABLE );
    set_TxModes( lpPS, dest, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FVF );
    if ( D3DSI_GETOPCODE( dwInst ) == D3DSIO_TEXM3x3 )
        set_TxModes( lpPS, dest, PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FORCE | ATI_PS_TXMODE_COORDTYPE_3D );
    else
        set_TxModes( lpPS, dest, PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE );

    PS_Compile_DP3( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION, PS_MOD_WRITEMASK_B(dwDest), D3DSP_NOSWIZZLE, dwSrc0Row3, D3DPDTOS(dwDest), lpD3DPSRegs );
    PS_Compile_DP3( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION, PS_MOD_WRITEMASK_R(dwDest), D3DSP_NOSWIZZLE, dwSrc0Row1, dwSrc1, lpD3DPSRegs );
    PS_Compile_DP3( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION, PS_MOD_WRITEMASK_G(dwDest), D3DSP_NOSWIZZLE, dwSrc0Row2, dwSrc2, lpD3DPSRegs );
    LoopbackAddress( lpPS, dest, dest );

    return PS_OK;
}

PS_RESULT Rad2CompileTexm3x3specInst( struct _ATID3DCNTX *pCtxt,
                                      LPRAD2PIXELSHADER lpPS,
                                      LPD3DPSREGISTERS lpD3DPSRegs,
                                      DWORD dwInst, DWORD dwDest,
                                      DWORD dwSrc0, DWORD dwSrc1,
                                      DWORD dwSrc2, DWORD dwSrc3,
                                      DWORD dwSrc0Row1, DWORD dwSrc0Row2,
                                      DWORD dwSrc0Row3)
{
    dwDest = D3DCONVERT_DESTCLAMP( dwDest, ATI_D3DSPDM_WRAP );
    Rad2CompileTexm3x3texInst( pCtxt, lpPS, lpD3DPSRegs, dwInst, dwDest, dwSrc0, dwSrc2, dwSrc3, dwSrc0Row1, dwSrc0Row2, dwSrc0Row3 );

    if ( D3DSI_GETOPCODE(dwInst) == D3DSIO_TEXM3x3VSPEC )
    {
        DWORD src1 = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwSrc1 );
        lpPS->dwUseFlags |= ATI_PS_USE_VSPEC_EYE;
        lpPS->dwEyeTexUnit = src1;
        set_TxModes( lpPS, src1, PS_ADDRESS_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FORCE | ATI_PS_TXMODE_COORDTYPE_3D );
        set_TxModes( lpPS, src1, PS_BLENDING_PASS, ATI_PS_TXMODE_DISABLE );
        lpPS->dwEyeComponentLocations[0] = D3DSI_GETREGNUM(dwSrc2);
        lpPS->dwEyeComponentLocations[1] = D3DSI_GETREGNUM(dwSrc3);
        lpPS->dwEyeComponentLocations[2] = D3DSI_GETREGNUM(dwDest);
    }

    PS_Compile_DP3( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION, D3DPSTOD(dwSrc3), D3DSP_NOSWIZZLE, dwSrc1, D3DPDTOS(dwDest), lpD3DPSRegs );
    PS_Compile_MUL( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION, D3DPSTOD(dwSrc3), D3DSP_NOSWIZZLE, dwSrc3, D3DPDTOS(dwDest), lpD3DPSRegs );
    PS_Compile_DP3( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION, D3DPSTOD(dwSrc2), D3DSP_NOSWIZZLE, D3DPDTOS(dwDest), D3DPDTOS(dwDest), lpD3DPSRegs );
    PS_Compile_MAD( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION, dwDest, D3DSP_NOSWIZZLE, dwSrc2, dwSrc1 | D3DSPSM_NEG, dwSrc3, lpD3DPSRegs );

    return PS_OK;
}

PS_RESULT Rad2CompilePixShaderInst( struct _ATID3DCNTX *pCtxt, DWORD *pToken,
                                    LPRAD2PIXELSHADER lpPS,
                                    PS_RESULT psLastResult,
                                    LPD3DPSREGISTERS lpD3DPSRegs )
{
    PS_RESULT dwType = PS_RESULT_NONE;
    DWORD dwInst = *pToken++;
    DWORD dwDest = 0;
    DWORD dwSrc0 = 0;
    DWORD dwSrc1 = 0;
    DWORD dwSrc2 = 0;

    HSLASSERT( lpPS != NULL );

    if( PS_IS_ARG(*pToken) )
        dwDest = *pToken++;
    if( PS_IS_ARG(*pToken) )
        dwSrc0 = *pToken++;
    if( PS_IS_ARG(*pToken) )
        dwSrc1 = *pToken++;
    if( PS_IS_ARG(*pToken) )
        dwSrc2 = *pToken++;

    dwType = PS_GetOpType( dwInst, dwDest );
    if ( dwInst & D3DSI_COISSUE )
    {
        lpPS->dwInstrCount[lpPS->iState.eCurrPhase]--;
    }
    if ( lpPS->iState.eCurrPhase == PS_BLENDING_PASS && D3DSI_GETREGTYPE(dwDest) == D3DSPR_TEMP && D3DSI_GETREGNUM(dwDest) == 0 && dwInst != D3DSIO_TEXCOORD && dwInst != D3DSIO_TEX )
    {
        if ( dwType & PS_RBG_INSTRUCTION )
            lpPS->iState.intRGBTerminalIns = lpPS->dwInstrCount[PS_BLENDING_PASS];
        if ( dwType & PS_ALPHA_INSTRUCTION )
            lpPS->iState.intAlphaTerminalIns = lpPS->dwInstrCount[PS_BLENDING_PASS];
    }
    if ( lpPS->dwPixelShaderVersion == PS_1_4 )
    {
        if ( D3DSI_GETREGTYPE(dwSrc0) == D3DSPR_TEMP )
            dwSrc0 = D3DSPRCONVERT( TEXTURE, dwSrc0 );
        if ( D3DSI_GETREGTYPE(dwSrc1) == D3DSPR_TEMP )
            dwSrc1 = D3DSPRCONVERT( TEXTURE, dwSrc1 );
        if ( D3DSI_GETREGTYPE(dwSrc2) == D3DSPR_TEMP )
            dwSrc2 = D3DSPRCONVERT( TEXTURE, dwSrc2 );
        if ( D3DSI_GETREGTYPE(dwDest) == D3DSPR_TEMP )
            dwDest = D3DSPRCONVERT( TEXTURE, dwDest );
    }

    switch ( D3DSI_GETOPCODE(dwInst) )
    {
    case D3DSIO_NOP:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling no-op" );
        return PS_RESULT_NONE;
    case D3DSIO_MOV:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling mov on phase=%d and slot=%d", lpPS->iState.eCurrPhase, PS_GetCurrentInstr_Count(lpPS) );
        PS_Compile_MOV( pCtxt, lpPS, lpPS->iState.eCurrPhase, dwType, dwDest, D3DSP_NOSWIZZLE, dwSrc0, lpD3DPSRegs );
        return PS_RESULT_NONE;
    case D3DSIO_ADD:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling add on phase=%d and slot=%d", lpPS->iState.eCurrPhase, PS_GetCurrentInstr_Count(lpPS) );
        PS_Compile_ADD( pCtxt, lpPS, lpPS->iState.eCurrPhase, dwType, dwDest, D3DSP_NOSWIZZLE, dwSrc0, dwSrc1, lpD3DPSRegs );
        return PS_RESULT_NONE;
    case D3DSIO_SUB:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling sub on phase=%d and slot=%d", lpPS->iState.eCurrPhase, PS_GetCurrentInstr_Count(lpPS) );
        PS_Compile_SUB( pCtxt, lpPS, lpPS->iState.eCurrPhase, dwType, dwDest, D3DSP_NOSWIZZLE, dwSrc0, dwSrc1, lpD3DPSRegs );
        return PS_RESULT_NONE;
    case D3DSIO_MAD:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling mad on phase=%d and slot=%d", lpPS->iState.eCurrPhase, PS_GetCurrentInstr_Count(lpPS) );
        PS_Compile_MAD( pCtxt, lpPS, lpPS->iState.eCurrPhase, dwType, dwDest, D3DSP_NOSWIZZLE, dwSrc0, dwSrc1, dwSrc2, lpD3DPSRegs );
        return PS_RESULT_NONE;
    case D3DSIO_MUL:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling mul on phase=%d and slot=%d", lpPS->iState.eCurrPhase, PS_GetCurrentInstr_Count(lpPS) );
        PS_Compile_MUL( pCtxt, lpPS, lpPS->iState.eCurrPhase, dwType, dwDest, D3DSP_NOSWIZZLE, dwSrc0, dwSrc1, lpD3DPSRegs );
        return PS_RESULT_NONE;
    case D3DSIO_DP3:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling dp3 on phase=%d and slot=%d", lpPS->iState.eCurrPhase, PS_GetCurrentInstr_Count(lpPS) );
        PS_Compile_DP3( pCtxt, lpPS, lpPS->iState.eCurrPhase, dwType, dwDest, D3DSP_NOSWIZZLE, dwSrc0, dwSrc1, lpD3DPSRegs );
        return PS_RESULT_NONE;
    case D3DSIO_DP4:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling dp4 on phase=%d and slot=%d", lpPS->iState.eCurrPhase, PS_GetCurrentInstr_Count(lpPS) );
        PS_Compile_DP4( pCtxt, lpPS, lpPS->iState.eCurrPhase, dwType, dwDest, D3DSP_NOSWIZZLE, dwSrc0, dwSrc1, lpD3DPSRegs );
        return PS_RESULT_NONE;
    case D3DSIO_LRP:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling lrp on phase=%d and slot=%d", lpPS->iState.eCurrPhase, PS_GetCurrentInstr_Count(lpPS) );
        PS_Compile_LRP( pCtxt, lpPS, lpPS->iState.eCurrPhase, dwType, dwDest, D3DSP_NOSWIZZLE, dwSrc0, dwSrc1, dwSrc2, lpD3DPSRegs );
        return PS_RESULT_NONE;
    case D3DSIO_TEXCOORD:
    {
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling texcoord" );
        DWORD unit = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwDest );
        DWORD set = D3DSI_GETREGNUM(dwDest);
        if ( lpPS->dwPixelShaderVersion == PS_1_4 )
        {
            E_PS_SHADER_PASS pass = lpPS->iState.eCurrPhase;
            if ( PS_isTxUnitUsed( lpPS, unit, pass ) == FALSE )
            {
                set_TxModes( lpPS, unit, pass, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FVF );
            }
            lpPS->TxModes[pass][unit].dwTexCoordSet = set;
            if ( (dwSrc0 & D3DSP_SWIZZLE_MASK) == PS_SWIZZLE_RGAA )
                lpPS->dwTexCoordDepthMode[set] = 0x1000;
            switch ( dwSrc0 & D3DSP_SRCMOD_MASK )
            {
            case D3DSPSM_DZ:
            case D3DSPSM_DW:
                lpPS->TxModes[pass][unit].modes |= ATI_PS_TXMODE_TEX_PROJ;
                break;
            }
        }
        else
        {
            set_TxModes( lpPS, unit, lpPS->iState.eCurrPhase, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FVF );
        }
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: register t%d mapped to TexUnit %d", D3DSI_GETREGNUM(dwDest), unit );
        return PS_RESULT_NONE;
    }
    case D3DSIO_TEXKILL:
    {
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling texkill" );
        DWORD unit = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwDest );
        set_TxModes( lpPS, unit, lpPS->iState.eCurrPhase, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_TEXKILL | ATI_PS_TXMODE_COORDTYPE_FORCE | ATI_PS_TXMODE_COORDTYPE_3D );
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: register t%d mapped to TexUnit %d", D3DSI_GETREGNUM(dwDest), unit );
        return PS_RESULT_NONE;
    }
    case D3DSIO_TEX:
    {
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling tex" );
        DWORD unit = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwDest );
        DWORD set = D3DSI_GETREGNUM(dwDest);
        set_TxModes( lpPS, unit, lpPS->iState.eCurrPhase, ATI_PS_TXMODE_ENABLE );
        if ( lpPS->dwPixelShaderVersion == PS_1_4 )
        {
            E_PS_SHADER_PASS pass = lpPS->iState.eCurrPhase;
            if ( pass == PS_BLENDING_PASS && lpPS->iState.BSRegSrc[set] != PS_TEX_NOT_USED )
            {
                set_TxModes( lpPS, unit, PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_COORDTYPE_LOOPBACK );
            }
            lpPS->TxModes[pass][unit].dwTexCoordSet = set;
            if ( (dwSrc0 & D3DSP_SWIZZLE_MASK) == PS_SWIZZLE_RGAA )
                lpPS->dwTexCoordDepthMode[set] = 0x1000;
            switch ( dwSrc0 & D3DSP_SRCMOD_MASK )
            {
            case D3DSPSM_DZ:
            case D3DSPSM_DW:
                lpPS->TxModes[pass][unit].modes |= ATI_PS_TXMODE_TEX_PROJ;
                break;
            }
        }
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: register t%d mapped to TexUnit %d", D3DSI_GETREGNUM(dwDest), unit );
        return PS_RESULT_NONE;
    }
    case D3DSIO_TEXBEM:
    case D3DSIO_TEXBEML:
    case D3DSIO_TEXBEM_LEGACY:
    case D3DSIO_TEXBEML_LEGACY:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling texbem(l) on ADDR_slots %d, %d", PS_GetAddrInstr_Count(lpPS), PS_GetAddrInstr_Count(lpPS) + 1 );
        Rad2CompileBumpmapInst( pCtxt, lpPS, lpD3DPSRegs, dwInst, dwDest, dwSrc0 );
        return PS_RESULT_NONE;
    case D3DSIO_TEXREG2AR:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling texreg2ar" );
        Rad2CompileTexReg2Lookup( pCtxt, lpPS, lpD3DPSRegs, dwInst, dwDest, dwSrc0, PS_SWIZZLE_AARG );
        return PS_RESULT_NONE;
    case D3DSIO_TEXREG2GB:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling texreg2gb" );
        Rad2CompileTexReg2Lookup( pCtxt, lpPS, lpD3DPSRegs, dwInst, dwDest, dwSrc0, PS_SWIZZLE_AGBA );
        return PS_RESULT_NONE;
    case D3DSIO_TEXM3x3PAD:
        lpPS->seqState.dwSrc0[lpPS->seqState.dwNumInst] = dwSrc0;
        lpPS->seqState.dwDest[lpPS->seqState.dwNumInst] = dwDest;
        lpPS->seqState.dwNumInst++;
        HSLASSERT( lpPS->seqState.dwNumInst < PS_MAX_TEXSEQ );
        return PS_RESULT_NONE;
    case D3DSIO_TEXM3x2TEX:
    case D3DSIO_TEXM3x2DEPTH:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling texm3x2tex" );
        HSLASSERT( lpPS->seqState.dwNumInst == 1 );
        Rad2CompileTexm3x2Inst( pCtxt, lpPS, lpD3DPSRegs,
                                dwInst,
                                dwDest,
                                dwSrc0,
                                D3DPDTOS(lpPS->seqState.dwDest[0]),
                                lpPS->seqState.dwSrc0[0],
                                dwSrc0 );
        break;
    case D3DSIO_TEXM3x3TEX:
    case D3DSIO_TEXM3x3:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling texm3x3tex or texm3x3" );
        HSLASSERT( lpPS->seqState.dwNumInst == 2 );
        Rad2CompileTexm3x3texInst( pCtxt, lpPS, lpD3DPSRegs,
                                   dwInst,
                                   dwDest,
                                   dwSrc0,
                                   D3DPDTOS(lpPS->seqState.dwDest[0]),
                                   D3DPDTOS(lpPS->seqState.dwDest[1]),
                                   lpPS->seqState.dwSrc0[0],
                                   lpPS->seqState.dwSrc0[1],
                                   dwSrc0 );
        break;
    case D3DSIO_TEXM3x3SPEC:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling texm3x3spec" );
        HSLASSERT( lpPS->seqState.dwNumInst == 2 );
        Rad2CompileTexm3x3specInst( pCtxt, lpPS, lpD3DPSRegs,
                                    dwInst,
                                    dwDest,
                                    dwSrc0,
                                    dwSrc1,
                                    D3DPDTOS(lpPS->seqState.dwDest[0]),
                                    D3DPDTOS(lpPS->seqState.dwDest[1]),
                                    lpPS->seqState.dwSrc0[0],
                                    lpPS->seqState.dwSrc0[1],
                                    dwSrc0 );
        break;
    case D3DSIO_TEXM3x3VSPEC:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling texm3x3vspec" );
        HSLASSERT( lpPS->seqState.dwNumInst == 2 );
        Rad2CompileTexm3x3specInst( pCtxt, lpPS, lpD3DPSRegs,
                                    dwInst,
                                    dwDest,
                                    dwSrc0,
                                    D3DPSPS(TEMP, 1),
                                    D3DPDTOS(lpPS->seqState.dwDest[0]),
                                    D3DPDTOS(lpPS->seqState.dwDest[1]),
                                    lpPS->seqState.dwSrc0[0],
                                    lpPS->seqState.dwSrc0[1],
                                    dwSrc0 );
        break;
    case D3DSIO_CND:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling cnd on phase=%d and slot=%d", lpPS->iState.eCurrPhase, PS_GetCurrentInstr_Count(lpPS) );
        PS_Compile_CND( pCtxt, lpPS, lpPS->iState.eCurrPhase, dwType, dwDest, D3DSP_NOSWIZZLE, dwSrc0, dwSrc1, dwSrc2, lpD3DPSRegs );
        return PS_RESULT_NONE;
    case D3DSIO_DEF:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling def" );
        HSLASSERT( D3DSI_GETREGTYPE(dwDest) == D3DSPR_CONST );
        HSLASSERT( D3DSI_GETREGNUM(dwDest) < ATI_RAD2_NUM_PS_CONSTS );
        // TODO
        lpPS->dwPSDirtyFlags |= ( 1 << dwDest );
        return PS_RESULT_NONE;
    case D3DSIO_TEXREG2RGB:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling texreg2rgb" );
        Rad2CompileTexReg2Lookup( pCtxt, lpPS, lpD3DPSRegs, dwInst, dwDest, dwSrc0, D3DSP_NOSWIZZLE );
        return PS_RESULT_NONE;
    case D3DSIO_TEXDP3TEX:
    case D3DSIO_TEXDP3:
        Rad2CompileTexDP3Inst( pCtxt, lpPS, lpD3DPSRegs, dwInst, dwDest, dwSrc0 );
        return PS_RESULT_NONE;
    case D3DSIO_TEXDEPTH:
    {
        DWORD unit = PS_MapD3DTexRegisterToHW( lpPS, lpD3DPSRegs, dwDest );
        set_TxModes( lpPS, unit, lpPS->iState.eCurrPhase, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FORCE | ATI_PS_TXMODE_COORDTYPE_DEPTH );
        lpPS->dwUseFlags |= ATI_PS_USE_TEXDEPTH;
        return PS_RESULT_NONE;
    }
    case D3DSIO_CMP:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling cmp on phase=%d and slot=%d", lpPS->iState.eCurrPhase, PS_GetCurrentInstr_Count(lpPS) );
        PS_Compile_CMP( pCtxt, lpPS, lpPS->iState.eCurrPhase, dwType, dwDest, D3DSP_NOSWIZZLE, dwSrc0, dwSrc1, dwSrc2, lpD3DPSRegs );
        return PS_RESULT_NONE;
    case D3DSIO_BEM:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Compiling tex on ADDR_slots %d, %d", PS_GetAddrInstr_Count(lpPS), PS_GetAddrInstr_Count(lpPS) + 1 );
        PS_Compile_DOT2ADD( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION,
                            PS_MOD_WRITEMASK_R(dwDest),
                            D3DSP_NOSWIZZLE,
                            dwSrc1,
                            D3DPSPS(CONST, ATI_PS_ROTMAT0_REGNUM),
                            PS_MOD_SWIZZLE_RRRR(dwSrc0),
                            lpD3DPSRegs );
        PS_Compile_DOT2ADD( pCtxt, lpPS, PS_ADDRESS_PASS, PS_RBG_INSTRUCTION,
                            PS_MOD_WRITEMASK_GB(dwDest),
                            D3DSP_NOSWIZZLE,
                            dwSrc1,
                            D3DPSPS(CONST, ATI_PS_ROTMAT1_REGNUM),
                            PS_MOD_SWIZZLE_GGGG(dwSrc0),
                            lpD3DPSRegs );
        return PS_RESULT_NONE;
    case D3DSIO_PHASE:
        HSLDPF( E_PIXELSHADER_DATA, "...Rad2: Processing PHASE Token" );
        for ( DWORD i = 0; i < ATI_RAD2_NUM_TEX_UNITS; ++i )
        {
            DWORD unit = lpPS->iState.BSRegSrc[i];
            if ( unit != PS_TEX_NOT_USED )
            {
                set_TxModes( lpPS, lpD3DPSRegs->dwTexRegs[i], PS_BLENDING_PASS, ATI_PS_TXMODE_ENABLE | ATI_PS_TXMODE_BYPASS | ATI_PS_TXMODE_COORDTYPE_FORCE | ATI_PS_TXMODE_COORDTYPE_3D );
                LoopbackAddress( lpPS, lpD3DPSRegs->dwTexRegs[unit], lpD3DPSRegs->dwTexRegs[i] );
            }
        }
        lpPS->iState.eCurrPhase = PS_BLENDING_PASS;
        return PS_RESULT_NONE;
    default:
        HSLDPF( E_ERROR_MESSAGE, "Rad2: unsupported pixel shader instruction encountered" );
        return PS_RESULT_NONE;
    }
    lpPS->seqState = {};
    return PS_RESULT_NONE;
}
