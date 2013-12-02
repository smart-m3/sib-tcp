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

#ifndef __HDX_DEX_DAEMON_H__
#define __HDX_DEX_DAEMON_H__

#include "HipParamFactory.h"
#include "IPSocket.h"
#include "IPv4Socket.h"
#include "HipECKey.h"
#include "Exception.h"
#include "Runnable.h"
#include "Context.h"
#include "SafePtr.h"
#include "AesCtr.h"

#include <string>
#include <iostream>

namespace HDX
{
	class CHipECKey;
	class CThreadPool;
	class CConnectionFactory;


	class CsadbEntry
	{
		public:
			CsadbEntry(TCharArray alHIT, TCharArray arHIT, TCharArray alEK, TCharArray arEK, long &arIP)
				: rIP(arIP)
				, lHIT(alHIT)
				, rHIT(arHIT)
				, lEK(alEK)
				, rEK(arEK)
			{
			    std::cout << "Encrypt:\n";
			    encr = new CAesCtr(tKey128, alEK, arEK);
			    std::cout << "Decrypt:\n";
			    decr = new CAesCtr(tKey128, arEK, alEK);

			    
			    
			}
			
			CsadbEntry()
			{
			    encr = decr = NULL;
			}
			
			~CsadbEntry()
			{
//			    std::cout << "csadbentry destructor\n";
			    if (encr != NULL) delete encr;
			    encr = NULL;
			    if (decr != NULL) delete decr;
			    decr = NULL;
//			    std::cout << "csadbentry destructor ended\n";
			}
			
			long rIP;
//			in_addr lIP;
			TCharArray lHIT;
			TCharArray rHIT;
			TCharArray lEK;
			TCharArray rEK;
			CAesCtr *encr;
			CAesCtr *decr;
	};

	typedef std::vector <CsadbEntry*> TsadbType;
	
	class Csadb
	{
		private:
			 TsadbType db;
			 CMutex iMutex;
		public:
			Csadb() 
			{
			};
			
			
			void StoreSA(const TCharArray&, const TCharArray&, const TCharArray&, const TCharArray&, long);
			
			CsadbEntry *FindSA(long&);
			
			void RemoveSA(long&);
			
			~Csadb() 
			{
				db.clear();
			};
	};
	
	class CDexDaemon
	{
		public:
			CDexDaemon() throw (MException);
			void Start() throw (MException);
			void Stop() throw (MException);
			void Initiate(const TCharArray&,
				const std::string&)
				throw (MException);
			~CDexDaemon();
			Csadb sadb;
			
		private:
			bool iRunning;
			CThreadPool* iPool;
			CSafePtr<CConnectionFactory> iConnFactory;
			const CHipECKey* iHipECKey;

			template <class T>
			friend class CConnectionHandler;
			friend class CClientHandler;
			friend class CConnection;

	};
};

#endif
