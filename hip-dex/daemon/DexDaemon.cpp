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

#include "DexDaemon.h"
#include "HipDefines.h"
#include "Defines.h"
#include "SocketException.h"
#include "ConnectionFactory.h"
#include "ConnectionHandler.h"
#include "HipDhGroupList.h"
#include "HipPacket.h"
#include "HipPacketI1.h"
#include "ThreadPool.h"
#include "IPSocket.h"
#include "Connection.h"
#include "HipECKey.h"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <cstdio>

// C interface
#include "hipdexc.h"

using namespace HDX;


HDX::TCharArray sPrivKey;
HDX::TCharArray sPubKey;


void print_array(const HDX::TCharArray& aArray)
{
	std::cout << std::dec << "[" << aArray.size() << "] : ";
	for (size_t i = 0; i < aArray.size(); ++i) {
		if (i > 0)
		    std::cout << ", ";
		std::cout << "0x" <<  std::hex << (int)aArray[i];
	}
	std::cout << std::endl;
};



void Csadb::StoreSA(const TCharArray &alHIT, const TCharArray &arHIT, const TCharArray &alEK, const TCharArray &arEK, long arIP)
{
    iMutex.Lock();
//    std::cout << "Adding new to: " << (int)((arIP>>24) &255) << '.' << ((arIP>>16) &255) << '.' << ((arIP>>8) &255) << '.' << (arIP &255) << "\n";
    for (TsadbType::iterator i = db.begin(); i != db.end(); ++i) {
	if ((*i)->rIP == arIP) {
	//    std::cout << "Already exists. Deleting.\n";
	    delete (*i);
	    db.erase(i, i+1);
	    break;
	}
    }
//    std::cout << "Added\n";
    db.push_back(new CsadbEntry(alHIT, arHIT, alEK, arEK, arIP));
    iMutex.Unlock();
}

CsadbEntry *Csadb::FindSA(long &arIP)
{
    iMutex.Lock();
//    std::cout << "Search SA to: " << ((arIP>>24) &255) << '.' << ((arIP>>16) &255) << '.' << ((arIP>>8) &255) << '.' << (arIP &255) << '\n';
    for (unsigned int i = 0; i < db.size(); i++) {
	//std::cout << "checking: " << ((db[i]->rIP>>24) &255) << '.' << ((db[i]->rIP>>16) &255) << '.' << ((db[i]->rIP>>8) &255) << '.' << (db[i]->rIP &255) << '\n';
	if (db[i]->rIP == arIP) {
	//    std::cout << "Found!\n";
	    iMutex.Unlock();
	    return db[i];
	}
    }
    //std::cout << "Not found\n";
    iMutex.Unlock();
    return NULL;
}

void Csadb::RemoveSA(long &arIP)
{
    iMutex.Lock();
    //std::cout << "Removing SA to: " << ((arIP>>24) &255) << '.' << ((arIP>>16) &255) << '.' << ((arIP>>8) &255) << '.' << (arIP &255) << '\n';
    for (TsadbType::iterator i = db.begin(); i != db.end(); ++i) {
	//std::cout << "checking: " << (((*i)->rIP>>24) &255) << '.' << (((*i)->rIP>>16) &255) << '.' << (((*i)->rIP>>8) &255) << '.' << ((*i)->rIP &255) << '\n';
	if ((*i)->rIP == arIP) {       
	//    std::cout << "Removing\n";
	    delete (*i);
	    db.erase(i, i+1);
	    iMutex.Unlock();
	//    std::cout << "Removed\n";
	    return;
	}
    }
//    std::cout << "Not found\n";
    iMutex.Unlock();
}

CDexDaemon::CDexDaemon() throw (MException)
	: iRunning(false), iPool(NULL),
	  iConnFactory(new CConnectionFactory()),
	  iHipECKey(new CHipECKey(tPrime192,
		sPrivKey, sPubKey))
{
	iPool = new CThreadPool(HDX_NUM_THREADS);
};

CDexDaemon::~CDexDaemon()
{
	delete iPool;
	delete iHipECKey;
};

void CDexDaemon::Start() throw (MException)
{
	if (!iRunning)
	{
		iRunning = true;

		CConnectionHandler<CIPv4Socket>* pHandler
			= new CConnectionHandler<CIPv4Socket>(this);

		iPool->Assign(pHandler);
	}
};

void CDexDaemon::Stop() throw (MException)
{
	if (iRunning)
	{
		iRunning = false;

		if (iPool)
			iPool->Close();
	}
};

