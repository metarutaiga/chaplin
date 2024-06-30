

//    
//  Workfile: hwVertexShader.c
//
//
//  Trade secret of ATI Technologies, Inc.
//  Copyright 2000, ATI Technologies, Inc., (unpublished)
//
//  All rights reserved.
//  The year included in the foregoing notice is the year of creation of the work.
//
//
//  This file contains hardware-dependent code for vertex shaders. This
//  code presently compiles for both Radeon 100 and Radeon 200 with some
//  #if (RADEON_ASIC ...) check in some of the inline routines. This file
//  accompanies hwVertexShader.c. hwVertexShader.c does not contain any
//  references to the preprocessor define RADEON_ASIC.
//  
//  All device independent vertex shader code is in VertexShader.c and 
//  VertexShader.h.

#include "inc/hwvertexshader.h"

void VS_HwAssembleDstOp(DWORD* pdwOutDstOp, DWORD dwAtiOp, DWORD dwDxToken, DWORD *pdwOutVtxEncoding, BOOL* pbPosUpdated, DWORD dwWriteMaskOverride)
{
    HSLDPF( E_GENERAL_ENTRY_EXIT, "VS_HwAssembleDstOp: Entry" );

    DWORD d3dRegNum = D3DSI_GETREGNUM( dwDxToken );
    DWORD d3dRegType = D3DSI_GETREGTYPE( dwDxToken );
    DWORD d3dRegWriteMask = dwWriteMaskOverride ? ( dwWriteMaskOverride * D3DSP_WRITEMASK_0 ) : D3DSI_GETWRITEMASK( dwDxToken );

    DWORD dwRegType = ATI_PVS_REG_TEMP;
    DWORD dwRegOffset = d3dRegNum;
    BOOL bPosUpdated = FALSE;
    switch ( d3dRegType )
    {
    case D3DSPR_TEMP:
        break;
    case D3DSPR_ADDR:
        dwRegType = ATI_PVS_REG_A0;
        d3dRegWriteMask = D3DSP_WRITEMASK_ALL;
        break;
    case D3DSPR_RASTOUT:
        dwRegOffset = 0;
        switch ( d3dRegNum )
        {
        case D3DSRO_POSITION:
            bPosUpdated = TRUE;
            dwRegType = ATI_PVS_REG_OUT_POS;
            *pdwOutVtxEncoding |= ATI_PVS_OUT_POS;
            break;
        case D3DSRO_FOG:
            dwRegType = ATI_PVS_REG_OUT_FOG;
            *pdwOutVtxEncoding |= ATI_PVS_OUT_DISCRETE_FOG;
            break;
        case D3DSRO_POINT_SIZE:
            dwRegType = ATI_PVS_REG_OUT_PNT_SIZE;
            *pdwOutVtxEncoding |= ATI_PVS_OUT_POINT_SIZE;
            break;
        default:
            dwRegType = ATI_PVS_REG_OUT_POS;
            *pdwOutVtxEncoding |= ATI_PVS_OUT_POS;
            break;
        }
        break;
    case D3DSPR_ATTROUT:
        dwRegType = ATI_PVS_REG_OUT_CLR;
        *pdwOutVtxEncoding |= d3dRegNum * ATI_PVS_OUT_COLOR_0;
        break;
    case D3DSPR_TEXCRDOUT:
        dwRegType = ATI_PVS_REG_OUT_TEX;
        *pdwOutVtxEncoding |= d3dRegNum * ATI_PVS_OUT_TEX_COORD_0;
        if ( d3dRegNum > 5 )
            dwAtiOp = 0;
        break;
    default:
        dwRegType = ATI_PVS_REG_TEMP;
        break;
    }

    DWORD dwWriteEna = 0;
    if ( d3dRegWriteMask == D3DSP_WRITEMASK_ALL )
    {
        dwWriteEna = ATI_PVS_DST_WRITE_ENA_ALL;
    }
    else
    {
        if ( d3dRegWriteMask & D3DSP_WRITEMASK_0 )
            dwWriteEna |= ATI_PVS_DST_WRITE_ENA_X;
        if ( d3dRegWriteMask & D3DSP_WRITEMASK_1 )
            dwWriteEna |= ATI_PVS_DST_WRITE_ENA_Y;
        if ( d3dRegWriteMask & D3DSP_WRITEMASK_2 )
            dwWriteEna |= ATI_PVS_DST_WRITE_ENA_Z;
        if ( d3dRegWriteMask & D3DSP_WRITEMASK_3 )
            dwWriteEna |= ATI_PVS_DST_WRITE_ENA_W;
    }

    *pdwOutDstOp = ATI_PVS_MAKE_DST_OP( dwAtiOp, dwRegType, dwRegOffset, dwWriteEna );
    *pbPosUpdated = bPosUpdated;

    HSLDPF( E_GENERAL_ENTRY_EXIT, "VS_HwAssembleDstOp: Exit" );
}

