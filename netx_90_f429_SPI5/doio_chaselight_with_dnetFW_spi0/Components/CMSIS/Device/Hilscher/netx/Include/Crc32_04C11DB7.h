/*!************************************************************************//*!
 * \file   Crc32_04C11DB7.h
 * \brief  Header file for CRC32 Poly 0x104C11DB7 implementation
 * $Revision: $
 * $Date: 2023-08-30 13:39:42 +0300 (Wed, 30 Aug 2023) $
 * \author     dmilkov
 * \copyright Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 * \note Exclusion of Liability for this demo software:
 * The following software is intended for and must only be used for reference and in an
 * evaluation laboratory environment. It is provided without charge and is subject to
 * alterations. There is no warranty for the software, to the extent permitted by
 * applicable law. Except when otherwise stated in writing the copyright holders and/or
 * other parties provide the software "as is" without warranty of any kind, either
 * expressed or implied.
 * Please refer to the Agreement in README_DISCLAIMER.txt, provided together with this file!
 * By installing or otherwise using the software, you accept the terms of this Agreement.
 * If you do not agree to the terms of this Agreement, then do not install or use the
 * Software!
 ******************************************************************************/

#ifndef __CRC32_04C11DB7_H
#define __CRC32_04C11DB7_H

/**
 * \brief Implements CRC32 polynomial 0x104C11DB7
 * \note Polynomial: x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
 * \param pvData [in] points to data to incorporate
 * \param ulDataLen [in] is the length of the data
 * \return checksum value
 * */
uint32_t
CRC32_Calculate(const void* pvData, size_t ulDataLen);

/**
 * \brief Continue existing CRC32 0x104C11DB7 checksum with data
 * Implements CRC32 polynomial 0x104C11DB7
 *
 * \note Polynomial:  x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1, No XOR
 * \param ulCurrentCrc [in] is the current checksum (state)
 * \param pabData [in] points to data to incorporate
 * \param ulDataLen [in] is the length of the data
 * \return new checksum value
 * */
uint32_t CRC32_Update(uint32_t ulCurrentCrc, const uint8_t* pabData, size_t ulDataLen);

#endif
