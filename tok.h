/*!
\file tok.h
\brief Contains the various prototypes for the tokenserver

\date   Started 11/6/99
\author George
*/

#ifndef __mtCF_h__
#define __mtCF_h__


/****************************************************************************
* A set of defines that can be modified by the user
*****************************************************************************/
/*--------------------------------------------------------------------------
 Specifies the width of the elementary data type that holds itemid and 
 userid information.

 Possible values:
   32 : Use 32 bit signed integers
   64 : Use 64 bit signed integers
--------------------------------------------------------------------------*/
#define IDXTYPEWIDTH 32


/*--------------------------------------------------------------------------
 Specifies the data type that will hold information about the ratings.

 Possible values:
   32  : single precission floating point
   64  : double precission floating point
--------------------------------------------------------------------------*/
#define WGTTYPE 32


/*--------------------------------------------------------------------------
 Specifies if the __thread storage directive is available by the compiler
 to indicate thread local storage. This storage directive is available in
 most systems using gcc compiler but it may not be available in other
 systems.

 Possible values:
  0 : Not available and do not use thread local storage
  1 : It is available and the __thread modifier will be used
--------------------------------------------------------------------------*/
#define HAVE_THREADLOCALSTORAGE 1


/****************************************************************************
* In principle, nothing needs to be changed beyond this point, unless the
* int32_t and int64_t cannot be found in the normal places.
*****************************************************************************/

/* Uniform definitions for various compilers */
#if defined(_MSC_VER)
  #define COMPILER_MSC
#endif
#if defined(__ICC)
  #define COMPILER_ICC
#endif
#if defined(__GNUC__)
  #define COMPILER_GCC
#endif


#if defined(COMPILER_MSC)
  /*#include <ctrdefs.h>*/
  #define __thread __declspec( thread )

  typedef __int32                 int32_t;
  typedef __int64                 int64_t;
  typedef unsigned __int32        uint32_t;
  typedef unsigned __int64        uint64_t;
#else
  #include <stdint.h>
  #include <inttypes.h>
  #include <sys/types.h>
  #include <float.h>
#endif


#if IDXTYPEWIDTH == 32

  #define IDX_MAX INT32_MAX
#elif IDXTYPEWIDTH == 64

  #define IDX_MAX INT64_MAX
#else
  #error "Incorrect user-supplied value for IDXTYPEWIDTH"
#endif

#if WGTTYPE == 32
  typedef float wgt_t;

  #define SCWGT         "f"
  #define PRWGT         "f"
  #define WGT_MAX       FLT_MAX
  #define WGT_EPSILON   FLT_EPSILON
  #define wabs          fabsf
  #define wsqrt         sqrtf
  #define wround        roundf
  #define wrint         rintf

  #define WGTEQ(x,y) ((fabsf((x)-(y)) <= FLT_EPSILON))
#elif WGTTYPE == 64
  typedef double wgt_t;

  #define SCWGT         "lf"
  #define PRWGT         "lf"
  #define WGT_MAX       DBL_MAX
  #define WGT_EPSILON   DBL_EPSILON
  #define wabs          fabs
  #define wsqrt         sqrt
  #define wround        round
  #define wrint         rint

  #define WGTEQ(x,y) ((fabs((x)-(y)) <= DBL_EPSILON))
#else
  #error "Incorrect user-supplied value for WGTTYPE"
#endif

typedef void *mptr_t;


#if !defined __cdecl
#define __cdecl
#endif

#if !defined _declspec
#define _declspec(x)
#endif


/*------------------------------------------------------------------------
* Constant definitions 
*-------------------------------------------------------------------------*/
/* Different bits for the options argument */
/* Ignore ratings when computing the neighbors for top-N identification */
//#define MTCF_OPTIONS_UNRATED_TOPN_NBRS     2    

/* Compute the ratings of the recommending items using an explicit 
   rating prediction algorithm */
//#define MTCF_OPTIONS_USE_PREDICTION        4    

/* Compute predictiosn for 2*topN and from there select the highest
   rated items to return for recommendations */
//#define MTCF_OPTIONS_BEST_FROM_TOP2N       8    


/* CFPRG's Header line */
//#define CFPRG_NAME            "MyTrybeCF 1.0pre1, Copyright 2007, George Karypis"


#endif
