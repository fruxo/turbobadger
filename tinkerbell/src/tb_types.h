// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_TYPES_H
#define TB_TYPES_H

#include <string.h>

#ifndef nullptr
#define nullptr NULL
#endif

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#ifndef MAX
#define MAX(a,b)		(((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)		(((a) < (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)		(((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

#ifndef CLAMP
#define CLAMP(value, min, max) ((value > max) ? max : ((value < min) ? min : value))
#endif

#endif // TB_TYPES_H

