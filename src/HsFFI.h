// #warning "Need to use autoconf to generate HsFFI.h"

#ifndef __HSFFI_H__
#define __HSFFI_H__

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
#ifndef Args
typedef signed   char      int8_t;
typedef signed   short     int16_t;
typedef signed   int       int32_t;
typedef signed   long long int64_t;
#endif

#if 0
typedef unsigned char HsChar;
#else
typedef char HsChar;
#endif
typedef int          HsInt;        
typedef int8_t       HsInt8;         
typedef int16_t      HsInt16;        
typedef int32_t      HsInt32;        
typedef int64_t      HsInt64;        
typedef unsigned int HsWord;       
typedef uint8_t      HsWord8;        
typedef uint16_t     HsWord16;       
typedef uint32_t     HsWord32;       
typedef uint64_t     HsWord64;       
typedef float        HsFloat;      
typedef double       HsDouble;     
typedef int          HsBool;         
typedef void*        HsAddr;       
typedef void*        HsPtr;          
typedef void         (*HsFunPtr)(void);
typedef void*        HsForeignPtr;   
typedef void*        HsStablePtr;  

typedef int          HugsStackPtr;
typedef void*        HugsForeign;   
typedef int          HugsStablePtr;  

typedef void (*HugsPrim) (HugsStackPtr); /* primitive function	   */

#ifndef Args  // hack hack
struct hugs_primitive {		         /* table of primitives		   */
    char*  ref;				 /* primitive reference string	   */
    int	   arity;			 /* primitive function arity	   */
    HugsPrim imp;		         /* primitive implementation	   */
};

struct hugs_primInfo {
    void                  (*controlFun)(int);
    struct hugs_primitive *primFuns;
    struct hugs_primInfo  *nextPrimInfo;
};
#else
#define hugs_primInfo primInfo
#endif

#define HS_CHAR_MIN             0
#define HS_CHAR_MAX             0xFF
#define HS_BOOL_FALSE           0
#define HS_BOOL_TRUE            1
#define HS_BOOL_MIN             HS_BOOL_FALSE
#define HS_BOOL_MAX             HS_BOOL_TRUE
#define HS_INT_MIN              __INT32_MIN
#define HS_INT_MAX              __INT32_MAX
#define HS_INT8_MIN             __INT8_MIN
#define HS_INT8_MAX             __INT8_MAX
#define HS_INT16_MIN            __INT16_MIN
#define HS_INT16_MAX            __INT16_MAX
#define HS_INT32_MIN            __INT32_MIN
#define HS_INT32_MAX            __INT32_MAX
#define HS_INT64_MIN            __INT64_MIN
#define HS_INT64_MAX            __INT64_MAX
#define HS_WORD8_MAX            __UINT8_MAX
#define HS_WORD16_MAX           __UINT16_MAX
#define HS_WORD32_MAX           __UINT32_MAX
#define HS_WORD64_MAX           __UINT64_MAX

#define HS_FLOAT_RADIX          FLT_RADIX
#define HS_FLOAT_ROUNDS         FLT_ROUNDS
#define HS_FLOAT_EPSILON        FLT_EPSILON
#define HS_FLOAT_DIG            FLT_DIG
#define HS_FLOAT_MANT_DIG       FLT_MANT_DIG
#define HS_FLOAT_MIN            FLT_MIN
#define HS_FLOAT_MIN_EXP        FLT_MIN_EXP
#define HS_FLOAT_MIN_10_EXP     FLT_MIN_10_EXP
#define HS_FLOAT_MAX            FLT_MAX
#define HS_FLOAT_MAX_EXP        FLT_MAX_EXP
#define HS_FLOAT_MAX_10_EXP     FLT_MAX_10_EXP

#define HS_DOUBLE_RADIX         DBL_RADIX
#define HS_DOUBLE_ROUNDS        DBL_ROUNDS
#define HS_DOUBLE_EPSILON       DBL_EPSILON
#define HS_DOUBLE_DIG           DBL_DIG
#define HS_DOUBLE_MANT_DIG      DBL_MANT_DIG
#define HS_DOUBLE_MIN           DBL_MIN
#define HS_DOUBLE_MIN_EXP       DBL_MIN_EXP
#define HS_DOUBLE_MIN_10_EXP    DBL_MIN_10_EXP
#define HS_DOUBLE_MAX           DBL_MAX
#define HS_DOUBLE_MAX_EXP       DBL_MAX_EXP
#define HS_DOUBLE_MAX_10_EXP    DBL_MAX_10_EXP

