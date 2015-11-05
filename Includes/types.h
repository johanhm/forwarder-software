/***************************************************************************************************
*                                                                                                  *
*  FILENAME:                                                                                      */
/**\file        types.h
*
*  \brief       Definition of all global types and macros.
*
*/
/***************************************************************************************************
* (c) Copyright by BOSCH REXROTH AG, all rights reserved                                           *
****************************************************************************************************
*
* PROJECT:                   API RC30 platform
*
* FILE VERSION INFORMATION:  $Revision: 5258 $
*                            $Date: 2013-01-25 10:48:14 +0100 (Fr, 25 Jan 2013) $
*                            $Author: oub2si $
*
* REVISION HISTORY:
*
***************************************************************************************************/

#ifndef _TYPES_H_D
#define _TYPES_H_D

//*** include files ********************************************************************************
// List of include files needed in this module.

//*** defines **************************************************************************************
// List of defines needed in this header file and defines to be made available to other modules.

// ----- common definitons -----
#ifndef HUGE                        // required for compatibility of diagnosis parameter structures
  #define HUGE                      // to series 2x which are generated by BODAS-service
#endif

#ifndef DISABLE
  #define DISABLE     0x00u
#endif
#ifndef ENABLE
  #define ENABLE      0x01u
#endif
#ifndef NOTEXISTING
  #define NOTEXISTING     0
#endif
#ifndef NO_ERRORS
  #define NO_ERRORS   0x00u
#endif
#ifndef VARIABLE_INIT
  #define VARIABLE_INIT   0
#endif
#ifndef NIBBLE_SHIFT
  #define NIBBLE_SHIFT    4
#endif
#ifndef ZERO
  #define ZERO            0
#endif

#ifndef OFF
  #define OFF (0)
#endif

#ifndef ON
  #define ON (1)
#endif


// ----- bit conversion macros -----
/** macro to convert 8 single bit values into one 8-bit value */
#define   BIT8(a,b,c,d,e,f,g,h) \
          (((a)<<7)|((b) << 6)|((c) << 5)|((d) << 4 )|((e) << 3)|((f) << 2)|((g) <<1 )| (h))
/** macro to convert 16 single bit values into one 16-bit value */
#define   BIT16(i,j,k,l,m,n,o,p,a,b,c,d,e,f,g,h)     \
          (((WORD) BIT8((i),(j),(k),(l),(m),(n),(o),(p)) << 8) | \
           ((WORD) BIT8((a),(b),(c),(d),(e),(f),(g),(h))))

// ----- macros for functions of standard libraries -----
/** string compare function */
#define STRCMP_DF(a,b)      strcmp(a,b)
/** extended string compare function */
#define STRNCMP_DF(a,b,c)   strncmp(a,b,c)
/** string copy function */
#define STRCPY_DF(a,b)      strcpy(a,b)
/** extended string copy function */
#define STRNCPY_DF(a,b,c)   strncpy(a,b,c)
/** string length function */
#define STRLEN_DF(a)        strlen(a)

// ----- macro for alignment of structures to guarantee same structure size in B-design and C -----
#ifdef BODAS_RTS
#define INSERT_PADDING_D __attribute__ ((aligned (4)))
#else
#define INSERT_PADDING_D
#endif


//*** typdefs/structures ***************************************************************************
// Typedefs/structures to be made available to other modules.

// ----- standard data types according to BRH-CO/EME coding rules -----
typedef float                    flt;
typedef double                   dflt;

typedef unsigned char  bit1;
typedef unsigned char  bit2;
typedef unsigned char  bit3;
typedef unsigned char  bit4;
typedef unsigned char  bit5;
typedef unsigned char  bit6;
typedef unsigned char  bit7;

typedef unsigned short bit9;
typedef unsigned short bit10;
typedef unsigned short bit11;
typedef unsigned short bit12;
typedef unsigned short bit13;
typedef unsigned short bit14;
typedef unsigned short bit15;

typedef unsigned long  bit17;
typedef unsigned long  bit18;
typedef unsigned long  bit19;
typedef unsigned long  bit20;
typedef unsigned long  bit21;
typedef unsigned long  bit22;
typedef unsigned long  bit23;
typedef unsigned long  bit24;
typedef unsigned long  bit25;
typedef unsigned long  bit26;
typedef unsigned long  bit27;
typedef unsigned long  bit28;
typedef unsigned long  bit29;
typedef unsigned long  bit30;
typedef unsigned long  bit31;

/*
typedef unsigned char            boolean;
typedef unsigned char            uint8;
typedef volatile unsigned char   vuint8;
typedef signed   char            sint8;
typedef unsigned short           uint16;
typedef volatile unsigned short  vuint16;
typedef signed   short           sint16;
typedef unsigned long            uint32;
typedef volatile unsigned long   vuint32;
typedef signed   long            sint32;
*/

//*** public data **********************************************************************************
// Declarations of variables to be made available to other modules.


//*** public functions *****************************************************************************
// Declarations of functions to be made available to other modules.


//**************************************************************************************************

#undef _TYPES_D          // reseting definition, prevents double definition of a variable if
                         // includes are nested
#endif  // _TYPES_H_D