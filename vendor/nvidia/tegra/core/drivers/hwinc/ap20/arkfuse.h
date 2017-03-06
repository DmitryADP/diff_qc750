//
// DO NOT EDIT - generated by simspec!
//

#ifndef ___ARKFUSE_H_INC_
#define ___ARKFUSE_H_INC_
// --------------------------------------------------------------------------
//
// Copyright (c) 2007, NVIDIA Corp.
// All Rights Reserved.
//
// This is UNPUBLISHED PROPRIETARY SOURCE CODE of NVIDIA Corp.;
// the contents of this file may not be disclosed to third parties, copied or
// duplicated in any form, in whole or in part, without the prior written
// permission of NVIDIA Corp.
//
// RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
// and Computer Software clause at DFARS 252.227-7013, and/or in similar or
// successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
// rights reserved under the Copyright Laws of the United States.
//
// --------------------------------------------------------------------------
//
// The kfuse block stores downstream and upstream HDCP keys for use by HDMI module.
//
// The HDCP keys are encrypted, then ECC encoded,
// before writing into kfuse.
// Typically, the keys are programmed in manufacturing flow, but could
// also be programmed by SW in-system.
//
// Terminology:
//   keyglob: structure containing concatenated HDCP upstream and downstream KSVs and keys.  576 bytes.
//   encrypt: apply proprietary encryption to keyglob using secret keys; performed by manufacturing SW.
//   decrypt: convert keyglob from encrypted form to plaintext; performed within HDMI.
//   ECC: error-correcting code used in kfuse to recover from defective kfuse bits; can correct up to 3 bad bits
//     per 128-bit block.
//   encode: convert keyglob to ECC-encode raw; performed by manufacturing SW.
//   decode: convert from raw ECC-encoded raw form; performed within KFUSE
//   raw: kfuse data in its ECC-encoded state
//   word: 32-bit native row size of kfuse, and host interfaces
//   block: 128-bit data group upon which ECC is performed; consists of 4 adjacent words.
//      Also, unrelated 128-bit data group upon which decryption is performed.
// 
// The encrypted keyglob is protected by ECC and CRC; upon deassertion of reset or SW RESTART command,
// the raw kfuse array is read and ECC-decoded, and SW can access the encrypted keyglob.  SW then copies
// the encrypted keyglob to HDMI for decryption.  SW does not, and cannot, access the encryption keys.
//
// kfuse module has two modes of operation:
// - ECC mode: HW automatically reads kfuse, decodes it, and makes decoded keyglob available to SW via
//   a RAM.  This is normal mode of operation.
// - raw mode: SW can read and write the raw kfuse data, and has low-level control over the kfuse array.  
//   In this mode, FUSECTRL is used to initiate commands.  This mode is used only for debug.
//
// In normal ECC mode, upon initialization of HDMI module, SW should:
// 1. bring kfuse out of reset, and wait for STATE.DONE=1
// 2. check that CRCPASS=1 and ERR_FATAL=0.  If not, HDCP is unavailable; SW could attempt
//    to fall back to CryptoROM.
// 3. using KEYADDR and KEYS registers, copy encrypted keyglob to HDMI, and have HDMI decrypt
// 4. do HDMI HDCP downstream and/or upstream authentication as needed
//
// KFUSE organization:
// - The raw kfuse contains 192 32-bits words (6144 bits).  
//   The raw read and write operations operate on 32-bit words.
// - For ECC, raw data is arranged as 48 128-bit blocks (also 6144 bits; 4 words per block).  
//   ECC operates on 128-bit blocks.
// - The ECC-decoded keyglob data is 144 32-bit words (576 bytes).  
//   The KEYADDR is word addressed, and KEYS returns 32-bit words.
// - The HDMI decryptor uses 128-bit chunks of keyglob; each chunk uses 4 consecutive words from KEYS
//
// --------------------------------------------------
// raw mode registers

// Register KFUSE_FUSECTRL_0  // KFUSE control.  Used for low-level manipulation of raw kfuse bits, not normal use.
#define KFUSE_FUSECTRL_0                        _MK_ADDR_CONST(0x0)
#define KFUSE_FUSECTRL_0_SECURE                         0x0
#define KFUSE_FUSECTRL_0_WORD_COUNT                     0x1
#define KFUSE_FUSECTRL_0_RESET_VAL                      _MK_MASK_CONST(0x0)
#define KFUSE_FUSECTRL_0_RESET_MASK                     _MK_MASK_CONST(0x3)
#define KFUSE_FUSECTRL_0_SW_DEFAULT_VAL                         _MK_MASK_CONST(0x0)
#define KFUSE_FUSECTRL_0_SW_DEFAULT_MASK                        _MK_MASK_CONST(0x0)
#define KFUSE_FUSECTRL_0_READ_MASK                      _MK_MASK_CONST(0xf0003)
#define KFUSE_FUSECTRL_0_WRITE_MASK                     _MK_MASK_CONST(0x3)
// Trigger a command
#define KFUSE_FUSECTRL_0_CMD_SHIFT                      _MK_SHIFT_CONST(0)
#define KFUSE_FUSECTRL_0_CMD_FIELD                      (_MK_MASK_CONST(0x3) << KFUSE_FUSECTRL_0_CMD_SHIFT)
#define KFUSE_FUSECTRL_0_CMD_RANGE                      1:0
#define KFUSE_FUSECTRL_0_CMD_WOFFSET                    0x0
#define KFUSE_FUSECTRL_0_CMD_DEFAULT                    _MK_MASK_CONST(0x0)
#define KFUSE_FUSECTRL_0_CMD_DEFAULT_MASK                       _MK_MASK_CONST(0x3)
#define KFUSE_FUSECTRL_0_CMD_SW_DEFAULT                 _MK_MASK_CONST(0x0)
#define KFUSE_FUSECTRL_0_CMD_SW_DEFAULT_MASK                    _MK_MASK_CONST(0x0)
#define KFUSE_FUSECTRL_0_CMD_IDLE                       _MK_ENUM_CONST(0)
#define KFUSE_FUSECTRL_0_CMD_READ                       _MK_ENUM_CONST(1)
#define KFUSE_FUSECTRL_0_CMD_WRITE                      _MK_ENUM_CONST(2)
#define KFUSE_FUSECTRL_0_CMD_VERIFY                     _MK_ENUM_CONST(3)

