

//    
//  Workfile: hwVertexShader.h
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

#ifndef _HWVERTEXSHADER_H_
#define _HWVERTEXSHADER_H_

#include "../../inc/common/atiddhsl.h"

///////////////////////////////////////////////////////////////////////////////
// Defines
///////////////////////////////////////////////////////////////////////////////

//
// Radeon 200-specific defines for the PVS code store and constant store
//
#define ATI_R200_PVS_CODE_START_LOW         128
#define ATI_R200_PVS_CODE_START_HIGH        384
#define ATI_R200_PVS_CODE_SIZE_SEG          64
#define ATI_R200_PVS_CODE_SIZE              (2 * ATI_R200_PVS_CODE_SIZE_SEG)
#define ATI_R200_PVS_CODE_STRIDE            1

#define ATI_R200_PVS_STATE_START_LOW        0
#define ATI_R200_PVS_STATE_START_HIGH       256
#define ATI_R200_PVS_STATE_SIZE_SEG         96
#define ATI_R200_PVS_STATE_SIZE             (2 * ATI_R200_PVS_STATE_SIZE_SEG)
#define ATI_R200_PVS_STATE_STRIDE           1


#define ATI_PVS_CODE_SEG    1
#define ATI_PVS_STATE_SEG   2

//
// PVS opcode and operand defines (hopefully not Radeon 200-specific)
//

// Registers
#define ATI_PVS_REG_TEMP                    0x0
#define ATI_PVS_REG_IN                      0x1
#define ATI_PVS_REG_CONST                   0x2
#define ATI_PVS_REG_A0                      0x3
#define ATI_PVS_REG_OUT_POS                 0x4
#define ATI_PVS_REG_OUT_CLR                 0x5
#define ATI_PVS_REG_OUT_TEX                 0x6
#define ATI_PVS_REG_OUT_FOG                 0x7
#define ATI_PVS_REG_OUT_PNT_SIZE            0x8
#define ATI_PVS_REG_GARBAGE                 0x9

// Destination defines
#define ATI_PVS_OP_VE_NO_OP                 0x00
#define ATI_PVS_OP_VE_DP4                   0x01
#define ATI_PVS_OP_VE_MUL                   0x02
#define ATI_PVS_OP_VE_ADD                   0x03
#define ATI_PVS_OP_VE_MAD                   0x04
#define ATI_PVS_OP_VE_DST                   0x05
#define ATI_PVS_OP_VE_FRC                   0x06
#define ATI_PVS_OP_VE_MAX                   0x07
#define ATI_PVS_OP_VE_MIN                   0x08
#define ATI_PVS_OP_VE_SGE                   0x09
#define ATI_PVS_OP_VE_SLT                   0x0a
#define ATI_PVS_OP_VE_MUL_X2_ADD            0x0b
#define ATI_PVS_OP_VE_MUL_CLAMP             0x0c
#define ATI_PVS_OP_VE_FLT2FIX_FLR           0x0d
#define ATI_PVS_OP_VE_FLT2FIX_RND           0x0e
#define ATI_PVS_OP_VE_MAD_MACRO             0x80

#define ATI_PVS_OP_ME_NO_OP                 0x40
#define ATI_PVS_OP_ME_EXP_BASE_2_DX         0x41
#define ATI_PVS_OP_ME_LOG_BASE_2_DX         0x42
#define ATI_PVS_OP_ME_EXP_BASE_E_FF         0x43
#define ATI_PVS_OP_ME_LIT_DX                0x44
#define ATI_PVS_OP_ME_POW_FF                0x45
#define ATI_PVS_OP_ME_RCP_DX                0x46
#define ATI_PVS_OP_ME_RCP_FF                0x47
#define ATI_PVS_OP_ME_RSQ_DX                0x48
#define ATI_PVS_OP_ME_RSQ_FF                0x49
#define ATI_PVS_OP_ME_MUL                   0x4a
#define ATI_PVS_OP_ME_EXP_BASE_2_FULL_DX    0x4b
#define ATI_PVS_OP_ME_LOG_BASE_2_FULL_DX    0x4c

#define ATI_PVS_OP_MACRO_MODE               (1 << 7)

#define ATI_PVS_OP_MASK                     0x000000ff

