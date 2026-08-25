/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT file system module  R0.12c                             /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2017, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/----------------------------------------------------------------------------*/


#include "ff.h"			/* Declarations of FatFs API */

#if _USE_LFN != 0   /* Enable Long File Name */
/*-----------------------------------------------------------------------*/
/* ff_convert() & ff_wtoupper() is for _USE_LFN                          */
/* FIX: FAEDEVELOP-138                                                   */
/* Add by Jerry.Chen, 2025-06-12                                         */
/*-----------------------------------------------------------------------*/
WCHAR ff_convert (	/* Converted code, 0 means conversion error */
	WCHAR	chr,	/* Character code to be converted */
	UINT	dir		/* 0: Unicode to OEM code, 1: OEM code to Unicode */
)
{
	return chr;
}

WCHAR ff_wtoupper (	/* Returns upper converted character */
	WCHAR chr		/* Unicode character to be upper converted (BMP only) */
)
{
	return chr;
}
#endif