// kfuse controller state, for polling and debugging.
#define KFUSE_FUSECTRL_0_STATE_SHIFT                    _MK_SHIFT_CONST(16)
#define KFUSE_FUSECTRL_0_STATE_FIELD                    (_MK_MASK_CONST(0xf) << KFUSE_FUSECTRL_0_STATE_SHIFT)
#define KFUSE_FUSECTRL_0_STATE_RANGE                    19:16
#define KFUSE_FUSECTRL_0_STATE_WOFFSET                  0x0
#define KFUSE_FUSECTRL_0_STATE_DEFAULT                  _MK_MASK_CONST(0x0)
#define KFUSE_FUSECTRL_0_STATE_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define KFUSE_FUSECTRL_0_STATE_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define KFUSE_FUSECTRL_0_STATE_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)
#define KFUSE_FUSECTRL_0_STATE_IN_RESET                 _MK_ENUM_CONST(0)
#define KFUSE_FUSECTRL_0_STATE_POST_RESET                       _MK_ENUM_CONST(1)
#define KFUSE_FUSECTRL_0_STATE_IDLE                     _MK_ENUM_CONST(2)
#define KFUSE_FUSECTRL_0_STATE_READ_SETUP                       _MK_ENUM_CONST(3)
#define KFUSE_FUSECTRL_0_STATE_READ_STROBE                      _MK_ENUM_CONST(4)
#define KFUSE_FUSECTRL_0_STATE_SAMPLE_FUSES                     _MK_ENUM_CONST(5)
#define KFUSE_FUSECTRL_0_STATE_READ_OHLD                        _MK_ENUM_CONST(6)
#define KFUSE_FUSECTRL_0_STATE_WRITE_SETUP                      _MK_ENUM_CONST(7)
#define KFUSE_FUSECTRL_0_STATE_WRITE_ADDR_SETUP                 _MK_ENUM_CONST(8)
#define KFUSE_FUSECTRL_0_STATE_WRITE_PROGRAM                    _MK_ENUM_CONST(9)
#define KFUSE_FUSECTRL_0_STATE_WRITE_ADDR_HOLD                  _MK_ENUM_CONST(10)


// Register KFUSE_FUSEADDR_0  // word address; applies to following CMD. Does not autoincrement.
#define KFUSE_FUSEADDR_0                        _MK_ADDR_CONST(0x4)
#define KFUSE_FUSEADDR_0_SECURE                         0x0
#define KFUSE_FUSEADDR_0_WORD_COUNT                     0x1
#define KFUSE_FUSEADDR_0_RESET_VAL                      _MK_MASK_CONST(0x0)
#define KFUSE_FUSEADDR_0_RESET_MASK                     _MK_MASK_CONST(0xff)
#define KFUSE_FUSEADDR_0_SW_DEFAULT_VAL                         _MK_MASK_CONST(0x0)
#define KFUSE_FUSEADDR_0_SW_DEFAULT_MASK                        _MK_MASK_CONST(0x0)
#define KFUSE_FUSEADDR_0_READ_MASK                      _MK_MASK_CONST(0xff)
#define KFUSE_FUSEADDR_0_WRITE_MASK                     _MK_MASK_CONST(0xff)
#define KFUSE_FUSEADDR_0_ADDR_SHIFT                     _MK_SHIFT_CONST(0)
#define KFUSE_FUSEADDR_0_ADDR_FIELD                     (_MK_MASK_CONST(0xff) << KFUSE_FUSEADDR_0_ADDR_SHIFT)
#define KFUSE_FUSEADDR_0_ADDR_RANGE                     7:0
#define KFUSE_FUSEADDR_0_ADDR_WOFFSET                   0x0
#define KFUSE_FUSEADDR_0_ADDR_DEFAULT                   _MK_MASK_CONST(0x0)
#define KFUSE_FUSEADDR_0_ADDR_DEFAULT_MASK                      _MK_MASK_CONST(0xff)
#define KFUSE_FUSEADDR_0_ADDR_SW_DEFAULT                        _MK_MASK_CONST(0x0)
#define KFUSE_FUSEADDR_0_ADDR_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)


// Register KFUSE_FUSEDATA0_0  // read data; available after CMD=READ is triggered and STATE becomes IDLE
#define KFUSE_FUSEDATA0_0                       _MK_ADDR_CONST(0x8)
#define KFUSE_FUSEDATA0_0_SECURE                        0x0
#define KFUSE_FUSEDATA0_0_WORD_COUNT                    0x1
#define KFUSE_FUSEDATA0_0_RESET_VAL                     _MK_MASK_CONST(0x0)
#define KFUSE_FUSEDATA0_0_RESET_MASK                    _MK_MASK_CONST(0x0)
#define KFUSE_FUSEDATA0_0_SW_DEFAULT_VAL                        _MK_MASK_CONST(0x0)
#define KFUSE_FUSEDATA0_0_SW_DEFAULT_MASK                       _MK_MASK_CONST(0x0)
#define KFUSE_FUSEDATA0_0_READ_MASK                     _MK_MASK_CONST(0xffffffff)
#define KFUSE_FUSEDATA0_0_WRITE_MASK                    _MK_MASK_CONST(0x0)
#define KFUSE_FUSEDATA0_0_DATA_SHIFT                    _MK_SHIFT_CONST(0)
#define KFUSE_FUSEDATA0_0_DATA_FIELD                    (_MK_MASK_CONST(0xffffffff) << KFUSE_FUSEDATA0_0_DATA_SHIFT)
#define KFUSE_FUSEDATA0_0_DATA_RANGE                    31:0
#define KFUSE_FUSEDATA0_0_DATA_WOFFSET                  0x0
#define KFUSE_FUSEDATA0_0_DATA_DEFAULT                  _MK_MASK_CONST(0x0)
#define KFUSE_FUSEDATA0_0_DATA_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define KFUSE_FUSEDATA0_0_DATA_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define KFUSE_FUSEDATA0_0_DATA_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)