// DirectX macros
#define ATI_PVS_MAC_OP_M44                  0x0100
#define ATI_PVS_MAC_OP_M43                  0x0200
#define ATI_PVS_MAC_OP_M34                  0x0400
#define ATI_PVS_MAC_OP_M33                  0x0800
#define ATI_PVS_MAC_OP_M32                  0x1000

#define ATI_PVS_DST_REG_TYPE_SHIFT          8
#define ATI_PVS_DST_REG_TYPE_MASK           (0xf<<ATI_PVS_DST_REG_TYPE_SHIFT)
#define ATI_PVS_DST_REG_OFFSET_SHIFT        13
#define ATI_PVS_DST_REG_OFFSET_MASK         (0xff<<ATI_PVS_DST_REG_OFFSET_SHIFT)

#define ATI_PVS_DST_WRITE_ENA_X             (1   << 20)
#define ATI_PVS_DST_WRITE_ENA_Y             (1   << 21)
#define ATI_PVS_DST_WRITE_ENA_Z             (1   << 22)
#define ATI_PVS_DST_WRITE_ENA_W             (1   << 23)
#define ATI_PVS_DST_WRITE_ENA_ALL           ((ATI_PVS_DST_WRITE_ENA_X) | \
                                             (ATI_PVS_DST_WRITE_ENA_Y) | \
                                             (ATI_PVS_DST_WRITE_ENA_Z) | \
                                             (ATI_PVS_DST_WRITE_ENA_W))
#define ATI_PVS_DST_WRITE_ENA_XY            ((ATI_PVS_DST_WRITE_ENA_X) | \
                                             (ATI_PVS_DST_WRITE_ENA_Y))          
#define ATI_PVS_DST_WRITE_ENA_XYZ           ((ATI_PVS_DST_WRITE_ENA_X) | \
                                             (ATI_PVS_DST_WRITE_ENA_Y) | \
                                             (ATI_PVS_DST_WRITE_ENA_Z))
 

#define ATI_PVS_MAKE_DST_OP(dwOp, dwRegType, dwRegOffset, dwWriteEna)  \
          ((dwOp)                                          |           \
           ((dwRegType)   << ATI_PVS_DST_REG_TYPE_SHIFT)   |           \
           ((dwRegOffset) << ATI_PVS_DST_REG_OFFSET_SHIFT) |           \
           (dwWriteEna))

// Source defines

// Used by the dwFlags parameter of VS_HwAssembleSrcOp()
#define ATI_PVS_FLAG_SRC_X_FORCE_ZERO       0x00000001
#define ATI_PVS_FLAG_SRC_Y_FORCE_ZERO       0x00000002
#define ATI_PVS_FLAG_SRC_Z_FORCE_ZERO       0x00000004
#define ATI_PVS_FLAG_SRC_W_FORCE_ZERO       0x00000008
#define ATI_PVS_FLAG_SRC_X_FORCE_ONE        0x00000010
#define ATI_PVS_FLAG_SRC_Y_FORCE_ONE        0x00000020
#define ATI_PVS_FLAG_SRC_Z_FORCE_ONE        0x00000040
#define ATI_PVS_FLAG_SRC_W_FORCE_ONE        0x00000080

#define ATI_PVS_FLAG_SRC_ALL_FORCE_ZERO     (ATI_PVS_FLAG_SRC_X_FORCE_ZERO) | \
                                            (ATI_PVS_FLAG_SRC_Y_FORCE_ZERO) | \
                                            (ATI_PVS_FLAG_SRC_Z_FORCE_ZERO) | \
                                            (ATI_PVS_FLAG_SRC_W_FORCE_ZERO)

#define ATI_PVS_FLAG_SRC_ALL_FORCE_ONE      (ATI_PVS_FLAG_SRC_X_FORCE_ONE) | \
                                            (ATI_PVS_FLAG_SRC_Y_FORCE_ONE) | \
                                            (ATI_PVS_FLAG_SRC_Z_FORCE_ONE) | \
                                            (ATI_PVS_FLAG_SRC_W_FORCE_ONE)

#define ATI_PVS_SRC_ADDR_MODE               (1 << 4)

#define ATI_PVS_SRC_REG_TYPE_SHIFT          0
#define ATI_PVS_SRC_REG_TYPE_MASK           (0xf<<ATI_PVS_SRC_REG_TYPE_SHIFT)
#define ATI_PVS_SRC_REG_OFFSET_SHIFT        5
#define ATI_PVS_SRC_REG_OFFSET_MASK         (0xff<<ATI_PVS_SRC_REG_OFFSET_SHIFT)
#define ATI_PVS_SRC_ADDR_SEL_SHIFT          29


