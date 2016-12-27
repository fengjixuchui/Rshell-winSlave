// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>


//其他头文件

#include <conio.h>
#include <string>
#include <cstdio>
#include <vector>
#include <afxwin.h>
#include "Wininet.h"
#include <fstream>
#include<sstream>
#include<string> 
#include <UrlMon.h>
#include <afx.h>
#include <afxinet.h>
#include "stdlib.h"
#include "winsock2.h"
//引用库文件

#pragma comment(lib,"nafxcwd.lib")
//静态库编译下不包含User32.lib会报LNK1120错误
#pragma comment(lib, "User32.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Wininet.lib")
#pragma comment(lib, "urlmon.lib")