// Register KFUSE_FUSEWRDATA0_0  // write data; program this before triggering CMD=WRITE
#define KFUSE_FUSEWRDATA0_0                     _MK_ADDR_CONST(0xc)
#define KFUSE_FUSEWRDATA0_0_SECURE                      0x0
#define KFUSE_FUSEWRDATA0_0_WORD_COUNT                  0x1
#define KFUSE_FUSEWRDATA0_0_RESET_VAL                   _MK_MASK_CONST(0x0)
#define KFUSE_FUSEWRDATA0_0_RESET_MASK                  _MK_MASK_CONST(0xffffffff)
#define KFUSE_FUSEWRDATA0_0_SW_DEFAULT_VAL                      _MK_MASK_CONST(0x0)
#define KFUSE_FUSEWRDATA0_0_SW_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define KFUSE_FUSEWRDATA0_0_READ_MASK                   _MK_MASK_CONST(0x0)
#define KFUSE_FUSEWRDATA0_0_WRITE_MASK                  _MK_MASK_CONST(0xffffffff)
#define KFUSE_FUSEWRDATA0_0_WRDATA_SHIFT                        _MK_SHIFT_CONST(0)
#define KFUSE_FUSEWRDATA0_0_WRDATA_FIELD                        (_MK_MASK_CONST(0xffffffff) << KFUSE_FUSEWRDATA0_0_WRDATA_SHIFT)
#define KFUSE_FUSEWRDATA0_0_WRDATA_RANGE                        31:0
#define KFUSE_FUSEWRDATA0_0_WRDATA_WOFFSET                      0x0
#define KFUSE_FUSEWRDATA0_0_WRDATA_DEFAULT                      _MK_MASK_CONST(0x0)
#define KFUSE_FUSEWRDATA0_0_WRDATA_DEFAULT_MASK                 _MK_MASK_CONST(0xffffffff)
#define KFUSE_FUSEWRDATA0_0_WRDATA_SW_DEFAULT                   _MK_MASK_CONST(0x0)
#define KFUSE_FUSEWRDATA0_0_WRDATA_SW_DEFAULT_MASK                      _MK_MASK_CONST(0x0)

//
// --------------------------------------------------
// Fuse timing controls.  Should be no need to change any of these.  
// Refer to HDEFUSE datasheet.
//

// Register KFUSE_FUSETIME_RD1_0  
#define KFUSE_FUSETIME_RD1_0                    _MK_ADDR_CONST(0x10)
#define KFUSE_FUSETIME_RD1_0_SECURE                     0x0
#define KFUSE_FUSETIME_RD1_0_WORD_COUNT                         0x1
#define KFUSE_FUSETIME_RD1_0_RESET_VAL                  _MK_MASK_CONST(0x10101)
#define KFUSE_FUSETIME_RD1_0_RESET_MASK                         _MK_MASK_CONST(0xffffff)
#define KFUSE_FUSETIME_RD1_0_SW_DEFAULT_VAL                     _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_RD1_0_SW_DEFAULT_MASK                    _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_RD1_0_READ_MASK                  _MK_MASK_CONST(0xffffff)
#define KFUSE_FUSETIME_RD1_0_WRITE_MASK                         _MK_MASK_CONST(0xffffff)
#define KFUSE_FUSETIME_RD1_0_TSUR_MAX_SHIFT                     _MK_SHIFT_CONST(0)
#define KFUSE_FUSETIME_RD1_0_TSUR_MAX_FIELD                     (_MK_MASK_CONST(0xff) << KFUSE_FUSETIME_RD1_0_TSUR_MAX_SHIFT)
#define KFUSE_FUSETIME_RD1_0_TSUR_MAX_RANGE                     7:0
#define KFUSE_FUSETIME_RD1_0_TSUR_MAX_WOFFSET                   0x0
#define KFUSE_FUSETIME_RD1_0_TSUR_MAX_DEFAULT                   _MK_MASK_CONST(0x1)
#define KFUSE_FUSETIME_RD1_0_TSUR_MAX_DEFAULT_MASK                      _MK_MASK_CONST(0xff)
#define KFUSE_FUSETIME_RD1_0_TSUR_MAX_SW_DEFAULT                        _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_RD1_0_TSUR_MAX_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)

#define KFUSE_FUSETIME_RD1_0_TSUR_FUSEOUT_SHIFT                 _MK_SHIFT_CONST(8)
#define KFUSE_FUSETIME_RD1_0_TSUR_FUSEOUT_FIELD                 (_MK_MASK_CONST(0xff) << KFUSE_FUSETIME_RD1_0_TSUR_FUSEOUT_SHIFT)
#define KFUSE_FUSETIME_RD1_0_TSUR_FUSEOUT_RANGE                 15:8
#define KFUSE_FUSETIME_RD1_0_TSUR_FUSEOUT_WOFFSET                       0x0
#define KFUSE_FUSETIME_RD1_0_TSUR_FUSEOUT_DEFAULT                       _MK_MASK_CONST(0x1)
#define KFUSE_FUSETIME_RD1_0_TSUR_FUSEOUT_DEFAULT_MASK                  _MK_MASK_CONST(0xff)
#define KFUSE_FUSETIME_RD1_0_TSUR_FUSEOUT_SW_DEFAULT                    _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_RD1_0_TSUR_FUSEOUT_SW_DEFAULT_MASK                       _MK_MASK_CONST(0x0)

