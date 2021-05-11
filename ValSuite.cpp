

//#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE ((DPI_AWARENESS_CONTEXT)-2)
//#define _WIN32_WINNT_WIN10                  0x0A00 

//#define WINVER 0x0600


//#pragma comment(lib,"zlib")



#include <windows.h>
#include<windowsx.h>
#include <winuser.h> 

#include "shellscalingapi.h"

#include <ctime>
#include <cmath> 
#include <iostream>   
#include <iomanip> 
#include <sys/timeb.h>
#include <fstream>
#include <process.h> 
#include <conio.h>
#include <string> 
#include<stdio.h>  
#include<fcntl.h>
#include<io.h> 
using namespace std;
#include "valtimer.h"
#include<valwrapper.h>
#include"ValWindowManager.h"


//int loopcount = 0;

//int sides[2] ={1,1};
//int sidewid[2]={0,0};
int BIG_x = -1, BIG_y = -1;// cell offsets to boarder pixels for easy access

//int boarder_width[2]={0,0};
//COLORREF Boarder_color[2], Land_Color; 
//COLORREF num_color[8]; // define number colors 
//COLORREF bomb_color;
//int user_wid;
//int user_lon;
int	Mine_window_offset_x, Mine_window_offset_y, Mine_window_width, Mine_window_height;
HHOOK Hook, Hook2;
static bool DO_PAUSE = false, SHIFT_DOWN = false, CONTROL_DOWN = false, KEYBOARD_PROCESSING = false;
static bool F9FLAG = false, F8FLAG = false, F7FLAG = false, F6FLAG = false, F5FLAG = false, F3FLAG = false;
int startx[3], starty[3];



#define Boarder_color_left Boarder_color[0]    
#define Boarder_color_top Boarder_color[0]
#define Boarder_color_right Boarder_color[1]  
#define Boarder_color_down Boarder_color[1]
#define board_gather_offset 35 
#define BGA board_gather_offset 
#define _MWOx Mine_window_offset_x
#define _MWOy Mine_window_offset_y
#define _MWw  Mine_window_width
#define _MWh  Mine_window_height  
/*land color RGB          int conversion
land 192,192,192          12632256
bomb 0,0,0                0
corner 255,255,255        16777215
1-0,0,255;                16711680
2-0,128,0;                32768
3-255,0,0;                255

4-0,0,128                 8388608
5-128,0,0                 128
6-0,128,128               8421376

    x=12;  y=55; w=480;  h=256; // unstretched minewindow HWND offsets for mineboard

    HWNDrect board offsets: _MWOx+=20;  _MWOy+=128;  _MWw-=39;  _MWh-= 147;
*/
/***************************************macros and redefs *****************************************************/
#define Mouse_leftclick(){mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP,0,0,0,0);}
#define Mouse_rightclick(){mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP,0,0,0,0);}
#define LostSleep() Sleep(50);
#define GLOBALSLEEP();// Sleep(1200);
#define Title(s) SetConsoleTitleA(s)

// global compile options
#define Y_CONSOLE_START_POS 2  // this sets where clrscr starts clearing for varout
#define Y_CONSOLE_VAROUT_POS 1
#define Y_CONSOLE_VAROUT_DEPTH 1
static int GLOBAL_SLEEP_VALUE = 0;
#define _LOST_FOLDER "lost\\" 
#define _WON_FOLDER ""

static bool _DO_RECORD_LOST = false;
static bool _DO_RECORD_UNDT = false;
static bool _DO_RECORD_WON = true;
static int LOOP_OUT = 5;
static int CLICK_TYPE = 0;
#define _DO_PATTERN // undefine to turn off pattern recognition -depreciated

static int winn = 0, user_mines;
void gotoxy(int x, int y) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = { (short)x, (short)y };
    SetConsoleCursorPosition(handle, position);
}
//void getxy(int& x, int& y) {
//    _CONSOLE_SCREEN_BUFFER_INFOEX info;
//    GetConsoleScreenBufferInfoEx(GetStdHandle(STD_OUTPUT_HANDLE), &info);
//    x = info.dwCursorPosition.X;
//    y = info.dwCursorPosition.Y;
//    info.
//}
void getxy(int& x, int& y) {
    
    CONSOLE_SCREEN_BUFFER_INFO con;
    HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hcon != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(hcon, &con))
    {
        x = con.dwCursorPosition.X;
        y = con.dwCursorPosition.Y;
    }
    
}
void clrscr() {
    gotoxy(0, Y_CONSOLE_START_POS);
    HANDLE hOut; COORD Position; DWORD Written;
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    Position.X = 0; Position.Y = Y_CONSOLE_START_POS;

    FillConsoleOutputCharacter(hOut, ' ', 9000, Position, &Written);
    gotoxy(0, Y_CONSOLE_START_POS);
}
WCHAR* ctolpwstr(string& s) {
    char* a = (char*)s.c_str();
    WCHAR* p;
    int len = MultiByteToWideChar(CP_ACP, 0, a, -1, NULL, 0);
    p = new WCHAR[len];
    MultiByteToWideChar(CP_ACP, 0, a, -1, (LPWSTR)p, len);

    return p;
}

WCHAR* ctowstr(int s) {// working.
    WCHAR* m = new WCHAR[256];
    swprintf_s(m, 256, L"%d", s);
    return m;
}


void random_seed() {
    time_t seconds;
    time(&seconds);
    srand((unsigned int)seconds);
}
int random(int a) {
    return rand() % a;
}
HWND GetConsoleHwnd(void) {
    return GetConsoleWindow();
}
#define pause kbp_pause
void kbp_pause() {

    if (KEYBOARD_PROCESSING) {
        cout << "press f10 to unpause" << endl;
        //Title("paused");
        DO_PAUSE = true;
        while (DO_PAUSE)Sleep(7);
    }
    else system("Pause");

}


