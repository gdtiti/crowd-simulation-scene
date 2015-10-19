static int day = 200; // day的变化：从0到359
void myDisplay(void)
{
> glEnable(GL\_DEPTH\_TEST);
> glClear(GL\_COLOR\_BUFFER\_BIT | GL\_DEPTH\_BUFFER\_BIT);

> glMatrixMode(GL\_PROJECTION);
> glLoadIdentity();
> gluPerspective(75, 1, 1, 400000000);
> glMatrixMode(GL\_MODELVIEW);
> glLoadIdentity();
> gluLookAt(0, -200000000, 200000000, 0, 0, 0, 0, 0, 1);

> // 绘制红色的“太阳”
> glColor3f(1.0f, 0.0f, 0.0f);
> glutSolidSphere(69600000, 20, 20);
> // 绘制蓝色的“地球”
> glColor3f(0.0f, 0.0f, 1.0f);
> glRotatef(day/360.0\*360.0, 0.0f, 0.0f, -1.0f);
> glTranslatef(150000000, 0.0f, 0.0f);
> glutSolidSphere(15945000, 20, 20);
> // 绘制黄色的“月亮”
> glColor3f(1.0f, 1.0f, 0.0f);
> glRotatef(day/30.0\*360.0 - day/360.0\*360.0, 0.0f, 0.0f, -1.0f);
> glTranslatef(38000000, 0.0f, 0.0f);
> glutSolidSphere(4345000, 20, 20);

> glFlush();
}
在电脑中显示时有点小问题，在设置好球体的颜色后，显示出来的球体中有一些零星的小黑块，不知道是怎么回事