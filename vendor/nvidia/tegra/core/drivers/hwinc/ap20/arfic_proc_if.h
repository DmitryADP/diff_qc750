//
// DO NOT EDIT - generated by simspec!
//

#ifndef ___ARFIC_PROC_IF_H_INC_
#define ___ARFIC_PROC_IF_H_INC_
// --------------------------------------------------------------------------
//
// Copyright (c) 2008, NVIDIA Corp.
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
// This register spec is based on section 4.8 in the Cortex-A9 Mpcore TRM.
//
//
// secure mode

// Register FIC_PROC_IF_CONTROL_0  
#define FIC_PROC_IF_CONTROL_0                   _MK_ADDR_CONST(0x100)
#define FIC_PROC_IF_CONTROL_0_SECURE                    0x0
#define FIC_PROC_IF_CONTROL_0_WORD_COUNT                        0x1
#define FIC_PROC_IF_CONTROL_0_RESET_VAL                         _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_RESET_MASK                        _MK_MASK_CONST(0x1f)
#define FIC_PROC_IF_CONTROL_0_SW_DEFAULT_VAL                    _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_READ_MASK                         _MK_MASK_CONST(0x1f)
#define FIC_PROC_IF_CONTROL_0_WRITE_MASK                        _MK_MASK_CONST(0x1f)
// Secure enable for the Cortex-A9 processor interface
#define FIC_PROC_IF_CONTROL_0_ENABLE_S_SHIFT                    _MK_SHIFT_CONST(0)
#define FIC_PROC_IF_CONTROL_0_ENABLE_S_FIELD                    (_MK_MASK_CONST(0x1) << FIC_PROC_IF_CONTROL_0_ENABLE_S_SHIFT)
#define FIC_PROC_IF_CONTROL_0_ENABLE_S_RANGE                    0:0
#define FIC_PROC_IF_CONTROL_0_ENABLE_S_WOFFSET                  0x0
#define FIC_PROC_IF_CONTROL_0_ENABLE_S_DEFAULT                  _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_ENABLE_S_DEFAULT_MASK                     _MK_MASK_CONST(0x1)
#define FIC_PROC_IF_CONTROL_0_ENABLE_S_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_ENABLE_S_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)

// Non-secure enable for the Cortex-A9 processor interface
#define FIC_PROC_IF_CONTROL_0_ENABLE_NS_SHIFT                   _MK_SHIFT_CONST(1)
#define FIC_PROC_IF_CONTROL_0_ENABLE_NS_FIELD                   (_MK_MASK_CONST(0x1) << FIC_PROC_IF_CONTROL_0_ENABLE_NS_SHIFT)
#define FIC_PROC_IF_CONTROL_0_ENABLE_NS_RANGE                   1:1
#define FIC_PROC_IF_CONTROL_0_ENABLE_NS_WOFFSET                 0x0
#define FIC_PROC_IF_CONTROL_0_ENABLE_NS_DEFAULT                 _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_ENABLE_NS_DEFAULT_MASK                    _MK_MASK_CONST(0x1)
#define FIC_PROC_IF_CONTROL_0_ENABLE_NS_SW_DEFAULT                      _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_ENABLE_NS_SW_DEFAULT_MASK                 _MK_MASK_CONST(0x0)

// When a Cortex-A9 processor performs a secure read of
// the int_ack Register and the highest
// priority interrupt is non-secure, this bit controls
// the acknowledge response as follows:
// 0 = The Cortex-A9 processor interface returns an INTID
// value of 1022 and the interrupt remains Pending.
// 1 = The Cortex-A9 processor interface returns the INTID
// value of the non-secure interrupt and
// acknowledges the interrupt. The interrupt changes state
// to Active, or Active-and-pending.
// When a Cortex-A9 processor performs a secure write to
// the EOI Register to signal the completion
// of a non-secure interrupt, this bit controls if the
// Interrupt Controller clears the interrupt as follows:
// 0 = the Interrupt Controller ignores the write and the
// interrupt remains Active, or
// Active-and-pending
// 1 = the Interrupt Controller changes the interrupt
// status to Inactive, or Pending.
#define FIC_PROC_IF_CONTROL_0_ACK_CTL_SHIFT                     _MK_SHIFT_CONST(2)
#define FIC_PROC_IF_CONTROL_0_ACK_CTL_FIELD                     (_MK_MASK_CONST(0x1) << FIC_PROC_IF_CONTROL_0_ACK_CTL_SHIFT)
#define FIC_PROC_IF_CONTROL_0_ACK_CTL_RANGE                     2:2
#define FIC_PROC_IF_CONTROL_0_ACK_CTL_WOFFSET                   0x0
#define FIC_PROC_IF_CONTROL_0_ACK_CTL_DEFAULT                   _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_ACK_CTL_DEFAULT_MASK                      _MK_MASK_CONST(0x1)
#define FIC_PROC_IF_CONTROL_0_ACK_CTL_SW_DEFAULT                        _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_ACK_CTL_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)

