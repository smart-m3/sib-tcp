/**
 * Host Identity Protocol (HIP) Diet Exchange (DEX) C++ Library (HDX++)
 *
 * HDX++ is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HDX++ is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License and GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with HDX++. If not, see <http://www.gnu.org/licenses/>.
 *
 * This file is part of the HDX++ library and authored by Jani Pellikka
 * <jani.pellikka@iki.fi>. Copyright (C) 2011 by University of Oulu.
 */

#ifndef __HDX_PROTOCOL_DEFINES_H__
#define __HDX_PROTOCOL_DEFINES_H__

/* HIP-DEX Parameter Type Values */

#define HDX_TYPE_R1_COUNTER	  	  128
#define HDX_TYPE_PUZZLE		  	  257
#define HDX_TYPE_SOLUTION	  	  321
#define HDX_TYPE_HIP_CIPHER	  	  579
#define HDX_TYPE_ENCRYPTED	  	  641
#define HDX_TYPE_ENCRYPTED_KEY	  643
#define HDX_TYPE_HOST_ID		  705
#define HDX_TYPE_HIT_SUITE_LIST	  715
#define HDX_TYPE_DH_GROUP_LIST	 2151
#define HDX_TYPE_HIP_MAC_3		61507
#define HDX_TYPE_ECHO_RES_USIG	63425
#define HDX_TYPE_ECHO_REQ_USIG	63661

#define HDX_TYPE_PACKET_I1	 0x01
#define HDX_TYPE_PACKET_R1	 0x02
#define HDX_TYPE_PACKET_I2	 0x03
#define HDX_TYPE_PACKET_R2	 0x04

#define HDX_SIZE_PARAM_HDR 4
#define HDX_SIZE_PACKET_HDR 40

/* Other HIP-DEX Related Defines */

#define HDX_IPPROTO_NONE 59
#define HDX_LENGTH_HIT 16

#define ALGORITHM_ECDH 11
#define CURVE_SECP160R1 1
#define CURVE_SECP192R1 2
#define CURVE_SECP224R1 3
#define HIT_ECDH_DEX 8

#define NULL_ENCRYPT 1
#define AES_128_CBC 2
#define THREEDES_CBC 3

#define DH_GROUP_ECP160 7
#define DH_GROUP_ECP256 8
#define DH_GROUP_ECP384 9
#define DH_GROUP_ECP521 10
#define DH_GROUP_ECP192 11
#define DH_GROUP_ECP224 12

#endif
