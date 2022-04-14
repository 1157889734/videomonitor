#ifndef _GB2312TOUTF8_H
#define _GB2312TOUTF8_H


	class CHelpUint
	{
		CHelpUint::CHelpUint()
		{

		};
	public:
		static void GB2312ToUTF_8(int *pOutLen, char *pOut, char *pText, int pLen, int OutBuflen);
		static void UTF_8ToGB2312(int *pOutLen, char *pOut, char *pText, int pLen, int OutBuflen);
	};


#endif