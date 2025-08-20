#pragma once
#include <rcommon/rcommon.h>
#include <Wincrypt.h>
#include <rcommon/RDataBuf.h>


typedef class RCOMMON_API RDataBuf<BYTE> RDataBufByte;


class RCOMMON_API RCrypt
{
public:
	RCrypt(void);
	~RCrypt(void);

	void SetCryptKey(const BYTE* a_pKey, UINT a_iSize);	
	void CryptData(const BYTE* a_pData, UINT a_iDataSize, LPBYTE* a_ppCrypted, UINT* a_pCryptedSize);
	void DecryptData(const BYTE* a_pData, UINT a_iDataSize, LPBYTE* a_ppCrypted, UINT* a_pCryptedSize);
private:
	void CreateCryptKey(const BYTE* a_pKey, UINT a_iLen);
	void CreateDefaultCryptKey();
	void Release();

	HCRYPTPROV m_hCryptProv; 
	HCRYPTKEY  m_hKey; 
	RDataBufByte m_data; // for encrypting
};