#define KFUSE_FUSETIME_RD1_0_THR_MAX_SHIFT                      _MK_SHIFT_CONST(16)
#define KFUSE_FUSETIME_RD1_0_THR_MAX_FIELD                      (_MK_MASK_CONST(0xff) << KFUSE_FUSETIME_RD1_0_THR_MAX_SHIFT)
#define KFUSE_FUSETIME_RD1_0_THR_MAX_RANGE                      23:16
#define KFUSE_FUSETIME_RD1_0_THR_MAX_WOFFSET                    0x0
#define KFUSE_FUSETIME_RD1_0_THR_MAX_DEFAULT                    _MK_MASK_CONST(0x1)
#define KFUSE_FUSETIME_RD1_0_THR_MAX_DEFAULT_MASK                       _MK_MASK_CONST(0xff)
#define KFUSE_FUSETIME_RD1_0_THR_MAX_SW_DEFAULT                 _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_RD1_0_THR_MAX_SW_DEFAULT_MASK                    _MK_MASK_CONST(0x0)


// Register KFUSE_FUSETIME_RD2_0  
#define KFUSE_FUSETIME_RD2_0                    _MK_ADDR_CONST(0x14)
#define KFUSE_FUSETIME_RD2_0_SECURE                     0x0
#define KFUSE_FUSETIME_RD2_0_WORD_COUNT                         0x1
#define KFUSE_FUSETIME_RD2_0_RESET_VAL                  _MK_MASK_CONST(0x3)
#define KFUSE_FUSETIME_RD2_0_RESET_MASK                         _MK_MASK_CONST(0xffff)
#define KFUSE_FUSETIME_RD2_0_SW_DEFAULT_VAL                     _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_RD2_0_SW_DEFAULT_MASK                    _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_RD2_0_READ_MASK                  _MK_MASK_CONST(0xffff)
#define KFUSE_FUSETIME_RD2_0_WRITE_MASK                         _MK_MASK_CONST(0xffff)
#define KFUSE_FUSETIME_RD2_0_TWIDTH_RD_SHIFT                    _MK_SHIFT_CONST(0)
#define KFUSE_FUSETIME_RD2_0_TWIDTH_RD_FIELD                    (_MK_MASK_CONST(0xffff) << KFUSE_FUSETIME_RD2_0_TWIDTH_RD_SHIFT)
#define KFUSE_FUSETIME_RD2_0_TWIDTH_RD_RANGE                    15:0
#define KFUSE_FUSETIME_RD2_0_TWIDTH_RD_WOFFSET                  0x0
#define KFUSE_FUSETIME_RD2_0_TWIDTH_RD_DEFAULT                  _MK_MASK_CONST(0x3)
#define KFUSE_FUSETIME_RD2_0_TWIDTH_RD_DEFAULT_MASK                     _MK_MASK_CONST(0xffff)
#define KFUSE_FUSETIME_RD2_0_TWIDTH_RD_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_RD2_0_TWIDTH_RD_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)


// Register KFUSE_FUSETIME_PGM1_0  
#define KFUSE_FUSETIME_PGM1_0                   _MK_ADDR_CONST(0x18)
#define KFUSE_FUSETIME_PGM1_0_SECURE                    0x0
#define KFUSE_FUSETIME_PGM1_0_WORD_COUNT                        0x1
#define KFUSE_FUSETIME_PGM1_0_RESET_VAL                         _MK_MASK_CONST(0x10101)
#define KFUSE_FUSETIME_PGM1_0_RESET_MASK                        _MK_MASK_CONST(0xffffff)
#define KFUSE_FUSETIME_PGM1_0_SW_DEFAULT_VAL                    _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_PGM1_0_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_PGM1_0_READ_MASK                         _MK_MASK_CONST(0xffffff)
#define KFUSE_FUSETIME_PGM1_0_WRITE_MASK                        _MK_MASK_CONST(0xffffff)
#define KFUSE_FUSETIME_PGM1_0_TSUP_MAX_SHIFT                    _MK_SHIFT_CONST(0)
#define KFUSE_FUSETIME_PGM1_0_TSUP_MAX_FIELD                    (_MK_MASK_CONST(0xff) << KFUSE_FUSETIME_PGM1_0_TSUP_MAX_SHIFT)
#define KFUSE_FUSETIME_PGM1_0_TSUP_MAX_RANGE                    7:0
#define KFUSE_FUSETIME_PGM1_0_TSUP_MAX_WOFFSET                  0x0
#define KFUSE_FUSETIME_PGM1_0_TSUP_MAX_DEFAULT                  _MK_MASK_CONST(0x1)
#define KFUSE_FUSETIME_PGM1_0_TSUP_MAX_DEFAULT_MASK                     _MK_MASK_CONST(0xff)
#define KFUSE_FUSETIME_PGM1_0_TSUP_MAX_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_PGM1_0_TSUP_MAX_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)

#define KFUSE_FUSETIME_PGM1_0_TSUP_ADDR_SHIFT                   _MK_SHIFT_CONST(8)
#define KFUSE_FUSETIME_PGM1_0_TSUP_ADDR_FIELD                   (_MK_MASK_CONST(0xff) << KFUSE_FUSETIME_PGM1_0_TSUP_ADDR_SHIFT)
#define KFUSE_FUSETIME_PGM1_0_TSUP_ADDR_RANGE                   15:8
#define KFUSE_FUSETIME_PGM1_0_TSUP_ADDR_WOFFSET                 0x0
#define KFUSE_FUSETIME_PGM1_0_TSUP_ADDR_DEFAULT                 _MK_MASK_CONST(0x1)
#define KFUSE_FUSETIME_PGM1_0_TSUP_ADDR_DEFAULT_MASK                    _MK_MASK_CONST(0xff)
#define KFUSE_FUSETIME_PGM1_0_TSUP_ADDR_SW_DEFAULT                      _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_PGM1_0_TSUP_ADDR_SW_DEFAULT_MASK                 _MK_MASK_CONST(0x0)

