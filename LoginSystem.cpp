#include "LoginSystem.h"
#include "ui_LoginSystem.h"
#include "mainsdk.h"
#include <QMouseEvent>
#include <QWidget>

LoginSystem::LoginSystem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginSystem),
    keyButtonCount(0),
    bCapsOn(0),
    ShiftIndex(0)
{
    ui->setupUi(this);
    QDesktopWidget *desktop=QApplication::desktop();
    //获取系统分辨率
	int width=(desktop)->width();
	int height=(desktop)->height();

	
    resize(width,height);
    //setWindowFlags(Qt::FramelessWindowHint);//无边框
    setWindowFlags(Qt::FramelessWindowHint |Qt::Window);//无边框
   // showFullScreen();
   // setGeometry(0,0,SYSTEM_WIN_WIDTH,SYSTEM_WIN_HEIGHT);


    //QFont font;
    //font.setPointSize(MONITOR_FONT_SIZE);//字体大小
    //setFont(font);//其他控件一样
    QStringList qss;
    qss.append(QString("%1}").arg(QString::fromLocal8Bit(" font-family:'微软雅黑';")));
    setStyleSheet(qss.join(""));

    //signalMapper = new QSignalMapper(this);
    m_buGroup = new QButtonGroup();//按钮组
    connect(m_buGroup, SIGNAL(buttonClicked(int)),this, SLOT(buttonNumslot(int)));

    CreatKeyBoard();
    if(desktop->width()>=1024)
    {
       ResizeControl(width,height);
       SetControlStyleSheet();
    }
    else
    {
       ResizeControl_600P(width,height);
       SetControlStyleSheet_600P();
    }

  //  ui->lineEdit_Password->repaint();
    ui->pushButton_Login->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_Cancel->setFocusPolicy(Qt::NoFocus);
     ui->lineEdit_User->setText("admin");
      //  ui->lineEdit_Password->setFocusPolicy(Qt::ClickFocus);
     ui->lineEdit_Password->setEnabled(true);
     ui->lineEdit_Password->setFocus();
     ui->lineEdit_Password->setEchoMode(QLineEdit::Password);
}

	void GetCurrentProcessPath(char* strname, int len)//获取当前进程路径
	{
		char strdir[1024] = "";
		memset(strdir, 0, sizeof(strdir));
		if (GetModuleFileNameA(NULL, strdir, sizeof(strdir)))
		{
			char *p = strdir;
			char *name = strdir;
			char *path = strdir;
			while (p)
			{
				p = strstr(p, "\\");
				if (p)
				{
					p++;
					name = p;
				}
			}
			int i = name - 1 - path;
			if (i + 1 < len)
			{
				memcpy(strname, path, i);

				if (!strstr(strname, "\\"))//磁盘根目录添加
				{
					memcpy(strname + i, "\\", 1);
				}
			}
		}
	}
	//重写RegGetValue
	extern "C" LSTATUS MyRegGetValueA(HKEY hkey, LPCSTR lpSubKey, LPCSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData)
	{
		typedef LSTATUS(WINAPI *REGGETVALUEA)(HKEY hkey, LPCSTR lpSubKey, LPCSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData);
		REGGETVALUEA LoadLibAddy;
		LoadLibAddy = (REGGETVALUEA)GetProcAddress(GetModuleHandleA("advapi32.dll"), "RegGetValueA");
		DWORD type;


		if (LoadLibAddy)
		{
			return LoadLibAddy(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
		}
		else
		{
			if (dwFlags == RRF_RT_REG_SZ)
			{
				type = REG_SZ;
			}
			else if (dwFlags == RRF_RT_REG_DWORD)
			{
				type = REG_DWORD;
			}
			return RegQueryValueExA(hkey, lpValue, NULL, &type, (LPBYTE)pvData, pcbData);
		}
	}
	//1：设置程序开机自启 2：关闭程序开机自启
	void AutoStart()
	{
		HKEY hKey;
		//string strRegPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
		char lprun[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
		char appname[] = "\\videomonitor";
		//Bug4
	
		{
			//1、找到系统的启动项  
			if (RegOpenKeyExA(HKEY_CURRENT_USER, lprun, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) ///打开启动项       
			{
				{
					//2、得到本程序自身的全路径
					char strExeFullDir[1024];
					memset(strExeFullDir, 0, sizeof(strExeFullDir));
					GetCurrentProcessPath(strExeFullDir, sizeof(strExeFullDir));
					sprintf_s(strExeFullDir, "%s%s", strExeFullDir, appname);
					//MessageBoxA(NULL, strExeFullDir, "", NULL);
					//3、判断注册表项是否已经存在
					char strDir[MAX_PATH] = {};
					DWORD nLength = MAX_PATH;
					long result = MyRegGetValueA(hKey, nullptr, appname, RRF_RT_REG_SZ, 0, strDir, &nLength);
					//4、不存在
					if (result != ERROR_SUCCESS || strcmp(strExeFullDir, strDir) != 0)
					{
						//5、添加一个子Key,并设置值，"GISRestart"是应用程序名字（不加后缀.exe） 
						RegSetValueExA(hKey, appname, 0, REG_SZ, (LPBYTE)strExeFullDir,
							(lstrlenA(strExeFullDir) + 1) * sizeof(TCHAR));
						//6、关闭注册表
						RegCloseKey(hKey);
					}
				}
			}
			else
			{
				MessageBoxA(NULL, "警告\n系统参数错误,不能随系统启动", "", NULL);
			}
		}
		
	}



LoginSystem::~LoginSystem()
{
    delete ui;
}

void LoginSystem::CreatKeyBoard()
{

#if WIN32
	//AutoStart();
#endif
	
    CapKey.clear();
    ComKey.clear();
    //------1
    CapKey.push_back('~');
    CapKey.push_back('!');
    CapKey.push_back('@');
    CapKey.push_back('#');
    CapKey.push_back('$');
    CapKey.push_back('%');
    CapKey.push_back('^');
    CapKey.push_back('&');
    CapKey.push_back('*');
    CapKey.push_back('(');
    CapKey.push_back(')');
    CapKey.push_back('_');
    CapKey.push_back('+');
    CapKey.push_back(BSPACE);

    ComKey.push_back('`');
    ComKey.push_back('1');
    ComKey.push_back('2');
    ComKey.push_back('3');
    ComKey.push_back('4');
    ComKey.push_back('5');
    ComKey.push_back('6');
    ComKey.push_back('7');
    ComKey.push_back('8');
    ComKey.push_back('9');
    ComKey.push_back('0');
    ComKey.push_back('-');
    ComKey.push_back('=');
    ComKey.push_back(0x00);

    //-------2
    CapKey.push_back(TAB);
    CapKey.push_back('Q');
    CapKey.push_back('W');
    CapKey.push_back('E');
    CapKey.push_back('R');
    CapKey.push_back('T');
    CapKey.push_back('Y');
    CapKey.push_back('U');
    CapKey.push_back('I');
    CapKey.push_back('O');
    CapKey.push_back('P');
    CapKey.push_back('{');
    CapKey.push_back('}');
    CapKey.push_back('|');

    ComKey.push_back(0x00);
    ComKey.push_back('q');
    ComKey.push_back('w');
    ComKey.push_back('e');
    ComKey.push_back('r');
    ComKey.push_back('t');
    ComKey.push_back('y');
    ComKey.push_back('u');
    ComKey.push_back('i');
    ComKey.push_back('o');
    ComKey.push_back('p');
    ComKey.push_back('[');
    ComKey.push_back(']');
    ComKey.push_back('\\');


    //--------3
    CapKey.push_back(CAPS);
    CapKey.push_back('A');
    CapKey.push_back('S');
    CapKey.push_back('D');
    CapKey.push_back('F');
    CapKey.push_back('G');
    CapKey.push_back('H');
    CapKey.push_back('J');
    CapKey.push_back('K');
    CapKey.push_back('L');
    CapKey.push_back(':');
    CapKey.push_back('\"');
    CapKey.push_back(ENTER);

    ComKey.push_back(0x00);
    ComKey.push_back('a');
    ComKey.push_back('s');
    ComKey.push_back('d');
    ComKey.push_back('f');
    ComKey.push_back('g');
    ComKey.push_back('h');
    ComKey.push_back('j');
    ComKey.push_back('k');
    ComKey.push_back('l');
    ComKey.push_back(';');
    ComKey.push_back('\'');
    ComKey.push_back(0x00);

    //----------4
    CapKey.push_back(SHIFT);
    CapKey.push_back('Z');
    CapKey.push_back('X');
    CapKey.push_back('C');
    CapKey.push_back('V');
    CapKey.push_back('B');
    CapKey.push_back('N');
    CapKey.push_back('M');
    CapKey.push_back('<');
    CapKey.push_back('>');
    CapKey.push_back('?');
    CapKey.push_back(SHIFT2);

    ComKey.push_back(0x00);
    ComKey.push_back('z');
    ComKey.push_back('x');
    ComKey.push_back('c');
    ComKey.push_back('v');
    ComKey.push_back('b');
    ComKey.push_back('n');
    ComKey.push_back('m');
    ComKey.push_back(',');
    ComKey.push_back('.');
    ComKey.push_back('/');
    ComKey.push_back(0x00);

    QPushButton *button;
    for(int i=0;i<4;i++)
    {
        int linenum =i+1;
        if(linenum ==3)
        {
            for(int i=0;i<13;i++)
            {
                button =new QPushButton(ui->widget_KeyBoard);
                keyBoardButton[keyButtonCount]=button;
                keyBoardButton[keyButtonCount]->setFocusPolicy(Qt::NoFocus);
                m_buGroup->addButton(keyBoardButton[keyButtonCount],keyButtonCount);
                keyButtonCount++;
            }
        }
        else if(linenum ==4)
        {
            for(int i=0;i<12;i++)
            {
                button =new QPushButton(ui->widget_KeyBoard);
                keyBoardButton[keyButtonCount]=button;
                keyBoardButton[keyButtonCount]->setFocusPolicy(Qt::NoFocus);
                m_buGroup->addButton(keyBoardButton[keyButtonCount],keyButtonCount);
                keyButtonCount++;
            }
        }
        else
        {
           for(int i=0;i<14;i++)
           {
               button =new QPushButton(ui->widget_KeyBoard);
               keyBoardButton[keyButtonCount]=button;
               keyBoardButton[keyButtonCount]->setFocusPolicy(Qt::NoFocus);
               m_buGroup->addButton(keyBoardButton[keyButtonCount],keyButtonCount);
               keyButtonCount++;
           }
        }

    }

    ui->lineEdit_User->setFocus();
}


void LoginSystem::SetControlStyleSheet()
{
    QPixmap pixmapWin = QPixmap(":/imag/image/login_new_bg.png").scaled(this->size());
    QPalette paletteWin(this->palette());
    paletteWin.setBrush(QPalette::Background, QBrush(pixmapWin));
    this->setPalette(paletteWin);

    ui->lineEdit_User->setStyleSheet("background-color:rgb(0,0,0);color:white;border:0px solid rgb(0,0,0)");
    ui->lineEdit_Password->setStyleSheet("background-color:rgb(0,0,0);font-size:15pt;color:white;border:0px solid rgb(0,0,0)");

    ui->pushButton_Login->setStyleSheet("QPushButton{border-image: url(:/imag/image/button.png);"
                                        "color:white"
                                        "}");
    ui->pushButton_Cancel->setStyleSheet("QPushButton{border-image: url(:/imag/image/button.png);"
                                        "color:white"
                                        "}");
    QString text ;
    allButtons = ui->widget_KeyBoard->findChildren<QPushButton *>();
    for( int i=0;i<allButtons.count();i++)
    {
       if(CapKey[i]== BSPACE )
       {
           text ="Backspace";
       }
       else if(CapKey[i]== TAB)
       {
           text ="TAB";
       }
       else if(CapKey[i]== CAPS)
       {
           text ="CapsLock";
       }
       else if(CapKey[i]== ENTER)
       {
           text ="Enter";
       }
       else if(CapKey[i]== SHIFT)
       {
           text ="Shift";
       }
       else if(CapKey[i]== SHIFT2)
       {
           text ="Shift";
       }
       else
       {
           if(CapKey[i]== '&')
           {
                text =QString("%1%2%3").arg("&&").arg("\n").arg(ComKey[i]);
           }
           else
           {
                text =QString("%1%2%3").arg(CapKey[i]).arg("\n").arg(ComKey[i]);
           }
           keyBoardButton[i]->setText(text);
           keyBoardButton[i]->setStyleSheet("QPushButton{background-color:rgb(48,48,48);text-align: left; font-size: 8pt;color:white;border:1px solid rgb(134,134,134)}");
           continue;
       }
       keyBoardButton[i]->setText(text);
       keyBoardButton[i]->setStyleSheet("QPushButton{background-color:rgb(48,48,48);font-size: 8pt;color:white;border:1px solid rgb(134,134,134)}");


    }
}


void LoginSystem::SetControlStyleSheet_600P()
{
    QPixmap pixmapWin = QPixmap(":/imag/image_800/login_new_bg.png").scaled(this->size());
    QPalette paletteWin(this->palette());
    paletteWin.setBrush(QPalette::Background, QBrush(pixmapWin));
    this->setPalette(paletteWin);

    ui->lineEdit_User->setStyleSheet("background-color:rgb(0,0,0);color:white;border:0px solid rgb(0,0,0)");
    ui->lineEdit_Password->setStyleSheet("background-color:rgb(0,0,0);color:white;border:0px solid rgb(0,0,0)");

    ui->pushButton_Login->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/button.png);"
                                        "color:white"
                                        "}");
    ui->pushButton_Cancel->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/button.png);"
                                        "color:white"
                                        "}");
    QString text ;
    allButtons = ui->widget_KeyBoard->findChildren<QPushButton *>();
    for( int i=0;i<allButtons.count();i++)
    {
       if(CapKey[i]== BSPACE )
       {
           text ="Backspace";
       }
       else if(CapKey[i]== TAB)
       {
           text ="TAB";
       }
       else if(CapKey[i]== CAPS)
       {
           text ="CapsLock";
       }
       else if(CapKey[i]== ENTER)
       {
           text ="Enter";
       }
       else if(CapKey[i]== SHIFT)
       {
           text ="Shift";
       }
       else if(CapKey[i]== SHIFT2)
       {
           text ="Shift";
       }
       else
       {
           if(CapKey[i]== '&')
           {
                text =QString("%1%2%3").arg("&&").arg("\n").arg(ComKey[i]);
           }
           else
           {
                text =QString("%1%2%3").arg(CapKey[i]).arg("\n").arg(ComKey[i]);
           }
           keyBoardButton[i]->setText(text);
           keyBoardButton[i]->setStyleSheet("QPushButton{background-color:rgb(48,48,48);text-align: left; font-size: 6pt;color:white;border:1px solid rgb(134,134,134)}");
           continue;
       }
       keyBoardButton[i]->setText(text);
       keyBoardButton[i]->setStyleSheet("QPushButton{background-color:rgb(48,48,48);font-size: 6pt;color:white;border:1px solid rgb(134,134,134)}");


    }

}
void LoginSystem::ResizeControl(int width,int height)
{

    float fsp[2] = {0};
    fsp[0]=(float)width/SYSTEM_WIN_WIDTH;
    fsp[1]=(float)height/SYSTEM_WIN_HEIGHT;

    //编辑
    int UserNameX = 409*fsp[0];
    int UserNameY = 229*fsp[1];
    ui->lineEdit_User->move(UserNameX,UserNameY);
    ui->lineEdit_User->resize(MMS_LOGIN_EDIT_FRAME_WIDTH*fsp[0],MMS_LOGIN_EDIT_FRAME_HEIGHT*fsp[1]);

    int PasswordX = 409*fsp[0];
    int PasswordY = 319*fsp[1];
    ui->lineEdit_Password->move(PasswordX,PasswordY);
    ui->lineEdit_Password->resize(MMS_LOGIN_EDIT_FRAME_WIDTH*fsp[0],MMS_LOGIN_EDIT_FRAME_HEIGHT*fsp[1]);

    //Widget
    int KeyboardX = 158*fsp[0];
    int keyboardY = 413*fsp[1];
    ui->widget_KeyBoard->move(KeyboardX,keyboardY);
    ui->widget_KeyBoard->resize(MMS_LOGIN_KEYBOARD_WIDTH*fsp[0],MMS_LOGIN_KEYBOARD_HEIGHT*fsp[1]);


    int  LoginOkX =332*fsp[0];
    int  LoginOkY =601*fsp[1];
    ui->pushButton_Login->move(LoginOkX,LoginOkY);
    ui->pushButton_Login->resize(MMS_LOGIN_BUTTON_WIDTH*fsp[0],MMS_LOGIN_BUTTON_HEIGHT*fsp[1]);

    int  LoginCancelX =529*fsp[0];
    int  LoginCancelY =LoginOkY;
    ui->pushButton_Cancel->move(LoginCancelX,LoginCancelY);
    ui->pushButton_Cancel->resize(MMS_LOGIN_BUTTON_WIDTH*fsp[0],MMS_LOGIN_BUTTON_HEIGHT*fsp[1]);


    allButtons = ui->widget_KeyBoard->findChildren<QPushButton *>();
    int keyCount =0;
    int lineNum =0;
    int keyX =0;
    int keyY =0;
    for( int i=0;i<allButtons.count();i++)
    {
        if(i==0)
        {
            lineNum=1;
            keyX =1;
            keyY =32*fsp[1]*(lineNum-1)+lineNum*1;

            keyBoardButton[i]->move(keyX,keyY);
            keyBoardButton[i]->resize(47*fsp[0],32*fsp[1]);
            keyCount++;
            keyX +=47*fsp[0];
        }
        else if(CapKey.at(i) ==TAB)
        {
            lineNum=2;
            keyCount=0;
            keyX =1;
            keyY =32*fsp[1]*(lineNum-1)+lineNum*1;
            keyBoardButton[i]->move(keyX,keyY);
            keyBoardButton[i]->resize(67*fsp[0],32*fsp[1]);
            keyCount++;
            keyX +=67*fsp[0];
        }
        else if(CapKey.at(i) ==CAPS)
        {
            lineNum=3;
            keyCount=0;
            keyX =1;
            keyY =32*fsp[1]*(lineNum-1)+lineNum*1;
            keyBoardButton[i]->move(keyX,keyY);
            keyBoardButton[i]->resize(86*fsp[0],32*fsp[1]);
            keyCount++;
            keyX +=86*fsp[0];
        }
        else if(CapKey.at(i) ==SHIFT)
        {
            lineNum=4;
            keyCount=0;
            keyX =1;
            keyY =32*fsp[1]*(lineNum-1)+lineNum*1;
            keyBoardButton[i]->move(keyX,keyY);
            keyBoardButton[i]->resize(105*fsp[0],32*fsp[1]);
            keyCount++;
            keyX +=105*fsp[0];
        }
        else
        {
            if(CapKey.at(i)== BSPACE || CapKey.at(i)== '|' || CapKey.at(i)== ENTER ||CapKey.at(i)== SHIFT2)
            {
                int width = MMS_LOGIN_KEYBOARD_WIDTH*fsp[0]- keyX-1;
                if(width<=0)
                    width = 47;

                keyBoardButton[i]->move(keyX,keyY);
                keyBoardButton[i]->resize(width,32*fsp[1]);
                keyCount++;
                keyX +=width;
            }
            else
            {

                keyBoardButton[i]->move(keyX,keyY);
                keyBoardButton[i]->resize(47*fsp[0],32*fsp[1]);
                keyCount++;
                keyX +=47*fsp[0];
            }
        }
    }
}


