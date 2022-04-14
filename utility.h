


#ifndef PORTSIP_UTILITY_hxx
#define PORTSIP_UTILITY_hxx
#include <locale>
#include <algorithm>
#include <string>
#include <cstdint>
#include <vector>
#include <TCHAR.H>
#include <Windows.h>


using namespace std;
template<typename charT>
struct charEqual {
	charEqual(const std::locale& loc) : loc_(loc) {}
	bool operator()(charT ch1, charT ch2) {
		return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
	}
private:
	const std::locale& loc_;
};
template<typename T>
int ci_find_substr(const T& str1, const T& str2, const std::locale& loc = std::locale())
{
	typename T::const_iterator it = std::search(str1.begin(), str1.end(),
		str2.begin(), str2.end(), charEqual<typename T::value_type>(loc));
	if (it != str1.end()) return it - str1.begin();
	else return -1; // not found
}
#define TIMER_START(_X) auto _X##_start =GetTickCount(), _X##_stop = _X##_start
#define TIMER_STOP(_X) _X##_stop = GetTickCount()
#define TIMER_MSEC(_X) (_X##_stop - _X##_start)
class PortSipUtility
{
public:
	PortSipUtility();
	virtual ~PortSipUtility();

public:

	static wchar_t*				 char2Wchar(const char *str);
	static void					 freeWchar(wchar_t* wstr);
	static char*				 wchar2Char(const wchar_t *wstr);
	static	void				 freeChar(char* str);
	static std::wstring			 string2WString(const std::string& s);
	static std::string			 wstring2String(const std::wstring &s);
	static int					 compareStringNoCase(const std::string & s1, const std::string & s2);
	static bool					 IsContainsStr(string strSrc, string strContain);
	static std::wstring Utf82Unicode_ND(const std::string& utf8string);
	static string WideByte2Acsi_ND(wstring& wstrcode);
	static string UTF_82ASCII_ND(string& strUtf8Code);
	static wstring Acsi2WideByte_ND(string& strascii);
	static std::string Unicode2Utf8_ND(const std::wstring& widestring);
	static string ASCII2UTF_8_ND(string& strAsciiCode);
	static time_t StringToDatetime(const char *str);
	int GetLocalTimeZone();
	static void CheckFilePath(std::string  strFilePath);

	static bool removeFile(const std::string strFilePath);
	static bool initializeWinsock();
};
//





#endif

