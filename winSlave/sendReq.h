#pragma once
#include "stdafx.h"

//��ָ��url������, ���������Ľ��
//std::string sendR(const TCHAR * lpHostName, const TCHAR * lpUrl, std::string postData)
std::string sendR(std::string strServerName, std::string strFormActionUrl, std::string strPostStr)
{

	CString strFormData(strPostStr.c_str());    // ��Ҫ�ύ������

	CInternetSession sess(_T("HttpClient"));


	//���ᳬʱûЧ����, ���ͳ�ʱ����ճ�ʱ����, ���ᳬʱ�����ö��߳�WaitForSingleObject��ʵ��
	sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 5000); // 5000��������ӳ�ʱ
	sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 5000); // 5000����ķ��ͳ�ʱ
	sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 5000); // 5000����Ľ��ճ�ʱ
	sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 5000); // 5000����ķ��ͳ�ʱ
	sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 5000); // 5000����Ľ��ճ�ʱ
	sess.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1); // 1������

	CString strHeaders = _T("Content-Type: application/x-www-form-urlencoded\r\nCharset: utf8"); // ����ͷ
	//CString strHeaders = _T("Content-Type: multipart/form-data"); // ����ͷ


	CHttpConnection *pConnection = sess.GetHttpConnection(strServerName.c_str());

	CHttpFile *pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, strFormActionUrl.c_str());


	CString strSentence, strGetSentence = "";
	if (pFile)
	{
		try {
			BOOL result = pFile->SendRequest(strHeaders, (LPVOID)(LPCTSTR)strFormData, strFormData.GetLength());
			while (pFile->ReadString(strSentence))  // ��ȡ�ύ���ݺ�ķ��ؽ��
				strGetSentence = strGetSentence + strSentence + char(13) + char(10);
			pFile->Close();
			delete pFile;
			sess.Close();
			return std::string(strGetSentence.GetBuffer());
		}
		catch (CInternetException * m_pException) {
			return "";
		}
	}
	return "";

}

std::string localIP()
{

	WSADATA wsaData = { 0 };
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)

#ifdef _DEBUG
		return "WSAcannotStartup";
#elif NDEBUG
		return "CANNOTGETLOCALIP";
#endif 
		
	char szHostName[MAX_PATH] = { 0 };
	int nRetCode;
	nRetCode = gethostname(szHostName, sizeof(szHostName));
	PHOSTENT hostinfo;
	if (nRetCode != 0)

#ifdef _DEBUG
		return WSAGetLastError() + "CANNOTGETLOCALIP";
#elif NDEBUG
		return "CANNOTGETLOCALIP";
#endif 
		
	hostinfo = gethostbyname(szHostName);
	std::string local_ip = inet_ntoa(*(struct in_addr*)*hostinfo->h_addr_list);
	WSACleanup();
	return local_ip;

}