typedef struct {

  /* evaluate next argument */
  HsInt          (*getInt)         (void);
  HsWord         (*getWord)        (void);
  HsAddr    	 (*getAddr)        (void);
  HsFloat        (*getFloat)       (void);
  HsDouble       (*getDouble)      (void);
  HsChar         (*getChar)        (void);
  HugsForeign    (*getForeign)     (void);
  HugsStablePtr  (*getStablePtr)   (void);

  /* push part of result   */
  void           (*putInt)         (HsInt);
  void      	 (*putWord)        (HsWord);
  void      	 (*putAddr)        (HsAddr);
  void           (*putFloat)       (HsFloat);
  void           (*putDouble)      (HsDouble);
  void           (*putChar)        (HsChar);
  void      	 (*putForeign)     (HugsForeign, void (*)(HugsForeign));
  void           (*putStablePtr)   (HugsStablePtr);

  /* return n values in IO monad or Id monad */
  void      	 (*returnIO)       (HugsStackPtr, int);
  void      	 (*returnId)       (HugsStackPtr, int);
  int      	 (*runIO)          (int);

  /* free a stable pointer */	    			 
  void      	 (*freeStablePtr)  (HugsStablePtr);

  /* register the prim table */	    			 
  void      	 (*registerPrims)  (struct hugs_primInfo*);
			   
  /* garbage collect */
  void		 (*garbageCollect) (void);

  /* API3 additions follow */
  HugsStablePtr  (*lookupName)     (char*, char*);
  void           (*ap)             (int);
  void           (*getUnit)        (void);
  void*          (*mkThunk)        (HsFunPtr, HugsStablePtr);
  void           (*freeThunk)      (void*);
  HsBool         (*getBool)        (void);
  void           (*putBool)        (HsBool);

  /* API4 additions follow */
  HsInt8         (*getInt8)        (void);
  HsInt16        (*getInt16)       (void);
  HsInt32        (*getInt32)       (void);
  HsInt64        (*getInt64)       (void);
  HsWord8        (*getWord8)       (void);
  HsWord16       (*getWord16)      (void);
  HsWord32       (*getWord32)      (void);
  HsWord64       (*getWord64)      (void);
  HsPtr          (*getPtr)         (void);
  HsFunPtr       (*getFunPtr)      (void);
  HsForeignPtr   (*getForeignPtr)  (void);

  void           (*putInt8)        (HsInt8);
  void           (*putInt16)       (HsInt16);
  void           (*putInt32)       (HsInt32);
  void           (*putInt64)       (HsInt64);
  void           (*putWord8)       (HsWord8);
  void           (*putWord16)      (HsWord16);
  void           (*putWord32)      (HsWord32);
  void           (*putWord64)      (HsWord64);
  void           (*putPtr)         (HsPtr);
  void           (*putFunPtr)      (HsFunPtr);
  void           (*putForeignPtr)  (HsForeignPtr);

  void           (*putStablePtr4)  (HsStablePtr);
  HsStablePtr    (*getStablePtr4)  (void);
  void      	 (*freeStablePtr4) (HsStablePtr);

} HugsAPI4;

extern  HugsAPI4* hugsAPI4     (void);
typedef void (*InitModuleFun4) (HugsAPI4*);



/* Copied verbatim from prelude.h */

#ifdef _MSC_VER /* Microsoft Visual C++ */
#define DLLIMPORT(rty) __declspec(dllimport) rty
#define DLLEXPORT(rty) __declspec(dllexport) rty
#elif defined __BORLANDC__ 
#define DLLIMPORT(rty) rty far _import
#define DLLEXPORT(rty) rty far _export
#else 
#define DLLIMPORT(rty) rty
#define DLLEXPORT(rty) rty
#endif /* Don't need to declare DLL exports */

#endif /* __HSFFI_H__ */
