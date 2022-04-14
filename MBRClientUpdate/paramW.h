#pragma once

#include <string>
#include <iostream>
using namespace std;

class CParamW
{
public:
	CParamW()
	{

	}

	inline static void AddParam(string strTag, string strValue, string &strParam)
	{
		if(strTag=="")
		{
			return;
		}

		char strItem[200]={0};
		sprintf_s(strItem,200,"%s=%s,",strTag.c_str(),strValue.c_str());
		strParam += strItem;
	}

	inline static string GetParam(string strParam, string strTag)
	{
		if(strTag=="")
		{
			return "";
		}

		int iFind = strParam.find(strTag);
		if(iFind == -1)
		{
			return "";
		}
		int i1 = strParam.find("=",iFind)+1;
		int i2 = strParam.find(",",iFind);
		if(i1 == -1 || i2 == -1 || i1 >= i2)
		{
			return "";
		}
		string strValue = strParam.substr(i1,i2-i1);
		return strValue;

		return "";
	}

};