#define KFUSE_FUSETIME_PGM1_0_THP_ADDR_SHIFT                    _MK_SHIFT_CONST(16)
#define KFUSE_FUSETIME_PGM1_0_THP_ADDR_FIELD                    (_MK_MASK_CONST(0xff) << KFUSE_FUSETIME_PGM1_0_THP_ADDR_SHIFT)
#define KFUSE_FUSETIME_PGM1_0_THP_ADDR_RANGE                    23:16
#define KFUSE_FUSETIME_PGM1_0_THP_ADDR_WOFFSET                  0x0
#define KFUSE_FUSETIME_PGM1_0_THP_ADDR_DEFAULT                  _MK_MASK_CONST(0x1)
#define KFUSE_FUSETIME_PGM1_0_THP_ADDR_DEFAULT_MASK                     _MK_MASK_CONST(0xff)
#define KFUSE_FUSETIME_PGM1_0_THP_ADDR_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_PGM1_0_THP_ADDR_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)


// Register KFUSE_FUSETIME_PGM2_0  
#define KFUSE_FUSETIME_PGM2_0                   _MK_ADDR_CONST(0x1c)
#define KFUSE_FUSETIME_PGM2_0_SECURE                    0x0
#define KFUSE_FUSETIME_PGM2_0_WORD_COUNT                        0x1
#define KFUSE_FUSETIME_PGM2_0_RESET_VAL                         _MK_MASK_CONST(0xfa)
#define KFUSE_FUSETIME_PGM2_0_RESET_MASK                        _MK_MASK_CONST(0xffff)
#define KFUSE_FUSETIME_PGM2_0_SW_DEFAULT_VAL                    _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_PGM2_0_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_PGM2_0_READ_MASK                         _MK_MASK_CONST(0xffff)
#define KFUSE_FUSETIME_PGM2_0_WRITE_MASK                        _MK_MASK_CONST(0xffff)
// Programming pulse width.  Set to 10 uS / Tclk where Tclk is period of kfuse clock.
#define KFUSE_FUSETIME_PGM2_0_TWIDTH_PGM_SHIFT                  _MK_SHIFT_CONST(0)
#define KFUSE_FUSETIME_PGM2_0_TWIDTH_PGM_FIELD                  (_MK_MASK_CONST(0xffff) << KFUSE_FUSETIME_PGM2_0_TWIDTH_PGM_SHIFT)
#define KFUSE_FUSETIME_PGM2_0_TWIDTH_PGM_RANGE                  15:0
#define KFUSE_FUSETIME_PGM2_0_TWIDTH_PGM_WOFFSET                        0x0
#define KFUSE_FUSETIME_PGM2_0_TWIDTH_PGM_DEFAULT                        _MK_MASK_CONST(0xfa)
#define KFUSE_FUSETIME_PGM2_0_TWIDTH_PGM_DEFAULT_MASK                   _MK_MASK_CONST(0xffff)
#define KFUSE_FUSETIME_PGM2_0_TWIDTH_PGM_SW_DEFAULT                     _MK_MASK_CONST(0x0)
#define KFUSE_FUSETIME_PGM2_0_TWIDTH_PGM_SW_DEFAULT_MASK                        _MK_MASK_CONST(0x0)


// Register KFUSE_REGULATOR_0  
#define KFUSE_REGULATOR_0                       _MK_ADDR_CONST(0x20)
#define KFUSE_REGULATOR_0_SECURE                        0x0
#define KFUSE_REGULATOR_0_WORD_COUNT                    0x1
#define KFUSE_REGULATOR_0_RESET_VAL                     _MK_MASK_CONST(0x202)
#define KFUSE_REGULATOR_0_RESET_MASK                    _MK_MASK_CONST(0x10303)
#define KFUSE_REGULATOR_0_SW_DEFAULT_VAL                        _MK_MASK_CONST(0x0)
#define KFUSE_REGULATOR_0_SW_DEFAULT_MASK                       _MK_MASK_CONST(0x0)
#define KFUSE_REGULATOR_0_READ_MASK                     _MK_MASK_CONST(0x10303)
#define KFUSE_REGULATOR_0_WRITE_MASK                    _MK_MASK_CONST(0x10303)
#define KFUSE_REGULATOR_0_REF_CTRL_SHIFT                        _MK_SHIFT_CONST(0)
#define KFUSE_REGULATOR_0_REF_CTRL_FIELD                        (_MK_MASK_CONST(0x3) << KFUSE_REGULATOR_0_REF_CTRL_SHIFT)
#define KFUSE_REGULATOR_0_REF_CTRL_RANGE                        1:0
#define KFUSE_REGULATOR_0_REF_CTRL_WOFFSET                      0x0
#define KFUSE_REGULATOR_0_REF_CTRL_DEFAULT                      _MK_MASK_CONST(0x2)
#define KFUSE_REGULATOR_0_REF_CTRL_DEFAULT_MASK                 _MK_MASK_CONST(0x3)
#define KFUSE_REGULATOR_0_REF_CTRL_SW_DEFAULT                   _MK_MASK_CONST(0x0)
#define KFUSE_REGULATOR_0_REF_CTRL_SW_DEFAULT_MASK                      _MK_MASK_CONST(0x0)

#define KFUSE_REGULATOR_0_BIAS_CTRL_SHIFT                       _MK_SHIFT_CONST(8)
#define KFUSE_REGULATOR_0_BIAS_CTRL_FIELD                       (_MK_MASK_CONST(0x3) << KFUSE_REGULATOR_0_BIAS_CTRL_SHIFT)
#define KFUSE_REGULATOR_0_BIAS_CTRL_RANGE                       9:8
#define KFUSE_REGULATOR_0_BIAS_CTRL_WOFFSET                     0x0
#define KFUSE_REGULATOR_0_BIAS_CTRL_DEFAULT                     _MK_MASK_CONST(0x2)
#define KFUSE_REGULATOR_0_BIAS_CTRL_DEFAULT_MASK                        _MK_MASK_CONST(0x3)
#define KFUSE_REGULATOR_0_BIAS_CTRL_SW_DEFAULT                  _MK_MASK_CONST(0x0)
#define KFUSE_REGULATOR_0_BIAS_CTRL_SW_DEFAULT_MASK                     _MK_MASK_CONST(0x0)