// Enables the Cortex-A9 processor interface to send
// secure interrupts using the nFIQ signal.
#define FIC_PROC_IF_CONTROL_0_FIQ_ENABLE_SHIFT                  _MK_SHIFT_CONST(3)
#define FIC_PROC_IF_CONTROL_0_FIQ_ENABLE_FIELD                  (_MK_MASK_CONST(0x1) << FIC_PROC_IF_CONTROL_0_FIQ_ENABLE_SHIFT)
#define FIC_PROC_IF_CONTROL_0_FIQ_ENABLE_RANGE                  3:3
#define FIC_PROC_IF_CONTROL_0_FIQ_ENABLE_WOFFSET                        0x0
#define FIC_PROC_IF_CONTROL_0_FIQ_ENABLE_DEFAULT                        _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_FIQ_ENABLE_DEFAULT_MASK                   _MK_MASK_CONST(0x1)
#define FIC_PROC_IF_CONTROL_0_FIQ_ENABLE_SW_DEFAULT                     _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_FIQ_ENABLE_SW_DEFAULT_MASK                        _MK_MASK_CONST(0x0)

// Controls which Binary Pointer Register the Cortex-A9
// processor interface uses when it performs
// a pre-emptive calculation. The options are:
// 0 = secure interrupts use the bin_pt_s Register and
// non-secure interrupts use the bin_pt_ns Register
// 1 = secure read and writes access the secure binary
// point register directly. Non-secure writes are
// ignored, and non-secure reads return the value in the
// secure binary point register plus 1, with the
// addition saturating at a value of 7.
#define FIC_PROC_IF_CONTROL_0_SBPR_SHIFT                        _MK_SHIFT_CONST(4)
#define FIC_PROC_IF_CONTROL_0_SBPR_FIELD                        (_MK_MASK_CONST(0x1) << FIC_PROC_IF_CONTROL_0_SBPR_SHIFT)
#define FIC_PROC_IF_CONTROL_0_SBPR_RANGE                        4:4
#define FIC_PROC_IF_CONTROL_0_SBPR_WOFFSET                      0x0
#define FIC_PROC_IF_CONTROL_0_SBPR_DEFAULT                      _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_SBPR_DEFAULT_MASK                 _MK_MASK_CONST(0x1)
#define FIC_PROC_IF_CONTROL_0_SBPR_SW_DEFAULT                   _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CONTROL_0_SBPR_SW_DEFAULT_MASK                      _MK_MASK_CONST(0x0)


