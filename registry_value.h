#pragma once

#include <windows.h>
#include <rcommon/rstring.h>
#include <assert.h>
#include <iomanip>

class registry_value
{
	public:
		registry_value(const tstring& a_sName, HKEY a_hkeyBase) : m_hkey(0), m_hkeyBase(a_hkeyBase)
		{
			make_path(a_sName);
		}
		virtual ~registry_value() {}

		bool exists()
		{
			bool l_bRet = false;

			if(open(false))
			{
				DWORD l_dwType;
				DWORD l_dwSize;
				l_bRet = query_value(&l_dwType, &l_dwSize, 0);				
				close();
			}

			return l_bRet;
		}

		void remove_value(void)
		{
			if(open(true))
			{
				RegDeleteValue(m_hkey, m_sValueName.c_str());				
				close();
			}
		}

		void remove_key(void)
		{
			RegDeleteKey(m_hkeyBase, m_sKeyName.c_str());
		}

		bool get_size(LPDWORD a_pSize)
		{
			bool l_bOk = false;
			if(open(false))
			{
				l_bOk = (RegQueryValueEx(m_hkey, m_sValueName.c_str(), 0, NULL, NULL, a_pSize) == ERROR_SUCCESS);
				close();
			}
			return l_bOk;
		}

	protected:

		bool open(bool a_bWriteAccess)
		{
			assert(m_hkey == 0);

			if(a_bWriteAccess)
			{
				DWORD l_dwDisposition;
				if (RegCreateKeyEx(m_hkeyBase, m_sKeyName.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &m_hkey, &l_dwDisposition) == ERROR_SUCCESS)
					return true;
			}
			
			// try open read-only
			return (RegOpenKeyEx(m_hkeyBase, m_sKeyName.c_str(), 0, KEY_READ, &m_hkey) == ERROR_SUCCESS);
		}

		void close(void)
		{
			assert(m_hkey != 0);
			RegCloseKey(m_hkey);
			m_hkey = 0;
		}

		bool query_value(DWORD* a_pdwType, DWORD* a_pdwSize, LPVOID a_pBuf)
		{
			assert(m_hkey != 0);
			return RegQueryValueEx(m_hkey, m_sValueName.c_str(), 0, a_pdwType, reinterpret_cast<BYTE*>(a_pBuf), a_pdwSize) == ERROR_SUCCESS;
		}

		bool set_value(DWORD a_dwType, DWORD a_dwSize, LPCVOID a_pBuf)
		{
			assert(m_hkey != 0);
			return RegSetValueEx(m_hkey, m_sValueName.c_str(), 0, a_dwType, reinterpret_cast<const BYTE *>(a_pBuf), a_dwSize) == ERROR_SUCCESS;
		}

	private:

		void make_path(const tstring& a_sName)
		{
			tstring::size_type l_pos = a_sName.find_last_of('\\');
			assert(l_pos != tstring::npos);
            m_sKeyName = a_sName.substr(0, l_pos);
			m_sValueName = a_sName.substr(l_pos + 1);
		}

		HKEY m_hkey;
		HKEY m_hkeyBase;

		tstring m_sValueName;
		tstring m_sKeyName;
};


template<class T>
class registry_string : public registry_value
{
	public:
		registry_string(const tstring& a_sName, HKEY a_hkeyBase) : registry_value(a_sName, a_hkeyBase)
		{
		}

		operator T()
		{
			tstringstream l_ss;
			T returnval = T();

			if(open(false))
			{
				DWORD type, size;
				if(query_value(&type, &size, 0))
				{
					assert(type == REG_SZ);
					char* data = new char[size];

					if(query_value(0, &size, data))
					{
						l_ss.str(data);
						l_ss >> returnval;
					}

					delete[] data;
				}

				close();
			}

			return returnval;
		}

		const registry_string & operator=(const T & value)
		{
			tstringstream l_ss;

			if(open(true))
			{
				l_ss << std::setprecision(20) << value;
				tstring data = l_ss.str();

				set_value(REG_SZ, data.length() + 1, data.c_str());
				close();
			}

            return *this;
		}
};


template<>
class registry_string<tstring> : public registry_value
{
	public:
		registry_string(const tstring& name, HKEY base) : registry_value(name, base)
		{
		}

		tstring get_value(const tstring& a_sDef)
		{
			tstring l_sRet(a_sDef);

			if(open(false))
			{
				DWORD l_dwType;
				DWORD l_dwSize;
				if(query_value(&l_dwType, &l_dwSize, 0))
				{
					assert(l_dwType == REG_SZ);
					TCHAR* l_pData = new TCHAR[l_dwSize];

					if (query_value(0, &l_dwSize, l_pData))
						l_sRet = l_pData;

					delete[] l_pData;
				}

				close();
			}

			return l_sRet;
		}

