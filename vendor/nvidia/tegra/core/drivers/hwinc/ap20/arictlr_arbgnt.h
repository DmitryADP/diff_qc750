//
// DO NOT EDIT - generated by simspec!
//

#ifndef ___ARICTLR_ARBGNT_H_INC_
#define ___ARICTLR_ARBGNT_H_INC_
// --------------------------------------------------------------------------
//
// Copyright (c) 2007-2012, NVIDIA Corp.
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
// this spec file is for sw header generation
//
// hw should use headers generated from:
//     arintr_ctlr.spec
//
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
// arb_gnt specific interrupt controller registers
//
// Arbitration semaphores provide a mechanism by which the two processors can arbitrate
// for the use of various resources. These semaphores provide a hardware locking mechanism,
// so that when a processor is already using a resource, the second processor is not
// granted that resource. There are 32 bits of Arbitration semaphores provided in the system.
// The hardware does not enforce any resource association to these bits. It is left to the
// firmware to assign and use these bits.
// The setup/usage of the Arbitration Semaphores is described in the ararb_sema specfile.
//
// The Arbitration Semaphores can also generate an interrupt when a hardware resource
// becomes available.  The registers in this module configure these interrupts.
// When a 1 is set in the corresponding bit position of the Arbitration Semaphore Interrupt
// Source Register (CPU_enable or COP_enable), an interrupt will be generated when the
// processor achieves Grant Status for that resource.
//
// The current Grant status can be viewed in the CPU_STATUS or COP_STATUS registers.
//
// CPU Arbitration Semaphore Interrupt Status Register

// Register ARBGNT_CPU_STATUS_0
#define ARBGNT_CPU_STATUS_0                     _MK_ADDR_CONST(0x0)
#define ARBGNT_CPU_STATUS_0_SECURE                      0x0
#define ARBGNT_CPU_STATUS_0_WORD_COUNT                  0x1
#define ARBGNT_CPU_STATUS_0_RESET_VAL                   _MK_MASK_CONST(0x0)
#define ARBGNT_CPU_STATUS_0_RESET_MASK                  _MK_MASK_CONST(0xffffffff)
#define ARBGNT_CPU_STATUS_0_SW_DEFAULT_VAL                      _MK_MASK_CONST(0x0)
#define ARBGNT_CPU_STATUS_0_SW_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define ARBGNT_CPU_STATUS_0_READ_MASK                   _MK_MASK_CONST(0xffffffff)
#define ARBGNT_CPU_STATUS_0_WRITE_MASK                  _MK_MASK_CONST(0x0)
// Each bit is set by hardware when the corresponding  arbitration semaphore ownership is granted to CPU. Interrupt is cleared when  the CPU writes the ARB_SMP.PUT register with the corresponding bit set.
#define ARBGNT_CPU_STATUS_0_GNT31_GNG0_SHIFT                    _MK_SHIFT_CONST(0)
#define ARBGNT_CPU_STATUS_0_GNT31_GNG0_FIELD                    (_MK_MASK_CONST(0xffffffff) << ARBGNT_CPU_STATUS_0_GNT31_GNG0_SHIFT)
#define ARBGNT_CPU_STATUS_0_GNT31_GNG0_RANGE                    31:0
#define ARBGNT_CPU_STATUS_0_GNT31_GNG0_WOFFSET                  0x0
#define ARBGNT_CPU_STATUS_0_GNT31_GNG0_DEFAULT                  _MK_MASK_CONST(0x0)
#define ARBGNT_CPU_STATUS_0_GNT31_GNG0_DEFAULT_MASK                     _MK_MASK_CONST(0xffffffff)
#define ARBGNT_CPU_STATUS_0_GNT31_GNG0_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define ARBGNT_CPU_STATUS_0_GNT31_GNG0_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)

// CPU Arbitration Semaphore Interrupt Enable Register

// Register ARBGNT_CPU_ENABLE_0
#define ARBGNT_CPU_ENABLE_0                     _MK_ADDR_CONST(0x4)
#define ARBGNT_CPU_ENABLE_0_SECURE                      0x0
#define ARBGNT_CPU_ENABLE_0_WORD_COUNT                  0x1
#define ARBGNT_CPU_ENABLE_0_RESET_VAL                   _MK_MASK_CONST(0x0)
#define ARBGNT_CPU_ENABLE_0_RESET_MASK                  _MK_MASK_CONST(0xffffffff)
#define ARBGNT_CPU_ENABLE_0_SW_DEFAULT_VAL                      _MK_MASK_CONST(0x0)
#define ARBGNT_CPU_ENABLE_0_SW_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define ARBGNT_CPU_ENABLE_0_READ_MASK                   _MK_MASK_CONST(0xffffffff)
#define ARBGNT_CPU_ENABLE_0_WRITE_MASK                  _MK_MASK_CONST(0xffffffff)
// Writing a 1 in any bit position will enable the  corresponding arbitration semaphore interrupt.
#define ARBGNT_CPU_ENABLE_0_GER31_GER0_SHIFT                    _MK_SHIFT_CONST(0)
#define ARBGNT_CPU_ENABLE_0_GER31_GER0_FIELD                    (_MK_MASK_CONST(0xffffffff) << ARBGNT_CPU_ENABLE_0_GER31_GER0_SHIFT)
#define ARBGNT_CPU_ENABLE_0_GER31_GER0_RANGE                    31:0
#define ARBGNT_CPU_ENABLE_0_GER31_GER0_WOFFSET                  0x0
#define ARBGNT_CPU_ENABLE_0_GER31_GER0_DEFAULT                  _MK_MASK_CONST(0x0)
#define ARBGNT_CPU_ENABLE_0_GER31_GER0_DEFAULT_MASK                     _MK_MASK_CONST(0xffffffff)
#define ARBGNT_CPU_ENABLE_0_GER31_GER0_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define ARBGNT_CPU_ENABLE_0_GER31_GER0_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)

