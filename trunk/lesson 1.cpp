#include<windows.h>
#include<gl/glut.h>
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include"Vec3.h"
using namespace OpenSteer;
//#define Max_SV 2

HGLRC           hRC=NULL;							// ������ɫ��������
HDC             hDC=NULL;							// OpenGL��Ⱦ��������
HWND            hWnd=NULL;							// �������ǵĴ��ھ��
HINSTANCE       hInstance;							// ��������ʵ��

bool	keys[256];								// ������̰���������
bool	active=TRUE;								// ���ڵĻ��־��ȱʡΪTRUE
bool	fullscreen=TRUE;							// ȫ����־ȱʡ��ȱʡ�趨��ȫ��ģʽ

//GLUquadricObj *cylinder_obj;              //Quadratic object to render the cylinders

Vec3 target=Vec3(1.5,0.0,-6.0);//�򵥶���Ŀ���
class obstacle//�ϰ����࣬�����������Լ���Χ��뾶
{
public:
	obstacle()//���캯��
	{
		o_position=Vec3(1.0,0.4,-6.0);
		radius=0.3;
	}
	~obstacle(){}//�����������������Ϊ���࣬�붨��Ϊvirtual
	Vec3 o_position;//����
	float radius;//��Χ��뾶
};
obstacle OB;

class SimpleVehicle//�۲����
{
public:
	SimpleVehicle()//���캯��
	{
		m_position=Vec3(-1.5,1.5,-6.0);
		m_velocity=Vec3(0.0,0.0,0.0);
		mass=5.0;
		max_force=0.00005;
		max_speed=0.00005;
	}
	virtual ~SimpleVehicle(){}//��������
	Vec3 steerForSeek (const Vec3& target)//Seek�㷨�������뿴˵���ĵ�
	{
		const Vec3 desiredVelocity = target - m_position;
		return desiredVelocity -m_velocity;
	}//����������

	Vec3 truncate(const Vec3 &a,const float& b)
	{
		return a.length()>b ? a:a.normalize()*b;
	}//�ضϺ������Լ�д�ģ���a�Ĵ�С��b��ʱ��a�Ĵ�С���b��a�ķ��򲻱�

	void next_position(const Vec3 steering_direction)
	{
		steering_force=truncate(steering_direction,max_force);//������		
		acceleration=steering_force/mass;//���ٶ�
		m_velocity=0.001*truncate(m_velocity+acceleration,max_speed);//ǰ�����0.001����Ϊ�ڱ��������ٶ����̫���ˣ���ά��ʾ��Ե�ʣ��ҹ��ƣ�
		m_position=m_position+m_velocity;//��һ������
	}//����ģ������һ�����꣬���������������

	Vec3 avoid(const obstacle &OB)
	{
		float dist=(OB.o_position-m_position).length();//����۲�����ϰ���ľ��룬���Χ��뾶���Ƚϣ�����������������С���򷵻�������
		if(dist>OB.radius)
			return Vec3(0,0,0);
		else
			return -(OB.o_position-m_position)*(1/dist);//�������������ϰ�������ָ��۲�㣬��С������ȡ�˾���ĵ���
	}
	//�򻯵Ļ��ڰ�Χ�����ײ�����㷨������㷨�ܺܶ�ȱ�����磺
	//1����������۲���ٶȷ������ϰ���ָ��Ŀ��㣬��ô��������ϰ������������ڵ����ߵ�ĳһ����ͣס����������취���ı�������������ʼ�մ�ֱ���ٶȷ���
	//2��û����ǰԤ�⣬����㷨������������Χ���Ժ�ſ�ʼ���ģ���ȷ�ķ���Ӧ����Ԥ���������һ�����꣬����ڰ�Χ������ʩ����������
	//3����������ٶȹ��죬��ô���п�����һ��ģ���ڴ����ϰ����Ȼ��������ͨ����������ٶȺͰ�Χ��뾶���⡣������������ǽ���ʱ�򣬰�Χ�򲢲���ȫ���á�
	//4����������С�����⣬���뵹����˼��û�д�����Ӧ�����������Χ��ľ��룬�������������ϰ������ĵľ���

	Vec3 m_position;//�۲������
	Vec3 m_velocity;//�۲���ٶ�
	float mass;//�۲������
	float max_force;//������������������
	float max_speed;//����ٶȣ�ͬ��
	Vec3 steering_force;//ģ���������
	Vec3 acceleration;//���ٶ�

};

SimpleVehicle SV;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);				// WndProc�Ķ���

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)				// ����OpenGL���ڴ�С
{
	if (height==0)								// ��ֹ�����
	{
		height=1;							// ��Height��Ϊ1
	}

	glViewport(0, 0, width, height);					// ���õ�ǰ���ӿ�
	glMatrixMode(GL_PROJECTION);						// ѡ��ͶӰ����
	glLoadIdentity();							// ����ͶӰ����

	// �����ӿڵĴ�С
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);						// ѡ��ģ�͹۲����
	glLoadIdentity();							// ����ģ�͹۲����
}

int InitGL(GLvoid)								// �˴���ʼ��OpenGL������������
{
	glShadeModel(GL_SMOOTH);						// ������Ӱƽ��
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);					// ��ɫ����
	glClearDepth(1.0f);							// ������Ȼ���
	glEnable(GL_DEPTH_TEST);						// ������Ȳ���
	glDepthFunc(GL_LEQUAL);							// ������Ȳ��Ե�����
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// ����ϵͳ��͸�ӽ�������
	return TRUE;								// ��ʼ�� OK
}

int DrawGLScene(GLvoid)								// �����￪ʼ�������еĻ���
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// �����Ļ����Ȼ���
	glLoadIdentity();							// ���õ�ǰ��ģ�͹۲����
	glPushMatrix();
	glTranslatef(target.x,target.y,target.z);//����ǰ��ͼ�����Ƶ�target��						
	glColor3f(1.0f,1.0f,1.0f);//ѡ����ɫ
	glBegin(GL_QUADS);							//  ����������
	glVertex3f(-0.1f, 0.1f, 0.0f);					// ����
	glVertex3f( 0.1f, 0.1f, 0.0f);					// ����
	glVertex3f( 0.1f,-0.1f, 0.0f);					// ����
	glVertex3f(-0.1f,-0.1f, 0.0f);					// ����
	glEnd();								// �����λ��ƽ���
	glPopMatrix();
	glPushMatrix();
	glTranslatef(SV.m_position.x,SV.m_position.y,SV.m_position.z);	//����ǰ��ͼ�����Ƶ�SV��					
	glColor3f(1.0f,0.0f,1.0f);//ѡ����ɫ
	glBegin(GL_QUADS);							//  ����������
	glVertex3f(-0.1f, 0.1f, 0.0f);					// ����
	glVertex3f( 0.1f, 0.1f, 0.0f);					// ����
	glVertex3f( 0.1f,-0.1f, 0.0f);					// ����
	glVertex3f(-0.1f,-0.1f, 0.0f);					// ����
	glEnd();								// �����λ��ƽ���
	glPopMatrix();
	glPushMatrix();
	glTranslatef(OB.o_position.x,OB.o_position.y,OB.o_position.z);//����ǰ��ͼ�����Ƶ�OB��						
	glColor3f(1.0f,1.0f,0.0f);//ѡ����ɫ
	glBegin(GL_QUADS);							//  ����������
	glVertex3f(-0.1f, 0.1f, 0.0f);					// ����
	glVertex3f( 0.1f, 0.1f, 0.0f);					// ����
	glVertex3f( 0.1f,-0.1f, 0.0f);					// ����
	glVertex3f(-0.1f,-0.1f, 0.0f);					// ����
	glEnd();								// �����λ��ƽ���
	glPopMatrix();
	Vec3 steering_direction=SV.steerForSeek(target);//����seek������������
	Vec3 avoid_direction=SV.avoid(OB);//����avoid������������
	SV.next_position(steering_direction+avoid_direction);//������һ������,���������������֮��

	return TRUE;								//  һ�� OK
}