// Register FIC_PROC_IF_PRIORITY_MASK_0  
#define FIC_PROC_IF_PRIORITY_MASK_0                     _MK_ADDR_CONST(0x104)
#define FIC_PROC_IF_PRIORITY_MASK_0_SECURE                      0x0
#define FIC_PROC_IF_PRIORITY_MASK_0_WORD_COUNT                  0x1
#define FIC_PROC_IF_PRIORITY_MASK_0_RESET_VAL                   _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_PRIORITY_MASK_0_RESET_MASK                  _MK_MASK_CONST(0xf8)
#define FIC_PROC_IF_PRIORITY_MASK_0_SW_DEFAULT_VAL                      _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_PRIORITY_MASK_0_SW_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_PRIORITY_MASK_0_READ_MASK                   _MK_MASK_CONST(0xf8)
#define FIC_PROC_IF_PRIORITY_MASK_0_WRITE_MASK                  _MK_MASK_CONST(0xf8)
// Configures the priorty mask level for the Cortex-A9 processor.
#define FIC_PROC_IF_PRIORITY_MASK_0_PRIORITY_SHIFT                      _MK_SHIFT_CONST(3)
#define FIC_PROC_IF_PRIORITY_MASK_0_PRIORITY_FIELD                      (_MK_MASK_CONST(0x1f) << FIC_PROC_IF_PRIORITY_MASK_0_PRIORITY_SHIFT)
#define FIC_PROC_IF_PRIORITY_MASK_0_PRIORITY_RANGE                      7:3
#define FIC_PROC_IF_PRIORITY_MASK_0_PRIORITY_WOFFSET                    0x0
#define FIC_PROC_IF_PRIORITY_MASK_0_PRIORITY_DEFAULT                    _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_PRIORITY_MASK_0_PRIORITY_DEFAULT_MASK                       _MK_MASK_CONST(0x1f)
#define FIC_PROC_IF_PRIORITY_MASK_0_PRIORITY_SW_DEFAULT                 _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_PRIORITY_MASK_0_PRIORITY_SW_DEFAULT_MASK                    _MK_MASK_CONST(0x0)


// Register FIC_PROC_IF_BIN_PT_0  
#define FIC_PROC_IF_BIN_PT_0                    _MK_ADDR_CONST(0x108)
#define FIC_PROC_IF_BIN_PT_0_SECURE                     0x0
#define FIC_PROC_IF_BIN_PT_0_WORD_COUNT                         0x1
#define FIC_PROC_IF_BIN_PT_0_RESET_VAL                  _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_BIN_PT_0_RESET_MASK                         _MK_MASK_CONST(0x7)
#define FIC_PROC_IF_BIN_PT_0_SW_DEFAULT_VAL                     _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_BIN_PT_0_SW_DEFAULT_MASK                    _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_BIN_PT_0_READ_MASK                  _MK_MASK_CONST(0x7)
#define FIC_PROC_IF_BIN_PT_0_WRITE_MASK                         _MK_MASK_CONST(0x7)
// Configures the value of the binary point mask.
#define FIC_PROC_IF_BIN_PT_0_BINARY_POINT_SHIFT                 _MK_SHIFT_CONST(0)
#define FIC_PROC_IF_BIN_PT_0_BINARY_POINT_FIELD                 (_MK_MASK_CONST(0x7) << FIC_PROC_IF_BIN_PT_0_BINARY_POINT_SHIFT)
#define FIC_PROC_IF_BIN_PT_0_BINARY_POINT_RANGE                 2:0
#define FIC_PROC_IF_BIN_PT_0_BINARY_POINT_WOFFSET                       0x0
#define FIC_PROC_IF_BIN_PT_0_BINARY_POINT_DEFAULT                       _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_BIN_PT_0_BINARY_POINT_DEFAULT_MASK                  _MK_MASK_CONST(0x7)
#define FIC_PROC_IF_BIN_PT_0_BINARY_POINT_SW_DEFAULT                    _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_BIN_PT_0_BINARY_POINT_SW_DEFAULT_MASK                       _MK_MASK_CONST(0x0)