void LoginSystem::ResizeControl_600P(int width,int height)
{

    float fsp[2] = {0};
    fsp[0]=(float)width/SYSTEM_WIN_WIDTH_600P;
    fsp[1]=(float)height/SYSTEM_WIN_HEIGHT_600P;

    //编辑
    int UserNameX = 306*fsp[0];
    int UserNameY = 179*fsp[1];
    ui->lineEdit_User->move(UserNameX,UserNameY);
    ui->lineEdit_User->resize(MMS_LOGIN_EDIT_FRAME_WIDTH_600P*fsp[0],MMS_LOGIN_EDIT_FRAME_HEIGHT_600P*fsp[1]);

    int PasswordX = 306*fsp[0];
    int PasswordY = 249*fsp[1];
    ui->lineEdit_Password->move(PasswordX,PasswordY);
    ui->lineEdit_Password->resize(MMS_LOGIN_EDIT_FRAME_WIDTH_600P*fsp[0],MMS_LOGIN_EDIT_FRAME_HEIGHT_600P*fsp[1]);

    //Widget
    int KeyboardX = 124*fsp[0];
    int keyboardY = 323*fsp[1];
    ui->widget_KeyBoard->move(KeyboardX,keyboardY);
    ui->widget_KeyBoard->resize(MMS_LOGIN_KEYBOARD_WIDTH_600P*fsp[0],MMS_LOGIN_KEYBOARD_HEIGHT_600P*fsp[1]);


    int  LoginOkX =251*fsp[0];
    int  LoginOkY =466*fsp[1];
    ui->pushButton_Login->move(LoginOkX,LoginOkY);
    ui->pushButton_Login->resize(MMS_LOGIN_BUTTON_WIDTH_600P*fsp[0],MMS_LOGIN_BUTTON_HEIGHT_600P*fsp[1]);

    int  LoginCancelX =412*fsp[0];
    int  LoginCancelY =LoginOkY;
    ui->pushButton_Cancel->move(LoginCancelX,LoginCancelY);
    ui->pushButton_Cancel->resize(MMS_LOGIN_BUTTON_WIDTH_600P*fsp[0],MMS_LOGIN_BUTTON_HEIGHT_600P*fsp[1]);


    allButtons = ui->widget_KeyBoard->findChildren<QPushButton *>();
    int keyCount =0;
    int lineNum =0;
    int keyX =0;
    int keyY =0;
    for( int i=0;i<allButtons.count();i++)
    {
        if(i==0)
        {
            lineNum=1;
            keyX =1;
            keyY =25*fsp[1]*(lineNum-1)+lineNum*1;

            keyBoardButton[i]->move(keyX,keyY);
            keyBoardButton[i]->resize(38*fsp[0],25*fsp[1]);
            keyCount++;
            keyX +=38*fsp[0];
        }
        else if(CapKey.at(i) ==TAB)
        {
            lineNum=2;
            keyCount=0;
            keyX =1;
            keyY =25*fsp[1]*(lineNum-1)+lineNum*1;
            keyBoardButton[i]->move(keyX,keyY);
            keyBoardButton[i]->resize(53*fsp[0],25*fsp[1]);
            keyCount++;
            keyX +=53*fsp[0];
        }
        else if(CapKey.at(i) ==CAPS)
        {
            lineNum=3;
            keyCount=0;
            keyX =1;
            keyY =25*fsp[1]*(lineNum-1)+lineNum*1;
            keyBoardButton[i]->move(keyX,keyY);
            keyBoardButton[i]->resize(68*fsp[0],25*fsp[1]);
            keyCount++;
            keyX +=68*fsp[0];
        }
        else if(CapKey.at(i) ==SHIFT)
        {
            lineNum=4;
            keyCount=0;
            keyX =1;
            keyY =25*fsp[1]*(lineNum-1)+lineNum*1;
            keyBoardButton[i]->move(keyX,keyY);
            keyBoardButton[i]->resize(83*fsp[0],25*fsp[1]);
            keyCount++;
            keyX +=83*fsp[0];
        }
        else
        {
            if(CapKey.at(i)== BSPACE || CapKey.at(i)== '|' || CapKey.at(i)== ENTER ||CapKey.at(i)== SHIFT2)
            {
                int width = MMS_LOGIN_KEYBOARD_WIDTH_600P*fsp[0]- keyX-1;
                if(width<=0)
                    width = 38;

                keyBoardButton[i]->move(keyX,keyY);
                keyBoardButton[i]->resize(width,25*fsp[1]);
                keyCount++;
                keyX +=width;
            }
            else
            {
                keyBoardButton[i]->move(keyX,keyY);
                keyBoardButton[i]->resize(38*fsp[0],25*fsp[1]);
                keyCount++;
                keyX +=38*fsp[0];
            }
        }
    }
}


