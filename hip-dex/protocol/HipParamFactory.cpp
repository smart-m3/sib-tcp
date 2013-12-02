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

#include "HipParamFactory.h"
#include "HipParameter.h"
#include "HipDefines.h"

#include "HipR1Counter.h"
#include "HipPuzzle.h"
#include "HipSolution.h"
#include "HipHipCipher.h"
#include "HipEncrypted.h"
#include "HipEncryptedKey.h"
#include "HipHitSuiteList.h"
#include "HipDhGroupList.h"
#include "HipHostId.h"
#include "HipHipMac3.h"
#include "HipEchoReqUsig.h"
#include "HipEchoResUsig.h"

using namespace HDX;

CHipParamFactory::CHipParamFactory()
{
/*
	iParams[HDX_TYPE_R1_COUNTER] = &(CHipR1Counter::Create);
	iParams[HDX_TYPE_PUZZLE] = &(CHipPuzzle::Create);
	iParams[HDX_TYPE_SOLUTION] = &(CHipSolution::Create);
	iParams[HDX_TYPE_HIP_CIPHER] = &(CHipCipher::Create);
	iParams[HDX_TYPE_ENCRYPTED] = &(CHipEncrypted::Create);
	iParams[HDX_TYPE_ENCRYPTED_KEY] = &(CHipEncryptedKey::Create);
	iParams[HDX_TYPE_HOST_ID] = &(CHipHostId::Create);
	iParams[HDX_TYPE_HIT_SUITE_LIST] = &(CHipHitSuiteList::Create);
	iParams[HDX_TYPE_DH_GROUP_LIST] = &(CHipDhGroupList::Create);
	iParams[HDX_TYPE_HIP_MAC_3] = &(CHipHipMac3::Create);
	iParams[HDX_TYPE_ECHO_RES_USIG] = &(CHipEchoResUsig::Create);
	iParams[HDX_TYPE_ECHO_REQ_USIG] = &(CHipEchoReqUsig::Create);
*/
};

CHipParamFactory::~CHipParamFactory()
{
};

MHipParameter* CHipParamFactory::Create(TCharArray& aBytes) const
{
	return Create(aBytes, 0, aBytes.size());
};

MHipParameter* CHipParamFactory::Create(TCharArray& aBytes,
	std::size_t aOffset, std::size_t aLength) const
{
	if (((aOffset+aLength) > aBytes.size())
		|| (aLength < HDX_SIZE_PARAM_HDR))
		return NULL;

	unsigned short tType = ((aBytes[aOffset]&0xFF)<<8)
		|(aBytes[aOffset+1]&0xFF);

//	ParamList::const_iterator tIter = iParams.find(tType);
	MHipParameter* tParam = NULL;
	switch (tType) {
	case HDX_TYPE_R1_COUNTER: tParam = CHipR1Counter::Create(); break;
	case HDX_TYPE_PUZZLE: tParam = CHipPuzzle::Create(); break;
	case HDX_TYPE_SOLUTION: tParam = CHipSolution::Create(); break;
	case HDX_TYPE_HIP_CIPHER: tParam = CHipCipher::Create(); break;
	case HDX_TYPE_ENCRYPTED: tParam = CHipEncrypted::Create(); break;
	case HDX_TYPE_ENCRYPTED_KEY: tParam = CHipEncryptedKey::Create(); break;
	case HDX_TYPE_HOST_ID: tParam = CHipHostId::Create(); break;
	case HDX_TYPE_HIT_SUITE_LIST: tParam = CHipHitSuiteList::Create(); break;
	case HDX_TYPE_DH_GROUP_LIST: tParam = CHipDhGroupList::Create(); break;
	case HDX_TYPE_HIP_MAC_3: tParam = CHipHipMac3::Create(); break;
	case HDX_TYPE_ECHO_RES_USIG: tParam = CHipEchoResUsig::Create(); break;
	case HDX_TYPE_ECHO_REQ_USIG: tParam = CHipEchoReqUsig::Create(); break;
	}
	
	if (tParam != NULL)
	{ 
		if (tParam->FromBytes(aBytes, aOffset))
			return tParam;
		delete tParam;
	}

	return NULL;
};