// Power Good:  Before programming, SW must set to 1 after KFUSE_SRC power supply is valid.
// In AP20, USB 3.3V must be enabled as well.
#define KFUSE_REGULATOR_0_PWRGD_SHIFT                   _MK_SHIFT_CONST(16)
#define KFUSE_REGULATOR_0_PWRGD_FIELD                   (_MK_MASK_CONST(0x1) << KFUSE_REGULATOR_0_PWRGD_SHIFT)
#define KFUSE_REGULATOR_0_PWRGD_RANGE                   16:16
#define KFUSE_REGULATOR_0_PWRGD_WOFFSET                 0x0
#define KFUSE_REGULATOR_0_PWRGD_DEFAULT                 _MK_MASK_CONST(0x0)
#define KFUSE_REGULATOR_0_PWRGD_DEFAULT_MASK                    _MK_MASK_CONST(0x1)
#define KFUSE_REGULATOR_0_PWRGD_SW_DEFAULT                      _MK_MASK_CONST(0x0)
#define KFUSE_REGULATOR_0_PWRGD_SW_DEFAULT_MASK                 _MK_MASK_CONST(0x0)

//
// --------------------------------------------------
// ECC mode registers
//

// Register KFUSE_STATE_0  // Commands and status for ECC mode
#define KFUSE_STATE_0                   _MK_ADDR_CONST(0x80)
#define KFUSE_STATE_0_SECURE                    0x0
#define KFUSE_STATE_0_WORD_COUNT                        0x1
#define KFUSE_STATE_0_RESET_VAL                         _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_RESET_MASK                        _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_SW_DEFAULT_VAL                    _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_READ_MASK                         _MK_MASK_CONST(0x83033f3f)
#define KFUSE_STATE_0_WRITE_MASK                        _MK_MASK_CONST(0x83000000)
// Counter of current block during decode, for debugging
#define KFUSE_STATE_0_CURBLOCK_SHIFT                    _MK_SHIFT_CONST(0)
#define KFUSE_STATE_0_CURBLOCK_FIELD                    (_MK_MASK_CONST(0x3f) << KFUSE_STATE_0_CURBLOCK_SHIFT)
#define KFUSE_STATE_0_CURBLOCK_RANGE                    5:0
#define KFUSE_STATE_0_CURBLOCK_WOFFSET                  0x0
#define KFUSE_STATE_0_CURBLOCK_DEFAULT                  _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_CURBLOCK_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_CURBLOCK_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_CURBLOCK_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)

// If any ERR_* are set, contains offset of first errored block
#define KFUSE_STATE_0_ERRBLOCK_SHIFT                    _MK_SHIFT_CONST(8)
#define KFUSE_STATE_0_ERRBLOCK_FIELD                    (_MK_MASK_CONST(0x3f) << KFUSE_STATE_0_ERRBLOCK_SHIFT)
#define KFUSE_STATE_0_ERRBLOCK_RANGE                    13:8
#define KFUSE_STATE_0_ERRBLOCK_WOFFSET                  0x0
#define KFUSE_STATE_0_ERRBLOCK_DEFAULT                  _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_ERRBLOCK_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_ERRBLOCK_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_ERRBLOCK_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)

// When decode is complete, DONE=1
#define KFUSE_STATE_0_DONE_SHIFT                        _MK_SHIFT_CONST(16)
#define KFUSE_STATE_0_DONE_FIELD                        (_MK_MASK_CONST(0x1) << KFUSE_STATE_0_DONE_SHIFT)
#define KFUSE_STATE_0_DONE_RANGE                        16:16
#define KFUSE_STATE_0_DONE_WOFFSET                      0x0
#define KFUSE_STATE_0_DONE_DEFAULT                      _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_DONE_DEFAULT_MASK                 _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_DONE_SW_DEFAULT                   _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_DONE_SW_DEFAULT_MASK                      _MK_MASK_CONST(0x0)

// After DONE, indicates CRC pass/fail
#define KFUSE_STATE_0_CRCPASS_SHIFT                     _MK_SHIFT_CONST(17)
#define KFUSE_STATE_0_CRCPASS_FIELD                     (_MK_MASK_CONST(0x1) << KFUSE_STATE_0_CRCPASS_SHIFT)
#define KFUSE_STATE_0_CRCPASS_RANGE                     17:17
#define KFUSE_STATE_0_CRCPASS_WOFFSET                   0x0
#define KFUSE_STATE_0_CRCPASS_DEFAULT                   _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_CRCPASS_DEFAULT_MASK                      _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_CRCPASS_SW_DEFAULT                        _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_CRCPASS_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)

// Write 1 to re-start decoding, similar to deassertion of reset
#define KFUSE_STATE_0_RESTART_SHIFT                     _MK_SHIFT_CONST(24)
#define KFUSE_STATE_0_RESTART_FIELD                     (_MK_MASK_CONST(0x1) << KFUSE_STATE_0_RESTART_SHIFT)
#define KFUSE_STATE_0_RESTART_RANGE                     24:24
#define KFUSE_STATE_0_RESTART_WOFFSET                   0x0
#define KFUSE_STATE_0_RESTART_DEFAULT                   _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_RESTART_DEFAULT_MASK                      _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_RESTART_SW_DEFAULT                        _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_RESTART_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)