		operator tstring()
		{
			return this->get_value(_T(""));
		}

		const registry_string & operator=(const tstring & value)
		{
			if(open(true) == true)
			{
				set_value(REG_SZ, ((DWORD)value.length() + 1) * sizeof(TCHAR), value.c_str());
				close();
			}

            return *this;
		}
};


template<class T>
class registry_int : public registry_value
{
	public:
		registry_int(const tstring & name, HKEY base) : registry_value(name, base)
		{
		}

		T get_value(const T& a_def)
		{
			DWORD l_ret = DWORD(a_def);

			if(open(false))
			{
				DWORD l_dwType;
				DWORD l_dwSize = sizeof(DWORD);
				if(query_value(&l_dwType, &l_dwSize, &l_ret) == true)
				{
					assert(l_dwType == REG_DWORD);
				}

				close();
			}

#pragma warning(disable: 4800)	// disable bool conversion warning
			return T(l_ret);
#pragma warning(default: 4800)	// reenable bool conversion warning
		}

		operator T()
		{
			return this->get_value(T());
		}

		const registry_int & operator=(const T & value)
		{
			if(open(true))
			{
				DWORD data(value);
				set_value(REG_DWORD, sizeof(DWORD), &data);
				close();
			}

            return *this;
		}
};


template<class T>
class registry_binary : public registry_value
{
	public:
		registry_binary(const tstring& a_sName, HKEY a_hkeyBase) : registry_value(a_sName, a_hkeyBase)
		{
		}

		T get_value(const T& a_def)
		{
			T l_ret(a_def);

			if(open(false))
			{
				DWORD l_dwType;
				DWORD l_dwSize = sizeof(T);
				if(query_value(&l_dwType, &l_dwSize, &l_ret))
				{
					assert(l_dwType == REG_BINARY);
				}

				close();
			}

			return l_ret;

		}

		operator T()
		{
			return this->get_value(T());
		}

		const registry_binary & operator=(const T & value)
		{
			if(open(true))
			{
				set_value(REG_BINARY, sizeof(T), &value);
				close();
			}

            return *this;
		}
};


template<class T>
class registry_var_binary : public registry_value
{
	public:
		registry_var_binary(const tstring& a_sName, HKEY a_hkeyBase) : registry_value(a_sName, a_hkeyBase)
		{
		}

		T* get_value(T* a_pValue, size_t a_iElems)
		{
			if(open(false))
			{
				DWORD l_dwType;
				DWORD l_dwSize = static_cast<DWORD>(sizeof(T) * a_iElems);
				if(query_value(&l_dwType, &l_dwSize, a_pValue))
				{
					assert(l_dwType == REG_BINARY);
				}

				close();
			}

			return a_pValue;

		}

		operator T*()
		{
			return this->get_value(T());
		}

		const registry_var_binary & set_value(const T* a_pValue, size_t a_iElems)
		{
			if(open(true))
			{
				registry_value::set_value(REG_BINARY, static_cast<DWORD>(sizeof(T) * a_iElems), a_pValue);
				close();
			}

            return *this;
		}
};


// 3 classes below might be implemented in base class, but I chose this approach to avoid 
// additional memory usage
// disable warning "assignment operator could not be generated"
// it is by design - I don't want assignment operator
#pragma warning (disable:4512)  

template<class T>
class registry_string_def : public registry_string<T>
{
public:
	registry_string_def(const tstring& name, HKEY base, const T& a_def) : registry_string<T>(name, base), m_def(a_def)
	{
	}

	operator T()
	{
		return this->get_value(m_def);
	}

	const registry_string_def& operator=(const T & value)
	{
		registry_string<T>::operator=(value);
        return *this;
	}

private:
	const T& m_def;
};


template<class T>
class registry_binary_def : public registry_binary<T>
{
public:
	registry_binary_def(const tstring& name, HKEY base, const T& a_def) : registry_binary<T>(name, base), m_def(a_def)
	{
	}

	operator T()
	{
		return this->get_value(m_def);
	}

	const registry_binary_def& operator=(const T & value)
	{
		registry_binary<T>::operator=(value);
        return *this;
	}
private:
	const T& m_def;
};


template<class T>
class registry_int_def : public registry_int<T>
{
public:
	registry_int_def(const tstring& name, HKEY base, const T& a_def) : registry_int<T>(name, base), m_def(a_def)
	{
	}

	operator T()
	{
		return this->get_value(m_def);
	}

	const registry_int_def& operator=(const T & value)
	{
		registry_int<T>::operator=(value);
        return *this;
	}

private:
	const T& m_def;
};
#pragma warning (default:4512)