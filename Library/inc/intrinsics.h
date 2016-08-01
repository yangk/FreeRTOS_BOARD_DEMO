/**************************************************
 *
 * This file declares the ARM intrinsic inline functions.
 *
 * Copyright 1999-2006 IAR Systems. All rights reserved.
 *
 * $Revision: 48618 $
 *
 **************************************************/

#ifndef __INTRINSICS_INCLUDED
#define __INTRINSICS_INCLUDED

#include <ycheck.h>
#include <stdint.h>
#ifndef _SYSTEM_BUILD
  #pragma system_include
#endif

/*
 * Check that the correct C compiler is used.
 */

#if ((__TID__ >> 8) & 0x007F) != 79
#error "File intrinsics.h can only be used together with iccarm."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma language=save
#pragma language=extended

__ATTRIBUTES void    __no_operation(void);

__ATTRIBUTES void    __disable_interrupt(void);
__ATTRIBUTES void    __enable_interrupt(void);

typedef uint32_t __istate_t;

__ATTRIBUTES __istate_t __get_interrupt_state(void);
__ATTRIBUTES void __set_interrupt_state(__istate_t);

#ifdef __ARM_PROFILE_M__

/* System control access for Cortex-M cores */
__ATTRIBUTES uint32_t __get_PSR( void );
__ATTRIBUTES uint32_t __get_IPSR( void );
__ATTRIBUTES uint32_t __get_MSP( void );
__ATTRIBUTES void          __set_MSP( uint32_t );
__ATTRIBUTES uint32_t __get_PSP( void );
__ATTRIBUTES void          __set_PSP( uint32_t );
__ATTRIBUTES uint32_t __get_PRIMASK( void );
__ATTRIBUTES void          __set_PRIMASK( uint32_t );
__ATTRIBUTES uint32_t __get_CONTROL( void );
__ATTRIBUTES void          __set_CONTROL( uint32_t );

#ifdef __ARM7__

/* These are only available for v7M */
__ATTRIBUTES uint32_t __get_FAULTMASK( void );
__ATTRIBUTES void          __set_FAULTMASK(uint32_t);
__ATTRIBUTES uint32_t __get_BASEPRI( void );
__ATTRIBUTES void          __set_BASEPRI( uint32_t );

#endif /* __ARM7__ */
#else /* __ARM_PROFILE_M__ */

/* "Old" style intrerrupt control routines */
__ATTRIBUTES void __disable_irq(void);
__ATTRIBUTES void __enable_irq(void);
#endif /* __ARM_PROFILE_M__ */

__ATTRIBUTES void __disable_fiq(void);
__ATTRIBUTES void __enable_fiq(void);


/* ARM-mode intrinsics */

__ATTRIBUTES uint32_t __SWP( uint32_t, uint32_t * );
__ATTRIBUTES uint8_t __SWPB( uint8_t, uint8_t * );

typedef uint32_t __ul;

#define __cpid  unsigned __constrange(0,15)
#define __cpreg unsigned __constrange(0,15)
#define __cpopc unsigned __constrange(0,8)

/*  Co-processor access */
__ATTRIBUTES void          __MCR( __cpid coproc, __cpopc opcode_1, __ul src,
                                 __cpreg CRn, __cpreg CRm, __cpopc opcode_2 );
__ATTRIBUTES uint32_t __MRC( __cpid coproc, __cpopc opcode_1,
                                 __cpreg CRn, __cpreg CRm, __cpopc opcode_2 );
__ATTRIBUTES void          __MCR2( __cpid coproc, __cpopc opcode_1, __ul src,
                                  __cpreg CRn, __cpreg CRm, __cpopc opcode_2 );
__ATTRIBUTES uint32_t __MRC2( __cpid coproc, __cpopc opcode_1,
                                  __cpreg CRn, __cpreg CRm, __cpopc opcode_2 );

/* Load coprocessor register. */
__ATTRIBUTES void __LDC( __cpid coproc, __cpreg CRn, __ul const *src);
__ATTRIBUTES void __LDCL( __cpid coproc, __cpreg CRn, __ul const *src);
__ATTRIBUTES void __LDC2( __cpid coproc, __cpreg CRn, __ul const *src);
__ATTRIBUTES void __LDC2L( __cpid coproc, __cpreg CRn, __ul const *src);