void VS_HwAssembleSrcOp(struct _ATID3DCNTX* pCtxt, DWORD* pdwOutSrcOp, DWORD dwDxToken, DWORD dwSrcFlags, DWORD* pdwMaxConst, DWORD dwAddrSel, DWORD* pdwVecUsed, DWORD dwSrcMod, DWORD dwSwizzleOverride)
{
    HSLDPF( E_GENERAL_ENTRY_EXIT, "VS_HwAssembleSrcOp: Entry" );

    DWORD d3dRegType = D3DSI_GETREGTYPE( dwDxToken );
    DWORD d3dRegSwizzle = D3DVS_GETSWIZZLE( dwDxToken );
    DWORD d3dRegNum = D3DSI_GETREGNUM( dwDxToken );

    DWORD dwAddrMode = 0;
    DWORD dwRegType = ATI_PVS_REG_TEMP;
    DWORD dwRegOffset = d3dRegNum;
    switch ( d3dRegType )
    {
    case D3DSPR_TEMP:
        dwRegType = ATI_PVS_REG_TEMP;
        break;
    case D3DSPR_INPUT:
        dwRegType = ATI_PVS_REG_IN;
        *pdwVecUsed |= 1 << dwRegOffset;
        break;
    case D3DSPR_CONST:
        dwRegType = ATI_PVS_REG_CONST;
        if ( dwDxToken & 0x2000 )
        {
            dwAddrMode = ATI_PVS_SRC_ADDR_MODE;
            // TODO
        }
        else
        {
            dwAddrMode = 0;
            if ( *pdwMaxConst < dwRegOffset )
                *pdwMaxConst = dwRegOffset;
        }
        break;
    case D3DSPR_ADDR:
        dwRegType = ATI_PVS_REG_A0;
        break;
    default:
        dwRegType = ATI_PVS_REG_TEMP;
        break;
    }

    DWORD dwSwizzle = 0;
    if ( dwSwizzleOverride )
    {
        dwSwizzle = dwSwizzleOverride << ATI_PVS_SRC_SWIZZLE_SHIFT;
    }
    else if ( d3dRegSwizzle == D3DSP_NOSWIZZLE )
    {
        dwSwizzle = ATI_PVS_SRC_NO_SWIZZLE;
    }
    else
    {
        for ( DWORD i = 0; i < 4; ++i )
        {
            switch ( D3DVS_GETSWIZZLECOMP(dwDxToken, i) )
            {
            case 0:
                dwSwizzle |= ATI_PVS_SRC_SEL_X << ( ATI_PVS_SRC_SWIZZLE_SHIFT + i * ATI_PVS_SRC_SWIZZLE_SIZE );
                break;
            case 1:
                dwSwizzle |= ATI_PVS_SRC_SEL_Y << ( ATI_PVS_SRC_SWIZZLE_SHIFT + i * ATI_PVS_SRC_SWIZZLE_SIZE );
                break;
            case 2:
                dwSwizzle |= ATI_PVS_SRC_SEL_Z << ( ATI_PVS_SRC_SWIZZLE_SHIFT + i * ATI_PVS_SRC_SWIZZLE_SIZE );
                break;
            case 3:
                dwSwizzle |= ATI_PVS_SRC_SEL_W << ( ATI_PVS_SRC_SWIZZLE_SHIFT + i * ATI_PVS_SRC_SWIZZLE_SIZE );
                break;
            }
        }
    }

    if ( dwSrcFlags & ATI_PVS_FLAG_SRC_X_FORCE_ZERO )
        dwSwizzle = ( dwSwizzle & ~ATI_PVS_SRC_SWIZZLE_X(0) ) | ATI_PVS_SRC_SWIZZLE_X( ATI_PVS_SRC_SEL_FORCE_0 );
    if ( dwSrcFlags & ATI_PVS_FLAG_SRC_Y_FORCE_ZERO )
        dwSwizzle = ( dwSwizzle & ~ATI_PVS_SRC_SWIZZLE_Y(0) ) | ATI_PVS_SRC_SWIZZLE_Y( ATI_PVS_SRC_SEL_FORCE_0 );
    if ( dwSrcFlags & ATI_PVS_FLAG_SRC_Z_FORCE_ZERO )
        dwSwizzle = ( dwSwizzle & ~ATI_PVS_SRC_SWIZZLE_Z(0) ) | ATI_PVS_SRC_SWIZZLE_Z( ATI_PVS_SRC_SEL_FORCE_0 );
    if ( dwSrcFlags & ATI_PVS_FLAG_SRC_W_FORCE_ZERO )
        dwSwizzle = ( dwSwizzle & ~ATI_PVS_SRC_SWIZZLE_W(0) ) | ATI_PVS_SRC_SWIZZLE_W( ATI_PVS_SRC_SEL_FORCE_0 );
    if ( dwSrcFlags & ATI_PVS_FLAG_SRC_X_FORCE_ONE )
        dwSwizzle = ( dwSwizzle & ~ATI_PVS_SRC_SWIZZLE_X(0) ) | ATI_PVS_SRC_SWIZZLE_X( ATI_PVS_SRC_SEL_FORCE_1 );
    if ( dwSrcFlags & ATI_PVS_FLAG_SRC_Y_FORCE_ONE )
        dwSwizzle = ( dwSwizzle & ~ATI_PVS_SRC_SWIZZLE_Y(0) ) | ATI_PVS_SRC_SWIZZLE_Y( ATI_PVS_SRC_SEL_FORCE_1 );
    if ( dwSrcFlags & ATI_PVS_FLAG_SRC_Z_FORCE_ONE )
        dwSwizzle = ( dwSwizzle & ~ATI_PVS_SRC_SWIZZLE_Z(0) ) | ATI_PVS_SRC_SWIZZLE_Z( ATI_PVS_SRC_SEL_FORCE_1 );
    if ( dwSrcFlags & ATI_PVS_FLAG_SRC_W_FORCE_ONE )
        dwSwizzle = ( dwSwizzle & ~ATI_PVS_SRC_SWIZZLE_W(0) ) | ATI_PVS_SRC_SWIZZLE_W( ATI_PVS_SRC_SEL_FORCE_1 );

    DWORD dwNeg = 0;
    for ( DWORD i = 0; i < 4; ++ i)
    {
        if ( dwSrcMod & (1 << i) )
            dwNeg |= ATI_PVS_SRC_NEG_X << i;
    }

    *pdwOutSrcOp = ATI_PVS_MAKE_SRC( dwRegType, dwRegOffset, dwSwizzle, dwNeg, dwAddrMode, dwAddrSel );

    HSLDPF( E_GENERAL_ENTRY_EXIT, "VS_HwAssembleSrcOp: Exit" );
}

