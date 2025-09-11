#pragma once
#include <rcommon/RDynamicBuf.h>

template <class T>
class RDataBuf : public RDynamicBuf<T>
{
public:

	RDataBuf(void)
	{
		m_iSize = 0;
	}

	virtual ~RDataBuf(void)
	{
	}

	UINT GetDataSize(void) const { return m_iSize; }

	void SetData(const T* a_pData, UINT a_iSize)
	{
		T* l_pBuf = this->GetBuffer(a_iSize);
		m_iSize = a_iSize;
		memcpy(l_pBuf, a_pData, a_iSize);
	}

	void SetDataSize(UINT a_iSize) { m_iSize = a_iSize; }
	RDataBuf(const RDataBuf& a_databuf) 
	{ 
		SetData(a_databuf.GetRawBuffer(), a_databuf.GetDataSize());
	}

	RDataBuf& operator=(const RDataBuf& a_databuf) 
	{ 
		SetData(a_databuf.GetRawBuffer(), a_databuf.GetDataSize());
		return *this;
	}

	RDataBuf& operator+=(const RDataBuf& a_databuf) 
	{ 
		UINT l_iStart = this->m_iSize;

		T* l_pTemp = new T[m_iSize + a_databuf.GetDataSize()];
		memcpy(l_pTemp, this->GetBufferData(), m_iSize);
		memcpy(l_pTemp + l_iStart, a_databuf.GetRawBuffer(), a_databuf.GetDataSize());
		SetData(l_pTemp, m_iSize + a_databuf.GetDataSize());
		delete[] l_pTemp;
		return *this; 
	}

private:
	UINT m_iSize;
};