/* Store coprocessor register. */
__ATTRIBUTES void __STC( __cpid coproc, __cpreg CRn, __ul *dst);
__ATTRIBUTES void __STCL( __cpid coproc, __cpreg CRn, __ul *dst);
__ATTRIBUTES void __STC2( __cpid coproc, __cpreg CRn, __ul *dst);
__ATTRIBUTES void __STC2L( __cpid coproc, __cpreg CRn, __ul *dst);

/* Load coprocessor register (noindexed version with coprocessor option). */
__ATTRIBUTES void __LDC_noidx( __cpid coproc, __cpreg CRn, __ul const *src,
                              unsigned __constrange(0,255) option);

__ATTRIBUTES void __LDCL_noidx( __cpid coproc, __cpreg CRn, __ul const *src,
                               unsigned __constrange(0,255) option);

__ATTRIBUTES void __LDC2_noidx( __cpid coproc, __cpreg CRn, __ul const *src,
                               unsigned __constrange(0,255) option);

__ATTRIBUTES void __LDC2L_noidx( __cpid coproc, __cpreg CRn, __ul const *src,
                                unsigned __constrange(0,255) option);

/* Store coprocessor register (version with coprocessor option). */
__ATTRIBUTES void __STC_noidx( __cpid coproc, __cpreg CRn, __ul *dst,
                              unsigned __constrange(0,255) option);

__ATTRIBUTES void __STCL_noidx( __cpid coproc, __cpreg CRn, __ul *dst,
                               unsigned __constrange(0,255) option);

__ATTRIBUTES void __STC2_noidx( __cpid coproc, __cpreg CRn, __ul *dst,
                               unsigned __constrange(0,255) option);

__ATTRIBUTES void __STC2L_noidx( __cpid coproc, __cpreg CRn, __ul *dst,
                                unsigned __constrange(0,255) option);

#ifdef __ARM_PROFILE_M__
/* Status register access, v7M: */
__ATTRIBUTES uint32_t __get_APSR( void );
__ATTRIBUTES void          __set_APSR( uint32_t );
#else /* __ARM_PROFILE_M__ */
/* Status register access */
__ATTRIBUTES uint32_t __get_CPSR( void );
__ATTRIBUTES void          __set_CPSR( uint32_t );
#endif /* __ARM_PROFILE_M__ */

/* Floating-point status and control register access */
__ATTRIBUTES uint32_t __get_FPSCR( void );
__ATTRIBUTES void __set_FPSCR( uint32_t );

/* Architecture v5T, CLZ is also available in Thumb mode for Thumb2 cores */
__ATTRIBUTES uint8_t __CLZ( uint32_t );

/* Architecture v5TE */
#if !defined(__ARM_PROFILE_M__) || defined(__ARM_MEDIA__)
__ATTRIBUTES int32_t __QADD( int32_t, int32_t );
__ATTRIBUTES int32_t __QDADD( int32_t, int32_t );
__ATTRIBUTES int32_t __QSUB( int32_t, int32_t );
__ATTRIBUTES int32_t __QDSUB( int32_t, int32_t );

__ATTRIBUTES int32_t __QDOUBLE( int32_t );

__ATTRIBUTES int         __QFlag( void );
__ATTRIBUTES void __reset_Q_flag( void );
#endif

__ATTRIBUTES int         __QCFlag( void );
__ATTRIBUTES void __reset_QC_flag( void );

__ATTRIBUTES int32_t __SMUL( signed short, signed short );

/* Architecture v6, REV and REVSH are also available in thumb mode */
__ATTRIBUTES uint32_t __REV( uint32_t );
__ATTRIBUTES int32_t __REVSH( short );

__ATTRIBUTES uint32_t __REV16( uint16_t );
__ATTRIBUTES uint32_t __RBIT( uint32_t );

__ATTRIBUTES uint8_t  __LDREXB( uint8_t * );
__ATTRIBUTES uint16_t __LDREXH( uint16_t * );
__ATTRIBUTES uint32_t  __LDREX ( uint32_t * );
__ATTRIBUTES uint64_t __LDREXD( uint64_t * );

__ATTRIBUTES uint32_t  __STREXB( uint8_t, uint8_t * );
__ATTRIBUTES uint32_t  __STREXH( uint16_t, uint16_t * );
__ATTRIBUTES uint32_t  __STREX ( uint32_t, uint32_t * );
__ATTRIBUTES uint32_t  __STREXD( uint64_t, uint64_t * );