//#include"minemath.h"
//#include "minemath_autosave.h"
//#include "ValScreenwrapper.h" 
//#include "opus.h"
//#include "MagnumbDomains.h"
/*
int Var_Out_hold[10] = { 1,1,1,1,1,1,0,0,0,0 };
void Var_Out() {
    if (Var_Out_hold[0] == CLICK_TYPE && Var_Out_hold[1] == GLOBAL_SLEEP_VALUE)return;
    Var_Out_hold[0] = CLICK_TYPE;
    Var_Out_hold[1] = GLOBAL_SLEEP_VALUE;

    int x, y;
    getxy(x, y);
    gotoxy(15, Y_CONSOLE_VAROUT_POS); SHOW(CLICK_TYPE);
    gotoxy(30, Y_CONSOLE_VAROUT_POS); SHOW(GLOBAL_SLEEP_VALUE);
    if (y <= Y_CONSOLE_START_POS)y = Y_CONSOLE_START_POS;

    gotoxy(x, y);
}
void opus_output_info(string fname);
bool TOGGLE_OPUS_KBD = true;
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    int x = 0, y = 2;
    if (nCode == HC_ACTION) {
        if (wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT* lpm = (KBDLLHOOKSTRUCT*)lParam;

            switch (lpm->vkCode) {
            case VK_F2:
                if (GetAsyncKeyState(VK_LCONTROL)) {
                    ShowConsole();
                }
                else HideConsole();
                break;
            case VK_F3:
                F3FLAG = !F3FLAG;
                break;
            case VK_F4:
                (CLICK_TYPE > 1 ? CLICK_TYPE = 0 : CLICK_TYPE++);
                getxy(x, y);
                gotoxy(15, Y_CONSOLE_VAROUT_POS); SHOW(CLICK_TYPE);
                gotoxy(x, y);
                break;
            case VK_F5:
                clrscr();
                if (TOGGLE_OPUS_KBD) { opus_color_out(); opus_output_info("opusinfo.txt"); }
                F5FLAG = !F5FLAG;
                break;
            case VK_F6:
                F6FLAG = !F6FLAG;
                if (TOGGLE_OPUS_KBD)opus_reset();
                break;
            case VK_F7:
                F7FLAG = !F7FLAG;
                (!GLOBAL_SLEEP_VALUE ? GLOBAL_SLEEP_VALUE = 0 : GLOBAL_SLEEP_VALUE -= 10);
                getxy(x, y);
                gotoxy(30, Y_CONSOLE_VAROUT_POS); SHOW(GLOBAL_SLEEP_VALUE);
                gotoxy(x, y);

                break;
            case VK_F8:
                F8FLAG = !F8FLAG;
                GLOBAL_SLEEP_VALUE += 10;
                getxy(x, y);
                gotoxy(30, Y_CONSOLE_VAROUT_POS); SHOW(GLOBAL_SLEEP_VALUE);
                gotoxy(x, y);
                break;
            case VK_F9:
                F9FLAG = !F9FLAG;
                break;
            case VK_F11:
                clrscr();
                break;

            case VK_F10:
                if (DO_PAUSE = !DO_PAUSE)Title("paused");
                else Title("running");
                break;
            case VK_F12:
                CallNextHookEx(Hook2, nCode, wParam, lParam);
                exit(0);
                break;
            }
        }
    }
    return CallNextHookEx(Hook2, nCode, wParam, lParam);
}


void var_out_thread(void* a) {
    while (1) {
        Var_Out();
        Sleep(1000);
    }
}
*/
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    return CallNextHookEx(Hook2, nCode, wParam, lParam);
}
void setFontSize(int FontSize)
{
    CONSOLE_FONT_INFOEX info = { 0 };
    info.cbSize = sizeof(info);
    info.dwFontSize.Y = FontSize; // leave X as zero
    info.FontWeight = FW_NORMAL;
    wcscpy_s(info.FaceName, L"Lucida Console");
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &info);
}
void StayOnTop(int right = 500, int bottom = 75) {
    LPRECT rect = new RECT;
    HWND mywn = GetConsoleHwnd();
    GetWindowRect(mywn, rect);
    rect->left = 0;
    rect->top = 0;
    rect->right = right;
    rect->bottom = bottom;

    int wi = rect->right - rect->left;
    int hi = rect->bottom - rect->top;

    ::SetWindowPos(mywn,       // handle to window
        HWND_TOPMOST,  // placement-order handle
        rect->left,     // horizontal position
        rect->top,      // vertical position
        wi,  // width
        hi, // height
        SWP_SHOWWINDOW);// window-positioning options);
}

