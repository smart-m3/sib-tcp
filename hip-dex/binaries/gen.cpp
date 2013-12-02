#include "HipParamFactory.h"
#include "DexDaemon.h"
#include "HipECKey.h"
#include "Exception.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>

#include <iostream>



using namespace std;


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


void LoadKey(HDX::TCharArray &tPub, HDX::TCharArray &tPriv, const char * filename){
    FILE * f = fopen(filename, "rb");
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


static unsigned char rHit[] =
{
	0x20, 0x01, 0x00, 0x15, 0x5c, 0xdd, 0x1c, 0xc6,
	0xd5, 0x9e, 0xcd, 0x5d, 0x54, 0x4e, 0x6c, 0xb0
};



void test()
{
    HDX::TCharArray Priv(sPriv, sPriv+24);
    HDX::TCharArray Pub(sPub, sPub+49);
    HDX::CHipECKey ckey(HDX::tPrime192, Priv, Pub);
    print_array(ckey.ToHit());
    const EC_KEY *key = ckey.GetECKey();
    const EC_POINT* apub = EC_KEY_get0_public_key(key);
    const EC_GROUP* group = EC_KEY_get0_group(key);
    HDX::TCharArray hit(EC_POINT_point2oct(group, apub, POINT_CONVERSION_COMPRESSED,0, 0, 0));
    EC_POINT_point2oct(group, apub, POINT_CONVERSION_COMPRESSED,&hit[0], hit.size(), 0);
    print_array(hit);
    print_array(HDX::TCharArray(rHit+4,rHit+16));
}

int main()
{
	printf("** HIP DEX++ Key Generator V0.1\n");
	printf("Generating 192-bit key\n");
	HDX::CHipECKey ckey(HDX::tPrime192);
	const EC_KEY *key = ckey.GetECKey();
	printf("Storing the key into key.dat\n");
	FILE *f = fopen("key.dat", "wb");
	

	const EC_POINT* apub = EC_KEY_get0_public_key(key);
	const EC_GROUP* group = EC_KEY_get0_group(key);

	HDX::TCharArray tPub(EC_POINT_point2oct(group, apub,
		POINT_CONVERSION_UNCOMPRESSED, 0, 0, 0));
	EC_POINT_point2oct(group, apub,
		POINT_CONVERSION_UNCOMPRESSED,
		&tPub[0], tPub.size(), 0);
		
	const BIGNUM * apriv = EC_KEY_get0_private_key(key);
	HDX::TCharArray tPriv(BN_num_bytes(apriv));
	BN_bn2bin(apriv, &tPriv[0]);
	
	print_array(tPub);
	print_array(tPriv);

	int i;
	
	fprintf(f, "%c", (unsigned char)tPub.size());
	for (i = 0; i < tPub.size(); i++)
	    fprintf(f, "%c", tPub[i]);
	fprintf(f, "%c", (unsigned char)tPriv.size());
	for (i = 0; i < tPriv.size(); i++)
	    fprintf(f, "%c", tPriv[i]);
	fclose(f);
	
	freopen("HIT.txt", "w", stdout);
	print_array(ckey.ToHit());
	fclose(stdout);

	return 0;
};
