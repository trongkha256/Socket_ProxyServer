#include "stdafx.h"
#include "Header.h"

void khoiTaoServer()
{
	sockaddr_in address;
	SOCKET socketlangnghe;
	WSADATA Data;
	if (WSAStartup(0x202, &Data) != 0)
	{
		cout << endl;
		cout << "Loi khoi tao socket" << endl;
		WSACleanup();
		return;
	}
	address.sin_family = AF_INET; //IPV4
	address.sin_addr.s_addr = INADDR_ANY; //Thì IP sẽ chính là IP của mình. Các Server thường dùng để chọn địa chỉ IP của mình và lắng nghe kết nối..
	address.sin_port = htons(8888);
	socketlangnghe = socket(AF_INET, SOCK_STREAM, 0);
	if (socketlangnghe == INVALID_SOCKET)
	{
		cout << endl;
		cout << "Socket khoi tao khong thanh cong." << endl;
		WSACleanup();
		return;
	}
	if (bind(socketlangnghe, (sockaddr*)&address, sizeof(address)) != 0)
	{
		cout << endl;
		cout << "Bind socket khong thanh cong." << endl;
		WSACleanup();
		return;
	};
	if (listen(socketlangnghe, 5) != 0)
	{
		cout << endl;
		cout << "Khong the lang nghe." << endl;
		WSACleanup();
		return;
	}
	LoadBlackList(black_list);
	if (black_list.size() == 0) {
		cout << endl;
		cout << "File blacklist khong ton tai." << endl;
	}
	sock_langnghe = socketlangnghe;
	AfxBeginThread(ketNoiClientDenProxy, (LPVOID)socketlangnghe);
	CWinThread* p = AfxBeginThread(nhapKiTuDeDungServer, &temp);
}

UINT ketNoiClientDenProxy(void* ThamSoIP)
{
	SOCKET socket = (SOCKET)ThamSoIP;
	SocketServerClient SC;
	SOCKET Client;
	sockaddr_in diachi;
	int dodaidiachi = sizeof(diachi);
	Client = accept(socket, (sockaddr*)&diachi, &dodaidiachi);
	AfxBeginThread(ketNoiClientDenProxy, ThamSoIP); //Cách tốt nhất để khởi chạy 1 thread
	char Buffer[20000]; //biến temp lưu trữ 
	int dodai;
	SC.stopServerSocket = 0;
	SC.stopClientSocket = 0;
	SC.Client = Client;
	int num = recv(SC.Client, Buffer, 20000, 0); //Nhận tên trang trả về từ client trả về nếu bằng 0 thì thành công.
	if (num == -1) {
		cout << endl;
		cout << "Nhan yeu cau khong thanh cong." << endl;
		if (!SC.stopClientSocket) {
			closesocket(SC.Client);
			SC.stopClientSocket = 1;
		}
	}
	if (num == 0) {
		cout << endl;
		cout << "Client da dung ket noi" << endl;
		if (!SC.stopClientSocket) {
			closesocket(SC.Client);
			SC.stopClientSocket = 1; //=1 thì dừng lại =0 thì chạy tiếp
		}
	}
	if (num >= 20000) Buffer[num - 1] = 0;
	else if (num > 0) Buffer[num] = 0;
	else Buffer[0] = 0;
	cout << endl;
	cout << "Client da nhan " << num << "  goi du lieu :" << endl << "[" << Buffer << "]";
	string buf(Buffer), address;
	int port;
	layDiaChiPort(buf, address, port); //Lay ten mien như http//truyentranhtuan.com
	bool check = 0;
	if (!kiemTraTenServer(address)) { //Kiemtraten miền đó nằm ở trong Blacklist không. Nếu có thì chạy và hiện ra dòng Forbiden
		num = send(SC.Client, Forbidden.c_str(), Forbidden.size(), 0); //gửi dòng thong báo  FOrbident đến client và trả về số lượng của nó
		check = 1;
		Sleep(2000);
	}
	ThamSo T;
	T.handle = CreateEvent(NULL, TRUE, FALSE, NULL); //Khởi tạo đầu của biến Handle
	T.diachi = address;
	T.port = port;
	T.SocketSC = &SC;
	if (check == 0) { //Nếu không có trong blacklist thì nó mới truy cập dc từ Proxy dên Server
		CWinThread* pThread = AfxBeginThread(ketNoiProxyDenServer, (LPVOID)&T); 
		WaitForSingleObject(T.handle, 6000); //Báo hiệu sắp hết thời gian hoặc nó dừng hay chết.
		CloseHandle(T.handle);
		while (SC.stopClientSocket == 0 && SC.stopServerSocket == 0) {
			num = send(SC.Server, buf.c_str(), buf.size(), 0);
			if (num == -1) {
				cout << "Gui Failed, Loi: " << GetLastError() << endl;
				if (SC.stopServerSocket == 0) {
					closesocket(SC.Server);
					SC.stopServerSocket = 1;
				}
				continue;
			}
			num = recv(SC.Client, Buffer, 20000, 0);
			if (num == -1) {
				cout << "Nhan Failed, Loi: " << GetLastError() << endl;
				if (SC.stopClientSocket == 0) {
					closesocket(SC.Client);
					SC.stopClientSocket = 1;
				}
				continue;
			}
			if (num == 0) {
				cout << endl;
				cout << "Client ket thuc. " << endl;
				if (SC.stopClientSocket == 0) {
					closesocket(SC.Server);
					SC.stopClientSocket = 1;
				}
				break;
			}
			if (num >= 20000) Buffer[num - 1] = 0;
			else if (num > 0) Buffer[num] = 0;
			else Buffer[0] = 0;
			cout << endl;
			cout << "Client da nhan" << num << " du lieu :\n[" << Buffer << "]";
		}
		if (SC.stopServerSocket == 0) {
			closesocket(SC.Server);
			SC.stopServerSocket = 1;
		}
		if (SC.stopClientSocket == 0) {
			closesocket(SC.Client);
			SC.stopClientSocket = 1;
		}
		WaitForSingleObject(pThread->m_hThread, 20000);
	}
	else {
		if (SC.stopClientSocket == 0) {
			closesocket(SC.Client);
			SC.stopClientSocket = 1;
		}
	}
	return 0;
}