// COP Arbitration Semaphore Interrupt Status Register

// Register ARBGNT_COP_STATUS_0
#define ARBGNT_COP_STATUS_0                     _MK_ADDR_CONST(0x8)
#define ARBGNT_COP_STATUS_0_SECURE                      0x0
#define ARBGNT_COP_STATUS_0_WORD_COUNT                  0x1
#define ARBGNT_COP_STATUS_0_RESET_VAL                   _MK_MASK_CONST(0x0)
#define ARBGNT_COP_STATUS_0_RESET_MASK                  _MK_MASK_CONST(0xffffffff)
#define ARBGNT_COP_STATUS_0_SW_DEFAULT_VAL                      _MK_MASK_CONST(0x0)
#define ARBGNT_COP_STATUS_0_SW_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define ARBGNT_COP_STATUS_0_READ_MASK                   _MK_MASK_CONST(0xffffffff)
#define ARBGNT_COP_STATUS_0_WRITE_MASK                  _MK_MASK_CONST(0x0)
// Each bit is set by hardware when the corresponding  arbitration semaphore ownership is granted to COP. Interrupt is cleared when  the COP writes the ARB_SMP.PUT register with the corresponding bit set.
#define ARBGNT_COP_STATUS_0_GNT31_GNG0_SHIFT                    _MK_SHIFT_CONST(0)
#define ARBGNT_COP_STATUS_0_GNT31_GNG0_FIELD                    (_MK_MASK_CONST(0xffffffff) << ARBGNT_COP_STATUS_0_GNT31_GNG0_SHIFT)
#define ARBGNT_COP_STATUS_0_GNT31_GNG0_RANGE                    31:0
#define ARBGNT_COP_STATUS_0_GNT31_GNG0_WOFFSET                  0x0
#define ARBGNT_COP_STATUS_0_GNT31_GNG0_DEFAULT                  _MK_MASK_CONST(0x0)
#define ARBGNT_COP_STATUS_0_GNT31_GNG0_DEFAULT_MASK                     _MK_MASK_CONST(0xffffffff)
#define ARBGNT_COP_STATUS_0_GNT31_GNG0_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define ARBGNT_COP_STATUS_0_GNT31_GNG0_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)

// COP Arbitration Semaphore Interrupt Enable Register

// Register ARBGNT_COP_ENABLE_0
#define ARBGNT_COP_ENABLE_0                     _MK_ADDR_CONST(0xc)
#define ARBGNT_COP_ENABLE_0_SECURE                      0x0
#define ARBGNT_COP_ENABLE_0_WORD_COUNT                  0x1
#define ARBGNT_COP_ENABLE_0_RESET_VAL                   _MK_MASK_CONST(0x0)
#define ARBGNT_COP_ENABLE_0_RESET_MASK                  _MK_MASK_CONST(0xffffffff)
#define ARBGNT_COP_ENABLE_0_SW_DEFAULT_VAL                      _MK_MASK_CONST(0x0)
#define ARBGNT_COP_ENABLE_0_SW_DEFAULT_MASK                     _MK_MASK_CONST(0x0)
#define ARBGNT_COP_ENABLE_0_READ_MASK                   _MK_MASK_CONST(0xffffffff)
#define ARBGNT_COP_ENABLE_0_WRITE_MASK                  _MK_MASK_CONST(0xffffffff)
// Writing a 1 in any bit position will enable the  corresponding arbitration semaphore interrupt.
#define ARBGNT_COP_ENABLE_0_GER31_GER0_SHIFT                    _MK_SHIFT_CONST(0)
#define ARBGNT_COP_ENABLE_0_GER31_GER0_FIELD                    (_MK_MASK_CONST(0xffffffff) << ARBGNT_COP_ENABLE_0_GER31_GER0_SHIFT)
#define ARBGNT_COP_ENABLE_0_GER31_GER0_RANGE                    31:0
#define ARBGNT_COP_ENABLE_0_GER31_GER0_WOFFSET                  0x0
#define ARBGNT_COP_ENABLE_0_GER31_GER0_DEFAULT                  _MK_MASK_CONST(0x0)
#define ARBGNT_COP_ENABLE_0_GER31_GER0_DEFAULT_MASK                     _MK_MASK_CONST(0xffffffff)
#define ARBGNT_COP_ENABLE_0_GER31_GER0_SW_DEFAULT                       _MK_MASK_CONST(0x0)
#define ARBGNT_COP_ENABLE_0_GER31_GER0_SW_DEFAULT_MASK                  _MK_MASK_CONST(0x0)


//
// REGISTER LIST
//
#define LIST_ARICTLR_ARBGNT_REGS(_op_) \
_op_(ARBGNT_CPU_STATUS_0) \
_op_(ARBGNT_CPU_ENABLE_0) \
_op_(ARBGNT_COP_STATUS_0) \
_op_(ARBGNT_COP_ENABLE_0)


//
// ADDRESS SPACES
//

#define BASE_ADDRESS_ARBGNT     0x00000000

//
// ARICTLR_ARBGNT REGISTER BANKS
//

#define ARBGNT0_FIRST_REG 0x0000 // ARBGNT_CPU_STATUS_0
#define ARBGNT0_LAST_REG 0x000c // ARBGNT_COP_ENABLE_0

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

#endif // ifndef ___ARICTLR_ARBGNT_H_INC_
