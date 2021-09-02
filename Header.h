#pragma once
#include "stdafx.h"
#include "Proxy.h"
#include "afxsock.h" //Thư viện của socket
#include <iostream>
#include <string>
#include <vector>
#include <sstream> //Thu viện cho chuỗi
#include <fstream>

CWinApp theApp;

using namespace std;
//SOCKET gồm: int af,
//int type,
//int protocol
//SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
//af: Là một con số ID để quyết định Socket của chúng ta sử dụng giao thức(protocol) để kết nối.
//- AF_INET : TCP / IP(Phổ biến nhất hiện nay->dùng địa chỉ IP để truyền dữ liệu)
//- AF_NETBIOS : NetBIOS(Giao thức dùng tên máy để truyền dữ liệu)
//- AF_APPLETALK : AppleTalk
//- AF_ATM : ATM
//SOCK_STREAM: Đây là giao thức TCP.Nó ngược với UDP vì nó đảm bảo giữa bên gửi và bên nhận dữ liệu phải chính xác.Vì vậy 2 bên sẽ phải bắt tay rất nhiều lần khi truyền được dữ liệu(ví dụ như bên gửi sẽ gửi n gói tin(packet), bên nhận sẽ kiểm tra có bị mất hay sai gói tin nào hay không, nếu đủ thì nó sẽ yêu cầu bên gửi gửi tiếp n gói tin tiếp theo, ngược lại thì nó sẽ yêu cầu gửi lại)
//= > Ưu điểm : Chất lượng gởi tin cậy.
//= > Nhược điểm : Chậm hơn UDP.
//Những ứng dụng như WEB, MAIL, FTP, …
//Nếu là IPV4 thì IPPROTO_IP là 0





struct SocketServerClient {
	SOCKET Server;
	SOCKET Client;
	bool stopServerSocket;
	bool stopClientSocket;
};
struct ThamSo {
	string diachi;
	HANDLE handle; // HANDLE là con trỏ 32 bit để tham chiếu và quản lí các đối tượng 
	SocketServerClient* SocketSC;
	int port;
};
void khoiTaoServer();
UINT ketNoiClientDenProxy(void* ThamSoIP);
UINT ketNoiProxyDenServer(void* ThamSoIP);
void layDiaChiPort(string& buf, string& address, int& port);
void tachChuoi(string str, vector<string>& chuoi, char daucach = ' ');
wchar_t* chuyenCharArraySangDangLPCWSTR(const char* charArray);
void LoadBlackList(vector<string>& arr);
bool kiemTraTenServer(string tenServer);
sockaddr_in* layDiaChiIP(string tenServer, char* tenhost);
void dongServer();
UINT nhapKiTuDeDungServer(void* ThamSoIP);
vector<string> black_list;
bool temp = 1;
string Forbidden = "HTTP/1.0 403 Forbidden\r\n\Cache-Control: no-cache\r\n\You are not allowed to connect to this page\r\n";
SOCKET sock_langnghe;