// Write 1 to abort decoding in progress, then wait for STATE=IDLE.
#define KFUSE_STATE_0_STOP_SHIFT                        _MK_SHIFT_CONST(25)
#define KFUSE_STATE_0_STOP_FIELD                        (_MK_MASK_CONST(0x1) << KFUSE_STATE_0_STOP_SHIFT)
#define KFUSE_STATE_0_STOP_RANGE                        25:25
#define KFUSE_STATE_0_STOP_WOFFSET                      0x0
#define KFUSE_STATE_0_STOP_DEFAULT                      _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_STOP_DEFAULT_MASK                 _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_STOP_SW_DEFAULT                   _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_STOP_SW_DEFAULT_MASK                      _MK_MASK_CONST(0x0)

#define KFUSE_STATE_0_SOFTRESET_SHIFT                   _MK_SHIFT_CONST(31)
#define KFUSE_STATE_0_SOFTRESET_FIELD                   (_MK_MASK_CONST(0x1) << KFUSE_STATE_0_SOFTRESET_SHIFT)
#define KFUSE_STATE_0_SOFTRESET_RANGE                   31:31
#define KFUSE_STATE_0_SOFTRESET_WOFFSET                 0x0
#define KFUSE_STATE_0_SOFTRESET_DEFAULT                 _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_SOFTRESET_DEFAULT_MASK                    _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_SOFTRESET_SW_DEFAULT                      _MK_MASK_CONST(0x0)
#define KFUSE_STATE_0_SOFTRESET_SW_DEFAULT_MASK                 _MK_MASK_CONST(0x0)


// Register KFUSE_ERRCOUNT_0  // ECC decode error count; valid after DONE=1.
#define KFUSE_ERRCOUNT_0                        _MK_ADDR_CONST(0x84)
#define KFUSE_ERRCOUNT_0_SECURE                         0x0
#define KFUSE_ERRCOUNT_0_WORD_COUNT                     0x1
#define KFUSE_ERRCOUNT_0_RESET_VAL                      _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_RESET_MASK                     _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_SW_DEFAULT_VAL                         _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_SW_DEFAULT_MASK                        _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_READ_MASK                      _MK_MASK_CONST(0x7f7f7f7f)
#define KFUSE_ERRCOUNT_0_WRITE_MASK                     _MK_MASK_CONST(0x0)
// number of correctable 1-bit errors
#define KFUSE_ERRCOUNT_0_ERR_1_SHIFT                    _MK_SHIFT_CONST(0)
#define KFUSE_ERRCOUNT_0_ERR_1_FIELD                    (_MK_MASK_CONST(0x7f) << KFUSE_ERRCOUNT_0_ERR_1_SHIFT)
#define KFUSE_ERRCOUNT_0_ERR_1_RANGE                    6:0
#define KFUSE_ERRCOUNT_0_ERR_1_WOFFSET                  0x0
#define KFUSE_ERRCOUNT_0_ERR_1_DEFAULT                  _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_1_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_1_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_1_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)

// number of correctable 2-bit errors
#define KFUSE_ERRCOUNT_0_ERR_2_SHIFT                    _MK_SHIFT_CONST(8)
#define KFUSE_ERRCOUNT_0_ERR_2_FIELD                    (_MK_MASK_CONST(0x7f) << KFUSE_ERRCOUNT_0_ERR_2_SHIFT)
#define KFUSE_ERRCOUNT_0_ERR_2_RANGE                    14:8
#define KFUSE_ERRCOUNT_0_ERR_2_WOFFSET                  0x0
#define KFUSE_ERRCOUNT_0_ERR_2_DEFAULT                  _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_2_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_2_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_2_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)

// number of correctable 3-bit errors
#define KFUSE_ERRCOUNT_0_ERR_3_SHIFT                    _MK_SHIFT_CONST(16)
#define KFUSE_ERRCOUNT_0_ERR_3_FIELD                    (_MK_MASK_CONST(0x7f) << KFUSE_ERRCOUNT_0_ERR_3_SHIFT)
#define KFUSE_ERRCOUNT_0_ERR_3_RANGE                    22:16
#define KFUSE_ERRCOUNT_0_ERR_3_WOFFSET                  0x0
#define KFUSE_ERRCOUNT_0_ERR_3_DEFAULT                  _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_3_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_3_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_3_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)

// number of uncorrectable errors
#define KFUSE_ERRCOUNT_0_ERR_FATAL_SHIFT                        _MK_SHIFT_CONST(24)
#define KFUSE_ERRCOUNT_0_ERR_FATAL_FIELD                        (_MK_MASK_CONST(0x7f) << KFUSE_ERRCOUNT_0_ERR_FATAL_SHIFT)
#define KFUSE_ERRCOUNT_0_ERR_FATAL_RANGE                        30:24
#define KFUSE_ERRCOUNT_0_ERR_FATAL_WOFFSET                      0x0
#define KFUSE_ERRCOUNT_0_ERR_FATAL_DEFAULT                      _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_FATAL_DEFAULT_MASK                 _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_FATAL_SW_DEFAULT                   _MK_MASK_CONST(0x0)
#define KFUSE_ERRCOUNT_0_ERR_FATAL_SW_DEFAULT_MASK                      _MK_MASK_CONST(0x0)