#define ATI_PVS_SRC_SEL_X                   0x0
#define ATI_PVS_SRC_SEL_Y                   0x1
#define ATI_PVS_SRC_SEL_Z                   0x2
#define ATI_PVS_SRC_SEL_W                   0x3
#define ATI_PVS_SRC_SEL_FORCE_0             0x4
#define ATI_PVS_SRC_SEL_FORCE_1             0x5
#define ATI_PVS_SRC_SEL_FIELD               0x7

#define ATI_PVS_SRC_SELECT_X                0x0
#define ATI_PVS_SRC_SELECT_Y                0x1
#define ATI_PVS_SRC_SELECT_Z                0x2
#define ATI_PVS_SRC_SELECT_W                0x3
#define ATI_PVS_SRC_SELECT_FORCE_ZERO       0x4
#define ATI_PVS_SRC_SELECT_FORCE_ONE        0x5


#define ATI_PVS_SRC_SWIZZLE_SHIFT           13
#define ATI_PVS_SRC_SWIZZLE_SIZE             3
#define ATI_PVS_SRC_SWIZZLE_X_SHIFT         ((ATI_PVS_SRC_SWIZZLE_SHIFT) + (0*ATI_PVS_SRC_SWIZZLE_SIZE))
#define ATI_PVS_SRC_SWIZZLE_Y_SHIFT         ((ATI_PVS_SRC_SWIZZLE_SHIFT) + (1*ATI_PVS_SRC_SWIZZLE_SIZE))
#define ATI_PVS_SRC_SWIZZLE_Z_SHIFT         ((ATI_PVS_SRC_SWIZZLE_SHIFT) + (2*ATI_PVS_SRC_SWIZZLE_SIZE))
#define ATI_PVS_SRC_SWIZZLE_W_SHIFT         ((ATI_PVS_SRC_SWIZZLE_SHIFT) + (3*ATI_PVS_SRC_SWIZZLE_SIZE))

#define ATI_PVS_SRC_SEL_MASK                ((0x7 << ATI_PVS_SRC_SWIZZLE_X_SHIFT) | \
                                             (0x7 << ATI_PVS_SRC_SWIZZLE_Y_SHIFT) | \
                                             (0x7 << ATI_PVS_SRC_SWIZZLE_Z_SHIFT) | \
                                             (0x7 << ATI_PVS_SRC_SWIZZLE_W_SHIFT))

#define ATI_PVS_SRC_SWIZZLE_X(dwSel)        ((dwSel) << (ATI_PVS_SRC_SWIZZLE_X_SHIFT))
#define ATI_PVS_SRC_SWIZZLE_Y(dwSel)        ((dwSel) << (ATI_PVS_SRC_SWIZZLE_Y_SHIFT))
#define ATI_PVS_SRC_SWIZZLE_Z(dwSel)        ((dwSel) << (ATI_PVS_SRC_SWIZZLE_Z_SHIFT))
#define ATI_PVS_SRC_SWIZZLE_W(dwSel)        ((dwSel) << (ATI_PVS_SRC_SWIZZLE_W_SHIFT))

#define ATI_PVS_SRC_NO_SWIZZLE              ((ATI_PVS_SRC_SWIZZLE_X(ATI_PVS_SRC_SEL_X)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Y(ATI_PVS_SRC_SEL_Y)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Z(ATI_PVS_SRC_SEL_Z)) | \
                                             (ATI_PVS_SRC_SWIZZLE_W(ATI_PVS_SRC_SEL_W)))

#define ATI_PVS_SRC_FORCE_0000              ((ATI_PVS_SRC_SWIZZLE_X(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Y(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Z(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_W(ATI_PVS_SRC_SEL_FORCE_0)))

#define ATI_PVS_SRC_FORCE_1111              ((ATI_PVS_SRC_SWIZZLE_X(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Y(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Z(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_W(ATI_PVS_SRC_SEL_FORCE_1)))

#define ATI_PVS_SRC_FORCE_0001              ((ATI_PVS_SRC_SWIZZLE_X(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Y(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Z(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_W(ATI_PVS_SRC_SEL_FORCE_1)))