bool LoginSystem::IsSpecialButton(char but)
{
    bool bRet =false;
    if(but == '~' || but =='!' || but =='@' || but =='#' || but =='$' ||but =='%' ||
            but =='^' || but =='&' || but =='*' || but == '('|| but== ')' || but=='_' ||
            but =='+')
    {
        bRet =true;
    }

    return bRet;

}

void LoginSystem::SetEdit(char but)
{
    if(ui->lineEdit_User->hasFocus())//输入框1焦点
    {
        ui->lineEdit_User->insert(QString( but));
    }
    else if(ui->lineEdit_Password->hasFocus())//输入框2焦点
    {
        ui->lineEdit_Password->insert(QString( but));
    }

}

void LoginSystem::buttonNumslot(int btn)
{

    if(CapKey[btn] ==TAB)
    {
       if(ui->lineEdit_User->hasFocus())//输入框1焦点)
       {
           ui->lineEdit_Password->setFocus();
       }
       else if(ui->lineEdit_Password->hasFocus())
       {
           ui->lineEdit_User->setFocus();
       }

    }
    else if(CapKey[btn] ==CAPS)
    {
       if(bCapsOn)
       {
            keyBoardButton[btn]->setStyleSheet("QPushButton{background-color:rgb(48,48,48);font-size: 10pt;color:white;border:1px solid rgb(134,134,134)}");
            bCapsOn =0;
       }
       else
       {
            keyBoardButton[btn]->setStyleSheet("QPushButton{background-color:white;font-size: 10pt;color:black;border:1px solid rgb(134,134,134)}");
            bCapsOn =1;
       }
    }
    else if(CapKey[btn] ==SHIFT ||CapKey[btn] ==SHIFT2)
    {

       if(ShiftIndex ==0)
       {
            keyBoardButton[btn]->setStyleSheet("QPushButton{background-color:white;font-size: 10pt;color:black;border:1px solid rgb(134,134,134)}");
            ShiftIndex =btn;
       }

    }
    else if(CapKey[btn] == BSPACE)
    {
        if(ui->lineEdit_User->hasFocus())//输入框1焦点
        {
            if(!ui->lineEdit_User->selectedText().isEmpty())
            {
                 ui->lineEdit_User->del();

            }
            else
            {
                ui->lineEdit_User->backspace();
            }

        }
        else if(ui->lineEdit_Password->hasFocus())//输入框2焦点
        {
            if(!ui->lineEdit_Password->selectedText().isEmpty())
            {
                 ui->lineEdit_Password->del();

            }
            else
            {
                ui->lineEdit_Password->backspace();
            }
        }


    }
    else if(CapKey[btn] == ENTER)
    {
        on_pushButton_Login_clicked();
    }
    else
    {
        if(IsSpecialButton(CapKey[btn]))
        {
            if(ShiftIndex)
            {
               SetEdit(CapKey[btn]);
               keyBoardButton[ShiftIndex]->setStyleSheet("QPushButton{background-color:rgb(48,48,48);font-size: 10pt;color:white;border:1px solid rgb(134,134,134)}");
               ShiftIndex =0;
            }
            else
            {
              SetEdit(ComKey[btn]);
            }

        }
        else
        {
            if(bCapsOn)
            {
                if(ShiftIndex)
                {
                   SetEdit(ComKey[btn]);
                   keyBoardButton[ShiftIndex]->setStyleSheet("QPushButton{background-color:rgb(48,48,48);font-size: 10pt;color:white;border:1px solid rgb(134,134,134)}");
                   ShiftIndex =0;
                }
                else
                {
                   SetEdit(CapKey[btn]);
                }
            }
            else
            {
                if(ShiftIndex)
                {
                   SetEdit(CapKey[btn]);
                   keyBoardButton[ShiftIndex]->setStyleSheet("QPushButton{background-color:rgb(48,48,48);font-size: 10pt;color:white;border:1px solid rgb(134,134,134)}");
                   ShiftIndex =0;
                }
                else
                {
                   SetEdit(ComKey[btn]);
                }
            }
        }
    }



}
void LoginSystem::keyPressEvent(QKeyEvent *k)