// Register FIC_PROC_IF_INT_ACK_0  
#define FIC_PROC_IF_INT_ACK_0                   _MK_ADDR_CONST(0x10c)
#define FIC_PROC_IF_INT_ACK_0_SECURE                    0x0
#define FIC_PROC_IF_INT_ACK_0_WORD_COUNT                        0x1
#define FIC_PROC_IF_INT_ACK_0_RESET_VAL                         _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_INT_ACK_0_RESET_MASK                        _MK_MASK_CONST(0x1fff)
#define FIC_PROC_IF_INT_ACK_0_SW_DEFAULT_VAL                    _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_INT_ACK_0_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_INT_ACK_0_READ_MASK                         _MK_MASK_CONST(0x1fff)
#define FIC_PROC_IF_INT_ACK_0_WRITE_MASK                        _MK_MASK_CONST(0x0)
// Returns the INTID of the interrupt that requries
// servicing by the Cortex-A9 processor    :
// 15-0 = STI[15:0]
// 31-16 = PPI[15:0]
// 255-32 = SPI[223:0]
// 1020 = reserved
// 1021 = reserved
// 1022 = the highest priority interrutp that requires
//        servicing is non-secure
// 1023 = no outstanding interrupts
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_SHIFT                   _MK_SHIFT_CONST(0)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_FIELD                   (_MK_MASK_CONST(0x3ff) << FIC_PROC_IF_INT_ACK_0_ACK_INTID_SHIFT)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_RANGE                   9:0
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_WOFFSET                 0x0
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_DEFAULT                 _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_DEFAULT_MASK                    _MK_MASK_CONST(0x3ff)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_SW_DEFAULT                      _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_SW_DEFAULT_MASK                 _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_STI_LOW                 _MK_ENUM_CONST(0)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_STI_HIGH                        _MK_ENUM_CONST(15)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_PPI_LOW                 _MK_ENUM_CONST(16)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_PPI_HIGH                        _MK_ENUM_CONST(31)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_SPI_LOW                 _MK_ENUM_CONST(32)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_SPI_HIGH                        _MK_ENUM_CONST(160)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_HIGHEST_PRI_NONSECURE                   _MK_ENUM_CONST(1022)
#define FIC_PROC_IF_INT_ACK_0_ACK_INTID_NO_OUTSTANDING_INTR                     _MK_ENUM_CONST(1023)

// Returns the CPUID of the Cortex-A9 processor that
// requested the software interrupt
#define FIC_PROC_IF_INT_ACK_0_ACK_SOURCE_CPUID_SHIFT                    _MK_SHIFT_CONST(10)
#define FIC_PROC_IF_INT_ACK_0_ACK_SOURCE_CPUID_FIELD                    (_MK_MASK_CONST(0x7) << FIC_PROC_IF_INT_ACK_0_ACK_SOURCE_CPUID_SHIFT)
#define FIC_PROC_IF_INT_ACK_0_ACK_SOURCE_CPUID_RANGE                    12:10
#define FIC_PROC_IF_INT_ACK_0_ACK_SOURCE_CPUID_WOFFSET                  0x0
#define FIC_PROC_IF_INT_ACK_0_ACK_SOURCE_CPUID_DEFAULT                  _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_INT_ACK_0_ACK_SOURCE_CPUID_DEFAULT_MASK                     _MK_MASK_CONST(0x7)
#define FIC_PROC_IF_INT_ACK_0_ACK_SOURCE_CPUID_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_INT_ACK_0_ACK_SOURCE_CPUID_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)


// Register FIC_PROC_IF_EOI_0  
#define FIC_PROC_IF_EOI_0                       _MK_ADDR_CONST(0x110)
#define FIC_PROC_IF_EOI_0_SECURE                        0x0
#define FIC_PROC_IF_EOI_0_WORD_COUNT                    0x1
#define FIC_PROC_IF_EOI_0_RESET_VAL                     _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_EOI_0_RESET_MASK                    _MK_MASK_CONST(0x1fff)
#define FIC_PROC_IF_EOI_0_SW_DEFAULT_VAL                        _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_EOI_0_SW_DEFAULT_MASK                       _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_EOI_0_READ_MASK                     _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_EOI_0_WRITE_MASK                    _MK_MASK_CONST(0x1fff)
// After the Cortex-A9 processor completes its interrupt
// service routine, it sets this field to the
// INTID of the interrupt that it serviced:
// 15-0 = STI[15:0]
// 31-16 = PPI[15:0]
// 255-32 = SPI[223:0]
// 1023-1020 = reserved
#define FIC_PROC_IF_EOI_0_EOI_INTID_SHIFT                       _MK_SHIFT_CONST(0)
#define FIC_PROC_IF_EOI_0_EOI_INTID_FIELD                       (_MK_MASK_CONST(0x3ff) << FIC_PROC_IF_EOI_0_EOI_INTID_SHIFT)
#define FIC_PROC_IF_EOI_0_EOI_INTID_RANGE                       9:0
#define FIC_PROC_IF_EOI_0_EOI_INTID_WOFFSET                     0x0
#define FIC_PROC_IF_EOI_0_EOI_INTID_DEFAULT                     _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_EOI_0_EOI_INTID_DEFAULT_MASK                        _MK_MASK_CONST(0x3ff)
#define FIC_PROC_IF_EOI_0_EOI_INTID_SW_DEFAULT                  _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_EOI_0_EOI_INTID_SW_DEFAULT_MASK                     _MK_MASK_CONST(0x0)