#define ATI_PVS_SRC_FORCE_1001              ((ATI_PVS_SRC_SWIZZLE_X(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Y(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Z(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_W(ATI_PVS_SRC_SEL_FORCE_1)))

#define ATI_PVS_SRC_FORCE_0101              ((ATI_PVS_SRC_SWIZZLE_X(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Y(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Z(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_W(ATI_PVS_SRC_SEL_FORCE_1)))

#define ATI_PVS_SRC_FORCE_0011              ((ATI_PVS_SRC_SWIZZLE_X(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Y(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Z(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_W(ATI_PVS_SRC_SEL_FORCE_1)))

#define ATI_PVS_SRC_FORCE_1011              ((ATI_PVS_SRC_SWIZZLE_X(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Y(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Z(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_W(ATI_PVS_SRC_SEL_FORCE_1)))

#define ATI_PVS_SRC_FORCE_1101              ((ATI_PVS_SRC_SWIZZLE_X(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Y(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Z(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_W(ATI_PVS_SRC_SEL_FORCE_1)))

#define ATI_PVS_SRC_FORCE_0111              ((ATI_PVS_SRC_SWIZZLE_X(ATI_PVS_SRC_SEL_FORCE_0)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Y(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_Z(ATI_PVS_SRC_SEL_FORCE_1)) | \
                                             (ATI_PVS_SRC_SWIZZLE_W(ATI_PVS_SRC_SEL_FORCE_1)))


#define ATI_PVS_SRC_NEG_X                   (1 << 25)
#define ATI_PVS_SRC_NEG_Y                   (1 << 26)
#define ATI_PVS_SRC_NEG_Z                   (1 << 27)
#define ATI_PVS_SRC_NEG_W                   (1 << 28)
#define ATI_PVS_SRC_NEG_ALL                 (ATI_PVS_SRC_NEG_X | ATI_PVS_SRC_NEG_Y | \
                                             ATI_PVS_SRC_NEG_Z | ATI_PVS_SRC_NEG_W)

#define ATI_PVS_MAKE_SRC(dwRegType, dwRegOffset, dwSwizzle, dwNeg, dwAddrMode, dwSrcAddrSel)  \
          (((dwRegType)   << ATI_PVS_SRC_REG_TYPE_SHIFT)   |                    \
           ((dwRegOffset) << ATI_PVS_SRC_REG_OFFSET_SHIFT) |                    \
           (dwSwizzle)                                     |                    \
           (dwNeg)                                         |                    \
           (dwAddrMode)                                    |                    \
           (dwSrcAddrSel) << ATI_PVS_SRC_ADDR_SEL_SHIFT)

#define ATI_PVS_MAKE_SRC_FORCE_ZERO(dwSrcOp)                                    \
          (((dwSrcOp) &~ ATI_PVS_SRC_SEL_MASK) |                                \
           (ATI_PVS_SRC_FORCE_ZERO_ALL))

#define ATI_PVS_MAKE_SRC_FORCE_ONE(dwSrcOp)                                     \
          (((dwSrcOp) &~ ATI_PVS_SRC_SEL_MASK) |                                \
           (ATI_PVS_SRC_FORCE_ONE_ALL))


//
// dwFlags field of ATIPVSMGMT structure
//
#define ATI_PVSMGMT_FLAG_PVS  0x00000001  // Supports PVS

//
// defines for Chaplin PSC registers
//
#define ATI_PSC_DT_FLOAT_1         0x0
#define ATI_PSC_DT_FLOAT_2         0x1
#define ATI_PSC_DT_FLOAT_3         0x2
#define ATI_PSC_DT_FLOAT_4         0x3
#define ATI_PSC_DT_BYTE            0x4
#define ATI_PSC_DT_D3DCOLOR        0x5
#define ATI_PSC_DT_SHORT_2         0x6
#define ATI_PSC_DT_SHORT_4         0x7
#define ATI_PSC_DT_VECTOR_3_TTT    0x8
#define ATI_PSC_DT_VECTOR_3_EET    0x9

#define ATI_PSC_SKIP_DWORD_SHIFT   4
#define ATI_PSC_DST_VEC_SHIFT      8
#define ATI_PSC_LAST_VEC_SHIFT     13
#define ATI_PSC_SIGNED_SHIFT       14
#define ATI_PSC_NORMALIZE_SHIFT    15

// Some PSC defines
#define PSC_LAST_VEC              0x2000
#define PSC_SIGNED_BIT            0x4000
#define PSC_NORMALIZE_BIT         0x8000

