//==============================================================================
//
//  File        : mmp_err.h
//  Description : Top level system error definition.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================
/**
 @file mmp_err.h
 @brief The header file of MMP error codes
 
 This is a common file used in firmware and the host side, it describle the error codes that shared between
 firmware and host side
 
 @author Penguin Torng
 @version 1.0 Original Version
*/

#ifndef _MMP_ERR2_H_
#define _MMP_ERR2_H_

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================
#undef  MODULE_ERR_SHIFT
#define	MODULE_ERR_SHIFT		24

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMP_MODULE_2
{
	MMP_RAWPROC = 41,

} MMP_MODULE_2;

typedef enum _MMP_ERR_2
{
    MMP_RAWPROC_ERR_PARAMETER = (MMP_RAWPROC << MODULE_ERR_SHIFT) | 0x000001
} MMP_ERR_2;

#endif // _MMP_ERR_H_
