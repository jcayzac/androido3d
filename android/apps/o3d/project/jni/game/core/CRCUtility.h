//	CRCUtility.h
//	Copyright (C) 2008 Chris Pruett.		c_pruett@efn.org
//
//	FarClip Engine
//
//	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
//
//			http://www.apache.org/licenses/LICENSE-2.0
//
//	Unless required by applicable law or agreed to in writing, software
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//	See the License for the specific language governing permissions and
//	limitations under the License.

 
 #ifndef _CRCUTILITY_H
 #define _CRCUTILITY_H
 
 // crc function stolen from 
// http:// www.netrino.com/Connecting/2000-01/index.html

 template<typename T>
 T calculateCRC(const unsigned char* pData, const size_t byteCount)
 {
	const int width = sizeof(T) * 8;
	const int topBit = (1 << (width - 1));
	const int polynomial = 0xD8;	// 11011 followed by 0's
	
	T remainder = 0;	

		// Perform modulo-2 division, a byte at a time.
		for (int byte = 0; byte < static_cast<int>(byteCount); ++byte)
		{
				// Bring the next byte into the remainder.
				remainder ^= (pData[byte] << (width - 8));

				// Perform modulo-2 division, a bit at a time.
				for (unsigned char bit = 8; bit > 0; --bit)
				{
						 // Try to divide the current data bit.
						if (remainder & topBit)
						{
								remainder = (remainder << 1) ^ polynomial;
						}
						else
						{
								remainder = (remainder << 1);
						}
				}
		}

		// The final remainder is the CRC result.
		return (remainder);
 }
 
 #endif //_CRCUTILITY_H

