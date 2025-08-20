#include "StdAfx.h"
#include "RCrypt.h"
#include <rcommon/RSystemExc.h>
#include <rcommon/ROwnExc.h>

#define KEYLENGTH			0x00800000
#define ENCRYPT_ALGORITHM	CALG_RC4 
#define ENCRYPT_BLOCK_SIZE	8 


RCrypt::RCrypt(void)
{
	m_hCryptProv = NULL; 
	m_hKey = NULL; 
}


RCrypt::~RCrypt(void)
{
	Release();
}


void RCrypt::Release()
{
	if (m_hKey != NULL)
	{
		CryptDestroyKey(m_hKey);
		m_hKey = NULL;
	}
	if (m_hCryptProv != NULL)
	{
		CryptReleaseContext(m_hCryptProv, 0);
		m_hCryptProv = NULL;
	}
}


void RCrypt::SetCryptKey(const BYTE* a_pKey, UINT a_iSize)
{
	Release();
	CreateCryptKey(a_pKey, a_iSize);
}


void RCrypt::CreateCryptKey(const BYTE* a_pKey, UINT a_iLen)
{
	HCRYPTHASH l_hHash;
	if(!CryptAcquireContext(&m_hCryptProv, NULL, NULL, PROV_RSA_FULL, 0))
	{
		if(!::CryptAcquireContext(&m_hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) 
		{
			RSystemExc l_exc(::GetLastError()); 
			throw l_exc;
		}
	}

	if(!CryptCreateHash(m_hCryptProv, CALG_MD5, 0, 0, &l_hHash))
	{ 
		RSystemExc l_exc(::GetLastError()); 
		throw l_exc;
	}  

	if(!CryptHashData(l_hHash, a_pKey, a_iLen, 0))
	{
		RSystemExc l_exc(::GetLastError()); 
		throw l_exc;
	}

	if(!CryptDeriveKey(m_hCryptProv, ENCRYPT_ALGORITHM, l_hHash, KEYLENGTH, &m_hKey))
	{
		RSystemExc l_exc(::GetLastError()); 
		throw l_exc;
	}

	if(!CryptDestroyHash(l_hHash))
	{
		RSystemExc l_exc(::GetLastError()); 
		throw l_exc;
	}
}


void RCrypt::CryptData(const BYTE* a_pData, UINT a_iDataSize, LPBYTE* a_ppCrypted, UINT* a_pCryptedSize)
{
	// crypting not available (SetCryptKey wasn't called)
	if (m_hKey == NULL)
	{
		return;
	}
	DWORD l_dwCrypted = a_iDataSize;
	*a_pCryptedSize = a_iDataSize + ENCRYPT_BLOCK_SIZE;
	m_data.AllocBuffer(*a_pCryptedSize);
	memcpy(m_data, a_pData, a_iDataSize);
	if(!CryptEncrypt(m_hKey, 0, TRUE, 0, m_data, &l_dwCrypted, *a_pCryptedSize))
	{ 
		RSystemExc l_exc(::GetLastError()); 
		throw l_exc;
	}
	*a_ppCrypted = m_data;
}


void RCrypt::DecryptData(const BYTE* a_pData, UINT a_iDataSize, LPBYTE* a_ppCrypted, UINT* a_pCryptedSize)
{
	// crypting not available (SetCryptKey wasn't called)
	if (m_hKey == NULL)
	{
		return;
	}
	DWORD l_dwCrypted = a_iDataSize;
	m_data.AllocBuffer(a_iDataSize);
	memcpy(m_data, a_pData, a_iDataSize);
	if(!CryptDecrypt(m_hKey, 0, TRUE, 0, m_data, &l_dwCrypted))
	{
		RSystemExc l_exc(::GetLastError()); 
		throw l_exc;
	}
	*a_ppCrypted = m_data;
	*a_pCryptedSize = a_iDataSize - ENCRYPT_BLOCK_SIZE;
}
