#include "utility.h"
#include <io.h>
#include <direct.h>
#include <iomanip>
#include <regex>


using namespace std;

PortSipUtility::PortSipUtility()
{
	
}


PortSipUtility::~PortSipUtility()
{

}
int PortSipUtility::compareStringNoCase(const string & s1, const string & s2)
{
	
	string::const_iterator iter1 = s1.begin();
	string::const_iterator iter2 = s2.begin();


	while ((iter1!=s1.end()) && (iter2!=s2.end()))
	{
		if (toupper(*iter1) != toupper(*iter2))
		{
			return (toupper(*iter1)<toupper(*iter2)) ? -1 : 1;
		}
		++iter1;
		++iter2;
	}

	return (s2.size() == s1.size()) ? 0 : (s1.size() < s2.size()) ? -1 : 1;	
}

bool PortSipUtility::removeFile(const std::string strFilePath)
{
	if (strFilePath.empty()==true)
	{
		return false;
	}
	if (remove(strFilePath.c_str()) == 0)
	{
		return true;
	}
	return false;
}

char* PortSipUtility::wchar2Char(const wchar_t *wstr)
{
	if (!wstr) return 0;

	int dCharacters = WideCharToMultiByte(CP_ACP, 0, wstr, wcslen(wstr), 0, 0, 0, 0);
	char *str = new char[dCharacters + 1];
	WideCharToMultiByte(CP_ACP,0, wstr, wcslen(wstr), str, dCharacters, 0, 0);
	str[dCharacters] = '\0';
	return str;
};
void PortSipUtility::freeChar(char* str)
{
	if (str)
	{
		delete[]str;
	}
}
wchar_t* PortSipUtility::char2Wchar(const char *str)
{
	if (!str) return 0;
	int dCharacters = MultiByteToWideChar( CP_ACP, 0, str,strlen(str), NULL,0);
	wchar_t *wszStr = new wchar_t[dCharacters+1];          
	MultiByteToWideChar( CP_ACP, 0, str,strlen(str), wszStr,dCharacters);
	wszStr[dCharacters] = '\0';
	return wszStr;
};
void PortSipUtility::freeWchar(wchar_t* str)
{
	if (str)
	{
		delete[]str;
	}
}
std::wstring PortSipUtility::string2WString(const std::string& s)
{
	wchar_t* buf = char2Wchar(s.c_str());
	std::wstring r(buf);
	freeWchar(buf);
	return r;
}
std::string PortSipUtility::wstring2String(const std::wstring &s)
{
	char* buf = wchar2Char(s.c_str());
	std::string r(buf);
	freeChar(buf);
	return r;
}



std::wstring PortSipUtility::Utf82Unicode_ND(const std::string& utf8string)
{
	int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, NULL, 0);
	if (widesize == ERROR_NO_UNICODE_TRANSLATION)
	{
		throw std::exception("Invalid UTF-8 sequence.");
	}
	if (widesize == 0)
	{
		throw std::exception("Error in conversion.");
	}
	std::vector<wchar_t> resultstring(widesize);
	int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, &resultstring[0], widesize);
	if (convresult != widesize)
	{
		throw std::exception("Error in conversion.");
	}
	return std::wstring(&resultstring[0]);
}
string PortSipUtility::WideByte2Acsi_ND(wstring& wstrcode)
{
	int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, NULL, 0, NULL, NULL);
	if (asciisize == ERROR_NO_UNICODE_TRANSLATION)
	{
		throw std::exception("Invalid UTF-8 sequence.");
	}
	if (asciisize == 0)
	{
		throw std::exception("Error in conversion.");
	}
	std::vector<char> resultstring(asciisize);
	int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, &resultstring[0], asciisize, NULL, NULL);
	if (convresult != asciisize)
	{
		throw std::exception("Error in conversion.");
	}
	return std::string(&resultstring[0]);
}
string PortSipUtility::UTF_82ASCII_ND(string& strUtf8Code)
{
	string strRet("");
	wstring wstr = Utf82Unicode_ND(strUtf8Code);
	strRet = WideByte2Acsi_ND(wstr);
	return strRet;
}
/////////////////////////////////////////////////////////////////////// 
wstring PortSipUtility::Acsi2WideByte_ND(string& strascii)
{
	int widesize = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, NULL, 0);
	if (widesize == ERROR_NO_UNICODE_TRANSLATION)
	{
		throw std::exception("Invalid UTF-8 sequence.");
	}
	if (widesize == 0)
	{
		throw std::exception("Error in conversion.");
	}
	std::vector<wchar_t> resultstring(widesize);
	int convresult = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, &resultstring[0], widesize);
	if (convresult != widesize)
	{
		throw std::exception("Error in conversion.");
	}
	return std::wstring(&resultstring[0]);
}
std::string PortSipUtility::Unicode2Utf8_ND(const std::wstring& widestring)
{
	int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, NULL, 0, NULL, NULL);
	if (utf8size == 0)
	{
		throw std::exception("Error in conversion.");
	}
	std::vector<char> resultstring(utf8size);
	int convresult = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, &resultstring[0], utf8size, NULL, NULL);
	if (convresult != utf8size)
	{
		throw std::exception("Error in conversion.");
	}
	return std::string(&resultstring[0]);
}
string PortSipUtility::ASCII2UTF_8_ND(string& strAsciiCode)
{
	string strRet("");
	wstring wstr = Acsi2WideByte_ND(strAsciiCode);
	strRet = Unicode2Utf8_ND(wstr);
	return strRet;
}



void PortSipUtility::CheckFilePath(std::string  strFilePath)
{
	if (_access(strFilePath.c_str(), 0) == -1)
	{
		int flag = _mkdir(strFilePath.c_str());
		if (flag != 0)
		{
			std::string strMsgError = "there is something error where create dir:";
			strMsgError += strFilePath;
		}
	}
}
time_t PortSipUtility::StringToDatetime(const char *str)
{
	tm tm_;
	int year, month, day, hour, minute, second;
	sscanf(str, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
	tm_.tm_year = year - 1900;
	tm_.tm_mon = month - 1;
	tm_.tm_mday = day;
	tm_.tm_hour = hour;
	tm_.tm_min = minute;
	tm_.tm_sec = second;
	tm_.tm_isdst = 0;
	time_t t_ = mktime(&tm_); 
	return t_;  
}



 bool  PortSipUtility::IsContainsStr(string strSrc, string strContain)
{
	string::size_type idx = strSrc.find(strContain);
	if (idx != string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
};
