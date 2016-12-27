#include "stdafx.h"
#include "getFile.h"

#define RECVPACK_SIZE 2048
#define CONN_TIME_OUT 2000

bool getF(std::string command, std::string l) {

	std::string dwnLoc = command;
	string_replace(dwnLoc, l, "");

	//提取文件名
	std::string fileName;

	int posi = dwnLoc.find_last_of("/");
	fileName = dwnLoc.substr(posi + 1, dwnLoc.length());


	//将string的内容复制到char*指针再传递，是因为string的类型是const char*
	char *dwnLocC = new char[dwnLoc.size() + 1];
	char *fileNameC = new char[fileName.size() + 1];
	strcpy(dwnLocC, dwnLoc.c_str());
	strcpy(fileNameC, fileName.c_str());

#ifdef _DEBUG
	printf("dwnLocC is:%s\nfileNameC is:%s\n",dwnLocC, fileNameC);
#endif

	bool act = downloadFiles(dwnLocC, fileNameC);


	delete[] dwnLocC;
	delete[] fileNameC;
	return act;


}


//替换字符
void string_replace(std::string & strBig, const std::string & strsrc, const std::string &strdst)
{
	std::string::size_type pos = 0;
	std::string::size_type srclen = strsrc.size();
	std::string::size_type dstlen = strdst.size();
	while ((pos = strBig.find(strsrc, pos)) != std::string::npos)
	{
		strBig.replace(pos, srclen, strdst);
		pos += dstlen;
	}
}


//下载文件并保存为新文件名
bool downloadFiles(char* url, char *strSaveFile) {

	CString _url(url);
	CString _strSaveFile(strSaveFile);

	bool ret = false;
	CInternetSession Sess("lpload");
	Sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, CONN_TIME_OUT); //连接超时
	Sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT, CONN_TIME_OUT); //发送超时
	Sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, CONN_TIME_OUT); //接收超时
	Sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, CONN_TIME_OUT); //发送超时
	Sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, CONN_TIME_OUT); //接收超时
	DWORD dwFlag = INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD;

	CHttpFile* cFile = NULL;
	char      *pBuf = NULL;
	int        nBufLen = 0;
	do {
		try {
			cFile = (CHttpFile*)Sess.OpenURL(_url, 1, dwFlag);
			DWORD dwStatusCode;
			cFile->QueryInfoStatusCode(dwStatusCode);
			if (dwStatusCode == HTTP_STATUS_OK) {
				//查询文件长度
				DWORD nLen = 0;
				cFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, nLen);
				//CString strFilename = GetFileName(url,TRUE);
				nBufLen = nLen;
				if (nLen <= 0) break;

				//分配接收数据缓存
				pBuf = (char*)malloc(nLen + 8);
				ZeroMemory(pBuf, nLen + 8);

				char *p = pBuf;
				while (nLen>0) {
					//每次下载8K
					int n = cFile->Read(p, (nLen<RECVPACK_SIZE) ? nLen : RECVPACK_SIZE);
					//接收完成退出循环
					if (n <= 0) break;

					//接收缓存后移
					p += n;
					//剩余长度递减
					nLen -= n;
				}

				//如果未接收完中断退出
				if (nLen != 0) break;

				//接收成功保存到文件

				CFile file(_strSaveFile, CFile::modeCreate | CFile::modeWrite);
				file.Write(pBuf, nBufLen);
				file.Close();
				ret = true;
			}
		}
		catch (...) {
			break;
		}
	} while (0);

	//释放缓存
	if (pBuf) {
		free(pBuf);
		pBuf = NULL;
		nBufLen = 0;
	}

	//关闭下载连接
	if (cFile) {
		cFile->Close();
		Sess.Close();
		delete cFile;
	}
	return ret;
}