/*
 *      Copyright (C) 2010 Alwin Esch (Team XBMC)
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "requestpacket.h"
#include "vnsicommand.h"
#include "tools.h"
#include "platform/sockets/tcp.h"

uint32_t cRequestPacket::serialNumberCounter = 1;

cRequestPacket::cRequestPacket()
{
  buffer        = NULL;
  bufSize       = 0;
  bufUsed       = 0;
  lengthSet     = false;
  serialNumber  = 0;
  opcode        = 0;
}

cRequestPacket::~cRequestPacket()
{
  free(buffer);
}

bool cRequestPacket::init(uint32_t topcode, bool stream, bool setUserDataLength, size_t userDataLength)
{
  if (buffer) return false;

  if (setUserDataLength)
  {
    bufSize = headerLength + userDataLength;
    lengthSet = true;
  }
  else
  {
    bufSize = 512;
    userDataLength = 0; // so the below will write a zero
  }

  buffer = (uint8_t*)malloc(bufSize);
  if (!buffer) return false;

  if (!stream)
    channel     = VNSI_CHANNEL_REQUEST_RESPONSE;
  else
    channel     = VNSI_CHANNEL_STREAM;
  serialNumber  = serialNumberCounter++;
  opcode        = topcode;

  uint32_t ul;
  ul = htonl(channel);
  memcpy(&buffer[0], &ul, sizeof(uint32_t));
  ul = htonl(serialNumber);
  memcpy(&buffer[4], &ul, sizeof(uint32_t));
  ul = htonl(opcode);
  memcpy(&buffer[8], &ul, sizeof(uint32_t));
  ul = htonl(userDataLength);
  memcpy(&buffer[userDataLenPos], &ul, sizeof(uint32_t));
  bufUsed = headerLength;

  return true;
}

bool cRequestPacket::add_String(const char* string)
{
  size_t len = strlen(string) + 1;
  if (!checkExtend(len)) return false;
  memcpy(buffer + bufUsed, string, len);
  bufUsed += len;
  if (!lengthSet)
  {
    uint32_t tmp = htonl(bufUsed - headerLength);
    memcpy(&buffer[userDataLenPos], &tmp, sizeof(uint32_t));
  }
  return true;
}

bool cRequestPacket::add_U8(uint8_t c)
{
  if (!checkExtend(sizeof(uint8_t))) return false;
  buffer[bufUsed] = c;
  bufUsed += sizeof(uint8_t);
  if (!lengthSet)
  {
    uint32_t tmp = htonl(bufUsed - headerLength);
    memcpy(&buffer[userDataLenPos], &tmp, sizeof(uint32_t));
  }
  return true;
}

bool cRequestPacket::add_S32(int32_t l)
{
  if (!checkExtend(sizeof(int32_t))) return false;
  int32_t tmp = htonl(l);
  memcpy(&buffer[bufUsed], &tmp, sizeof(int32_t));
  bufUsed += sizeof(int32_t);
  if (!lengthSet)
  {
    uint32_t tmp = htonl(bufUsed - headerLength);
    memcpy(&buffer[userDataLenPos], &tmp, sizeof(uint32_t));
  }
  return true;
}

bool cRequestPacket::add_U32(uint32_t ul)
{
  if (!checkExtend(sizeof(uint32_t))) return false;
  uint32_t tmp = htonl(ul);
  memcpy(&buffer[bufUsed], &tmp, sizeof(uint32_t));
  bufUsed += sizeof(uint32_t);
  if (!lengthSet)
  {
    uint32_t tmp = htonl(bufUsed - headerLength);
    memcpy(&buffer[userDataLenPos], &tmp, sizeof(uint32_t));
  }
  return true;
}

bool cRequestPacket::add_U64(uint64_t ull)
{
  if (!checkExtend(sizeof(uint64_t))) return false;
  uint64_t tmp = htonll(ull);
  memcpy(&buffer[bufUsed], &tmp, sizeof(uint64_t));
  bufUsed += sizeof(uint64_t);
  if (!lengthSet)
  {
    uint32_t tmp = htonl(bufUsed - headerLength);
    memcpy(&buffer[userDataLenPos], &tmp, sizeof(uint32_t));
  }
  return true;
}

bool cRequestPacket::add_S64(int64_t ll)
{
  if (!checkExtend(sizeof(int64_t))) return false;
  int64_t tmp = htonll(ll);
  memcpy(&buffer[bufUsed], &tmp, sizeof(int64_t));
  bufUsed += sizeof(int64_t);
  if (!lengthSet)
  {
    uint32_t tmp = htonl(bufUsed - headerLength);
    memcpy(&buffer[userDataLenPos], &tmp, sizeof(uint32_t));
  }
  return true;
}

bool cRequestPacket::checkExtend(size_t by)
{
  if (lengthSet) return true;
  if ((bufUsed + by) <= bufSize) return true;
  uint8_t* newBuf = (uint8_t*)realloc(buffer, bufUsed + by);
  if (!newBuf)
  {
    newBuf = (uint8_t*)malloc(bufUsed + by);
    if (!newBuf) {
      return false;
    }
    memcpy(newBuf, buffer, bufUsed);
    free(buffer);
  }
  buffer = newBuf;
  bufSize = bufUsed + by;
  return true;
}