// Register KFUSE_KEYADDR_0  // For reading keyglob data after decode is DONE.
#define KFUSE_KEYADDR_0                 _MK_ADDR_CONST(0x88)
#define KFUSE_KEYADDR_0_SECURE                  0x0
#define KFUSE_KEYADDR_0_WORD_COUNT                      0x1
#define KFUSE_KEYADDR_0_RESET_VAL                       _MK_MASK_CONST(0x10000)
#define KFUSE_KEYADDR_0_RESET_MASK                      _MK_MASK_CONST(0x100ff)
#define KFUSE_KEYADDR_0_SW_DEFAULT_VAL                  _MK_MASK_CONST(0x0)
#define KFUSE_KEYADDR_0_SW_DEFAULT_MASK                         _MK_MASK_CONST(0x0)
#define KFUSE_KEYADDR_0_READ_MASK                       _MK_MASK_CONST(0x100ff)
#define KFUSE_KEYADDR_0_WRITE_MASK                      _MK_MASK_CONST(0x100ff)
// Word address (0..144)
#define KFUSE_KEYADDR_0_ADDR_SHIFT                      _MK_SHIFT_CONST(0)
#define KFUSE_KEYADDR_0_ADDR_FIELD                      (_MK_MASK_CONST(0xff) << KFUSE_KEYADDR_0_ADDR_SHIFT)
#define KFUSE_KEYADDR_0_ADDR_RANGE                      7:0
#define KFUSE_KEYADDR_0_ADDR_WOFFSET                    0x0
#define KFUSE_KEYADDR_0_ADDR_DEFAULT                    _MK_MASK_CONST(0x0)
#define KFUSE_KEYADDR_0_ADDR_DEFAULT_MASK                       _MK_MASK_CONST(0xff)
#define KFUSE_KEYADDR_0_ADDR_SW_DEFAULT                 _MK_MASK_CONST(0x0)
#define KFUSE_KEYADDR_0_ADDR_SW_DEFAULT_MASK                    _MK_MASK_CONST(0x0)

// when set, ADDR is incremented by 1 after each read of KEYS
#define KFUSE_KEYADDR_0_AUTOINC_SHIFT                   _MK_SHIFT_CONST(16)
#define KFUSE_KEYADDR_0_AUTOINC_FIELD                   (_MK_MASK_CONST(0x1) << KFUSE_KEYADDR_0_AUTOINC_SHIFT)
#define KFUSE_KEYADDR_0_AUTOINC_RANGE                   16:16
#define KFUSE_KEYADDR_0_AUTOINC_WOFFSET                 0x0
#define KFUSE_KEYADDR_0_AUTOINC_DEFAULT                 _MK_MASK_CONST(0x1)
#define KFUSE_KEYADDR_0_AUTOINC_DEFAULT_MASK                    _MK_MASK_CONST(0x1)
#define KFUSE_KEYADDR_0_AUTOINC_SW_DEFAULT                      _MK_MASK_CONST(0x0)
#define KFUSE_KEYADDR_0_AUTOINC_SW_DEFAULT_MASK                 _MK_MASK_CONST(0x0)


// Register KFUSE_KEYS_0  // decoded keyglob data; after each read, ADDR is incremented if AUTOINC==1
#define KFUSE_KEYS_0                    _MK_ADDR_CONST(0x8c)
#define KFUSE_KEYS_0_SECURE                     0x0
#define KFUSE_KEYS_0_WORD_COUNT                         0x1
#define KFUSE_KEYS_0_RESET_VAL                  _MK_MASK_CONST(0x0)
#define KFUSE_KEYS_0_RESET_MASK                         _MK_MASK_CONST(0x0)
#define KFUSE_KEYS_0_SW_DEFAULT_VAL                     _MK_MASK_CONST(0x0)
#define KFUSE_KEYS_0_SW_DEFAULT_MASK                    _MK_MASK_CONST(0x0)
#define KFUSE_KEYS_0_READ_MASK                  _MK_MASK_CONST(0xffffffff)
#define KFUSE_KEYS_0_WRITE_MASK                         _MK_MASK_CONST(0x0)
#define KFUSE_KEYS_0_DATA_SHIFT                 _MK_SHIFT_CONST(0)
#define KFUSE_KEYS_0_DATA_FIELD                 (_MK_MASK_CONST(0xffffffff) << KFUSE_KEYS_0_DATA_SHIFT)
#define KFUSE_KEYS_0_DATA_RANGE                 31:0
#define KFUSE_KEYS_0_DATA_WOFFSET                       0x0
#define KFUSE_KEYS_0_DATA_DEFAULT                       _MK_MASK_CONST(0x0)
#define KFUSE_KEYS_0_DATA_DEFAULT_MASK                  _MK_MASK_CONST(0x0)
#define KFUSE_KEYS_0_DATA_SW_DEFAULT                    _MK_MASK_CONST(0x0)
#define KFUSE_KEYS_0_DATA_SW_DEFAULT_MASK                       _MK_MASK_CONST(0x0)


//
// REGISTER LIST
//
#define LIST_ARKFUSE_REGS(_op_) \
_op_(KFUSE_FUSECTRL_0) \
_op_(KFUSE_FUSEADDR_0) \
_op_(KFUSE_FUSEDATA0_0) \
_op_(KFUSE_FUSEWRDATA0_0) \
_op_(KFUSE_FUSETIME_RD1_0) \
_op_(KFUSE_FUSETIME_RD2_0) \
_op_(KFUSE_FUSETIME_PGM1_0) \
_op_(KFUSE_FUSETIME_PGM2_0) \
_op_(KFUSE_REGULATOR_0) \
_op_(KFUSE_STATE_0) \
_op_(KFUSE_ERRCOUNT_0) \
_op_(KFUSE_KEYADDR_0) \
_op_(KFUSE_KEYS_0)


//
// ADDRESS SPACES
//

#define BASE_ADDRESS_KFUSE      0x00000000

//
// ARKFUSE REGISTER BANKS
//

#define KFUSE0_FIRST_REG 0x0000 // KFUSE_FUSECTRL_0
#define KFUSE0_LAST_REG 0x0020 // KFUSE_REGULATOR_0
#define KFUSE1_FIRST_REG 0x0080 // KFUSE_STATE_0
#define KFUSE1_LAST_REG 0x008c // KFUSE_KEYS_0

#ifndef _MK_SHIFT_CONST
  #define _MK_SHIFT_CONST(_constant_) _constant_
#endif
#ifndef _MK_MASK_CONST
  #define _MK_MASK_CONST(_constant_) _constant_
#endif
#ifndef _MK_ENUM_CONST
  #define _MK_ENUM_CONST(_constant_) (_constant_ ## UL)
#endif
#ifndef _MK_ADDR_CONST
  #define _MK_ADDR_CONST(_constant_) _constant_
#endif

#endif // ifndef ___ARKFUSE_H_INC_