// After the Cortex-A9 processor completes the interrupt
// service routine for an STI, it sets this to the source
// CPUID of the STI that it serviced
#define FIC_PROC_IF_EOI_0_EOI_SOURCE_CPUID_SHIFT                        _MK_SHIFT_CONST(10)
#define FIC_PROC_IF_EOI_0_EOI_SOURCE_CPUID_FIELD                        (_MK_MASK_CONST(0x7) << FIC_PROC_IF_EOI_0_EOI_SOURCE_CPUID_SHIFT)
#define FIC_PROC_IF_EOI_0_EOI_SOURCE_CPUID_RANGE                        12:10
#define FIC_PROC_IF_EOI_0_EOI_SOURCE_CPUID_WOFFSET                      0x0
#define FIC_PROC_IF_EOI_0_EOI_SOURCE_CPUID_DEFAULT                      _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_EOI_0_EOI_SOURCE_CPUID_DEFAULT_MASK                 _MK_MASK_CONST(0x7)
#define FIC_PROC_IF_EOI_0_EOI_SOURCE_CPUID_SW_DEFAULT                   _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_EOI_0_EOI_SOURCE_CPUID_SW_DEFAULT_MASK                      _MK_MASK_CONST(0x0)


// Register FIC_PROC_IF_RUN_PRIORITY_0  
#define FIC_PROC_IF_RUN_PRIORITY_0                      _MK_ADDR_CONST(0x114)
#define FIC_PROC_IF_RUN_PRIORITY_0_SECURE                       0x0
#define FIC_PROC_IF_RUN_PRIORITY_0_WORD_COUNT                   0x1
#define FIC_PROC_IF_RUN_PRIORITY_0_RESET_VAL                    _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_RUN_PRIORITY_0_RESET_MASK                   _MK_MASK_CONST(0xf0)
#define FIC_PROC_IF_RUN_PRIORITY_0_SW_DEFAULT_VAL                       _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_RUN_PRIORITY_0_SW_DEFAULT_MASK                      _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_RUN_PRIORITY_0_READ_MASK                    _MK_MASK_CONST(0xf0)
#define FIC_PROC_IF_RUN_PRIORITY_0_WRITE_MASK                   _MK_MASK_CONST(0x0)
// Returns the priority level of the highest priority
// interrupt that is running on the Cortex-A9 processor
#define FIC_PROC_IF_RUN_PRIORITY_0_RUN_PRIORITY_SHIFT                   _MK_SHIFT_CONST(4)
#define FIC_PROC_IF_RUN_PRIORITY_0_RUN_PRIORITY_FIELD                   (_MK_MASK_CONST(0xf) << FIC_PROC_IF_RUN_PRIORITY_0_RUN_PRIORITY_SHIFT)
#define FIC_PROC_IF_RUN_PRIORITY_0_RUN_PRIORITY_RANGE                   7:4
#define FIC_PROC_IF_RUN_PRIORITY_0_RUN_PRIORITY_WOFFSET                 0x0
#define FIC_PROC_IF_RUN_PRIORITY_0_RUN_PRIORITY_DEFAULT                 _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_RUN_PRIORITY_0_RUN_PRIORITY_DEFAULT_MASK                    _MK_MASK_CONST(0xf)
#define FIC_PROC_IF_RUN_PRIORITY_0_RUN_PRIORITY_SW_DEFAULT                      _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_RUN_PRIORITY_0_RUN_PRIORITY_SW_DEFAULT_MASK                 _MK_MASK_CONST(0x0)