GLvoid KillGLWindow(GLvoid)							// �������ٴ���
{
	if (fullscreen)								// ���Ǵ���ȫ��ģʽ��?
	{
		ChangeDisplaySettings(NULL,0);					// �ǵĻ����л�������
		ShowCursor(TRUE);						// ��ʾ���ָ��
	}

	if (hRC)								// ����ӵ��OpenGL��Ⱦ��������?
	{
		if (!wglMakeCurrent(NULL,NULL))					// �����ܷ��ͷ�DC��RC������?
		{
			MessageBox(NULL,"�ͷ�DC��RCʧ�ܡ�","�رմ���",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))					// �����ܷ�ɾ��RC?
		{
			MessageBox(NULL,"�ͷ�RCʧ�ܡ�","�رմ���",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;							// ��RC��Ϊ NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// �����ܷ��ͷ� DC?
	{
		MessageBox(NULL,"�ͷ�DCʧ�ܡ�","�رմ���",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;							// �� DC ��Ϊ NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// �ܷ����ٴ���?
	{
		MessageBox(NULL,"�ͷŴ��ھ��ʧ�ܡ�","�رմ���",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;							// �� hWnd ��Ϊ NULL
	}

	if (!UnregisterClass("OpenG",hInstance))				// �ܷ�ע����?
	{
		MessageBox(NULL,"����ע�������ࡣ","�رմ���",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;							// �� hInstance ��Ϊ NULL
	}
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;						// �������ƥ��Ľ��
	WNDCLASS	wc;							// ������ṹ
	DWORD		dwExStyle;						// ��չ���ڷ��
	DWORD		dwStyle;						// ���ڷ��
	RECT WindowRect;							// ȡ�þ��ε����ϽǺ����½ǵ�����ֵ
	WindowRect.left=(long)0;						// ��Left   ��Ϊ 0
	WindowRect.right=(long)width;						// ��Right  ��ΪҪ��Ŀ��
	WindowRect.top=(long)0;							// ��Top    ��Ϊ 0
	WindowRect.bottom=(long)height;						// ��Bottom ��ΪҪ��ĸ߶�
	fullscreen=fullscreenflag;						// ����ȫ��ȫ����־
	hInstance		= GetModuleHandle(NULL);			// ȡ�����Ǵ��ڵ�ʵ��
	wc.style		= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;		// �ƶ�ʱ�ػ�����Ϊ����ȡ��DC
	wc.lpfnWndProc		= (WNDPROC) WndProc;				// WndProc������Ϣ
	wc.cbClsExtra		= 0;						// �޶��ⴰ������
	wc.cbWndExtra		= 0;						// �޶��ⴰ������
	wc.hInstance		= hInstance;					// ����ʵ��
	wc.hIcon		= LoadIcon(NULL, IDI_WINLOGO);			// װ��ȱʡͼ��
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);			// װ�����ָ��
	wc.hbrBackground	= NULL;						// GL����Ҫ����
	wc.lpszMenuName		= NULL;						// ����Ҫ�˵�
	wc.lpszClassName	= "OpenG";					// �趨������
	if (!RegisterClass(&wc))						// ����ע�ᴰ����
	{
		MessageBox(NULL,"ע�ᴰ��ʧ��","����",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// �˳�������FALSE
	}
	if (fullscreen)								// Ҫ����ȫ��ģʽ��?
	{
		DEVMODE dmScreenSettings;						// �豸ģʽ
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));			// ȷ���ڴ����Ϊ��
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);			// Devmode �ṹ�Ĵ�С
		dmScreenSettings.dmPelsWidth	= width;				// ��ѡ��Ļ���
		dmScreenSettings.dmPelsHeight	= height;				// ��ѡ��Ļ�߶�
		dmScreenSettings.dmBitsPerPel	= bits;					// ÿ������ѡ��ɫ�����
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
		// ����������ʾģʽ�����ؽ����ע: CDS_FULLSCREEN ��ȥ��״̬����
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// ��ģʽʧ�ܣ��ṩ����ѡ��˳����ڴ��������С�
			if (MessageBox(NULL,"ȫ��ģʽ�ڵ�ǰ�Կ�������ʧ�ܣ�\nʹ�ô���ģʽ��","NeHe G",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;				// ѡ�񴰿�ģʽ(Fullscreen=FALSE)
			}
			else
			{
				// ����һ���Ի��򣬸����û��������
				MessageBox(NULL,"���򽫱��ر�","����",MB_OK|MB_ICONSTOP);
				return FALSE;					//  �˳������� FALSE
			}
		}
	}

	if (fullscreen)								// �Դ���ȫ��ģʽ��?
	{
		dwExStyle=WS_EX_APPWINDOW;					// ��չ������
		dwStyle=WS_POPUP;						// ������
		ShowCursor(FALSE);						// �������ָ��
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// ��չ������
		dwStyle=WS_OVERLAPPEDWINDOW;					//  ������
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// �������ڴﵽ����Ҫ��Ĵ�С

	if (!(hWnd=CreateWindowEx(	dwExStyle,				// ��չ������
		"OpenG",				// ������
		title,					// ���ڱ���
		WS_CLIPSIBLINGS |			// ����Ĵ���������
		WS_CLIPCHILDREN |			// ����Ĵ���������
		dwStyle,				// ѡ��Ĵ�������
		0, 0,					// ����λ��
		WindowRect.right-WindowRect.left,	// ��������õĴ��ڿ��
		WindowRect.bottom-WindowRect.top,	// ��������õĴ��ڸ߶�
		NULL,					// �޸�����
		NULL,					// �޲˵�
		hInstance,				// ʵ��
		NULL)))					// ����WM_CREATE�����κζ���
	{
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL,"���ܴ���һ�������豸������","����",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=					// /pfd ���ߴ���������ϣ���Ķ�����������ʹ�õ����ظ�ʽ
	{
		sizeof(PIXELFORMATDESCRIPTOR),					// ������ʽ�������Ĵ�С
		1,								// �汾��
		PFD_DRAW_TO_WINDOW |						// ��ʽ֧�ִ���
		PFD_SUPPORT_OPENGL |						// ��ʽ����֧��OpenGL
		PFD_DOUBLEBUFFER,						// ����֧��˫����
		PFD_TYPE_RGBA,							// ���� RGBA ��ʽ
		bits,								// ѡ��ɫ�����
		0, 0, 0, 0, 0, 0,						// ���Ե�ɫ��λ
		0,								// ��Alpha����
		0,								// ����Shift Bit
		0,								// ���ۼӻ���
		0, 0, 0, 0,							// ���Ծۼ�λ
		16,								// 16λ Z-���� (��Ȼ���)
		0,								// ���ɰ建��
		0,								// �޸�������
		PFD_MAIN_PLANE,							// ����ͼ��
		0,								// Reserved
		0, 0, 0								// ���Բ�����
	};

	if (!(hDC=GetDC(hWnd)))							// ȡ���豸��������ô?
	{
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL,"���ܴ���һ����ƥ������ظ�ʽ","����",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))				// Windows �ҵ���Ӧ�����ظ�ʽ����?
	{
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL,"�����������ظ�ʽ","����",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))				// �ܹ��������ظ�ʽô?
	{
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL,"�����������ظ�ʽ","����",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))					// �ܷ�ȡ����ɫ������?
	{
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL,"���ܴ���OpenGL��Ⱦ������","����",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))						// ���Լ�����ɫ������
	{
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL,"���ܼ��ǰ��OpenGL��Ȼ������","����",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// ��ʾ����
	SetForegroundWindow(hWnd);						// ����������ȼ�
	SetFocus(hWnd);								// ���ü��̵Ľ������˴���
	ReSizeGLScene(width, height);						// ����͸�� GL ��Ļ

	if (!InitGL())								// ��ʼ���½���GL����
	{
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	return TRUE;								// �ɹ�
}

LRESULT CALLBACK WndProc(	HWND	hWnd,					// ���ڵľ��
	UINT	uMsg,					// ���ڵ���Ϣ
	WPARAM	wParam,					// ���ӵ���Ϣ����
	LPARAM	lParam)					// ���ӵ���Ϣ����
{

	switch (uMsg)								// ���Windows��Ϣ
	{

	case WM_ACTIVATE:						// ���Ӵ��ڼ�����Ϣ
		{
			if (!HIWORD(wParam))					// �����С��״̬
			{
				active=TRUE;					// �����ڼ���״̬
			}
			else
			{
				active=FALSE;					// �����ټ���
			}

			return 0;						// ������Ϣѭ��
		}

	case WM_SYSCOMMAND:						// ϵͳ�ж�����
		{
			switch (wParam)						// ���ϵͳ����
			{
			case SC_SCREENSAVE:				// ����Ҫ����?
			case SC_MONITORPOWER:				// ��ʾ��Ҫ����ڵ�ģʽ?
				return 0;					// ��ֹ����
			}
			break;							// �˳�
		}

	case WM_CLOSE:							// �յ�Close��Ϣ?
		{
			PostQuitMessage(0);					// �����˳���Ϣ
			return 0;						// ����
		}

	case WM_KEYDOWN:						// �м�����ô?
		{
			keys[wParam] = TRUE;					// ����ǣ���ΪTRUE
			return 0;						// ����
		}

	case WM_KEYUP:							// �м��ſ�ô?
		{
			keys[wParam] = FALSE;					// ����ǣ���ΪFALSE
			return 0;						// ����
		}

	case WM_SIZE:							// ����OpenGL���ڴ�С
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));		// LoWord=Width,HiWord=Height
			return 0;						// ����
		}
	}
	// �� DefWindowProc��������δ�������Ϣ��
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,				// ��ǰ����ʵ��
	HINSTANCE	hPrevInstance,				// ǰһ������ʵ��
	LPSTR		lpCmdLine,				// �����в���
	int		nCmdShow)				// ������ʾ״̬
{
	MSG	msg;								// Windowsx��Ϣ�ṹ
	BOOL	done=FALSE;							// �����˳�ѭ����Bool ����
	// ��ʾ�û�ѡ������ģʽ
	if (MessageBox(NULL,"������ȫ��ģʽ������ô��", "����ȫ��ģʽ",MB_YESNO|MB_ICONQUESTION)==IDNO)
	{
		fullscreen=FALSE;						// FALSEΪ����ģʽ
	}
	// ����OpenGL����
	if (!CreateGLWindow("opensteer�㷨��ʾ1",800,600,16,fullscreen))
	{
		return 0;							// ʧ���˳�
	}
	while(!done)								// ����ѭ��ֱ�� done=TRUE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))			// ����Ϣ�ڵȴ���?
		{
			if (msg.message==WM_QUIT)				// �յ��˳���Ϣ?
			{
				done=TRUE;					// �ǣ���done=TRUE
			}
			else							// ���ǣ���������Ϣ
			{
				TranslateMessage(&msg);				// ������Ϣ
				DispatchMessage(&msg);				// ������Ϣ
			}
		}
		else								// ���û����Ϣ
		{
			// ���Ƴ���������ESC��������DrawGLScene()���˳���Ϣ
			if (active)						// ���򼤻��ô?
			{
				if (keys[VK_ESCAPE])				// ESC ������ô?
				{
					done=TRUE;				// ESC �����˳��ź�
				}
				else						// �����˳���ʱ��ˢ����Ļ
				{
					DrawGLScene();				// ���Ƴ���
					SwapBuffers(hDC);			// �������� (˫����)
				}
			}
			if (keys[VK_F1])					// F1��������ô?
			{
				keys[VK_F1]=FALSE;				// ���ǣ�ʹ��Ӧ��Key�����е�ֵΪ FALSE
				KillGLWindow();					// ���ٵ�ǰ�Ĵ���
				fullscreen=!fullscreen;				// �л� ȫ�� / ���� ģʽ
				// �ؽ� OpenGL ����
				if (!CreateGLWindow("NeHe's OpenGL ������",640,480,16,fullscreen))
				{
					return 0;				// �������δ�ܴ����������˳�
				}
			}
		}
	}
	// �رճ���
	KillGLWindow();								// ���ٴ���
	return (msg.wParam);							// �˳�����
}