void VS_HwAssemble(DWORD pCode[4], DWORD* pDxCode)
{
    HSLDPF( E_GENERAL_ENTRY_EXIT, "VS_HwAssemble: Entry" );

    DWORD dwInst = *pDxCode++;
    DWORD dwAtiOp = ATI_PVS_OP_VE_NO_OP;
    switch ( D3DSI_GETOPCODE(dwInst) )
    {
        case D3DSIO_NOP:
            break;
        case D3DSIO_MOV:
            // TODO
            dwAtiOp = ATI_PVS_OP_VE_ADD;
            break;
        case D3DSIO_ADD:
            dwAtiOp = ATI_PVS_OP_VE_ADD;
            break;
//      case D3DSIO_SUB:
        case D3DSIO_MAD:
            dwAtiOp = ATI_PVS_OP_VE_MAD;
            break;
        case D3DSIO_MUL:
            dwAtiOp = ATI_PVS_OP_VE_MUL;
            break;
        case D3DSIO_RCP:
            dwAtiOp = ATI_PVS_OP_ME_RCP_DX;
            break;
        case D3DSIO_RSQ:
            dwAtiOp = ATI_PVS_OP_ME_RSQ_DX;
            break;
        case D3DSIO_DP3:
        case D3DSIO_DP4:
            dwAtiOp = ATI_PVS_OP_VE_DP4;
            break;
        case D3DSIO_MIN:
            dwAtiOp = ATI_PVS_OP_VE_MIN;
            break;
        case D3DSIO_MAX:
            dwAtiOp = ATI_PVS_OP_VE_MAX;
            break;
        case D3DSIO_SLT:
            dwAtiOp = ATI_PVS_OP_VE_SLT;
            break;
        case D3DSIO_SGE:
            dwAtiOp = ATI_PVS_OP_VE_SGE;
            break;
        case D3DSIO_EXP:
            dwAtiOp = ATI_PVS_OP_ME_EXP_BASE_2_FULL_DX;
            break;
        case D3DSIO_LOG:
            dwAtiOp = ATI_PVS_OP_ME_LOG_BASE_2_FULL_DX;
            break;
        case D3DSIO_LIT:
            dwAtiOp = ATI_PVS_OP_ME_LIT_DX;
            break;
        case D3DSIO_DST:
            dwAtiOp = ATI_PVS_OP_VE_DST;
            break;
//      case D3DSIO_LRP:
        case D3DSIO_FRC:
            dwAtiOp = ATI_PVS_OP_VE_FRC;
            break;
        case D3DSIO_M4x4:
            dwAtiOp = ATI_PVS_MAC_OP_M44;
            break;
        case D3DSIO_M4x3:
            dwAtiOp = ATI_PVS_MAC_OP_M43;
            break;
        case D3DSIO_M3x4:
            dwAtiOp = ATI_PVS_MAC_OP_M34;
            break;
        case D3DSIO_M3x3:
            dwAtiOp = ATI_PVS_MAC_OP_M33;
            break;
        case D3DSIO_M3x2:
            dwAtiOp = ATI_PVS_MAC_OP_M32;
            break;
        case D3DSIO_EXPP:
            dwAtiOp = ATI_PVS_OP_ME_EXP_BASE_2_DX;
            break;
        case D3DSIO_LOGP:
            dwAtiOp = ATI_PVS_OP_ME_LOG_BASE_2_DX;
            break;
        case D3DSIO_DEF:
            HSLDPF( E_ERROR_MESSAGE, "VS_HwAssemble: ERROR: Only VS 2.0 support this type. " );
            break;
        default:
            HSLDPF( E_ERROR_MESSAGE, "VS_HwAssemble: ERROR: Pixel shader instruction: 0x%08x", dwInst );
            break;
    }

    if ( dwAtiOp != ATI_PVS_OP_VE_NO_OP )
    {
        DWORD dwOutVtxEncoding = 0;
        DWORD dwMaxConst = 0;
        DWORD dwVecUsed = 0;
        BOOL bPosUpdated = FALSE;
        if ( *pDxCode & 0x80000000 )
            VS_HwAssembleDstOp( &pCode[0], dwAtiOp, *pDxCode++, &dwOutVtxEncoding, &bPosUpdated, 0 );
        if ( *pDxCode & 0x80000000 )
            VS_HwAssembleSrcOp( nullptr, &pCode[1], *pDxCode++, 0, &dwMaxConst, 0, &dwVecUsed, 0, 0 );
        if ( *pDxCode & 0x80000000 )
            VS_HwAssembleSrcOp( nullptr, &pCode[2], *pDxCode++, 0, &dwMaxConst, 0, &dwVecUsed, 0, 0 );
        if ( *pDxCode & 0x80000000 )
            VS_HwAssembleSrcOp( nullptr, &pCode[3], *pDxCode++, 0, &dwMaxConst, 0, &dwVecUsed, 0, 0 );
    }

    HSLDPF( E_GENERAL_ENTRY_EXIT, "VS_HwAssemble: Exit" );
}
