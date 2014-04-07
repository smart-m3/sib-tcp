#include "HipParamFactory.h"
#include "DexDaemon.h"
#include "Exception.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>

static HDX::CDexDaemon* pDaemon = NULL;
static bool sRunning = false;

unsigned char sPriv[24] =
{
	0xe6, 0x48, 0x4a, 0x4f, 0x72, 0x89, 0xcc, 0x28,
	0x62, 0x7e, 0x77, 0x66, 0xa8, 0x14, 0x98, 0xe3,
	0x1e, 0x84, 0xd1, 0x1c, 0x2f, 0x12, 0x0f, 0x98
};

unsigned char sPub[49] =
{
	0x04, 0x5c, 0xdd, 0x1c, 0xc6, 0xd5, 0x9e, 0xcd,
	0x5d, 0x54, 0x4e, 0x6c, 0xb0, 0xa7, 0xe1, 0x63,
	0x65, 0x49, 0xdc, 0xbd, 0x59, 0x63, 0xb9, 0x5b,
	0xf3, 0x03, 0xd8, 0xfd, 0x03, 0xca, 0x6a, 0x74,
	0xa7, 0x0b, 0x88, 0xb8, 0x6a, 0x6f, 0x7c, 0x4b,
	0xd0, 0x36, 0x66, 0x39, 0xb9, 0x41, 0x5f, 0x02,
	0xfc
};

extern HDX::TCharArray sPrivKey;
extern HDX::TCharArray sPubKey;

void SignalStop(int)
{
	if (pDaemon && sRunning)
	{
		pDaemon->Stop();
		sRunning = false;
	}
};

int main()
{
	std::printf("** HIP DEX++ Responder V0.1\n");
	std::printf("** Press Ctrl-C to quit...\n");

	sPrivKey = HDX::TCharArray(sPriv, sPriv+24);
	sPubKey = HDX::TCharArray(sPub, sPub+49);

	if (getuid() != 0)
	{
		std::printf("** Program must be run as root.\n");
		return 1;
	}

	try
	{
		if ((pDaemon = new HDX::CDexDaemon()))
		{
			pDaemon->Start();
			sRunning = true;

			std::signal(SIGINT, SignalStop);

			while (sRunning)
			{
				sleep(1);
			}

			delete pDaemon;
		}
	}
	catch (HDX::MException& cEx)
	{
		std::printf("** %s\n", cEx.Message().c_str());
		return 1;
	}

	return 0;
};
