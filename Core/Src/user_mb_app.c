/*
 * FreeModbus Libary: user callback functions and buffer define in slave mode
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: user_mb_app.c,v 1.60 2013/11/23 11:49:05 Armink $
 */
#include "user_mb_app.h"

/*------------------------Slave mode use these variables----------------------*/
//Slave mode:DiscreteInputs variables
USHORT   usSDiscInStart                               = S_DISCRETE_INPUT_START;
#if S_DISCRETE_INPUT_NDISCRETES%8
UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8+1];
#else
UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8]  ;
#endif
//Slave mode:Coils variables
USHORT   usSCoilStart                                 = S_COIL_START;
#if S_COIL_NCOILS%8
UCHAR    ucSCoilBuf[S_COIL_NCOILS/8+1]                ;
#else
UCHAR    ucSCoilBuf[S_COIL_NCOILS/8]                  ;
#endif
//Slave mode:InputRegister variables
USHORT   usSRegInStart                                = S_REG_INPUT_START;
USHORT   usSRegInBuf[S_REG_INPUT_NREGS]               ;
//Slave mode:HoldingRegister variables
USHORT   usSRegHoldStart                              = S_REG_HOLDING_START;
USHORT   usSRegHoldBuf[S_REG_HOLDING_NREGS]           ;

/**
 * Modbus slave input register callback function.
 *
 * @param pucRegBuffer input register buffer
 * @param usAddress input register address
 * @param usNRegs input register number
 *
 * @return result
 */
eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegInputBuf;
    USHORT          usRegInStart;

    pusRegInputBuf = usSRegInBuf;
    usRegInStart = usSRegInStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= S_REG_INPUT_START)
            && (usAddress + usNRegs <= S_REG_INPUT_START + S_REG_INPUT_NREGS))
    {
        iRegIndex = usAddress - usRegInStart;
        while (usNRegs > 0)
        {
            *pucRegBuffer++ = (UCHAR) (pusRegInputBuf[iRegIndex] >> 8);
            *pucRegBuffer++ = (UCHAR) (pusRegInputBuf[iRegIndex] & 0xFF);
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

/**
 * Modbus slave holding register callback function.
 *
 * @param pucRegBuffer holding register buffer
 * @param usAddress holding register address
 * @param usNRegs holding register number
 * @param eMode read or write
 *
 * @return result
 */
eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress,
        USHORT usNRegs, eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegHoldingBuf;
    USHORT          usRegHoldStart;

    pusRegHoldingBuf = usSRegHoldBuf;
    usRegHoldStart = usSRegHoldStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= S_REG_HOLDING_START)
            && (usAddress + usNRegs <= S_REG_HOLDING_START + S_REG_HOLDING_NREGS))
    {
        iRegIndex = usAddress - usRegHoldStart;
        switch (eMode)
        {
        /* read current register values from the protocol stack. */
        case MB_REG_READ:
            while (usNRegs > 0)
            {
                *pucRegBuffer++ = (UCHAR) (pusRegHoldingBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (UCHAR) (pusRegHoldingBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break;

        /* write current register values with new values from the protocol stack. */
        case MB_REG_WRITE:
            while (usNRegs > 0)
            {
                pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
            break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/**
 * Modbus slave coils callback function.
 *
 * @param pucRegBuffer coils buffer
 * @param usAddress coils address
 * @param usNCoils coils number
 * @param eMode read or write
 *
 * @return result
 */
eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress,
        USHORT usNCoils, eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex , iRegBitIndex , iNReg;
    UCHAR *         pucCoilBuf;
    USHORT          usCoilStart;
    iNReg =  usNCoils / 8 + 1;

    pucCoilBuf = ucSCoilBuf;
    usCoilStart = usSCoilStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if( ( usAddress >= S_COIL_START ) &&
        ( usAddress + usNCoils <= S_COIL_START + S_COIL_NCOILS ) )
    {
        iRegIndex = (USHORT) (usAddress - usCoilStart) / 8;
        iRegBitIndex = (USHORT) (usAddress - usCoilStart) % 8;
        switch ( eMode )
        {
        /* read current coil values from the protocol stack. */
        case MB_REG_READ:
            while (iNReg > 0)
            {
                *pucRegBuffer++ = xMBUtilGetBits(&pucCoilBuf[iRegIndex++],
                        iRegBitIndex, 8);
                iNReg--;
            }
            pucRegBuffer--;
            /* last coils */
            usNCoils = usNCoils % 8;
            /* filling zero to high bit */
            *pucRegBuffer = *pucRegBuffer << (8 - usNCoils);
            *pucRegBuffer = *pucRegBuffer >> (8 - usNCoils);
            break;

            /* write current coil values with new values from the protocol stack. */
        case MB_REG_WRITE:
            while (iNReg > 1)
            {
                xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, 8,
                        *pucRegBuffer++);
                iNReg--;
            }
            /* last coils */
            usNCoils = usNCoils % 8;
            /* xMBUtilSetBits has bug when ucNBits is zero */
            if (usNCoils != 0)
            {
                xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, usNCoils,
                        *pucRegBuffer++);
            }
            break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/**
 * Modbus slave discrete callback function.
 *
 * @param pucRegBuffer discrete buffer
 * @param usAddress discrete address
 * @param usNDiscrete discrete number
 *
 * @return result
 */
eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex , iRegBitIndex , iNReg;
    UCHAR *         pucDiscreteInputBuf;
    USHORT          usDiscreteInputStart;
    iNReg =  usNDiscrete / 8 + 1;

    pucDiscreteInputBuf = ucSDiscInBuf;
    usDiscreteInputStart = usSDiscInStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= S_DISCRETE_INPUT_START)
            && (usAddress + usNDiscrete    <= S_DISCRETE_INPUT_START + S_DISCRETE_INPUT_NDISCRETES))
    {
        iRegIndex = (USHORT) (usAddress - usDiscreteInputStart) / 8;
        iRegBitIndex = (USHORT) (usAddress - usDiscreteInputStart) % 8;

        while (iNReg > 0)
        {
            *pucRegBuffer++ = xMBUtilGetBits(&pucDiscreteInputBuf[iRegIndex++],
                    iRegBitIndex, 8);
            iNReg--;
        }
        pucRegBuffer--;
        /* last discrete */
        usNDiscrete = usNDiscrete % 8;
        /* filling zero to high bit */
        *pucRegBuffer = *pucRegBuffer << (8 - usNDiscrete);
        *pucRegBuffer = *pucRegBuffer >> (8 - usNDiscrete);
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}



eMBErrorCode eMBSlaveSetCoil(USHORT bitOffset, UCHAR value)
{
  UCHAR ByteAddr, BitAddr;
  UCHAR wrtVal;
  USHORT DeltaBitOffset;
   
  /* ---------- Check address Correction -------------*/
#if S_COIL_START - 1 > 0
  if (bitOffset < S_COIL_START)
    return MB_ENOREG;
#endif
  if (bitOffset > (S_COIL_START + S_COIL_NCOILS - 1))
    return MB_ENOREG;
   
  DeltaBitOffset = bitOffset - S_COIL_START;
  
  ByteAddr = DeltaBitOffset / 8;
  BitAddr = DeltaBitOffset % 8;

  wrtVal = (UCHAR)1U << BitAddr;
  
  if (value == 0U)
  {
    ucSCoilBuf[ByteAddr] &= (UCHAR)(~wrtVal);
  }
  else
  {
    ucSCoilBuf[ByteAddr] |= wrtVal;
  }
   
  return MB_ENOERR;
}

eMBErrorCode eMBSlaveGetCoil(USHORT bitOffset, UCHAR* value)
{
  UCHAR ByteAddr, BitAddr;
  UCHAR readVal;
  USHORT DeltaBitOffset;
   
  /* ---------- Check address Correction -------------*/
#if S_COIL_START - 1 > 0
  if (bitOffset < S_COIL_START)
    return MB_ENOREG;
#endif
  if (bitOffset > (S_COIL_START + S_COIL_NCOILS - 1))
    return MB_ENOREG;
  
  DeltaBitOffset = bitOffset - S_COIL_START;
  
  ByteAddr = DeltaBitOffset / 8;
  BitAddr = DeltaBitOffset % 8;
  
  readVal = ucSCoilBuf[ByteAddr] >> BitAddr;
  readVal &= (UCHAR)1U;
   
  *value = readVal;
   
  return MB_ENOERR;
}

eMBErrorCode eMBSlaveSetDiscreteInput(USHORT bitOffset, UCHAR value)
{
  UCHAR ByteAddr, BitAddr;
  UCHAR wrtVal;
  USHORT DeltaBitOffset;

  /* ---------- Check address Correction -------------*/
#if S_DISCRETE_INPUT_START - 1 > 0
  if (bitOffset < S_DISCRETE_INPUT_START)
    return MB_ENOREG;
#endif
  if (bitOffset > (S_DISCRETE_INPUT_START + S_DISCRETE_INPUT_NDISCRETES -1))
    return MB_ENOREG;
  
  DeltaBitOffset = bitOffset - S_DISCRETE_INPUT_START;
  
  ByteAddr = DeltaBitOffset / 8;
  BitAddr = DeltaBitOffset % 8;
  
  wrtVal = (UCHAR)1U << BitAddr;
  
  if (value == 1)
  {
    ucSDiscInBuf[ByteAddr] |= wrtVal;
  }
  else
  {
    ucSDiscInBuf[ByteAddr] &= (UCHAR)(~wrtVal);
  }
  
  return MB_ENOERR;
}

eMBErrorCode eMBSlaveGetDiscreteInput(USHORT bitOffset, UCHAR* value)
{
  UCHAR ByteAddr, BitAddr;
  UCHAR readVal;
  USHORT DeltaBitOffset;
   
  /* ---------- Check address Correction -------------*/
#if S_DISCRETE_INPUT_START - 1 > 0
  if (bitOffset < S_DISCRETE_INPUT_START)
    return MB_ENOREG;
#endif
  if (bitOffset > (S_DISCRETE_INPUT_START + S_DISCRETE_INPUT_NDISCRETES - 1))
    return MB_ENOREG;
  
  DeltaBitOffset = bitOffset - S_DISCRETE_INPUT_START;
  
  ByteAddr = DeltaBitOffset / 8;
  BitAddr = DeltaBitOffset % 8;
  
  readVal = ucSDiscInBuf[ByteAddr] >> BitAddr;
  readVal &= (UCHAR)1U;
   
  *value = readVal;
   
  return MB_ENOERR;
}

eMBErrorCode eMBSlaveGetDataU16(eMBSlaveRegType regType, USHORT address, USHORT* value)
{
  USHORT BeginAddress;
  USHORT LastAddress;
   
  switch (regType)
  {
    case MB_SLAVE_REG_TYPE_INPUT:
    {
      /* ---------Check address Correction ------------ */
      BeginAddress = S_REG_INPUT_START;
      if (address < (BeginAddress - 1))
        return MB_ENOREG;
      
      LastAddress = S_REG_INPUT_START + S_REG_INPUT_NREGS - 1;
      if (address > LastAddress)
        return MB_ENOREG;
      
      /* --------Assign Data ------------*/
      *value = usSRegInBuf[address - BeginAddress];
      
      break;
    }
    case MB_SLAVE_REG_TYPE_HOLDING:
    {
      /* ------ Check address correction --------*/
      BeginAddress = S_REG_HOLDING_START;
      if (address < (BeginAddress - 1))
        return MB_ENOREG;
      
      LastAddress = S_REG_HOLDING_START + S_REG_HOLDING_NREGS - 1;
      if (address > LastAddress)
        return MB_ENOREG;
      
      /* -------- Assign Data -------------*/
      *value = usSRegHoldBuf[address - BeginAddress];
   
      break;
    }
    default:
      return MB_EINVAL;
  }
  
  return MB_ENOERR;
}

eMBErrorCode eMBSlaveSetDataU16(eMBSlaveRegType regType, USHORT address, USHORT value)
{
  USHORT BeginAddress;
  USHORT LastAddress;
   
  switch (regType)
  {
    case MB_SLAVE_REG_TYPE_INPUT:
    {
      /* ------ Check address correction --------*/
      BeginAddress = S_REG_INPUT_START;
      if (address < (BeginAddress - 1))
        return MB_ENOREG;
      
      LastAddress = S_REG_INPUT_START + S_REG_INPUT_NREGS - 1;
      if (address > LastAddress)
        return MB_ENOREG;

      /* -------- Assign Data -------------*/
      usSRegInBuf[address - BeginAddress] = value;
   
      break;
    }
    case MB_SLAVE_REG_TYPE_HOLDING:
    {
      /* ------ Check address correction --------*/
      BeginAddress = S_REG_HOLDING_START;
      if (address < (BeginAddress - 1))
        return MB_ENOREG;
      
      LastAddress = S_REG_HOLDING_START + S_REG_HOLDING_NREGS - 1;
      if (address > LastAddress)
        return MB_ENOREG;
      
      /* -------- Assign Data -------------*/
      usSRegHoldBuf[address - BeginAddress] = value;
      
      break;
    }
    default:
      return MB_EINVAL;
  }
  
  return MB_ENOERR;
}
 
eMBErrorCode eMBSlaveGetDataU32(eMBSlaveRegType regType, USHORT address, ULONG* value)
{
  USHORT BeginAddress;
  USHORT LastAddress;
   
  USHORT HighData = 0;
  USHORT LowData = 0;
  
  switch (regType)
  {
    case MB_SLAVE_REG_TYPE_INPUT:
    {
      /* ------ Check address correction --------*/
      BeginAddress = S_REG_INPUT_START;
      if (address < (BeginAddress - 1))
        return MB_ENOREG;
      
      LastAddress = S_REG_INPUT_START + S_REG_INPUT_NREGS - 2;
      if (address > LastAddress)
        return MB_ENOREG;
      
      /* -------- Assign Data -------------*/
      LowData = usSRegInBuf[address - BeginAddress];
      HighData = usSRegInBuf[address - BeginAddress + 1];
   
      break;
    }
    case MB_SLAVE_REG_TYPE_HOLDING:
    {
      /* ------ Check address correction --------*/
      BeginAddress = S_REG_HOLDING_START;
      if (address < (BeginAddress - 1))
        return MB_ENOREG;
      
      LastAddress = S_REG_HOLDING_START + S_REG_HOLDING_NREGS - 2;
      if (address > LastAddress)
        return MB_ENOREG;
      
      /* -------- Assign Data -------------*/
      LowData = usSRegHoldBuf[address - BeginAddress];
      HighData = usSRegHoldBuf[address - BeginAddress + 1];
      
      break;
    }
    default:
      return MB_EINVAL;
  }
  
  *value = (((ULONG)HighData) << 16) | ((ULONG)LowData);
  
  return MB_ENOERR;
}

eMBErrorCode eMBSlaveSetDataU32(eMBSlaveRegType regType, USHORT address, ULONG value)
{
  USHORT BeginAddress;
  USHORT LastAddress;
  
  USHORT HighData = (USHORT)(value >> 16U);
  USHORT LowData = (USHORT)(value & 0x0000FFFF);
  
  switch (regType)
  {
    case MB_SLAVE_REG_TYPE_INPUT:
    {
      /* ------ Check address correction --------*/
      BeginAddress = S_REG_INPUT_START;
      if (address < (BeginAddress - 1))
        return MB_ENOREG;
      
      LastAddress = S_REG_INPUT_START + S_REG_INPUT_NREGS - 2;
      if (address > LastAddress)
        return MB_ENOREG;
      
      /* -------- Assign Data -------------*/
      usSRegInBuf[address - BeginAddress ] = LowData;
      usSRegInBuf[address - BeginAddress + 1] = HighData;
     
      break;
    }
    case MB_SLAVE_REG_TYPE_HOLDING:
    {
      /* ------ Check address correction --------*/
      BeginAddress = S_REG_HOLDING_START;
      if (address < (BeginAddress - 1))
        return MB_ENOREG;
      
      LastAddress = S_REG_HOLDING_START + S_REG_HOLDING_NREGS - 2;
      if (address > LastAddress)
        return MB_ENOREG;
      
      /* -------- Assign Data -------------*/
      usSRegHoldBuf[address - BeginAddress] = LowData;
      usSRegHoldBuf[address - BeginAddress + 1] = HighData;
   
      break;
    }
    default:
      return MB_EINVAL;
  }
  
  return MB_ENOERR;
}