UINT ketNoiProxyDenServer(void* ThamSoIP)
{
	int count = 0;
	ThamSo* T = (ThamSo*)ThamSoIP;
	string name = T->diachi;
	int port = T->port;
	int sta;
	int diachi;
	char host[32] = "";
	sockaddr_in* server = NULL;
	cout << "Ten server: " << name << endl;
	server = layDiaChiIP(name, host);
	if (server == NULL) {
		cout << endl;
		cout << "Lay dia chi IP khong thanh cong." << endl;
		send(T->SocketSC->Client, Forbidden.c_str(), Forbidden.size(), 0);
		return -1;
	}
	if (strlen(host) > 0) {
		cout << "Dang ket noi toi:" << host << endl;
		int num;
		char Buf[20000];
		SOCKET Server;
		Server = socket(AF_INET, SOCK_STREAM, 0);
		if (!(connect(Server, (sockaddr*)server, sizeof(sockaddr)) == 0)) {
			cout << "Khong the ket noi";
			send(T->SocketSC->Client, Forbidden.c_str(), Forbidden.size(), 0);

			return -1;
		}
		else {
			cout << "Ket noi thanh cong." << endl;
			T->SocketSC->Server = Server;
			T->SocketSC->stopServerSocket == 0;
			SetEvent(T->handle);
			int c = 0;
			while (T->SocketSC->stopClientSocket == 0 && T->SocketSC->stopServerSocket == 0) {
				num = recv(T->SocketSC->Server, Buf, 20000, 0);
				if (num == -1) {
					closesocket(T->SocketSC->Server);
					T->SocketSC->stopServerSocket = 1;
					break;
				}
				if (num == 0) {
					cout << endl;
					cout << "Server da ket thuc." << endl;
					closesocket(T->SocketSC->Server);
					T->SocketSC->stopServerSocket = 1;
				}
				num = send(T->SocketSC->Client, Buf, num, 0);
				if (num == -1) {
					cout << "Gui Failed, Loi: " << GetLastError() << endl;
					closesocket(T->SocketSC->Client);
					T->SocketSC->stopClientSocket = 1;
					break;
				}
				if (num >= 20000) Buf[num - 1] = 0;
				else if (num > 0) Buf[num] = 0;
				else Buf[0] = 0;
				cout << endl;
				cout << "Server da nhan " << num << " goi du lieu :\n[" << Buf << "]";
				ZeroMemory(Buf, 20000);
			}
			if (T->SocketSC->stopClientSocket == 0) {
				closesocket(T->SocketSC->Client);
				T->SocketSC->stopClientSocket = 1;
			}
			if (T->SocketSC->stopServerSocket == 0) {
				closesocket(T->SocketSC->Server);
				T->SocketSC->stopServerSocket = 1;
			}
		}
	}
	return 0;
}

