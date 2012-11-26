#include<windows.h>
#include<gl/glut.h>
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include"Vec3.h"
using namespace OpenSteer;
//#define Max_SV 2

HGLRC           hRC=NULL;							// 窗口着色描述表句柄
HDC             hDC=NULL;							// OpenGL渲染描述表句柄
HWND            hWnd=NULL;							// 保存我们的窗口句柄
HINSTANCE       hInstance;							// 保存程序的实例

bool	keys[256];								// 保存键盘按键的数组
bool	active=TRUE;								// 窗口的活动标志，缺省为TRUE
bool	fullscreen=TRUE;							// 全屏标志缺省，缺省设定成全屏模式

//GLUquadricObj *cylinder_obj;              //Quadratic object to render the cylinders

Vec3 target=Vec3(1.5,0.0,-6.0);//简单定义目标点
class obstacle//障碍物类，定义了坐标以及包围球半径
{
public:
	obstacle()//构造函数
	{
		o_position=Vec3(1.0,0.4,-6.0);
		radius=0.3;
	}
	~obstacle(){}//析构函数，如果定义为基类，请定义为virtual
	Vec3 o_position;//坐标
	float radius;//包围球半径
};
obstacle OB;

class SimpleVehicle//观察点类
{
public:
	SimpleVehicle()//构造函数
	{
		m_position=Vec3(-1.5,1.5,-6.0);
		m_velocity=Vec3(0.0,0.0,0.0);
		mass=5.0;
		max_force=0.00005;
		max_speed=0.00005;
	}
	virtual ~SimpleVehicle(){}//析构函数
	Vec3 steerForSeek (const Vec3& target)//Seek算法，具体请看说明文档
	{
		const Vec3 desiredVelocity = target - m_position;
		return desiredVelocity -m_velocity;
	}//返回引导力

	Vec3 truncate(const Vec3 &a,const float& b)
	{
		return a.length()>b ? a:a.normalize()*b;
	}//截断函数，自己写的，当a的大小比b大时将a的大小变成b，a的方向不变

	void next_position(const Vec3 steering_direction)
	{
		steering_force=truncate(steering_direction,max_force);//引导力		
		acceleration=steering_force/mass;//加速度
		m_velocity=0.001*truncate(m_velocity+acceleration,max_speed);//前面乘了0.001是因为在本程序中速度相对太大了（二维显示的缘故，我估计）
		m_position=m_position+m_velocity;//下一点坐标
	}//计算模拟后的下一点坐标，传入参数是引导力

	Vec3 avoid(const obstacle &OB)
	{
		float dist=(OB.o_position-m_position).length();//计算观察点与障碍物的距离，与包围球半径做比较，大于则无引导力，小于则返回引导力
		if(dist>OB.radius)
			return Vec3(0,0,0);
		else
			return -(OB.o_position-m_position)*(1/dist);//引导力方向由障碍物重心指向观察点，大小我这里取了距离的倒数
	}
	//简化的基于包围球的碰撞避免算法，这个算法很很多缺陷例如：
	//1、试想如果观察点速度方向由障碍物指向目标点，那么物体会在障碍物与物体现在点连线的某一点上停住不动，解决办法：改变引导力方向变成始终垂直于速度方向
	//2、没有提前预测，这个算法是在物体进入包围球以后才开始检测的，正确的方法应该是预测物体的下一点坐标，如果在包围球内则施加引导力。
	//3、如果物体速度过快，那么很有可能在一步模拟内穿过障碍物，当然，这点可以通过定义最大速度和包围球半径避免。但当考虑诸如墙面的时候，包围球并不完全适用。
	//4、引导力大小的问题，距离倒数的思想没有错，但是应该是物体离包围面的距离，而不是物体离障碍物中心的距离

	Vec3 m_position;//观察点坐标
	Vec3 m_velocity;//观察点速度
	float mass;//观察点质量
	float max_force;//最大引导力，本身决定
	float max_speed;//最大速度，同上
	Vec3 steering_force;//模拟的引导力
	Vec3 acceleration;//加速度

};

SimpleVehicle SV;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);				// WndProc的定义

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)				// 重置OpenGL窗口大小
{
	if (height==0)								// 防止被零除
	{
		height=1;							// 将Height设为1
	}

	glViewport(0, 0, width, height);					// 重置当前的视口
	glMatrixMode(GL_PROJECTION);						// 选择投影矩阵
	glLoadIdentity();							// 重置投影矩阵

	// 设置视口的大小
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);						// 选择模型观察矩阵
	glLoadIdentity();							// 重置模型观察矩阵
}