// Register FIC_PROC_IF_HI_PEND_0  
#define FIC_PROC_IF_HI_PEND_0                   _MK_ADDR_CONST(0x118)
#define FIC_PROC_IF_HI_PEND_0_SECURE                    0x0
#define FIC_PROC_IF_HI_PEND_0_WORD_COUNT                        0x1
#define FIC_PROC_IF_HI_PEND_0_RESET_VAL                         _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_HI_PEND_0_RESET_MASK                        _MK_MASK_CONST(0x1fff)
#define FIC_PROC_IF_HI_PEND_0_SW_DEFAULT_VAL                    _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_HI_PEND_0_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_HI_PEND_0_READ_MASK                         _MK_MASK_CONST(0x1fff)
#define FIC_PROC_IF_HI_PEND_0_WRITE_MASK                        _MK_MASK_CONST(0x0)
// Returns the INTID of the highest priority pending interrupt
// 15-0 = STI[15:0]
// 31-16 = PPI[15:0]
// 255-32 = SPI[223:0]
// 1020 = reserved
// 1021 = reserved
// 1022 = the highest priority interrupT that requires
//        servicing is non-secure
// 1023 = no outstanding interrupts
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_SHIFT                  _MK_SHIFT_CONST(0)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_FIELD                  (_MK_MASK_CONST(0x3ff) << FIC_PROC_IF_HI_PEND_0_PEND_INTID_SHIFT)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_RANGE                  9:0
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_WOFFSET                        0x0
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_DEFAULT                        _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_DEFAULT_MASK                   _MK_MASK_CONST(0x3ff)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_SW_DEFAULT                     _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_SW_DEFAULT_MASK                        _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_STI_LOW                        _MK_ENUM_CONST(0)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_STI_HIGH                       _MK_ENUM_CONST(15)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_PPI_LOW                        _MK_ENUM_CONST(16)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_PPI_HIGH                       _MK_ENUM_CONST(31)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_SPI_LOW                        _MK_ENUM_CONST(32)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_SPI_HIGH                       _MK_ENUM_CONST(160)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_HIGHEST_PRI_NONSECURE                  _MK_ENUM_CONST(1022)
#define FIC_PROC_IF_HI_PEND_0_PEND_INTID_NO_OUTSTANDING_INTR                    _MK_ENUM_CONST(1023)

// Returns the CPUID of the Cortex-A9 processor that is
// requesting the software interrupt
#define FIC_PROC_IF_HI_PEND_0_PEND_SOURCE_CPUID_SHIFT                   _MK_SHIFT_CONST(10)
#define FIC_PROC_IF_HI_PEND_0_PEND_SOURCE_CPUID_FIELD                   (_MK_MASK_CONST(0x7) << FIC_PROC_IF_HI_PEND_0_PEND_SOURCE_CPUID_SHIFT)
#define FIC_PROC_IF_HI_PEND_0_PEND_SOURCE_CPUID_RANGE                   12:10
#define FIC_PROC_IF_HI_PEND_0_PEND_SOURCE_CPUID_WOFFSET                 0x0
#define FIC_PROC_IF_HI_PEND_0_PEND_SOURCE_CPUID_DEFAULT                 _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_HI_PEND_0_PEND_SOURCE_CPUID_DEFAULT_MASK                    _MK_MASK_CONST(0x7)
#define FIC_PROC_IF_HI_PEND_0_PEND_SOURCE_CPUID_SW_DEFAULT                      _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_HI_PEND_0_PEND_SOURCE_CPUID_SW_DEFAULT_MASK                 _MK_MASK_CONST(0x0)