void layDiaChiPort(string& buf, string& address, int& port)
{
	const char* HTTP = "http://";
	vector<string> chuoi;
	tachChuoi(buf, chuoi);
	if (chuoi.size() > 0) {
		int pos = chuoi[1].find(HTTP);
		if (pos != -1) {
			string add = chuoi[1].substr(pos + strlen(HTTP));
			address = add.substr(0, add.find('/'));
			port = 80;
			string temp;
			int len = strlen(HTTP) + address.length();
			while (len > 0) {
				temp.push_back(' ');
				len--;
			}
			buf = buf.replace(buf.find(HTTP + address), strlen(HTTP) + address.length(), temp);
		}
	}
}

void tachChuoi(string str, vector<string>& chuoi, char daucach)
{
	istringstream ss(str);
	string token;
	while (getline(ss, token, daucach)) {
		chuoi.push_back(token);
	}
}

wchar_t* chuyenCharArraySangDangLPCWSTR(const char* charArray)
{	
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

void LoadBlackList(vector<string>& arr)
{
	fstream file;
	file.open("blacklist.conf", ios::in | ios::out);
	if (file.is_open()) {
		while (!file.eof()) {
			string temp;
			getline(file, temp);
			if (temp.back() == '\n') {
				temp.pop_back();
			}
			arr.push_back(temp);
		}
	}
}

bool kiemTraTenServer(string tenServer) {
	if (black_list.size() > 0) {
		for (auto i : black_list)
		{
			if (i.find(tenServer) != string::npos) {
				cout << i.find(tenServer);
				return 0;
			}
		}
	}
	return 1;
}

sockaddr_in* layDiaChiIP(string tenServer, char* tenhost)
{
	int sta;
	sockaddr_in* server = NULL;
	if (tenServer.size() > 0) {
		if (isalpha(tenServer.at(0))) {
			addrinfo hints, * res = NULL;
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			if ((sta = getaddrinfo(tenServer.c_str(), "80", &hints, &res)) != 0) {
				cout << "Lay thong tin dia chi khong thanh cong: " << gai_strerror(sta);
				return NULL;
			}
			while (res->ai_next != NULL) {
				res = res->ai_next;
			}
			sockaddr_in* temp = (sockaddr_in*)res->ai_addr;
			inet_ntop(res->ai_family, &temp->sin_addr, tenhost, 32);
			server = (sockaddr_in*)res->ai_addr;
			unsigned long addr;
			inet_pton(AF_INET, tenhost, &addr);
			server->sin_addr.s_addr = addr;
			server->sin_port = htons(80);
			server->sin_family = AF_INET;
		}
		else {
			unsigned long ar;
			inet_pton(AF_INET, tenServer.c_str(), &ar);
			sockaddr_in art;
			art.sin_family = AF_INET;
			art.sin_addr.s_addr = ar;
			if ((sta = getnameinfo((sockaddr*)&art,
				sizeof(sockaddr), tenhost, NI_MAXHOST, NULL, NI_MAXSERV, NI_NUMERICSERV)) != 0) {
				cout << "Da co loi xay ra khong thanh cong.";
				return NULL;
			}
			server->sin_addr.s_addr = ar;
			server->sin_family = AF_INET;
			server->sin_port = htons(80);
		}
	}

	return server;
}
void dongServer()
{
	cout << "Socket da duoc dong lai." << endl;
	closesocket(sock_langnghe);
	WSACleanup();
}

UINT nhapKiTuDeDungServer(void* ThamSoIP)
{
	bool* temp = (bool*)ThamSoIP;
	while (*temp) {
		char c;
		c = getchar();
		if (c == '0') {
			*temp = 0;
		}
	}
	return 0;
}

int main()
{
	int n= 0;

	HMODULE h = ::GetModuleHandle(nullptr);

	if (h!= nullptr)
	{
		if (!AfxWinInit(h, nullptr, ::GetCommandLine(), 0))
		{
			wprintf(L"Error: Khoi tao MFC khong thanh cong.");
			n= 1;
		}
		else
		{
			khoiTaoServer();
			while (temp) {
				Sleep(20000);
			}
			dongServer();
		}
	}
	else
	{
		wprintf(L"Error : Lay mo dun khong thanh cong.\n");
		n = 1;
	}

	return 0;
}