void do_msg_loop() {
    MSG msg;
    Hook2 = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    KEYBOARD_PROCESSING = true;
    while (1) {
        GetMessage(&msg, NULL, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

}

//#include"rageguage.h"
/*
void main_rageguage() {
    opusvs.Set(300, 300, 700, 400);
    opusvs.create_transparent_window2(300, 300, 700, 490);
    //opusvs.create_drawing_console(300, 300, 750, 490);
    _beginthread(pauls_mess2, 0, NULL);

    do_msg_loop();

}
*/
//notes on msonline.com score sender:
//win.php  key=r = 3rand chars, 1 radn*225)+25+3 , 4 rand chars  
//time
//name=name 
//s:ad,i:ab
//h:ae?1:0 is highscore. 
// ad=Math.round(((new Date()).getTime()-aa)/1000).  aa is time game ended- to stop bots from instantly submitting scores. should take a few seconds.
//switch(ab){case 1:Y="daily";break;case 2:Y="weekly";break;case 3:Y="monthly";break;case 4:Y="all-time"


/*
notes on long scale pattern recognition heuristic:
moved to deltainfiniti.com/long_pattern.txt

*/

/*
int mainrr() {// magnumbOpus runner 
   // cout << (int)RGB(0,123,123) << endl;
    //pause();


    //opusvs.create_transparent_window2(300, 300, 700, 490);
    //   do_msg_loop();
    //DISPLAY_DEVICE_TYPE dev;           
    //SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);    
    _beginthread(calibrate_timer_thread, 0, NULL);
    /*   opusvs.Set(300,300,800,800);
       opusvs.get_screen();
       opusvs.get_buffer();

       opusvs.bufwin_draw();
       Sleep(3000);
       opusvs.get_screen();
       opusvs.get_buffer();
       opusvs.bufwin_draw();

       do_msg_loop();*/
       
/*
       //HideConsole();
       //main_rageguage();
       //do_msg_loop();
    MSG msg;
    cout << "f4-click type f5-output f6-grid_reset f7&f8-sleep:v^ f10-pause  f11-clrscr f12-quit\n\n";
    setFontSize(11);

    //sides[0]=30;  
    //sides[1]=16; 

    gotoxy(0, 3);
    //opusvs.create_transparent_window(); 
    cout << "starting Minemath process" << endl;

    StayOnTop(800, 1800);
    opus_HWND_move_resize(GetConsoleHwnd(), 60, 50, 800, 1000);
    opusvs.create_transparent_window2(840, 60, 730, 590);
    //opus_trainer_hwnd= trainer.create_transparent_window(865,585,1000,1000);//(300,300,800,800);  

    //opus_get_drawing_console(820,50,900,700);   

    KEYBOARD_PROCESSING = false;
    //_beginthread(var_out_thread,0,NULL);    
    Sleep(50);
    //_beginthread(opus_trainer_thread,0,NULL);

    //_beginthread(opus_initialize,0,NULL); 
    _beginthread(opus_main_thread_info, 0, NULL); //_info
    //_beginthread(opus_main_thread,0,NULL);
    //_beginthread(MagNumb_screen_calibrate_new, 0, NULL);
    //_beginthread(opus_main_thread_screen_info, 0, NULL); //_info
    //_beginthread(opus_trainer_screen_thread, 0, NULL);
    //_beginthread(pauls_mess2,0,NULL);     

    Hook2 = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    KEYBOARD_PROCESSING = true;
    while (1) {
        GetMessage(&msg, NULL, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

void draw200red(BYTE*& b) {
    BYTE* c = b;
    for (int i = 0; i < 200; i++) {
        //b[(i*4)+2]=255;
        c += 2; *c = 255; c += 2;
    }
}

void _mess(void* a) {
    cout << 1;
    opusvs.sprites.load_spritefield("link.bmp", "sprite.txt");
    cout << 1;
    opusvs.get_screen();
    opusvs.get_buffer();
    cout << 1;
    BYTE* b = (BYTE*)opusvs.bits;
    draw200red(b);
    opusvs.sprites.write_spritefield((BYTE*&)opusvs.bits, 0, 0, 1800, 1800);
    //opusvs.bits=(char*)opusvs.sprites.sprite_field;
    cout << 1;
    opusvs.bufwin_draw();
    pause();
    opusvs.vsprite_findsprites();
    pause();
    opusvs.bufwin_draw();
    pause();
    int plc = 11;
    while (1) {
        opusvs.white_buffer();
        opusvs.sprites.write_sprite((BYTE*)opusvs.bits, 50, 50, 1800, plc);
        opusvs.sprites.write_sprite((BYTE*)opusvs.bits, 50, 220, 1800, plc + 10);
        opusvs.sprites.write_sprite((BYTE*)opusvs.bits, 50, 370, 1800, plc + 20);
        //opusvs.sprites.write_sprite((BYTE*)opusvs.bits,50,540,800,plc+30);
        plc++;
        if (plc > 18)plc = 11;
        opusvs.bufwin_draw();
        Sleep(60);
    }
    pause();

}

int maint() {// magnumbOpus runner   

    //DISPLAY_DEVICE_TYPE dev;           
    //SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);      
    _beginthread(calibrate_timer_thread, 0, NULL);
    opusvs.Set(300, 300, 1800, 1800);
    MSG msg;

    setFontSize(11);

    //sides[0]=30;     
    //sides[1]=16;       

    gotoxy(0, 3);
    StayOnTop(800, 1800);
    opus_HWND_move_resize(GetConsoleHwnd(), 60, 50, 800, 1000);
    opus_trainer_hwnd = trainer.create_transparent_window(865, 585, 1800, 1800);//(300,300,800,800);  
    opusvs.condc = GetDC(opus_trainer_hwnd);

    //opus_get_drawing_console(820,50,900,700);    
    //DRAW_HWND=opusvs.create_drawing_console();         
    //atexit(destroy_drawing);   
    KEYBOARD_PROCESSING = false;
    //_beginthread(var_out_thread,0,NULL);    
    Sleep(50);
    //_beginthread(opus_trainer_thread,0,NULL); 
    //_beginthread(opus_initialize,0,NULL); 
    //_beginthread(opus_main_thread,0,NULL);   
    _beginthread(_mess, 0, NULL);

    Hook2 = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    KEYBOARD_PROCESSING = true;
    while (1) {
        GetMessage(&msg, NULL, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
*/

/*HWND editctl;
HINSTANCE hinstance;
HWND testhwnd;
void looping(void* a) {
    Sleep(50);

    char buffer[5000];
    while (1) {
        opusvs.get_screen();
        opusvs.get_buffer();
        opusvs.bufwin_draw();
        GetWindowTextA(editctl, buffer, 5000);
        cout << buffer << endl;
        Sleep(1000);
    }
}
LPRECT RECTb(RECT& a, int x, int y, int w, int h) {

    a.top = y;
    a.left = x;
    a.bottom = h;
    a.right = w;
    return (LPRECT)&a;
}
void func1(int a) {
    cout << "1  " << a << endl;
}
void func2(void* a) {
    cout << "2" << endl;
}
void func3(_beginthread_proc_type fc) {
    (*fc)(NULL);//void*);
}
_beginthread_proc_type funcregs[20];

void func4(int index) {
    (*funcregs[index])(NULL);
}
void func5(int index, void* arg) {
    (*funcregs[index])(arg);
}

void func6() {
    cout << "6" << endl;
}
void callfunc(void* a) {
    ((void(__cdecl*)(void))(a))();
}
void(*funcs[256])();
void test_function1() {
    funcregs[0] = (_beginthread_proc_type)func1;
    funcregs[1] = func2;
    funcs[0] = &func6;
    funcs[1] = (void(__cdecl*)(void)) func2;
    func3(*func2);
    func4(1);
    func4(0);
    func5(0, (void*)3);
    funcs[0]();
    funcs[1]();
    _beginthread(callfunc, 0, (void*)&func2);
    cout << sizeof(void*) << endl;;
    return;

    short a[4] = { 65534,2,3,0 };
    short* b = (short*)a;
    int* c = (int*)a;
    cout << c[0] << endl;
    cout << c[1] << endl;
    foreach(x, 6) {
        cout << b[x] << endl;
    }

}
*/
//#include"valtextcontrol.h"
/*
int mainr() {  // window control testing stuff
    test_function1();
    pause();
    SystemParametersInfo(SPI_SETFONTSMOOTHING,
        TRUE,
        0,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETFONTSMOOTHINGTYPE,
        0,
        (PVOID)FE_FONTSMOOTHINGCLEARTYPE,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETFONTSMOOTHINGCONTRAST,
        0,
        (PVOID)1600,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    hinstance = GetModuleHandle(NULL);
    opusvs.Set(0, 0, 300, 300);
    testhwnd = opusvs.create_transparent_window2(100, 100, 400, 400);
    PAINTSTRUCT lps;
    HDC tempdc = BeginPaint(testhwnd, (LPPAINTSTRUCT)&lps);
    RECT a;
    RECTb(a, 140, 140, 340, 330);
    DrawTextA(tempdc, "sometext", 8, &a, DT_EDITCONTROL);
    EndPaint(testhwnd, (LPPAINTSTRUCT)&lps);
    Sleep(3000);


    opusvs.get_screen();
    opusvs.get_buffer();
    opusvs.white_buffer();
    opusvs.buf_drawtext(10, 10, 10, 10, "hithere");
    opusvs.bufwin_draw();
    cout << "drew text\n";
    Sleep(3000);
    //    opusvs.black_buffer();
    opusvs.white_buffer_sect(0, 15, 80, 12);
    opusvs.bufwin_draw();
    cout << "drew white on buffer\n";
    Sleep(3000);
    opusvs.white_buffer();
    ValTextControl vtc("Arial", 90, 90, 40, 40);
    vtc.content = "Hi world testing 123";

    vtc.SetFont("Comic Sans MS");
    foreach(x, 9000) {
        //vtc.fontdat.lfOrientation +=30;
        vtc.fontdat.lfEscapement += 2;
        vtc.color = RGB(random(254), random(254), random(254));
        opusvs.white_buffer();
        vtc.DIBdraw(opusvs.condc, opusvs.newbitmap);
        cout << vtc.lastbounds.left << " " << vtc.lastbounds.top << "  " << vtc.lastbounds.right << "  " << vtc.lastbounds.bottom << " \n\n ";
        opusvs.buf_drawrect(vtc.lastbounds.left, vtc.lastbounds.top, vtc.lastbounds.right, vtc.lastbounds.bottom);
        opusvs.bufwin_draw();
        //cout << "drew vtc out" << endl;
        //Sleep(1);
    }
    Sleep(3000);
    editctl = CreateWindowEx(WS_EX_TRANSPARENT, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_AUTOHSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 0, 0, 100, 50, testhwnd, (HMENU)56, hinstance, 0);
    SetWindowTextA(editctl, "hi");
    Sleep(100);


    Sleep(100);


    _beginthread(looping, 0, NULL);
    do_msg_loop();

    return 0;

}
void inline mousepos(int& x, int& y) {
    POINT pt;
    GetCursorPos(&pt);
    x = pt.x;
    y = pt.y;

}

void mouse_pixel_follow_thread(void* a) {
    int x, y;
    opusvs.condc = GetDC(GetConsoleHwnd());
    int size = 100;
    while (1) {
        mousepos(x, y);
        opusvs.Set(x - (size / 2), y - (size / 2), size, size);

        opusvs.get_screen();
        opusvs.get_buffer();
        cout << "mousepos:" << x << ":" << y << "  RGB:" << creftostr((COLORREF)opusvs.buf_Get_Pixel(15, 15)) << "            \r";

        opusvs.buf_drawrect((size / 2) - 2, (size / 2) - 2, 5, 5);
        opusvs.bufwin_draw(75, 75);
        Sleep(20);

    }
}

int main3() {
    mouse_pixel_follow_thread(NULL);
    return 0;
}
*/


//#include"ValWindow.h"



//#include"valwrapper.h"
/*string& csl_vl_format(string& str) {
    ;
    str_replace_all(str, "`", "`1");
    str_replace_all(str, "=", "`0");
    str_replace_all(str, ",", "`2");
    str_replace_all(str, "\n", "`3");
    return str;
}
string& csl_vl_unformat(string& str) {
    str_replace_all(str, "`0", "=");
    str_replace_all(str, "`1", "`");
    str_replace_all(str, "`2", ",");
    str_replace_all(str, "`3", "\n");
    return str;
}
*/
/*
string vlisttodsl_safe(varlist& vl,string delim) {
    vlip vv = vl.ini();
    string name, val, ret = "";
    while (vl.getnext(vv, name, val)) {
        ret += csl_vl_format(name) + "=" + csl_vl_format(val);
        if (vv->next != vl.end)ret += delim;
    }
    return ret;
}
int  vlistSaveFile_safe(varlist& vl,string filename) {
    ofstream ofile(filename.c_str());
    ofile << vlisttodsl_safe("\n");
    ofile.close();
    return 1;
}

int vlistLoadFile_safe(varlist& vl,string filename) {
    ifstream ifile(filename.c_str());
    if (!ifile.good())return 0;
    string line, n, v;
    int lines = 0;
    cout << endl;
    while (!ifile.eof()) {
        getline(ifile, line);
        breakarg(line, n, v, '=');
        vl.set(csl_vl_unformat(n), csl_vl_unformat(v));
        lines++;
        if (lines % 13 == 0)cout << lines << "\r";
    }
    //    setjumps();
    return  size;

}
*/
/*
string shortlist_tocsl(shortlist& sl) {
    string ret = "", h;
    cnptr pt = sl.start;
    do {
        h = csl_vl_format(pt->name) + "=" + csl_vl_format(pt->value);
        ret += csl_vl_format(h) + ",";
    } while ((pt = pt->next) != NULL);
    return ret;
}
void csl_toshortlist(string csl, shortlist& sl) {
    string tmp = csl, h, n, v;
    while (tmp != "") {
        h = tokenizecsl(tmp, false);
        csl_vl_unformat(h);
        breakarg(h, n, v);
        sl.set(csl_vl_unformat(n), csl_vl_unformat(v));
    }

}
*/
/*
void shortlist_set_varlist(shortlist& sl, varlist& vl) {
    cnptr pt = sl.start;
    do {
        vl.set(pt->name, pt->value);
    } while ((pt = pt->next) != NULL);

}
varlist shortlist_to_varlist(shortlist& sl) {
    cnptr pt = sl.start;
    varlist vl;
    do {
        vl.set(pt->name, pt->value);
    } while ((pt = pt->next) != NULL);
    return vl;
}
*/
#include"Neopets.h"
void outstatus(void* a) {
    NeopetsUser* user = (NeopetsUser*)a;
    while (1) {
        cout << "\r                          \r";
        cout << user->status;
        Sleep(300);
    }
}
/*
int main234() {
    shortlist sl;
    sl.set("so=me", "val,ue");
    sl.set("blas=h", "de=bl,ah");
    string csl = shortlist_tocsl(sl);
    cout << csl << endl << endl;
    shortlist s2;

    csl_toshortlist(csl, s2);
    cout << s2.cookieval() << endl;
    cout << "\n\n\n\n\n";
    string h = "\n\n\n\n\n\n";
    cout << enc_vl_format(h);
    pause();
    return 0;
}
*/
//#include"ValWindowManager.h"
string global_status = "";
//ValWindowManager WindowMan;
void test_parents(void* hwnd) {
    //HWND hWnd = WindowMan.fromHWND((HWND)hwnd);
    ValWindow* mywind = WindowMan.fromHWND((HWND)hwnd);
    /*ValWindow* mywind=NULL;
    for (int i = 0; i < WindowMan.windowslen; i++)
        if (WindowMan.windows[i]->myhwnd == hWnd)mywind = WindowMan.windows[i];
    */
    cout << mywind->eval_edit("u") << endl;
    if (mywind == NULL) {
        cout << "cannot find window parent" << endl;
        pause();
    }
    else {
        cout << " window title: " << mywind->windowtitle << endl;
    }
}
void create_splash_window() {

}

//#include "strlist_sup_dedupe.h"

NeopetsUser user;

void neologin(void* hwnd) {
    ValWindow* mywind = WindowMan.fromHWND((HWND)hwnd);
    string un = mywind->eval_edit("username");
    string pw = mywind->eval_edit("password");
    if (un == "" || pw == "") {
        user.status = "blank username/password!";
        cout << "blank username or password error" << endl;
        return;
    }
    cout << "logging in as " << un << " with pw: *" << pw << "*" << pw.length() << endl;
    user.status = "starting up";
    user.login(un, pw);

}
void neoboardscrape(void* hwnd) {
    ValWindow* mywind = WindowMan.fromHWND((HWND)hwnd);
    boards_username_scrape_nolog(&user);
}

int listfile_numlines(string fname) {
    ifstream ifile(fname.c_str());
    if (!ifile.good()) return 0;
    int ret = 0;
    string txt;

    while (!ifile.eof()) {
        getline(ifile, txt, '\n');
        ret++;
        //cout << ret << txt << "                \r";
    }
    return ret;

}
int listfile_numlines_dedupe(string fname, string fname2) {

    ifstream ifile(fname.c_str());
    if (!ifile.good()) return 0;
    int ret = 0;
    string txt;
    stringlist list;
    strlist_dedupe dupelist;
    ValStopwatch timer;
    timer.LapStart();
    while (!ifile.eof()) {
        getline(ifile, txt, '\n');
        ret++;
        //list.set(txt);
        dupelist.dedupe_queue(txt);
        //if(ret%1769==0)cout << ret << txt << "                \r";

    }
    cout << timer.Lap() << "ms to load file " << endl;
    //list.save(fname2);
    //cout << "deduped size " << list.getsize() << endl;
    //dupelist.onepass_dedupe();
    dupelist.twopass_dedupe();
    cout << "deduped size2 : " << dupelist.dedupedlen << endl;
    cout << timer.Lap() << " ms to dedupe " << endl;

    //sleep(5000);
    //dupelist.dedupeout();
    return ret;

}
void make_huge_dedupe_testfile() {
    ofstream ofile("dedupetest.txt");
    string s, n;
    foreach(i, 100000) {
        s = randstr(8);
        n = randstr(10);
        ofile << s << "\n";
        ofile << n << "\n";
        ofile << s << "\n";
        ofile << n << "\n";
    }
    ofile.close();
}
void msgloop_kill(void* a) {

    Sleep(6000);
    // put global loading stuff in here instead of sleep
    // start all global threads here, timer_calibrate comes to mind
    // this method will be a required function like main for end users
    // can be called without splash screen, inline if desired. 
    bool* b = (bool*)a;
    *b = true;
    cout << "changed bool " << endl;

}
void splashthread(void* a) {
    string* b = (string*)a;
    ValWindow splashsc(*b, 500, 500, 500, 500, VW_splash);
    //Sleep(3000);
    bool kill = false;
    _beginthread(msgloop_kill, 0, (void*)&kill);

    MSG msg;
    while (1) {
        if (PeekMessageA((LPMSG)&msg, NULL, 0, 0, PM_NOREMOVE)) {
            GetMessageA(&msg, NULL, 0, 0);
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (kill)break;
    }
    ShowWindow(splashsc.myhwnd, SW_HIDE);
    cout << "thread killed" << endl;

}
/*
// this method can be copied and expanded to do loading proceedures
struct splashscreen_struct {
    string str;
    bool b;
    int time;
    int x, y, w, h;
};
splashscreen_struct build_splashscreen_struct(string s, int time, int x = 0, int y = 0, int w = 0, int h = 0) {
    splashscreen_struct ret;
    ret.str = s;
    ret.b = false;
    ret.time = time;
    ret.x = x;
    ret.y = y;
    ret.w = w;
    ret.h = h;
}void VW_splashscreenthread(void* a) {
    splashscreen_struct* info = (splashscreen_struct*)a;
    if (info->x <1)info->x = GetSystemMetrics(SM_CXSCREEN) / 2;
    if (info->y <1)info->y = GetSystemMetrics(SM_CXSCREEN) / 2;
    ValWindow splashsc(info->str, info->x, info->y, info->w, info->x, VW_splash);
    //Sleep(3000);

    _beginthread(VW_slpashscreen_msgloop_kill, 0, (void*)&info);

    MSG msg;
    while (1) {
        if (PeekMessageA((LPMSG)&msg, NULL, 0, 0, PM_NOREMOVE)) {
            GetMessageA(&msg, NULL, 0, 0);
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (kill)break;
    }
    ShowWindow(splashsc.myhwnd, SW_HIDE);

}*/


void fix_varlist_newlines(string fname, string to) {
    ifstream ifile(fname.c_str());
    ofstream ofile(to.c_str());

    if (!ifile.good()) return;
    string hold;
    while (!ifile.eof()) {
        getline(ifile, hold, '\n');
        if (findnext(hold, "=", 0) != -1 && hold[findnext(hold, "=", 0) + 1] != 'I' && hold[0] != '<') ofile << hold << "\n";

    }

}
void varlist_to_nameslist(string fname, string to) {
    ifstream ifile(fname.c_str());
    ofstream ofile(to.c_str());

    if (!ifile.good()) return;
    string hold;
    while (!ifile.eof()) {
        getline(ifile, hold, '\n');
        hold = getbetween(hold, 0, "=");
        ofile << hold << "\n";

    }

}
BOOL CALLBACK EnumFamCallBack(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID aFontCount)
{
    wcout << (WCHAR*)lplf->lfFaceName << endl;
    return true;

}
void enum_fontnames() {
    int aFontCount[] = { 0, 0, 0 };


    EnumFontFamilies(GetDC(GetDesktopWindow()), (LPCTSTR)NULL,
        (FONTENUMPROC)EnumFamCallBack, (LPARAM)aFontCount);
}
//#define VW_HAS_SPLASHSCREEN_LOAD
#define wmgData WindowMan.globaldata
void SPLASHSCREEN_LOAD() {
    _beginthread(calibrate_timer_thread, 0, NULL);
    WindowMan.globaldata.load("data.di");
    if (wmgData.valueof("callsign") == "") {
        //start new idenity window.
        //get new password.
        // set all default data points in wmgData
        //encrypt and save as localdata.die
    }

}
void DI_BOOTUP_RUNONCE() {
    cout << "before createsplash" << endl;
    string s = "mario.bmp";
    s = "testing_splash.jpg";
    WindowMan.create_splash(s, 1000, 0, 0, 0, 0);
    WindowMan.wait_for_splash();

}
void do_msg_loopA_thread(void* a) {
    MSG msg;
    Hook2 = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    KEYBOARD_PROCESSING = true;
    while (1) {
        if (PeekMessageA((LPMSG)&msg, NULL, 0, 0, PM_NOREMOVE)) {
            GetMessageA(&msg, NULL, 0, 0);
            TranslateMessage(&msg);
            //TranslateAcceleratorA(&msg);
            DispatchMessageA(&msg);
        }
        if (WindowMan.Needs_create) {
            ValWindow* wind = WindowMan.RESERVED_create_new_window();
            WindowMan.newly_created_window = wind->myUUID;
            WindowMan.Needs_create = false;

        }
        
    }

}
void do_msg_loopA() {
    MSG msg;
    Hook2 = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    KEYBOARD_PROCESSING = true;
    while (1) {
        if (PeekMessageA((LPMSG)&msg, NULL, 0, 0, PM_NOREMOVE)) {
            GetMessageA(&msg, NULL, 0, 0);
            TranslateMessage(&msg);
            //TranslateAcceleratorA(&msg);
            DispatchMessageA(&msg);
        }
        /*if (WindowMan.Needs_create) {
            ValWindow* wind = WindowMan.RESERVED_create_new_window();
            WindowMan.newly_created_window = wind->myUUID;
            WindowMan.Needs_create = false;

        }*/
    }

}

void draw_login_window(void*a) {
    //enum_fontnames();
    //pause();
    Sleep(30);
    cout << "returned to the main thread.  " << endl;
    int height = 500, width = 500;
    ValWindow* wind = WindowMan.create_new_windowex("testing", 300, 300, height, width, 1);
    wind->myhdc = GetDC(wind->myhwnd);
    wind->is_master = true;
    width -= 20;
    height -= 40;
    //NeopetsUser user("valzeediann", "FuckNeo12");
    wind->add_global_stat_int(VC_GLOBAL_STAT_STRING, "status", "status:*S", VRECT{ 0, height - 25, 450,25 }, (void*)&user.status);
    //wind->add_global_stat_int(VC_GLOBAL_STAT_STRING, "status", "status:*S", VRECT{ 250, height-25, 250,25 }, (void*)&global_status);
    int offsetx = 45, offsety = 200;
    wind->add_control_ex_ts(VC_LABEL, "warning_label", "this is a master window. if you click the 'x' button, the program will exit", VRECT{ 0,0,500,25 }, VC_NOFUNC, false);
    wind->add_control_ex_ts(VC_LABEL, "username_label", "username:", VRECT{ offsetx,offsety,65,25 }, VC_NOFUNC, false);
    wind->add_control_ex_ts(VC_LABEL, "password_label", "password:", VRECT{ offsetx,offsety + 30,65,25 }, VC_NOFUNC, false);
    wind->add_editctrl("username", VRECT{ offsetx + 70, offsety, 150, 20 }, "valzeediann");  // fill autofill info from master data. 

    wind->add_passwordctrl_ts("password", VRECT{ offsetx + 70, offsety + 30, 150,20 }, "login_button");
    //wind->add_control_ex(VC_BUTTON, "login_button", "login", VRECT{ offsetx + 225,offsety,100,55 }, VC_NOFUNC, true);
    //wind->add_control_ex(VC_BUTTON, "boards", "get names", VRECT{ 40,320,350,75 }, VC_NOFUNC, true);
    wind->add_control_ex_ts(VC_BUTTON, "login_button", "login", VRECT{ offsetx + 225,offsety,100,55 }, (void*)&neologin, true);
    wind->add_control_ex_ts(VC_BUTTON, "boards", "get names", VRECT{ 40,320,350,75 }, (void*)&neoboardscrape, true);
    wind->full_render = true;

    //ValWindow* wind2 = WindowMan.create_new_window("testing2", 600, 600, 300, 300, 1);
    //wind2->add_global_stat_int(VC_GLOBAL_STAT_STRING, "status", "status:*S", VRECT{ 0, 300 - 25, 280,25 }, (void*)&user.status);
 //   ValGFX gfxtemp;
 //   gfxtemp.load_image("mario.bmp");
 //   gfxtemp.CopySectionToGFX(wind2->mygfx, 50, 50, 200, 200, 50, 50);
 //   wind2->GFXdraw();
   // wind2->add_control_ex(VC_BUTTON, "login", "login", VRECT{ 180,5,100,25 }, (void*)&neologin, true);
    //do_msg_loopA();
}
//wind->AddControl("type=VC_LABEL,title=infolabel,text=passwords cannot contain commas or equals sign (, =),x=3,y=250,w=400,h=30,numblurs=1,textfont=Modern,transparent=true");
#include<thread>
#include<mutex>
#include<atomic>
atomic<int> racetarget = 0;
int racetargets[256] = { 0 };
bool Wait_for_me = true;
std::mutex mtx;
 void testrace(int ind) {
    cout << "thread " << ind << " started\n";
    while (Wait_for_me)Sleep(1);
    mtx.lock();
    cout << "thread " << ind << "go\n";
    foreach(i, 50) {
        racetarget += ind;
        racetargets[ind]+=ind;
    }
    mtx.unlock();
    cout << "thread " << ind << " ended" << endl;
}
 volatile void start_threads() {
     unsigned int MAXINTe = -1;
     unsigned int MAXINT3 = (MAXINTe << 8) >> 8;
     unsigned short maxshort = -1;
     cout << MAXINTe << "   " << MAXINT3 << endl;
     cout << maxshort << endl;
     cout << MAXINT3 - maxshort << endl;
     pause();
    cout << "in start_threads" << endl;
    thread threads[10];
    int i = 0;
    threads[i] = thread(testrace, i); i++;
    threads[i] = thread(testrace, i); i++;
    threads[i] = thread(testrace, i); i++;
    threads[i] = thread(testrace, i); i++;
    threads[i] = thread(testrace, i); i++;
    threads[i] = thread(testrace, i); i++;
    threads[i] = thread(testrace, i); i++;
    threads[i] = thread(testrace, i); i++;
    threads[i] = thread(testrace, i); i++;
    threads[i] = thread(testrace, i); i++;
    //foreach(i, 10) {
    //    cout << "thread " << i << " awaiting\n";
    //     threads[i]= thread(testrace, i);
    //    cout << "thread " << i << " thrown\n";
    //}
    //Sleep(3000);
    Wait_for_me = false;
    cout << "go\n";
    i = 0;
    threads[i].join(); i++;
    threads[i].join(); i++;
    threads[i].join(); i++;
    threads[i].join(); i++;
    threads[i].join(); i++;
    threads[i].join(); i++;
    threads[i].join(); i++;
    threads[i].join(); i++;
    threads[i].join(); i++;
    threads[i].join(); i++;
    //Sleep(3000);
}
 void test_hashfuncs() {
     ValCrypt vc;
     vc.start("none9", 1357);
     //vc.EKout();
     //pause();
     int wrap = 3;
     vc.swapkey_calls = 0;
//     vc.super_swap(13579, 6, wrap);
 //    cout << vc.swapkey_calls << endl;
 //    vc.super_swap(13579, 6, wrap);
 //    cout << vc.swapkey_calls << endl;

 //    vc.super_swap(13579, 6, wrap);
  //   cout << vc.swapkey_calls << endl;
  //   vc.fillSKEY();
     //vc.EKout();
     cout << vc.swapskcalls << endl;
     string ehold = vc.encrypt_Sksr("thereissomeerrorwithspaces is some rotation error in this algo");
     cout << "crypt: " << ehold << endl;
     vc.start("none9", 1357);
     //vc.fillSKEY();
     cout << vc.decrypt_Sksr(ehold) << endl;;

     //vc.EKout();
     cout << vc.swapskcalls << endl;
     pause();
     string hold;
    // foreach(i, 100) {
    //     hold = randstr(10);
    //     cout << vc.crypto_pwhash(hold, 64, 50) << endl;
    // }


     std::string cycle = "I want to be cycled";
     //vc.cycle_stringa(cycle);
     //cout << cycle << endl;
     //pause();
     cout << vc.crypto_pwhash("paso1rd234740", 164, 50) << endl;
     cout << vc.crypto_pwhash("paso1rd234741", 164, 50) << endl;
     cout << vc.crypto_pwhash("paso1rd234731", 164, 50) << endl;
     cout << vc.crypto_pwhash("this is a test of a super long sentence style password", 164, 50) << endl;
     pause();
}


#include<ValWindowEditor.h>
 //ValWindowEditor* valeditor;
#include"Neopets.h"
 bool neo_loginfunc(string uname, string pw, string** status) {
     NeopetsUser* np = new NeopetsUser(uname, pw);
     *status = &(np->status);
     if (!np->init())np->login();
     WindowMan.register_data(np, "neouser", "neouser_" + uname);
     return np->loggedin;
 }
 void editor_thread(void* a) {
     valeditor.start_window_editor_thread();
 }
 void do_valwindoweditor() {
     //valeditor = new ValWindowEditor();
     
     _beginthread(open_VS_hypervisor_window_, 0, neo_loginfunc);
     
     _beginthread(editor_thread, 0, NULL);
     WindowMan.wait_for_splash();
     __VWM_msgpump();
 }
#define testoutt(a) #a
#include<future>
 bool sumfunc(int a) {
     cout << a << endl;
     cout << GetCurrentThreadId() << "\n";
     return true;
}
 

 unsigned long long int G_VS_Rand4(unsigned long long int max) {
     unsigned long long int t = rand() * rand();
     t %= max;
     return t;
 }


 unsigned long long int G_VS_Rand2() {
     unsigned long long int t = G_VS_randseed >> 32;
     t = t ^ ((G_VS_randseed << 32) >> 32);
     if (t % 2 == 0)G_VS_randseed += t;
     else G_VS_randseed -= t;
     cout << "t " << t << " gr " << G_VS_randseed << endl;
     return t ^ G_VS_randseed;
 }
 void G_VS_Rand() {
     unsigned long long int t = G_VS_randseed;
     t*=G_VS_randseed;
     t = t >> 7;
     cout << "t " << t << "  ";
     unsigned long int t2 = ((t/3) + (t/4) + (t/5)) << 2;
     cout << "t2 " << t2 << "  ";
     unsigned long int t3 = t2 % t;
     cout << "t3 " << t3 << "  ";
     unsigned long int t4 = (t2 - t) + t3;
     cout << "t4 " << t4 << "  ";


     if(t3%2==0)G_VS_randseed -= t;
     else G_VS_randseed += t;
     G_VS_randseed = G_VS_randseed ^ t4;
     if (G_VS_randseed == 0)G_VS_randseed = getticks();;

     cout << G_VS_randseed << endl;
 }
 void primegenerator() {
     cout << listfile_numlines("primes.txt") << endl;;
     pause();
     int* primearr = new int[10000000];
     memset(primearr, 0, sizeof(int) * 10000000);
     primearr[0] = 2;
     ofstream ofile("primes.txt");
     int targ = 3;
     int targsqrt = sqrt(targ);
     int len = 1;
     bool yesno = false;
     while (len < 10000000) {
         yesno = false;
         for (int i = 0; i < len && !yesno ; i++) {
             yesno = (targ % primearr[i]) == 0;
             if (targsqrt < primearr[i])break;
         }
         if (!yesno) {
             primearr[len] = targ;
             len++;
             cout << targ << endl;
             ofile << targ << endl;
         }
         targ += 2;
         targsqrt = sqrt(targ);
     }
     cout << "ended loop" << endl;
     delete[] primearr;
     pause();
 }

 inline bool ptestshort(int i) {
     return !(i % 3 == 0 || i % 5 == 0 || i % 7 == 0 || i % 11 == 0);

 }
 //int powmod2(int tis, int exp, int mod) { // adapted from bufferwrapper. recursive powmod
 //    if (exp == 0)return 1;
 //    if (exp % 2 == 0) {
 //        int a = exp / 2;
 //        int b = powmod2(tis, a, mod);
 //        int i = b;
 //        int c = b * i;
 //        int g = c % mod;
 //        return g;
 //    }
 //    else {
 //        int d = exp - 1;
 //        int e = powmod2(tis, d, mod);
 //        int f = tis * e;
 //        int h = f % mod;
 //        return h;
 //    }
 //    return -1; // never
 //}

 //bool val_miller_raben(int r, int n, int q) {
 //    int hold = n - 1;
 //    if (hold == 0)return 1;
 //    int x= (rand()%hold);
 //    x = powmod(x, n, q);
 //    if (x == 1 || x == hold)return 1;

 //    for (int i = 1; i < r; i++) {
 //        x = powmod(x, 2, q);
 //        if (x == hold)return 1;
 //        if (x == 1)return -1;
 //    }
 //    return -1;

 //}
 //
#include<mandelbrot.h>
#include<Valbreakout.h>
 

 void output_asciichart() {
     for (int i = 0; i < 255; i++) {
         cout << i;
         if (i < 100)cout << " ";
         if (i < 10)cout << " ";
         cout << ":" << (char)i<< "  ";
         if (i < 15)cout << endl;
         if (i % 10 == 0)cout << endl;
         if (i % 10 == 0 && i>170)cout << endl;
     }
 }
 void console_drawbox(int x, int y, int w, int h) {
     int bx=0, by=0;
     getxy(bx, by);
     cout << bx << ": " << by << endl;
     gotoxy(x, y);
     cout << (char)201;
     for (int i = 0; i < w - 2; i++)cout << (char)205;
     cout << (char)187;
     
     for (int n = 1; n < h; n++) {
         gotoxy(x, y + n);
         cout << (char)186;
         //for (int i = 0; i < w - 2; i++)cout << (char)176;
         for (int i = 0; i < w - 2; i++)cout << ' ';
         cout << (char)186;
         
     }
     gotoxy(x, y + h);
     cout << (char)200;
     for (int i = 0; i < w - 2; i++)cout << (char)205;
     cout << (char)188;
     gotoxy(bx, by);
 }

 int main_sorttest() {

     slip s = newSLIP();
     slip e = newSLIP();
     slip_link(s, e);
     s->name = "one";
     e->name = "ahh";
     s->last = NULL;
     e->next = NULL;
     slip t;
     for (int i = 0; i < 10; i++) {
         slip_insert(s, randstr((rand() % 6) + 4));
     }
     sliplist_output(s, e);
     slip_remove(s->next);
     pause();
     sliplist_output(s, e);
     pause();
     cout << "sorting\n";
     cout << "sorting size: " << sliplist_msort2(s, e) << endl;
     sliplist_output(s, e);
     pause();


 }
 int main42() {
     _beginthread(calibrate_timer_thread, 0, NULL);
     random_seed();
     Sleep(3000);
     //WindowMan.startup();
     //Sleep(300);
     speedtest_stringcomp();
     pause();
     //RESERVED_GLOBAL_VALDATANODE_BANK_FILL();
     //test_valdataregister();
     unsigned int a = getticks();
     cout << a << endl;
     a = (a << 8) >> 8;
     srand(a%RAND_MAX);
     G_VS_randseed = getticks();
     
     _beginthread(ValBreakout_thread, 0, NULL);
     _beginthread(do_mandelbrot_thread, 0, NULL);
     do_valwindoweditor();
 }
 void breakout_splash(void* a) {
     WindowMan.wait_for_splash();
     _beginthread(ValBreakout_thread, 0, NULL);
 }
 int main() {
    // _beginthread(calibrate_timer_thread, 0, NULL);
    // Sleep(3000);
     string s = "1,000";
     cout << stoint(s) << endl;




     _beginthread(breakout_splash, 0, NULL);
     _beginthread(do_mandelbrot_thread, 0, NULL);
     do_valwindoweditor();
 }

int main2() {
    random_seed();


    int one=0, two = 0,tre=0;
    for (int i = 13; i < 10000; i+=2) {
        if (!ptestshort(i))one++;
        else if (!probprime(i,50))two++;
        else tre++;
    }
    cout << "one,two,tre: " << one << "," << two <<","<<tre<< endl;
    pause();
    _beginthread(calibrate_timer_thread, 0, NULL);
    Sleep(3000);
    unsigned long long int testn = -1;
    cout << testn << endl;
    int arr[500];
    memset(arr, 0, sizeof(int) * 500);
    ValStopwatch watch;
    watch.Restart();
    
    for (int i = 0; i < 10000000; i++) {
        arr[G_VS_Rand3(500)]++;
    
    }
    cout << "my generation time: " <<watch.Lap() << endl;
    
    unsigned long long int min = 111111111, max = 0;
    for (int i = 0; i < 500; i++) {
        cout << "i: "<<i<<" :" << arr[i] << endl;
        if (arr[i] < min)min = arr[i];
        if (arr[i] > max) max = arr[i];
    }
    cout << "min,max: " << min << "," << max << endl;
    pause();
    //_beginthread(calibrate_timer_thread, 0, NULL);
    //Sleep(3000);
    TEST_VALSCHEDULER();
    
 /*
    int p = 3;
    std::future<bool> d = async(sumfunc, p);
    cout << GetCurrentThreadId() << "\n";
    cout << d.get() << endl;
    pause();
   
    string temps = " commas,, commas";
    cout << cslformat(temps) << endl;

    cout << VC_TYPE_tostr(300) << endl;
    stringlist ss;
    auto* s23 = &ss;
    void* ssss = (void*)&ss;
    cout << typeid(&ss).name()<<"  "<<testoutt(&ss) << endl;
    cout << typeid(s23).name() << endl;
    cout << typeid(ssss).name() << endl;
    //pause();
    */
    //__VWM_msgpump();
    WindowMan.wait_for_splash();
    do_valwindoweditor();

    /*
    test_hashfuncs();
    ValCrypt vc;

   
    //mt + V`OhSJw\ & P9 * `}PsC4@ > Y5\h3)gf7Jo & 8P + {Pta4IPress any key to continue . . .
    //Xf | rxHfb3`ut.!YntU"&tzaEcgS& W'DqM#     *Press any key to continue . . .
   //Zm7KpiGk,}_"s$0#9'Stk?T:,BxZ"=Z750-KNQ(!QC*Ma{Nx(PF)oA9x]v+'/"bg-[QZBZH#Lo9@S|B>*r|??8Pf6U(}")!*]w[QZ?y~Q6SEs6rc3%Q$c<_3Ev<B*&F`<A}iJ#QOTS*y;2Qf8^]fyH":}$y;{@8.B:>$7<!3=,z0JQA@vT"@z:)-,$!?Zp*K3H,Q%gs<:Q9$Mi=uH3?&39(d\8uY,Press any key to continue . . .

    //Zl6aqiGk,|^"r50#M&Rtk?T:,YxZ"<)641Uam5( !B=M`{my(QF)oA8y\w+1."cfUZUZY}%"MoI@R|C?9r|>u9Qg6U(}"()*\v[Q)uxP%REs6sbE$U$b=_2Ew=]*'G`=@}hK#mOTR*x:3%g9^ygye";|Ex:z@8/]:?U7<-E=,z0JU@@wU"@{;)E-5)uZq+eA%,%%gsQ;%9$Mi=tI3U&A8(d\8uY,Press any key to continue . . .
    //Zo5mriGk, ]"q90#M%Qtk?T:,YxZ" ? )572Uma5(# - A1Mc{ az(RF)oA; z_t += -"`eUY5ZYq)!NoE@Q|@<9r|=u:Rd6U(}" + % *_u[Q)u {| S % QEs6paE'5$a>_1Et>Q*$D`>C}kH#mOTQ*{90%d:^udyi"8E{9y@8,Q:<e7<-E=,z0J5C@tV"@x8)u.9%uZr(eM),%%gsQ8U9$Mi=wJ3U&M;(d\8uY,
    //XmCmU9l7[@EDUWWIqc!y?E&c->5]EY2^l-Idy["LQX!aidmmA--*uUHM)}TAi[9M*4Eq'-)/4m,{WJx2UJJao9:#e*E.i%LLt8HIZ)28)eM;qJtw5
    //XmCmU9l7[@EDUWWIqc!y?E&c->5]EY2^l-Idy["LQX!aidmmA--*uUHM)}TAi[9M*4Eq'-)/4m,{WJx2UJJao9:#e*E.i%LLt8HIZ)28)eM;qJtw5
    //XmCmU9l7[@EDUWWIqc!y ? E & c->5]EY2^ l - Idy["LQX!aidmmA--*uUHM)}TAi[9M*4Eq'-)/4m,{WJx2UJJao9:#e*E.i%LLt8HIZ)28)eM;qJtw5
    //f9^ v7 = 9e@\ $ ? -DL(1Ue"eA}`!mIUV0Reuxk>j%U5#JBF*kU\3~qJ%${A#GQh.nqUYUUf-}_:%=QZ*aX0h0eO4>M=mhj Y_N#@{\E4)Ii)Q)ehuw"F * QpF_ = U + U / > Ue + A > a@ * ae70A + 7UOM$r}al)J < ~OHZf ? Ay)IYY < 0wF) t6R:FaV ? H - c}xM2 = IVmxU~'Q%q1AnS><14e6Pia^aVX<9Af+1icDT7UYNDu`5Yu% Q;I
    //f9^v7=9e@\ $?-DL(1Ue"eA}`!mIUV0Reuxk>j%U5#JBF*kU\3~qJ%${A#GQh.nqUYUUf-}_:%=QZ*aX0h0eO4>M=mhj Y_N#@{\E4)Ii)Q)ehuw"F*QpF_=U+U/>Ue+A>a@*ae70A+7UOM$r}al)J<~OHZf?Ay)IYY<0wF) t6R:FaV?H-c}xM2=IVmxU~'Q%q1AnS><14e6Pia^aVX<9Af+1icDT7UYNDu`5Yu% Q;I
    //V,:U:'W\e/rzMrq%)5]yQOM.N_&Lu(ATVMU-9e=nh(8tV:I61d}=(-3a\a#aU%4dp7Im|QyRU}qSU2^-4D0RUYKnfivivQd<HS~6e+z`;kW=ZQ?Y%_e$8UIqU0kS1e_iQ#H2^;P1XxevNuw1T=Ea)eMI.|Q'}oJ10nG']"MyUSsCEI*Cuv@Gte]kXeu>Ip,eFU|eYk`YAe)Ueah5!jkNEbu%RFS1%5U}T^)34F2G:g^R,
    //7PEQ.zA,U?H\uI#0"tr5iUbx+KEaoX=H\PvA9XYwwK! UhVmym6e<XNl/N>4'0,G1_)-t4b}5Eu%S\I:!y5e9h}XQ(%AQ-49Y6eA_%uV3Mgl)!1e%4W|-ey.B'=)"P<7&9jEAI%Ada'l<t5hTYUt}5!)!u"iEAD}\BpumULU1_e])yny%CvAP|A<8)TMBq/1PIU^I$f5f>T9fU2#69SP~MT=au=MadW,J}nO7),G+Ru\bT{

    cout << vc.crypto_pwhash("paso1rd234740", 256, 50) << endl;

    pause();
    cout << "starting threads" << endl;
    start_threads();
    //Sleep(3000);
    cout << "racetarget:" << racetarget << endl;
    int tot = 0;
    foreach(i, 256)tot += racetargets[i];
    cout << "total " << tot << endl;
    pause();
    */
    cout << "before createsplash" << endl;
    string s = "mario.bmp";
    s = "testing_splash.jpg";
    WindowMan.create_splash(s, 1000, 0, 0, 0, 0);
    WindowMan.wait_for_splash();
    
    //_beginthread(do_msg_loopA_thread, 0, NULL);
    _beginthread(draw_login_window, 0, NULL);
    __VWM_msgpump();
    //draw_login_window(NULL);
    pause();
    do_msg_loopA();
}

int main3() {

    cout << "******************************************************************************\n";
cout << "****************************************** (c) Delta Infiniti Programming  ***\n";

cout << "****   ************  ********** |**** |***************************************\n";

cout << "***  *  *********      ******** |\\*** |***************************************\n";

cout << "**  ***  **********  ********** |*\\** |***************************************\n";

cout << "**       **  **  **  ***    *** |**\\* |** |-- ****__**************************\n";
cout << "**  ***  **  **  **  **  **  ** |***\\ |** |-****/ ** \\************************\n";

cout << "**  ***  **    ****  ***    *** |****\\|** |-- **\\ __ /************************\n";
cout << "******************************************************************************\n";

cout << "*    *  *****  *******  \\      / /\\ \\    __   __  __  __  ___                *\n";

cout << "*  *  *  ***  ********   \\    / /  \\ \\  /  \\ |   |   |  \\  |    /\\  |\\  |    *\n";
cout << "*    ***  *  ****  ***    \\  / /^^^^\\ \\ \\__  |-  |-  |  |  |   /__\\ | \\ |    *\n";

cout << "*  *  ***   **********     \\/ /      \\ \\___\\ |__ |__ |__/ _|_ /    \\|  \\|    *\n";
cout << "*    ****   *****  ***                                                       *\n";  

cout << "**********************  _____                                                *\n";
cout << "*  Produced For:    ** |                                                     *\n";
cout << "* HiddenBelow.com   ** |          __  _                                      *\n";

cout << "********************** |--   -|- |   | \\ |\\  |   /\\ \\                        *\n";
cout << "*** V E R S I O N : ** |      |  |-  |_/ | \\ |  /__\\ \\                       *\n";
   
cout << "***      000       *** |_____ |  |__ | \\ |  \\| /    \\ \\__                    *\n";
cout << "******************************************************************************\n";
    
    //fix_varlist_newlines("itemsdatabaseb.di", "itemnames.di");
    //varlist_to_nameslist("itemsdatabaseb.di", "itemnames.di");
    SHOW(sizeof(ValWindow));
    SHOW(sizeof(ValControl));
    SHOW(sizeof(ValGFX));
    SHOW(sizeof(ValEventHandler));
    SHOW(sizeof(void*));
    /*
    //ValWindow
    ValGFX valg;
    valg.load_image("testing_splash.jpg");

    valg.blur();
    valg.blur();
    valg.blur();
    cout << "after blur" << endl;
    //scrape_jellyneo_itemdatabase();
    //urlencode
    cout << "ext: " << extension("blah.jpg");
    */


    //Sleep(3000);
    random_seed();


    pause();
    //make_huge_dedupe_testfile();
    //Sleep(1000);
    //cout << "username list size: " << listfile_numlines_dedupe("dedupetest.txt","dedupe3.txt");
    cout << "before createsplash" << endl;
    string s = "mario.bmp";
    s = "testing_splash.jpg";
    WindowMan.create_splash(s, 1000, 0, 0, 0, 0);
    WindowMan.wait_for_splash();
    //pause();
    /*binary unfolding
    * at 3 set middle
    * at 5 inc middle set first branch middle -1
    * at

    */
    //string mario = "mario.bmp";
    //_beginthread(splashthread, 0, (void*)&mario);
    //pause();
    int height = 500, width = 500;
    ValWindow* wind = WindowMan.create_new_window("testing", 300, 300, height, width, 1);
    wind->is_master = true;
    width -= 20;
    height -= 40;
    //NeopetsUser user("valzeediann", "FuckNeo12");
    wind->add_global_stat_int(VC_GLOBAL_STAT_STRING, "status", "status:*S", VRECT{ 0, height - 25, 450,25 }, (void*)&user.status);
    //wind->add_global_stat_int(VC_GLOBAL_STAT_STRING, "status", "status:*S", VRECT{ 250, height-25, 250,25 }, (void*)&global_status);
    int offsetx = 45, offsety = 200;
    wind->add_control_ex(VC_LABEL, "warning_label", "this is a master window. if you click the 'x' button, the program will exit", VRECT{ 0,0,500,25 }, VC_NOFUNC, false);
    wind->add_control_ex(VC_LABEL, "username_label", "username:", VRECT{ offsetx,offsety,65,25 }, VC_NOFUNC, false);
    wind->add_control_ex(VC_LABEL, "password_label", "password:", VRECT{ offsetx,offsety + 30,65,25 }, VC_NOFUNC, false);
    wind->add_editctrl("username", VRECT{ offsetx + 70, offsety, 150, 25 }, "valzeediann");  // fill autofill info from master data. 

    wind->add_passwordctrl("password", VRECT{ offsetx + 70, offsety + 30, 150,25 }, "login_button");
    //wind->add_control_ex(VC_BUTTON,"login","login",VRECT{160,5,50,50},)
    wind->add_control_ex(VC_BUTTON, "login_button", "login", VRECT{ offsetx + 225,offsety,100,55 }, (void*)&neologin, true);
    wind->add_control_ex(VC_BUTTON, "boards", "get names", VRECT{ 40,320,350,75 }, (void*)&neoboardscrape, true);
    ValWindow* wind2 = WindowMan.create_new_window("testing2", 600, 600, 300, 300, 1);
    ValGFX gfxtemp;
    gfxtemp.load_image("mario.bmp");
    gfxtemp.CopySectionToGFX(wind2->mygfx, 50, 50, 200, 200, 50, 50);
    wind2->GFXdraw();
    wind2->add_control_ex(VC_BUTTON, "login", "login", VRECT{ 180,5,100,25 }, (void*)&neologin, true);
    
    //    _beginthread(outstatus, 0, (void*)&user);
        //user.login();
        //if (!user.init())user.login();
    cout << "returned to the main thread.  " << endl;
    do_msg_loopA();

    pause();
    return 0;
}
void neostart() {
    valwrapper inet("www.neopets.com");
    inet.get("/");
    Write_File("out.html", inet.cursource);
    inet.get("/login/");
    Sleep(1000);
    inet.post("/login.phtml", "username=valzeediann&password=FuckNeo12&templateLoginDest=%2Flogin%2F");
    cout << inet.headers << endl;
    Write_File("out3.html", inet.cursource);
    inet.get("/");
    Write_File("out2.html", inet.cursource);
}