// Programmable Shader Output Components
#define ATI_PVS_OUT_POS           0x00000001
#define ATI_PVS_OUT_DISCRETE_FOG  0x00000002
#define ATI_PVS_OUT_POINT_SIZE    0x00000004
// Place holder only              0x00000008
#define ATI_PVS_OUT_COLOR_0       0x00000010
#define ATI_PVS_OUT_COLOR_1       0x00000020
#define ATI_PVS_OUT_COLOR_2       0x00000040 // Presently unused by DirectX
#define ATI_PVS_OUT_COLOR_3       0x00000080 // Presently unused by DirectX
#define ATI_PVS_OUT_COLOR_4       0x00000100 // Presently unused by DirectX
#define ATI_PVS_OUT_COLOR_5       0x00000200 // Presently unused by DirectX
#define ATI_PVS_OUT_COLOR_6       0x00000400 // Presently unused by DirectX
#define ATI_PVS_OUT_COLOR_7       0x00000800 // Presently unused by DirectX
#define ATI_PVS_OUT_TEX_COORD_0   0x00001000
#define ATI_PVS_OUT_TEX_COORD_1   0x00002000
#define ATI_PVS_OUT_TEX_COORD_2   0x00004000
#define ATI_PVS_OUT_TEX_COORD_3   0x00008000
#define ATI_PVS_OUT_TEX_COORD_4   0x00010000
#define ATI_PVS_OUT_TEX_COORD_5   0x00020000
#define ATI_PVS_OUT_TEX_COORD_6   0x00040000 // Not supported on Chaplin
#define ATI_PVS_OUT_TEX_COORD_7   0x00080000 // Not supported on Chaplin


// Some PVS optimization defines

#define IsSingleComponent(dwValue) ((dwValue & (dwValue-1)) == 0)

// PVS registry flags
#define ATI_PVS_OPT_ENABLE_ALL           0x00000001  // Enables all optimizations
#define ATI_PVS_OPT_ENABLE_MOV           0x00000002  // Enables only the MOV optimization
#define ATI_PVS_OPT_ENABLE_ADDRESS       0x00000004  // Enables only the address optimization
#define ATI_PVS_OPT_ENABLE_DEPEND_GRAPH  0x00000008  // Enables only the dependency graph analysis
#define ATI_PVS_OPT_ENABLE_KILL_NEGATION 0x00000010  // Enables only the elimination of negation instructions
#define ATI_PVS_OPT_ENABLE_COMB_NEGATION 0x00000020  // Enables only the combining of negation instructions

// PVS code optimization flags
#define ATI_PVS_OPT_CAN_KILL_MOV    0x00000001   // Set if the MOV instruction into this output register
                                                 // can be eliminated
#define ATI_PVS_OPT_ADDRESS         0x00000002   // Set if the address register is to be optimized.
#define ATI_PVS_OPT_COMB_NEG        0x00000004   // Set if the moves can be combnined
#define ATI_PVS_OPT_KILL_NEG        0x00000008   // Set if the instruction is negating itself

#define ATI_PVS_OPT_MAD_TEMP_REG    0x00008000   // used to break up MAD instructions(not an optimization)

#define ATI_PVS_OPT_INPUT_SRC_MASK  0xf0000000   // Mask for the input register.

// Store relavant data for optimizing an instruction
#define ATI_PVS_OPT_DATA0_SHIFT     16
#define ATI_PVS_OPT_DATA0_MASK      0x000F0000
#define ATI_PVS_OPT_DATA1_SHIFT     20
#define ATI_PVS_OPT_DATA1_MASK      0x00F00000
#define ATI_PVS_OPT_DATA2_SHIFT     24
#define ATI_PVS_OPT_DATA2_MASK      0x0F000000
#define ATI_PVS_OPT_DATA3_SHIFT     28
#define ATI_PVS_OPT_DATA3_MASK      0xF0000000

// Readable #defines for each optimization
//mov
#define ATI_PVS_OPT_MOV_SRC_SHIFT          ATI_PVS_OPT_DATA0_SHIFT
#define ATI_PVS_OPT_MOV_SRC_MASK           ATI_PVS_OPT_DATA0_MASK

