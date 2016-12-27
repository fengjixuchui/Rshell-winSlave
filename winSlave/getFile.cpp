#include "stdafx.h"
#include "getFile.h"

#define RECVPACK_SIZE 2048
#define CONN_TIME_OUT 2000

bool getF(std::string command, std::string l) {

	std::string dwnLoc = command;
	string_replace(dwnLoc, l, "");

	//��ȡ�ļ���
	std::string fileName;

	int posi = dwnLoc.find_last_of("/");
	fileName = dwnLoc.substr(posi + 1, dwnLoc.length());


	//��string�����ݸ��Ƶ�char*ָ���ٴ��ݣ�����Ϊstring��������const char*
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


//�滻�ַ�
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


//�����ļ�������Ϊ���ļ���
bool downloadFiles(char* url, char *strSaveFile) {

	CString _url(url);
	CString _strSaveFile(strSaveFile);

	bool ret = false;
	CInternetSession Sess("lpload");
	Sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, CONN_TIME_OUT); //���ӳ�ʱ
	Sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT, CONN_TIME_OUT); //���ͳ�ʱ
	Sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, CONN_TIME_OUT); //���ճ�ʱ
	Sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, CONN_TIME_OUT); //���ͳ�ʱ
	Sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, CONN_TIME_OUT); //���ճ�ʱ
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
				//��ѯ�ļ�����
				DWORD nLen = 0;
				cFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, nLen);
				//CString strFilename = GetFileName(url,TRUE);
				nBufLen = nLen;
				if (nLen <= 0) break;

				//����������ݻ���
				pBuf = (char*)malloc(nLen + 8);
				ZeroMemory(pBuf, nLen + 8);

				char *p = pBuf;
				while (nLen>0) {
					//ÿ������8K
					int n = cFile->Read(p, (nLen<RECVPACK_SIZE) ? nLen : RECVPACK_SIZE);
					//��������˳�ѭ��
					if (n <= 0) break;

					//���ջ������
					p += n;
					//ʣ�೤�ȵݼ�
					nLen -= n;
				}

				//���δ�������ж��˳�
				if (nLen != 0) break;

				//���ճɹ����浽�ļ�

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

	//�ͷŻ���
	if (pBuf) {
		free(pBuf);
		pBuf = NULL;
		nBufLen = 0;
	}

	//�ر���������
	if (cFile) {
		cFile->Close();
		Sess.Close();
		delete cFile;
	}
	return ret;
}