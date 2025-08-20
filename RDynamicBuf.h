#pragma once

template <class T>
class RDynamicBuf
{
public:
	RDynamicBuf()
	{
		m_iAllocSize = 0;
		m_pBt = NULL;
	}

	~RDynamicBuf(void)
	{
		Release();
	}

	T* GetBuffer(size_t a_iSize)
	{
		AllocBuffer(a_iSize);
		return GetRawBuffer();
	}

	void Release(void)
	{
		if (m_pBt != NULL)
		{
			ASSERT(m_iAllocSize != 0);
			delete[] m_pBt;
			m_pBt = NULL;
			m_iAllocSize = 0;
		}
	}
	
	const T* GetBufferData() const { return m_pBt; }

	void AllocBuffer(size_t a_iSize)
	{
		if (a_iSize <= m_iAllocSize)
		{
			return;
		}
		if (m_pBt != NULL)
		{
			delete[] m_pBt;
		}
		m_pBt = new T[a_iSize];
		m_iAllocSize = a_iSize;
	}

	operator T*() const { return m_pBt; }
	T* GetRawBuffer() const { return m_pBt; }

protected:
	size_t GetBufferSize() const { return m_iAllocSize; }

private:
	T* m_pBt;
	size_t m_iAllocSize;
};