__ATTRIBUTES void __CLREX( void );

__ATTRIBUTES void __SEV( void );
__ATTRIBUTES void __WFE( void );
__ATTRIBUTES void __WFI( void );
__ATTRIBUTES void __YIELD( void );

__ATTRIBUTES void __PLI( void const * );
__ATTRIBUTES void __PLD( void const * );
__ATTRIBUTES void __PLDW( void const * );

__ATTRIBUTES uint32_t __SSAT     (uint32_t val,
                                      unsigned int __constrange( 1, 32 ) sat );
__ATTRIBUTES uint32_t __USAT     (uint32_t val,
                                      unsigned int __constrange( 0, 31 ) sat );


#ifdef __ARM_MEDIA__

/* Architecture v6 Media instructions.... */
__ATTRIBUTES uint32_t __SEL( uint32_t op1, uint32_t op2 );

__ATTRIBUTES uint32_t __SADD8    (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __SADD16   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __SSUB8    (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __SSUB16   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __SADDSUBX (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __SSUBADDX (uint32_t pair1, uint32_t pair2);

__ATTRIBUTES uint32_t __SHADD8   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __SHADD16  (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __SHSUB8   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __SHSUB16  (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __SHADDSUBX(uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __SHSUBADDX(uint32_t pair1, uint32_t pair2);

__ATTRIBUTES uint32_t __QADD8    (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __QADD16   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __QSUB8    (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __QSUB16   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __QADDSUBX (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __QSUBADDX (uint32_t pair1, uint32_t pair2);

__ATTRIBUTES uint32_t __UADD8    (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UADD16   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __USUB8    (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __USUB16   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UADDSUBX (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __USUBADDX (uint32_t pair1, uint32_t pair2);

__ATTRIBUTES uint32_t __UHADD8   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UHADD16  (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UHSUB8   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UHSUB16  (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UHADDSUBX(uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UHSUBADDX(uint32_t pair1, uint32_t pair2);

__ATTRIBUTES uint32_t __UQADD8   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UQADD16  (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UQSUB8   (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UQSUB16  (uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UQADDSUBX(uint32_t pair1, uint32_t pair2);
__ATTRIBUTES uint32_t __UQSUBADDX(uint32_t pair1, uint32_t pair2);

__ATTRIBUTES uint32_t __USAD8(uint32_t x, uint32_t y );
__ATTRIBUTES uint32_t __USADA8(uint32_t x, uint32_t y,
                                   uint32_t acc );

__ATTRIBUTES uint32_t __SSAT16   (uint32_t pair,
                                      unsigned int __constrange( 1, 16 ) sat );
__ATTRIBUTES uint32_t __USAT16   (uint32_t pair,
                                      unsigned int __constrange( 0, 15 ) sat );

__ATTRIBUTES uint32_t __SMUAD(uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __SMUSD(uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __SMUADX(uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __SMUSDX(uint32_t x, uint32_t y);

__ATTRIBUTES uint32_t __SMLAD(uint32_t x, uint32_t y, uint32_t sum);
__ATTRIBUTES uint32_t __SMLSD(uint32_t x, uint32_t y, uint32_t sum);
__ATTRIBUTES uint32_t __SMLADX(uint32_t x, uint32_t y, uint32_t sum);
__ATTRIBUTES uint32_t __SMLSDX(uint32_t x, uint32_t y, uint32_t sum);

__ATTRIBUTES uint64_t __SMLALD ( uint32_t pair1,
                                          uint32_t pair2,
                                          uint64_t acc );

__ATTRIBUTES uint64_t __SMLALDX( uint32_t pair1,
                                          uint32_t pair2,
                                          uint64_t acc );

__ATTRIBUTES uint64_t __SMLSLD ( uint32_t pair1,
                                          uint32_t pair2,
                                          uint64_t acc );

__ATTRIBUTES uint64_t __SMLSLDX( uint32_t pair1,
                                          uint32_t pair2,
                                          uint64_t acc );

__ATTRIBUTES uint32_t __PKHBT(uint32_t x,
                                  uint32_t y,
                                  unsigned __constrange(0,31) count);
__ATTRIBUTES uint32_t __PKHTB(uint32_t x,
                                  uint32_t y,
                                  unsigned __constrange(0,32) count);

__ATTRIBUTES uint32_t __SMLABB(uint32_t x,
                                   uint32_t y,
                                   uint32_t acc);
__ATTRIBUTES uint32_t __SMLABT(uint32_t x,
                                   uint32_t y,
                                   uint32_t acc);
__ATTRIBUTES uint32_t __SMLATB(uint32_t x,
                                   uint32_t y,
                                   uint32_t acc);
__ATTRIBUTES uint32_t __SMLATT(uint32_t x,
                                   uint32_t y,
                                   uint32_t acc);
__ATTRIBUTES uint32_t __SMLAWB(uint32_t x,
                                   uint32_t y,
                                   uint32_t acc);
__ATTRIBUTES uint32_t __SMLAWT(uint32_t x,
                                   uint32_t y,
                                   uint32_t acc);

__ATTRIBUTES uint32_t __SMMLA (uint32_t x,
                                   uint32_t y,
                                   uint32_t acc);
__ATTRIBUTES uint32_t __SMMLAR(uint32_t x,
                                   uint32_t y,
                                   uint32_t acc);
__ATTRIBUTES uint32_t __SMMLS (uint32_t x,
                                   uint32_t y,
                                   uint32_t acc);
__ATTRIBUTES uint32_t __SMMLSR(uint32_t x,
                                   uint32_t y,
                                   uint32_t acc);
__ATTRIBUTES uint32_t __SMMUL (uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __SMMULR(uint32_t x, uint32_t y);

__ATTRIBUTES uint32_t __SMULBB(uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __SMULBT(uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __SMULTB(uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __SMULTT(uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __SMULWB(uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __SMULWT(uint32_t x, uint32_t y);

__ATTRIBUTES uint32_t __SXTAB (uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __SXTAH (uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __UXTAB (uint32_t x, uint32_t y);
__ATTRIBUTES uint32_t __UXTAH (uint32_t x, uint32_t y);

__ATTRIBUTES uint64_t __UMAAL(uint32_t x,
                                       uint32_t y,
                                       uint32_t a,
                                       uint32_t b);

__ATTRIBUTES uint64_t __SMLALBB(uint32_t x,
                                         uint32_t y,
                                         uint64_t acc);
__ATTRIBUTES uint64_t __SMLALBT(uint32_t x,
                                         uint32_t y,
                                         uint64_t acc);
__ATTRIBUTES uint64_t __SMLALTB(uint32_t x,
                                         uint32_t y,
                                         uint64_t acc);
__ATTRIBUTES uint64_t __SMLALTT(uint32_t x,
                                         uint32_t y,
                                         uint64_t acc);

__ATTRIBUTES uint32_t __UXTB16(uint32_t x);
__ATTRIBUTES uint32_t __UXTAB16(uint32_t acc, uint32_t x);

__ATTRIBUTES uint32_t __SXTB16(uint32_t x);
__ATTRIBUTES uint32_t __SXTAB16(uint32_t acc, uint32_t x);

/*
 * The following intrinsic function aliases are supplied due to instruction
 * set changes made by ARM. All previous mnemonics of the form *ADDSUBX or
 * *SUBADDX have been changed into *ASX and *SAX, respectively.
 * For increased readability and consistency, the suggested intrinsic
 * functions to use are the new short form versions defined below.
 */

#define __SASX  __SADDSUBX
#define __SSAX  __SSUBADDX
#define __SHASX __SHADDSUBX
#define __SHSAX __SHSUBADDX
#define __QASX  __QADDSUBX
#define __QSAX  __QSUBADDX

#define __UASX  __UADDSUBX
#define __USAX  __USUBADDX
#define __UHASX __UHADDSUBX
#define __UHSAX __UHSUBADDX
#define __UQASX __UQADDSUBX
#define __UQSAX __UQSUBADDX
#endif /* __ARM_MEDIA__ */

/* Architecture v7 instructions.... */
__ATTRIBUTES void __DMB(void);
__ATTRIBUTES void __DSB(void);
__ATTRIBUTES void __ISB(void);

#define __fabs(x)  fabs(x)
#define __fabsf(x) fabsf(x)

#pragma language=restore

#ifdef __cplusplus
}
#endif

#endif  /* __INTRINSICS_INCLUDED */