// Register FIC_PROC_IF_ALIAS_BIN_PT_NS_0  
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0                   _MK_ADDR_CONST(0x11c)
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_SECURE                    0x0
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_WORD_COUNT                        0x1
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_RESET_VAL                         _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_RESET_MASK                        _MK_MASK_CONST(0x7)
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_SW_DEFAULT_VAL                    _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_READ_MASK                         _MK_MASK_CONST(0x7)
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_WRITE_MASK                        _MK_MASK_CONST(0x7)
// Alias of the BIN_PT_NS regiter. Only accessible in secure mode
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_BINARY_POINT_NS_SHIFT                     _MK_SHIFT_CONST(0)
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_BINARY_POINT_NS_FIELD                     (_MK_MASK_CONST(0x7) << FIC_PROC_IF_ALIAS_BIN_PT_NS_0_BINARY_POINT_NS_SHIFT)
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_BINARY_POINT_NS_RANGE                     2:0
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_BINARY_POINT_NS_WOFFSET                   0x0
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_BINARY_POINT_NS_DEFAULT                   _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_BINARY_POINT_NS_DEFAULT_MASK                      _MK_MASK_CONST(0x7)
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_BINARY_POINT_NS_SW_DEFAULT                        _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_ALIAS_BIN_PT_NS_0_BINARY_POINT_NS_SW_DEFAULT_MASK                   _MK_MASK_CONST(0x0)


// Register FIC_PROC_IF_CPU_IF_IDENT_0  
#define FIC_PROC_IF_CPU_IF_IDENT_0                      _MK_ADDR_CONST(0x120)
#define FIC_PROC_IF_CPU_IF_IDENT_0_SECURE                       0x0
#define FIC_PROC_IF_CPU_IF_IDENT_0_WORD_COUNT                   0x1
#define FIC_PROC_IF_CPU_IF_IDENT_0_RESET_VAL                    _MK_MASK_CONST(0x3901043b)
#define FIC_PROC_IF_CPU_IF_IDENT_0_RESET_MASK                   _MK_MASK_CONST(0xffffffff)
#define FIC_PROC_IF_CPU_IF_IDENT_0_SW_DEFAULT_VAL                       _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CPU_IF_IDENT_0_SW_DEFAULT_MASK                      _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CPU_IF_IDENT_0_READ_MASK                    _MK_MASK_CONST(0xffffffff)
#define FIC_PROC_IF_CPU_IF_IDENT_0_WRITE_MASK                   _MK_MASK_CONST(0x0)
// Returns the JEP106 code of the company that implemented
// the Cortex-A9 processor interface RTL.
#define FIC_PROC_IF_CPU_IF_IDENT_0_IMPLEMENTOR_SHIFT                    _MK_SHIFT_CONST(0)
#define FIC_PROC_IF_CPU_IF_IDENT_0_IMPLEMENTOR_FIELD                    (_MK_MASK_CONST(0xfff) << FIC_PROC_IF_CPU_IF_IDENT_0_IMPLEMENTOR_SHIFT)
#define FIC_PROC_IF_CPU_IF_IDENT_0_IMPLEMENTOR_RANGE                    11:0
#define FIC_PROC_IF_CPU_IF_IDENT_0_IMPLEMENTOR_WOFFSET                  0x0
#define FIC_PROC_IF_CPU_IF_IDENT_0_IMPLEMENTOR_DEFAULT                  _MK_MASK_CONST(0x43b)
#define FIC_PROC_IF_CPU_IF_IDENT_0_IMPLEMENTOR_DEFAULT_MASK                     _MK_MASK_CONST(0xfff)
#define FIC_PROC_IF_CPU_IF_IDENT_0_IMPLEMENTOR_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CPU_IF_IDENT_0_IMPLEMENTOR_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)

// Returns the revision number of the Interrupt Controller
#define FIC_PROC_IF_CPU_IF_IDENT_0_REVISION_NUMBER_SHIFT                        _MK_SHIFT_CONST(12)
#define FIC_PROC_IF_CPU_IF_IDENT_0_REVISION_NUMBER_FIELD                        (_MK_MASK_CONST(0xf) << FIC_PROC_IF_CPU_IF_IDENT_0_REVISION_NUMBER_SHIFT)
#define FIC_PROC_IF_CPU_IF_IDENT_0_REVISION_NUMBER_RANGE                        15:12
#define FIC_PROC_IF_CPU_IF_IDENT_0_REVISION_NUMBER_WOFFSET                      0x0
#define FIC_PROC_IF_CPU_IF_IDENT_0_REVISION_NUMBER_DEFAULT                      _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CPU_IF_IDENT_0_REVISION_NUMBER_DEFAULT_MASK                 _MK_MASK_CONST(0xf)
#define FIC_PROC_IF_CPU_IF_IDENT_0_REVISION_NUMBER_SW_DEFAULT                   _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CPU_IF_IDENT_0_REVISION_NUMBER_SW_DEFAULT_MASK                      _MK_MASK_CONST(0x0)