//address
#define ATI_PVS_OPT_ADDR_WRITEMASK_SHIFT   ATI_PVS_OPT_DATA0_SHIFT
#define ATI_PVS_OPT_ADDR_WRITEMASK_MASK    ATI_PVS_OPT_DATA0_MASK
#define ATI_PVS_OPT_ADDR_KEEP_INST_SHIFT   ATI_PVS_OPT_DATA1_SHIFT
#define ATI_PVS_OPT_ADDR_KEEP_INST_MASK    ATI_PVS_OPT_DATA1_MASK

//comb neg
#define ATI_PVS_OPT_COMB_NEG_WE_SHIFT      ATI_PVS_OPT_DATA0_SHIFT
#define ATI_PVS_OPT_COMB_NEG_WE_MASK       ATI_PVS_OPT_DATA0_MASK
#define ATI_PVS_OPT_COMB_NEG_MOD_SHIFT     ATI_PVS_OPT_DATA1_SHIFT
#define ATI_PVS_OPT_COMB_NEG_MOD_MASK      ATI_PVS_OPT_DATA1_MASK
#define ATI_PVS_OPT_COMB_NEG_SWIZ_SHIFT    ATI_PVS_OPT_DATA2_SHIFT  // this takes up slots 2 & 3
#define ATI_PVS_OPT_COMB_NEG_SWIZ_MASK    (ATI_PVS_OPT_DATA2_MASK | \
                                           ATI_PVS_OPT_DATA3_MASK)

//kill neg
#define ATI_PVS_OPT_KILL_NEG_MOD_SHIFT     ATI_PVS_OPT_DATA0_SHIFT
#define ATI_PVS_OPT_KILL_NEG_MOD_MASK      ATI_PVS_OPT_DATA0_MASK
#define ATI_PVS_OPT_KILL_NEG_REG_SHIFT     ATI_PVS_OPT_DATA1_SHIFT
#define ATI_PVS_OPT_KILL_NEG_REG_MASK      ATI_PVS_OPT_DATA1_MASK
#define ATI_PVS_OPT_KILL_NEG_KILL_SHIFT    ATI_PVS_OPT_DATA2_SHIFT
#define ATI_PVS_OPT_KILL_NEG_KILL_MASK     ATI_PVS_OPT_DATA2_MASK

//mad
#define ATI_PVS_OPT_MAD_TEMP_REG_SHIFT     ATI_PVS_OPT_DATA0_SHIFT
#define ATI_PVS_OPT_MAD_TEMP_REG_MASK      ATI_PVS_OPT_DATA0_MASK


// PVS Debug registry flags contained in dwPvsDebug
#define ATI_PVS_DEBUG_HIGHLIGHT            0x00000001 // Highlight shaders
#define ATI_PVS_DEBUG_OPT_STRONG_CHECK     0x00000002 // Force strong checking for the optimization
                                                      // controlled by ATI_PVS_OPT_ENABLE_DEPEND_GRAPH
#define ATI_PVS_DEBUG_USE_FLT2FIX_RND      0x00000004 // Use ATI_PVS_OP_VE_FLT2FIX_RND instead of
                                                      // ATI_PVS_OP_VE_FLT2FIX_FLR to write to ADDR registers
#define ATI_PVS_DEBUG_SET_DEFAULTS         0x00000008 // Force in default values for output registers
#define ATI_PVS_DEBUG_DONT_FIX_EXPP        0x00000010 
#define ATI_PVS_DEBUG_PRE_OPT_HEX_DUMP     0x00000020 // Interleave Hex dump with normal E_SHADER_DUMP


///////////////////////////////////////////////////////////////////////////////
// Function Prototypes
///////////////////////////////////////////////////////////////////////////////
void VS_HwAssembleDstOp(DWORD* pdwOutDstOp, DWORD dwAtiOp, DWORD dwDxToken, DWORD *pdwOutVtxEncoding,
                        BOOL* pbPosUpdated, DWORD dwWriteMaskOverride);
void VS_HwAssembleSrcOp(struct _ATID3DCNTX* pCtxt, DWORD* pdwOutSrcOp, DWORD dwDxToken, DWORD dwSrcFlags,
                        DWORD* pdwMaxConst, DWORD dwAddrSel, DWORD* pdwVecUsed, DWORD dwSrcMod,
                        DWORD dwSwizzleOverride);
void VS_HwAssemble(DWORD pCode[4], DWORD* pDxCode);

#endif // _HWVERTEXSHADER_H_

