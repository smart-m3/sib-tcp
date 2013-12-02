#include "HipParamFactory.h"
#include "DexDaemon.h"
#include "Exception.h"
#include "Defines.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>

static HDX::CDexDaemon* pDaemon = NULL;
static bool sRunning = false;

unsigned char sPriv[24] =
{
	0x47, 0x7d, 0x37, 0xc6, 0x3c, 0x71, 0x54, 0x2f,
	0xab, 0xfe, 0xc9, 0x0b, 0xb6, 0xd9, 0xdc, 0x46,
	0x30, 0x4c, 0x93, 0xaa, 0x73, 0x9c, 0x25, 0x44
};

unsigned char sPub[49] =
{
	0x04, 0xe5, 0xed, 0x11, 0xf6, 0x2f, 0x38, 0x35,
	0x57, 0xaf, 0x13, 0x8c, 0xb7, 0x60, 0x79, 0x29,
	0xd0, 0x2c, 0xf6, 0x90, 0x4e, 0x25, 0x8d, 0x1c,
	0x49, 0x52, 0x5b, 0x9f, 0x46, 0xa3, 0xcc, 0xb3,
	0xa6, 0x03, 0x49, 0xc2, 0x56, 0xd9, 0x4c, 0x81,
	0xaf, 0x80, 0xbd, 0x9c, 0x9d, 0xc5, 0xa1, 0x65,
	0x8b
};

extern HDX::TCharArray sPrivKey;
extern HDX::TCharArray sPubKey;

static unsigned char rHit[] =
{
	0x20, 0x01, 0x00, 0x15, 0x5c, 0xdd, 0x1c, 0xc6,
	0xd5, 0x9e, 0xcd, 0x5d, 0x54, 0x4e, 0x6c, 0xb0
};

void SignalStop(int)
{
	if (pDaemon && sRunning)
	{
		pDaemon->Stop();
		sRunning = false;
	}
};

int main(int argc, char** argv)
{
	std::printf("** HIP DEX++ Initiator V0.1\n");
	std::printf("** Press Ctrl-C to quit...\n");

	sPrivKey = HDX::TCharArray(sPriv, sPriv+24);
	sPubKey = HDX::TCharArray(sPub, sPub+49);

	if (getuid() != 0)
	{
		std::printf("** Program must be run as root.\n");
		return 1;
	}

	if (argc != 3)
	{
		std::printf("** Usage: ./initiator <interval> <handshakes>\n");
		return 1;
	}

	try
	{
		if ((pDaemon = new HDX::CDexDaemon()))
		{
			pDaemon->Start();

			int tInter = std::atoi(argv[1]);
			int tTimes = std::atoi(argv[2]);

			HDX::TCharArray tLHIT(rHit, rHit+16);

			while (tTimes > 0)
			{
				pDaemon->Initiate(tLHIT, HDX_RESPONDER_ADDR);
				sleep(tInter);
				--tTimes;
			}

			pDaemon->Stop();
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
