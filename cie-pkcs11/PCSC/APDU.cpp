
#include "APDU.h"
#include "../Util/TLV.h"
#include "../Util/util.h"
#include "../Crypto/DES3.h"
#include "../Crypto/MAC.h"
#include "Token.h"

//extern CLog Log;

APDU::APDU()  {
}
APDU::APDU(uint8_t CLA,uint8_t INS,uint8_t P1,uint8_t P2,uint8_t LC,uint8_t *pData,uint8_t LE)  {
	init_func
	if (LC>250) throw;
	btINS=INS;btCLA=CLA;btP1=P1;btP2=P2;btLC=LC;pbtData=pData;btLE=LE;
	bLC=true;bLE=true;
	exit_func
}
APDU::APDU(uint8_t CLA,uint8_t INS,uint8_t P1,uint8_t P2,uint8_t LC,uint8_t *pData)   {
	if (LC>251) throw;
	btINS=INS;btCLA=CLA;btP1=P1;btP2=P2;btLC=LC;pbtData=pData;btLE=0;
	bLC=true;bLE=false;
}
APDU::APDU(uint8_t CLA,uint8_t INS,uint8_t P1,uint8_t P2,uint8_t LE)   {
	btINS=INS;btCLA=CLA;btP1=P1;btP2=P2;btLE=LE;btLC=0;
	bLC=false;bLE=true;
}
APDU::APDU(uint8_t CLA,uint8_t INS,uint8_t P1,uint8_t P2)   {
	btINS=INS;btCLA=CLA;btP1=P1;btP2=P2;btLE=0;btLC=0;
	bLC=false;bLE=false;
}

APDU::~APDU()
{
}