void CDexDaemon::Initiate(const TCharArray& aHit,
	const std::string& aIPAddr) throw (MException)
{
	CIPSocket<CIPv4Socket>
		tSocket(HDX_PROTOCOL_NAME, aIPAddr);

	CHipDhGroupList* tList = new
		CHipDhGroupList(DH_GROUP_ECP192);

	CHipPacketI1 tPacketI1(tList);
	tPacketI1.SetSenderHit(iHipECKey->ToHit());
	tPacketI1.SetReceiverHit(aHit);
	CSafePtr<CConnection> tConn = (*iConnFactory)->Create(aHit);
	(*tConn)->tDhGroupList = tList->GetList();
	(*tConn)->currentState = tI1Sent;
	tSocket << tPacketI1.ToBytes();
};




CDexDaemon *HIPDEX_pDaemon;



void LoadKey(TCharArray &tPub, TCharArray &tPriv, const char * filename){
    FILE * f = fopen(filename, "rb");
    if (f == NULL) {
	std::cout << "File key.dat not found.\n";
	exit(1);
    }
    int i;
    unsigned char c;
    fscanf(f, "%c", &c);
    tPub.resize((int)c);
    for (i = 0; i < c; i++)
	fscanf(f, "%c", &tPub[i]);
    fscanf(f, "%c", &c);
    tPriv.resize((int)c);
    for (i = 0; i < c; i++)
	fscanf(f, "%c", &tPriv[i]);
}




int HIPDEX_Start()
{
    try {
	std::cout << "loading key\n";
	LoadKey(sPubKey, sPrivKey, "key.dat");
	if (HIPDEX_pDaemon == NULL) {
		HIPDEX_pDaemon = new CDexDaemon();
		HIPDEX_pDaemon->Start();
	}
    } catch (...){
	return 1;
    }
    return 0;
}

int HIPDEX_Stop()
{
    try {
	if (HIPDEX_pDaemon != NULL)
		HIPDEX_pDaemon->Stop();
    } catch (...) {
	return 1;
    }
    return 0;
}








int HIPDEX_Initiate(unsigned char *rHit, char * aIPAddr)
{
    try {
	if (HIPDEX_pDaemon == NULL) {
	    std::cout << "Not running\n";
	    return 2;
	}
	long addr = inet_addr(aIPAddr);
	HIPDEX_RemoveSA(addr);
	TCharArray HIT(rHit, rHit+16);
	
	HIPDEX_pDaemon->Initiate(HIT, aIPAddr);
	clock_t start, cur;
	start = clock();
	int count = 0;
	double diff;
	do {
	    cur = clock();
	    diff = ((double)cur - (double)start) / CLOCKS_PER_SEC;
	    if (diff > 1) {
		HIPDEX_pDaemon->Initiate(HIT, aIPAddr);
		start = cur;
		count++;
	    }
	    if (count > HDX_MAX_INITIATION_WAIT_COUNT) {
		return 1;
	    }
	} while (HIPDEX_GetSA(addr) == NULL);
    } catch(MException e) {
	std::cerr <<"Unable to Initiate: " << e.Message() << "\n";
	return 1;
    }
    return 0;

}




void *HIPDEX_GetSA(long rIP)
{
    CsadbEntry *entry = HIPDEX_pDaemon->sadb.FindSA(rIP);
    return (void *)entry;
}
    
void HIPDEX_RemoveSA(long rIP)
{
    HIPDEX_pDaemon->sadb.RemoveSA(rIP);
}
    
void HIPDEX_Encrypt(void* sa, char* in, char* out, int len)
{
    if (sa == NULL) {
	std::cout << "ERROR: null SA in encrypt\n";
	return;
    }
    CsadbEntry *entry = (CsadbEntry*) sa;
    TCharArray input(len);
    memcpy(&input[0], in, len);
    TCharArray cipher = entry->encr->Encrypt(input);
    memcpy(out, &cipher[0], cipher.size());
}
    
void HIPDEX_Decrypt(void* sa, char* in, char* out, int len)
{
    if (sa == NULL) {
	std::cout << "ERROR: null SA in decrypt\n";
	return;
    }
    CsadbEntry *entry = (CsadbEntry*) sa;
    TCharArray input(len);
    memcpy(&input[0], in, len);
    TCharArray cipher = entry->decr->Encrypt(input);
    memcpy(out, &cipher[0], cipher.size());
}