int InitGL(GLvoid)								// 此处开始对OpenGL进行所有设置
{
	glShadeModel(GL_SMOOTH);						// 启用阴影平滑
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);					// 黑色背景
	glClearDepth(1.0f);							// 设置深度缓存
	glEnable(GL_DEPTH_TEST);						// 启用深度测试
	glDepthFunc(GL_LEQUAL);							// 所作深度测试的类型
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// 告诉系统对透视进行修正
	return TRUE;								// 初始化 OK
}

int DrawGLScene(GLvoid)								// 从这里开始进行所有的绘制
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// 清除屏幕和深度缓存
	glLoadIdentity();							// 重置当前的模型观察矩阵
	glPushMatrix();
	glTranslatef(target.x,target.y,target.z);//将当前绘图中心移到target处						
	glColor3f(1.0f,1.0f,1.0f);//选择颜色
	glBegin(GL_QUADS);							//  绘制正方形
	glVertex3f(-0.1f, 0.1f, 0.0f);					// 左上
	glVertex3f( 0.1f, 0.1f, 0.0f);					// 右上
	glVertex3f( 0.1f,-0.1f, 0.0f);					// 左下
	glVertex3f(-0.1f,-0.1f, 0.0f);					// 右下
	glEnd();								// 正方形绘制结束
	glPopMatrix();
	glPushMatrix();
	glTranslatef(SV.m_position.x,SV.m_position.y,SV.m_position.z);	//将当前绘图中心移到SV处					
	glColor3f(1.0f,0.0f,1.0f);//选择颜色
	glBegin(GL_QUADS);							//  绘制正方形
	glVertex3f(-0.1f, 0.1f, 0.0f);					// 左上
	glVertex3f( 0.1f, 0.1f, 0.0f);					// 右上
	glVertex3f( 0.1f,-0.1f, 0.0f);					// 左下
	glVertex3f(-0.1f,-0.1f, 0.0f);					// 右下
	glEnd();								// 正方形绘制结束
	glPopMatrix();
	glPushMatrix();
	glTranslatef(OB.o_position.x,OB.o_position.y,OB.o_position.z);//将当前绘图中心移到OB处						
	glColor3f(1.0f,1.0f,0.0f);//选择颜色
	glBegin(GL_QUADS);							//  绘制正方形
	glVertex3f(-0.1f, 0.1f, 0.0f);					// 左上
	glVertex3f( 0.1f, 0.1f, 0.0f);					// 右上
	glVertex3f( 0.1f,-0.1f, 0.0f);					// 左下
	glVertex3f(-0.1f,-0.1f, 0.0f);					// 右下
	glEnd();								// 正方形绘制结束
	glPopMatrix();
	Vec3 steering_direction=SV.steerForSeek(target);//计算seek产生的引导力
	Vec3 avoid_direction=SV.avoid(OB);//计算avoid产生的引导力
	SV.next_position(steering_direction+avoid_direction);//计算下一点坐标,传入参数是两个力之和

	return TRUE;								//  一切 OK
}