{

	if(k->key() == Qt::Key_Enter) //判断是否是A键按下

	{

		on_pushButton_Login_clicked();

	}

}
// login sure
void LoginSystem::on_pushButton_Login_clicked()
{
    T_USER_INFO tUserInfo;
    memset(&tUserInfo, 0, sizeof(tUserInfo));
    QString sUserName = ui->lineEdit_User->text();
    QString sPassWord = ui->lineEdit_Password->text();
    if(sUserName == "admin" && sPassWord == "cftc123456")
    {
        strcpy(tUserInfo.acName, "admin");
        strcpy(tUserInfo.acPassWord, "cftc123456");
        tUserInfo.iUserType = E_USER_INFO_SUPER_ADMIN;
        MAIN_GetSdk()->SetUserInfo(&tUserInfo);
        emit ChangToUserWin(E_USER_INFO_SUPER_ADMIN);
        return;
    }

    QSqlDatabase  mydb  = QSqlDatabase::addDatabase("QSQLITE");
   //新建数据库（已有的情况下也可以用）
     mydb.setDatabaseName("MyDB");
   //已有数据库
    mydb.database("MyDB");
    mydb.open();
	

   // 带检查功能，如果已有数据表则不创建
    QString sqlCreatTab = QString("create table if not exists tb_power("
                          "serial_id serial primary key, "
                          "User_name varchar(32) ,"
                          "User_operator varchar(32)),"
                          "User_password varchar(32);");
    QSqlQuery query(mydb);
    bool isok = query.exec(sqlCreatTab);

    QString sqlSelect = QString("select * from tb_power;");
    isok=query.exec(sqlSelect);

    int count =0;
    bool bFindUser =0;
    int iOperator = 0;
    QString strName;
    QString strPassword;
    while (query.next())
    {
        strName = query.value("User_name").toString();
        iOperator = query.value("User_operator").toInt();
        strPassword = query.value("User_password").toString();
        if(strPassword == ui->lineEdit_Password->text() && strName ==ui->lineEdit_User->text())
        {
            bFindUser =1;
            break;
        }
        count++;
    }
    mydb.close();

    if(bFindUser)
    {
        strcpy(tUserInfo.acName, strName.toStdString().c_str());
        strcpy(tUserInfo.acPassWord, strPassword.toStdString().c_str());
        tUserInfo.iUserType = iOperator;
        MAIN_GetSdk()->SetUserInfo(&tUserInfo);
        emit ChangToUserWin(iOperator);
    }
    else
    {
        showWarn(this, FL8T("动车组视频监控系统"), FL8T("用户名或密码输入错误，请重新输入"));
        ui->lineEdit_Password->setFocus();
    }
}

//login cancel
void LoginSystem::on_pushButton_Cancel_clicked()
{
   emit ChangToUserWin(E_USER_INFO_OTHER);
}

int LoginSystem::showWarn(QWidget *parent, const QString &title, const QString &content)
{
    QMessageBox::warning(parent, title, content);
    this->showFullScreen();
    return 0;
}

void LoginSystem::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    QRect rt = ui->lineEdit_Password->geometry();
    QRect rtPwBtn(rt.right()+50, rt.top(), 192, rt.height());

    if(rtPwBtn.contains(pos))
    {
        if(QLineEdit::Password == ui->lineEdit_Password->echoMode())
        {
            ui->lineEdit_Password->setEchoMode(QLineEdit::Normal);
        }
        else
        {
            ui->lineEdit_Password->setEchoMode(QLineEdit::Password);
        }
    }
}