// Identifies the architecture version
#define FIC_PROC_IF_CPU_IF_IDENT_0_ARCHITECTURE_VERSION_SHIFT                   _MK_SHIFT_CONST(16)
#define FIC_PROC_IF_CPU_IF_IDENT_0_ARCHITECTURE_VERSION_FIELD                   (_MK_MASK_CONST(0xf) << FIC_PROC_IF_CPU_IF_IDENT_0_ARCHITECTURE_VERSION_SHIFT)
#define FIC_PROC_IF_CPU_IF_IDENT_0_ARCHITECTURE_VERSION_RANGE                   19:16
#define FIC_PROC_IF_CPU_IF_IDENT_0_ARCHITECTURE_VERSION_WOFFSET                 0x0
#define FIC_PROC_IF_CPU_IF_IDENT_0_ARCHITECTURE_VERSION_DEFAULT                 _MK_MASK_CONST(0x1)
#define FIC_PROC_IF_CPU_IF_IDENT_0_ARCHITECTURE_VERSION_DEFAULT_MASK                    _MK_MASK_CONST(0xf)
#define FIC_PROC_IF_CPU_IF_IDENT_0_ARCHITECTURE_VERSION_SW_DEFAULT                      _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CPU_IF_IDENT_0_ARCHITECTURE_VERSION_SW_DEFAULT_MASK                 _MK_MASK_CONST(0x0)

// Identifies the peripheral
#define FIC_PROC_IF_CPU_IF_IDENT_0_PART_NUMBER_SHIFT                    _MK_SHIFT_CONST(20)
#define FIC_PROC_IF_CPU_IF_IDENT_0_PART_NUMBER_FIELD                    (_MK_MASK_CONST(0xfff) << FIC_PROC_IF_CPU_IF_IDENT_0_PART_NUMBER_SHIFT)
#define FIC_PROC_IF_CPU_IF_IDENT_0_PART_NUMBER_RANGE                    31:20
#define FIC_PROC_IF_CPU_IF_IDENT_0_PART_NUMBER_WOFFSET                  0x0
#define FIC_PROC_IF_CPU_IF_IDENT_0_PART_NUMBER_DEFAULT                  _MK_MASK_CONST(0x390)
#define FIC_PROC_IF_CPU_IF_IDENT_0_PART_NUMBER_DEFAULT_MASK                     _MK_MASK_CONST(0xfff)
#define FIC_PROC_IF_CPU_IF_IDENT_0_PART_NUMBER_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define FIC_PROC_IF_CPU_IF_IDENT_0_PART_NUMBER_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)


//
// REGISTER LIST
//
#define LIST_ARFIC_PROC_IF_REGS(_op_) \
_op_(FIC_PROC_IF_CONTROL_0) \
_op_(FIC_PROC_IF_PRIORITY_MASK_0) \
_op_(FIC_PROC_IF_BIN_PT_0) \
_op_(FIC_PROC_IF_INT_ACK_0) \
_op_(FIC_PROC_IF_EOI_0) \
_op_(FIC_PROC_IF_RUN_PRIORITY_0) \
_op_(FIC_PROC_IF_HI_PEND_0) \
_op_(FIC_PROC_IF_ALIAS_BIN_PT_NS_0) \
_op_(FIC_PROC_IF_CPU_IF_IDENT_0)


//
// ADDRESS SPACES
//

#define BASE_ADDRESS_FIC_PROC_IF        0x00000100

//
// ARFIC_PROC_IF REGISTER BANKS
//

#define FIC_PROC_IF0_FIRST_REG 0x0100 // FIC_PROC_IF_CONTROL_0
#define FIC_PROC_IF0_LAST_REG 0x0120 // FIC_PROC_IF_CPU_IF_IDENT_0

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

#endif // ifndef ___ARFIC_PROC_IF_H_INC_