GLvoid KillGLWindow(GLvoid)							// 正常销毁窗口
{
	if (fullscreen)								// 我们处于全屏模式吗?
	{
		ChangeDisplaySettings(NULL,0);					// 是的话，切换回桌面
		ShowCursor(TRUE);						// 显示鼠标指针
	}

	if (hRC)								// 我们拥有OpenGL渲染描述表吗?
	{
		if (!wglMakeCurrent(NULL,NULL))					// 我们能否释放DC和RC描述表?
		{
			MessageBox(NULL,"释放DC或RC失败。","关闭错误",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))					// 我们能否删除RC?
		{
			MessageBox(NULL,"释放RC失败。","关闭错误",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;							// 将RC设为 NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// 我们能否释放 DC?
	{
		MessageBox(NULL,"释放DC失败。","关闭错误",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;							// 将 DC 设为 NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// 能否销毁窗口?
	{
		MessageBox(NULL,"释放窗口句柄失败。","关闭错误",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;							// 将 hWnd 设为 NULL
	}

	if (!UnregisterClass("OpenG",hInstance))				// 能否注销类?
	{
		MessageBox(NULL,"不能注销窗口类。","关闭错误",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;							// 将 hInstance 设为 NULL
	}
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;						// 保存查找匹配的结果
	WNDCLASS	wc;							// 窗口类结构
	DWORD		dwExStyle;						// 扩展窗口风格
	DWORD		dwStyle;						// 窗口风格
	RECT WindowRect;							// 取得矩形的左上角和右下角的坐标值
	WindowRect.left=(long)0;						// 将Left   设为 0
	WindowRect.right=(long)width;						// 将Right  设为要求的宽度
	WindowRect.top=(long)0;							// 将Top    设为 0
	WindowRect.bottom=(long)height;						// 将Bottom 设为要求的高度
	fullscreen=fullscreenflag;						// 设置全局全屏标志
	hInstance		= GetModuleHandle(NULL);			// 取得我们窗口的实例
	wc.style		= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;		// 移动时重画，并为窗口取得DC
	wc.lpfnWndProc		= (WNDPROC) WndProc;				// WndProc处理消息
	wc.cbClsExtra		= 0;						// 无额外窗口数据
	wc.cbWndExtra		= 0;						// 无额外窗口数据
	wc.hInstance		= hInstance;					// 设置实例
	wc.hIcon		= LoadIcon(NULL, IDI_WINLOGO);			// 装入缺省图标
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);			// 装入鼠标指针
	wc.hbrBackground	= NULL;						// GL不需要背景
	wc.lpszMenuName		= NULL;						// 不需要菜单
	wc.lpszClassName	= "OpenG";					// 设定类名字
	if (!RegisterClass(&wc))						// 尝试注册窗口类
	{
		MessageBox(NULL,"注册窗口失败","错误",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 退出并返回FALSE
	}
	if (fullscreen)								// 要尝试全屏模式吗?
	{
		DEVMODE dmScreenSettings;						// 设备模式
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));			// 确保内存清空为零
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);			// Devmode 结构的大小
		dmScreenSettings.dmPelsWidth	= width;				// 所选屏幕宽度
		dmScreenSettings.dmPelsHeight	= height;				// 所选屏幕高度
		dmScreenSettings.dmBitsPerPel	= bits;					// 每象素所选的色彩深度
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
		// 尝试设置显示模式并返回结果。注: CDS_FULLSCREEN 移去了状态条。
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// 若模式失败，提供两个选项：退出或在窗口内运行。
			if (MessageBox(NULL,"全屏模式在当前显卡上设置失败！\n使用窗口模式？","NeHe G",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;				// 选择窗口模式(Fullscreen=FALSE)
			}
			else
			{
				// 弹出一个对话框，告诉用户程序结束
				MessageBox(NULL,"程序将被关闭","错误",MB_OK|MB_ICONSTOP);
				return FALSE;					//  退出并返回 FALSE
			}
		}
	}

	if (fullscreen)								// 仍处于全屏模式吗?
	{
		dwExStyle=WS_EX_APPWINDOW;					// 扩展窗体风格
		dwStyle=WS_POPUP;						// 窗体风格
		ShowCursor(FALSE);						// 隐藏鼠标指针
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// 扩展窗体风格
		dwStyle=WS_OVERLAPPEDWINDOW;					//  窗体风格
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// 调整窗口达到真正要求的大小

	if (!(hWnd=CreateWindowEx(	dwExStyle,				// 扩展窗体风格
		"OpenG",				// 类名字
		title,					// 窗口标题
		WS_CLIPSIBLINGS |			// 必须的窗体风格属性
		WS_CLIPCHILDREN |			// 必须的窗体风格属性
		dwStyle,				// 选择的窗体属性
		0, 0,					// 窗口位置
		WindowRect.right-WindowRect.left,	// 计算调整好的窗口宽度
		WindowRect.bottom-WindowRect.top,	// 计算调整好的窗口高度
		NULL,					// 无父窗口
		NULL,					// 无菜单
		hInstance,				// 实例
		NULL)))					// 不向WM_CREATE传递任何东东
	{
		KillGLWindow();							// 重置显示区
		MessageBox(NULL,"不能创建一个窗口设备描述表","错误",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=					// /pfd 告诉窗口我们所希望的东东，即窗口使用的像素格式
	{
		sizeof(PIXELFORMATDESCRIPTOR),					// 上述格式描述符的大小
		1,								// 版本号
		PFD_DRAW_TO_WINDOW |						// 格式支持窗口
		PFD_SUPPORT_OPENGL |						// 格式必须支持OpenGL
		PFD_DOUBLEBUFFER,						// 必须支持双缓冲
		PFD_TYPE_RGBA,							// 申请 RGBA 格式
		bits,								// 选定色彩深度
		0, 0, 0, 0, 0, 0,						// 忽略的色彩位
		0,								// 无Alpha缓存
		0,								// 忽略Shift Bit
		0,								// 无累加缓存
		0, 0, 0, 0,							// 忽略聚集位
		16,								// 16位 Z-缓存 (深度缓存)
		0,								// 无蒙板缓存
		0,								// 无辅助缓存
		PFD_MAIN_PLANE,							// 主绘图层
		0,								// Reserved
		0, 0, 0								// 忽略层遮罩
	};

	if (!(hDC=GetDC(hWnd)))							// 取得设备描述表了么?
	{
		KillGLWindow();							// 重置显示区
		MessageBox(NULL,"不能创建一种相匹配的像素格式","错误",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))				// Windows 找到相应的象素格式了吗?
	{
		KillGLWindow();							// 重置显示区
		MessageBox(NULL,"不能设置像素格式","错误",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))				// 能够设置象素格式么?
	{
		KillGLWindow();							// 重置显示区
		MessageBox(NULL,"不能设置像素格式","错误",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))					// 能否取得着色描述表?
	{
		KillGLWindow();							// 重置显示区
		MessageBox(NULL,"不能创建OpenGL渲染描述表","错误",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))						// 尝试激活着色描述表
	{
		KillGLWindow();							// 重置显示区
		MessageBox(NULL,"不能激活当前的OpenGL渲然描述表","错误",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// 显示窗口
	SetForegroundWindow(hWnd);						// 略略提高优先级
	SetFocus(hWnd);								// 设置键盘的焦点至此窗口
	ReSizeGLScene(width, height);						// 设置透视 GL 屏幕

	if (!InitGL())								// 初始化新建的GL窗口
	{
		KillGLWindow();							// 重置显示区
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	return TRUE;								// 成功
}

LRESULT CALLBACK WndProc(	HWND	hWnd,					// 窗口的句柄
	UINT	uMsg,					// 窗口的消息
	WPARAM	wParam,					// 附加的消息内容
	LPARAM	lParam)					// 附加的消息内容
{

	switch (uMsg)								// 检查Windows消息
	{

	case WM_ACTIVATE:						// 监视窗口激活消息
		{
			if (!HIWORD(wParam))					// 检查最小化状态
			{
				active=TRUE;					// 程序处于激活状态
			}
			else
			{
				active=FALSE;					// 程序不再激活
			}

			return 0;						// 返回消息循环
		}

	case WM_SYSCOMMAND:						// 系统中断命令
		{
			switch (wParam)						// 检查系统调用
			{
			case SC_SCREENSAVE:				// 屏保要运行?
			case SC_MONITORPOWER:				// 显示器要进入节电模式?
				return 0;					// 阻止发生
			}
			break;							// 退出
		}

	case WM_CLOSE:							// 收到Close消息?
		{
			PostQuitMessage(0);					// 发出退出消息
			return 0;						// 返回
		}

	case WM_KEYDOWN:						// 有键按下么?
		{
			keys[wParam] = TRUE;					// 如果是，设为TRUE
			return 0;						// 返回
		}

	case WM_KEYUP:							// 有键放开么?
		{
			keys[wParam] = FALSE;					// 如果是，设为FALSE
			return 0;						// 返回
		}

	case WM_SIZE:							// 调整OpenGL窗口大小
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));		// LoWord=Width,HiWord=Height
			return 0;						// 返回
		}
	}
	// 向 DefWindowProc传递所有未处理的消息。
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,				// 当前窗口实例
	HINSTANCE	hPrevInstance,				// 前一个窗口实例
	LPSTR		lpCmdLine,				// 命令行参数
	int		nCmdShow)				// 窗口显示状态
{
	MSG	msg;								// Windowsx消息结构
	BOOL	done=FALSE;							// 用来退出循环的Bool 变量
	// 提示用户选择运行模式
	if (MessageBox(NULL,"你想在全屏模式下运行么？", "设置全屏模式",MB_YESNO|MB_ICONQUESTION)==IDNO)
	{
		fullscreen=FALSE;						// FALSE为窗口模式
	}
	// 创建OpenGL窗口
	if (!CreateGLWindow("opensteer算法演示1",800,600,16,fullscreen))
	{
		return 0;							// 失败退出
	}
	while(!done)								// 保持循环直到 done=TRUE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))			// 有消息在等待吗?
		{
			if (msg.message==WM_QUIT)				// 收到退出消息?
			{
				done=TRUE;					// 是，则done=TRUE
			}
			else							// 不是，处理窗口消息
			{
				TranslateMessage(&msg);				// 翻译消息
				DispatchMessage(&msg);				// 发送消息
			}
		}
		else								// 如果没有消息
		{
			// 绘制场景。监视ESC键和来自DrawGLScene()的退出消息
			if (active)						// 程序激活的么?
			{
				if (keys[VK_ESCAPE])				// ESC 按下了么?
				{
					done=TRUE;				// ESC 发出退出信号
				}
				else						// 不是退出的时候，刷新屏幕
				{
					DrawGLScene();				// 绘制场景
					SwapBuffers(hDC);			// 交换缓存 (双缓存)
				}
			}
			if (keys[VK_F1])					// F1键按下了么?
			{
				keys[VK_F1]=FALSE;				// 若是，使对应的Key数组中的值为 FALSE
				KillGLWindow();					// 销毁当前的窗口
				fullscreen=!fullscreen;				// 切换 全屏 / 窗口 模式
				// 重建 OpenGL 窗口
				if (!CreateGLWindow("NeHe's OpenGL 程序框架",640,480,16,fullscreen))
				{
					return 0;				// 如果窗口未能创建，程序退出
				}
			}
		}
	}
	// 关闭程序
	KillGLWindow();								// 销毁窗口
	return (msg.wParam);							// 退出程序
}