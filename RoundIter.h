#pragma once
// RoundIter.h: implementation of RRoundIter template class.
//
//////////////////////////////////////////////////////////////////////

template <class T> 
class RRoundIter  
{
public:
	RRoundIter(int a_iElemsCnt,	T a_val) : m_iElemsCnt(a_iElemsCnt), m_iCurrentElem(a_val) {};
	RRoundIter() {};
	T operator++();
	T operator++(int a_iDummy);
	T operator--();
	T operator--(int a_iDummy);
	void SetCount(int a_iElemsCnt) { m_iElemsCnt = a_iElemsCnt; m_iCurrentElem = static_cast<T>(0); } 
	int GetCount() const { return m_iElemsCnt; };
	void operator =(T a_iVal);
	operator T() const;
	friend T operator+(const RRoundIter& a_iter, int a_iVal);
	friend T operator+(int a_iVal, const RRoundIter& a_iter);
	friend T operator-(const RRoundIter& a_iter, int a_iVal);
	friend T operator-(int a_iVal, const RRoundIter& a_iter);

private:
	int m_iElemsCnt;
	T m_iCurrentElem;
};


// ---------------------------------------------------------
// increment operator (left side)
//
template <class T>
T RRoundIter<T>::operator++()
{
	int l_i = m_iCurrentElem;
	l_i++;
	if (l_i == m_iElemsCnt)
	{
		m_iCurrentElem = (T)0;
	}
	else
	{
		m_iCurrentElem = (T)l_i;
	}
	return m_iCurrentElem;
}

// ---------------------------------------------------------
// increment operator (right side)
//
template <class T>
T RRoundIter<T>::operator++(int)
{
	T l_iOld = m_iCurrentElem;
	++(*this);
	return l_iOld;
}


// ---------------------------------------------------------
// decrement operator (left side)
//
template <class T>
T RRoundIter<T>::operator--()
{
	if ((int)m_iCurrentElem == 0)
	{
		m_iCurrentElem = (T)(m_iElemsCnt - 1);
	}
	else
	{
		int l_i = m_iCurrentElem;
		m_iCurrentElem = (T)--l_i;
	}
	return m_iCurrentElem;
}

// ---------------------------------------------------------
// decrement operator (right side)
//
template <class T>
T RRoundIter<T>::operator--(int)
{
	T l_iOld = m_iCurrentElem;
	--(*this);
	return l_iOld;
}


// ---------------------------------------------------------
// conversion
//
template <class T>
RRoundIter<T>::operator T() const
{
	return (T)m_iCurrentElem;
}


// ---------------------------------------------------------
// assignment
//
template <class T>
void RRoundIter<T>::operator =(T a_iVal) 
{
	if (a_iVal < m_iElemsCnt)
	{
		m_iCurrentElem = a_iVal;
	}
}


// ---------------------------------------------------------
// additive operator
//
template <class T>
T operator+(const RRoundIter<T>& a_iter, int a_iVal)
{
	RRoundIter<T> l_iter = a_iter;
	if (a_iVal < 0)
	{
		return l_iter - (-a_iVal);
	}

	while ((--a_iVal) >= 0)
	{
		l_iter++;
	}
	return (T)l_iter;
}


// ---------------------------------------------------------
// additive operator
//
template <class T>
T operator+(int a_iVal, const RRoundIter<T>& a_iter)
{
	return a_iter + a_iVal;
}


// ---------------------------------------------------------
// subtractive operator
//
template <class T>
T operator-(const RRoundIter<T>& a_iter, int a_iVal)
{
	RRoundIter<T> l_iter = a_iter;
	if (a_iVal < 0)
	{
		return l_iter + (-a_iVal);
	}

	while ((--a_iVal) >= 0)
	{
		l_iter--;
	}
	return (T)l_iter;
}


// ---------------------------------------------------------
// subtractive operator
//
template <class T>
T operator-(int a_iVal, const RRoundIter<T>& a_iter)
{
	return a_iter - a_iVal;
}
