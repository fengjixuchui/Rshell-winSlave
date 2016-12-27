#pragma once
#include "stdafx.h"

//给指定url发请求, 返回请求后的结果
//std::string sendR(const TCHAR * lpHostName, const TCHAR * lpUrl, std::string postData)
std::string sendR(std::string strServerName, std::string strFormActionUrl, std::string strPostStr)
{

	CString strFormData(strPostStr.c_str());    // 需要提交的数据

	CInternetSession sess(_T("HttpClient"));


	//连结超时没效果的, 发送超时与接收超时有用, 连结超时可以用多线程WaitForSingleObject来实现
	sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 5000); // 5000毫秒的连接超时
	sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 5000); // 5000毫秒的发送超时
	sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 5000); // 5000毫秒的接收超时
	sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 5000); // 5000毫秒的发送超时
	sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 5000); // 5000毫秒的接收超时
	sess.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1); // 1次重试

	CString strHeaders = _T("Content-Type: application/x-www-form-urlencoded\r\nCharset: utf8"); // 请求头
	//CString strHeaders = _T("Content-Type: multipart/form-data"); // 请求头


	CHttpConnection *pConnection = sess.GetHttpConnection(strServerName.c_str());

	CHttpFile *pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, strFormActionUrl.c_str());


	CString strSentence, strGetSentence = "";
	if (pFile)
	{
		try {
			BOOL result = pFile->SendRequest(strHeaders, (LPVOID)(LPCTSTR)strFormData, strFormData.GetLength());
			while (pFile->ReadString(strSentence))  // 读取提交数据后的返回结果
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