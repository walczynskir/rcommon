#pragma once
#include <windows.h>
#include <tchar.h>
#include <rcommon/rstddef.h>
#include <rcommon/rstring.h>
#include <vector>
#include <optional>


struct LanguageOption {
    tstring m_sLangName;  //    eg. "English (UK)" / "Polski"
    LANGID m_idLang;      //    resource LANGID
};


class LanguageManager {
public:
    LanguageManager() : m_idxCurrentLang(0) {}

    
    // Auto-detect available languages from STRINGTABLE resources
    void DetectLanguages(int a_idLangName) 
    {
        m_vectLang.clear();
		m_idLangName = a_idLangName;


        // A stringtable block = ID / 16 + 1
        int l_idBlock = (m_idLangName / 16) + 1;

        // Enumerate languages for just that one block
        EnumResourceLanguages(NULL, RT_STRING, MAKEINTRESOURCE(l_idBlock), EnumResLangProc, reinterpret_cast<LONG_PTR>(this));
    }

    
    // allows manually add language - however it is better to use Detect Language procedure
    void AddLanguage(int a_idString, LANGID a_idLang)
    {
		TCHAR l_sLangName[64];
        if (::LoadString(::GetModuleHandle(NULL), a_idString, l_sLangName, ArraySize(l_sLangName)) > 0)
            m_vectLang.push_back({ l_sLangName, a_idLang});
    }

    // sets current language by id (usually app needs restart after setting language, as all resources are already loaded)
    void SetLanguage(int a_idCurrentLang)
    {
        if (a_idCurrentLang < 0 || static_cast<size_t>(a_idCurrentLang) >= m_vectLang.size())
            return;

        m_idxCurrentLang = a_idCurrentLang;
        SetThreadUILanguage(m_vectLang[a_idCurrentLang].m_idLang);
    }


    // sets current language by name (usually app needs restart after setting language, as all resources are already loaded)
    bool SetLanguage(LANGID a_idLang)
    {
        auto it = std::find_if(m_vectLang.begin(), m_vectLang.end(), [a_idLang](const LanguageOption& option) { return option.m_idLang == a_idLang; });

        if (it != m_vectLang.end()) {
            SetLanguage(static_cast<int>(std::distance(m_vectLang.begin(), it)));
            return true;
        }

        return false; // nie znaleziono
    }

    // sets current language by name (usually app needs restart after setting language, as all resources are already loaded)
    bool SetLanguage(const tstring& langName)
    {
        auto it = std::find_if(m_vectLang.begin(), m_vectLang.end(),
            [&langName](const LanguageOption& option) {
                return option.m_sLangName == langName;
            });

        if (it != m_vectLang.end()) {
            SetLanguage(static_cast<int>(std::distance(m_vectLang.begin(), it)));
            return true;
        }

        return false; // nie znaleziono
    }


    // returns current language ID
    std::optional<LANGID> GetCurrentLangID() const
    {
        if (m_idxCurrentLang >= 0 && m_idxCurrentLang < (int)m_vectLang.size())
            return m_vectLang[m_idxCurrentLang].m_idLang;
        return std::nullopt;
    }

    std::optional<tstring> GetCurrentLangName() const
    {
        if (m_idxCurrentLang >= 0 &&
            static_cast<size_t>(m_idxCurrentLang) < m_vectLang.size())
        {
            return m_vectLang[m_idxCurrentLang].m_sLangName;
        }
        return std::nullopt;
    }


    std::optional<LANGID> GetLangId(const tstring& a_sLangName)
    {
        auto it = std::find_if(m_vectLang.begin(), m_vectLang.end(),
            [&a_sLangName](const LanguageOption& option) {
                return option.m_sLangName == a_sLangName;
            });

        if (it != m_vectLang.end()) {
            return it->m_idLang;
        }

        return std::nullopt; // nie znaleziono
    }


    std::optional<tstring> GetLangName(LANGID a_idLang)
    {
        auto it = std::find_if(m_vectLang.begin(), m_vectLang.end(),
            [&a_idLang](const LanguageOption& option) {
                return option.m_idLang == a_idLang;
            });

        if (it != m_vectLang.end()) {
            return it->m_sLangName;
        }

        return std::nullopt; // nie znaleziono
    }

    //  Iterator support
    auto begin() { return m_vectLang.begin(); }
    auto end() { return m_vectLang.end(); }
    auto begin() const { return m_vectLang.begin(); }
    auto end()   const { return m_vectLang.end(); }

private:

	// enumerated through all languages 
    static BOOL CALLBACK EnumResLangProc(HMODULE a_hModule, LPCTSTR a_sType, LPCTSTR a_sName, LANGID a_idLang, LONG_PTR a_lParam)
    {
        LanguageManager* l_langMan = reinterpret_cast<LanguageManager*>(a_lParam);

        tstring l_slangName = l_langMan->LoadStringFromResources(a_idLang, l_langMan->m_idLangName);
        if (!l_slangName.empty()) {
            l_langMan->m_vectLang.push_back({ l_slangName, a_idLang });
        }

        return TRUE; // keep going
    }

	// loads string from resources for given language
    tstring LoadStringFromResources(LANGID a_idLang, int a_idLangName) 
    {
        HRSRC l_hRes = ::FindResourceExW(NULL, RT_STRING, MAKEINTRESOURCE((a_idLangName / 16) + 1), a_idLang);
        if (!l_hRes) 
            return _T("");

        HGLOBAL l_hGlob = ::LoadResource(NULL, l_hRes);
        if (!l_hGlob) 
            return _T("");

        LPCTSTR l_pResData = (LPCTSTR)::LockResource(l_hGlob);
        if (!l_pResData)
            return _T("");

        int l_idBlock = a_idLangName % 16;
        for (int l_iAt = 0; l_iAt <= l_idBlock; ++l_iAt) {
            WORD len = *l_pResData++;
            if (l_iAt == l_idBlock) {
                return tstring(l_pResData, len);
            }
            l_pResData += len;
        }

        return _T("");
    }


private:
    std::vector<LanguageOption> m_vectLang;
    int m_idxCurrentLang;
	int m_idLangName; // string ID for language name
};
