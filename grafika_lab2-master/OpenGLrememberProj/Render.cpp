#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

void normal(double a[], double b[], double c[]){
	double AB[] = { b[0] - a[0],b[1] - a[1] ,b[2] - a[2] };
	double AC[] = { c[0] - a[0],c[1] - a[1] ,2 };
	double nx = AB[1] * AC[2] - AC[1] * AB[2];
	double ny = -AB[0] * AC[2] + AC[0] * AB[2];
	double nz = AB[0] * AC[1] - AC[0] * AB[1];
	glNormal3d(-nx, -ny, -nz);
}


void texturequad(double a[], double b[]) {
	double c[] = { b[0],b[1],2 };
	double d[] = { a[0],a[1],2 };
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 0);
	glVertex3dv(a);
	glTexCoord2d(1, 0);
	glVertex3dv(b);
	glTexCoord2d(1, 1);
	glVertex3dv(c);
	glTexCoord2d(0, 1);
	glVertex3dv(d);
	glEnd();
}
double abs1(double a) {
	if (a < 0)a = a * -1;
	return a;
}
void Main() {
	double A[] = { 0,7,0 };
	double B[] = { 5,0,0 };
	double C[] = { 0,-1,0 };
	double D[] = { 3,-9,0 };
	double E[] = { -2,-7,0 };
	double F[] = { -2,-2,0 };
	double G[] = { -7,0,0 };
	double H[] = { -2, 1, 0 };
	glColor3b(1, 0, 0);
	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glVertex3dv(B);
	glVertex3dv(H);
	glVertex3dv(G);

	glVertex3dv(B);
	glVertex3dv(G);
	glVertex3dv(C);

	glVertex3dv(C);
	glVertex3dv(F);
	glVertex3dv(G);

	glVertex3dv(C);
	glVertex3dv(E);
	glVertex3dv(F);

	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(E);
	glEnd();
	
	glNormal3d(0, 0, 1);
	glBegin(GL_TRIANGLES);
	glVertex3d(B[0], B[1], B[2] + 2);
	glVertex3d(H[0], H[1], H[2] + 2);
	glVertex3d(G[0], G[1], G[2] + 2);

	glVertex3d(B[0], B[1], B[2] + 2);
	glVertex3d(G[0], G[1], G[2] + 2);
	glVertex3d(C[0], C[1], C[2] + 2);

	glVertex3d(C[0], C[1], C[2] + 2);
	glVertex3d(F[0], F[1], F[2] + 2);
	glVertex3d(G[0], G[1], G[2] + 2);

	glVertex3d(C[0], C[1], C[2] + 2);
	glVertex3d(F[0], F[1], F[2] + 2);
	glVertex3d(E[0], E[1], E[2] + 2);

	glVertex3d(C[0], C[1], C[2] + 2);
	glVertex3d(D[0], D[1], D[2] + 2);
	glVertex3d(E[0], E[1], E[2] + 2);

	glEnd();


	double A3[] = { 5,0,0 };
	double B3[] = { 0,-1,0 };
	double C3[] = { 0,-1,2 };
	double D3[] = { 5,0,2 };
	normal(A3, B3, A3);
	texturequad(A3, B3);
	double A4[] = { 0,-1,0 };
	double B4[] = { 3,-9,0 };
	normal(A4, B4, A4);
	texturequad(A4, B4);
	
	double A6[] = { -2, -7, 0 };
	double B6[] = { -2, -2, 0 };
	normal(A6, B6,A6);
	texturequad(A6, B6);
	double A7[] = { -2, -2, 0 };
	double B7[] = { -7, 0, 0 };
	normal(A7, B7,A7);
	texturequad(A7, B7);
	double A8[] = { -7, 0, 0 };
	double B8[] = { -2, 1, 0 };
	normal(A8, B8,A8);
	texturequad(A8, B8);
	double A9[] = { -2, 1, 0 };
	double B9[] = { 0, 7, 0 };
	normal(A9, B9,A9);
	texturequad(A9, B9);
}

void Four() {
	double A[] = { 0.5,-8,0 };
	double D[] = { 3,-9,0 };
	double E[] = { -2,-7,0 };
	double A1[] = { 0.5,-8,2 };
	double R = (sqrt(2.5 * 2.5 + 1));
	double B[] = { -2,-7,0 };
	double C[] = { -2,-7,0 };
	double angle = asin(1 / R);
	bool fl = 0;
	glBindTexture(GL_TEXTURE_2D, texId);
	for (double i = 0.0; i < 360; i = i + 0.2) {
		double x = A[0] + (R * cos((i + angle) * 3.14159 / 180.0));
		double y = A[1] + (R * sin((i + angle) * 3.14159 / 180.0));
		C[0] = x;
		C[1] = y;
		glColor3d(1, 0, 0);
	/*	
		if (i == 0) {
			glTexCoord2d(1, 0.5);
			glVertex3dv(C);
		}
		else if(i==90){
			glTexCoord2d(0.5, 1);
			glVertex3dv(C);
		}
		else if (i == 180) {
			glTexCoord2d(0, 0.5);
			glVertex3dv(C);
		}
		else if (i == 270) {
			glTexCoord2d(0.5, 0);
			glVertex3dv(C);
		}*/
		
		if ((C[0] <= D[0]) && (C[1] <= E[1])) {
			glNormal3d(0, 0, -1);
			
			glBegin(GL_TRIANGLES);

			glVertex3dv(A);
			
			glVertex3dv(B);
			
			glVertex3dv(C);
			glEnd();

			glNormal3d(0, 0, 1);

			glBegin(GL_TRIANGLES);
			glVertex3d(A[0], A[1], A[2] + 2);
			glVertex3d(B[0], B[1], B[2] + 2);
			glVertex3d(C[0], C[1], C[2] + 2);
			glEnd();

			normal(C, B,C);
			
			glBegin(GL_QUADS);
			glVertex3dv(C);
			glVertex3dv(B);
			glVertex3d(B[0], B[1], B[2] + 2);
			glVertex3d(C[0], C[1], C[2] + 2);
			glEnd();
			B[0] = C[0];
			B[1] = C[1];
		}
	}
	glNormal3d(0, 0, -1);
	glBegin(GL_TRIANGLES);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(D);
	glEnd();

	glNormal3d(0, 0, 1);

	glBegin(GL_TRIANGLES);
	glVertex3d(A[0], A[1], A[2] + 2);
	glVertex3d(B[0], B[1], B[2] + 2);
	glVertex3d(D[0], D[1], D[2] + 2);
	glEnd();

	normal(D, B, D);

	glBegin(GL_QUADS);
	glVertex3dv(D);
	glVertex3dv(B);
	glVertex3d(B[0], B[1], B[2] + 2);
	glVertex3d(D[0], D[1], D[2] + 2);
	glEnd();

}

void Five() {
	double A[] = { 0,7,0 };
	double B[] = { 5,0,0 };
	double C[] = { 0,-1,0 };
	double D[] = { 3,-9,0 };
	double E[] = { -2,-7,0 };
	double F[] = { -2,-2,0 };
	double G[] = { -7,0,0 };
	double H[] = { -2, 1, 0 };
	bool fl = false;
	double center[] = { 7.4,7,0 };
	double rad = sqrt((((center[0] - A[0])) * ((center[0] - A[0])) + ((center[1] + A[1])) * ((center[1] - A[1]))));
	double p0[] = { A[0],A[1],A[2] };
	double p1[] = { A[0],A[1],A[2] };


	for (double i = 0.0; i < 360; i = i + 0.2) {
		double x = center[0] + (rad * cos((i) * 3.14159 / 180.0));
		double y = center[1] + (rad * sin((i) * 3.14159 / 180.0));
		p1[0] = x;
		p1[1] = y;
		glColor3d(1, 0, 0);

		if ((p1[0] <= B[0]) && (p1[1] <= A[1])) {
			
			glNormal3d(0, 0, -1);
			glBegin(GL_TRIANGLES);
			glVertex3dv(H);
			glVertex3dv(p0);
			glVertex3dv(p1);
			glEnd();
			glNormal3d(0, 0, 1);
			glBegin(GL_TRIANGLES);
			glVertex3d(H[0], H[1], H[2] + 2);
			glVertex3d(p0[0], p0[1], p0[2] + 2);
			glVertex3d(p1[0], p1[1], p1[2] + 2);
			glEnd();
			normal(p0, p1, p0);
			glBegin(GL_QUADS);
			glVertex3dv(p0);
			glVertex3dv(p1);
			glVertex3d(p1[0], p1[1], p1[2] + 2);
			glVertex3d(p0[0], p0[1], p0[2] + 2);
			glEnd();
			p0[0] = p1[0];
			p0[1] = p1[1];
		}
	}
	p1[0] = B[0];
	p1[1] = B[1];

	glNormal3d(0, 0, -1);
	glBegin(GL_TRIANGLES);
	glVertex3dv(H);
	glVertex3dv(p0);
	glVertex3dv(p1);
	glEnd();
	glNormal3d(0, 0, 1);
	glBegin(GL_TRIANGLES);
	glVertex3d(H[0], H[1], H[2] + 2);
	glVertex3d(p0[0], p0[1], p0[2] + 2);
	glVertex3d(p1[0], p1[1], p1[2] + 2);
	glEnd();
	normal(p0, p1, p0);
	glBegin(GL_QUADS);
	glVertex3dv(p0);
	glVertex3dv(p1);
	glVertex3d(p1[0], p1[1], p1[2] + 2);
	glVertex3d(p0[0], p0[1], p0[2] + 2);
	glEnd();
	p0[0] = p1[0];
	p0[1] = p1[1];
}





void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	//Начало рисования квадратика станкина
	Main();
	Four();
	Five();

	//glBindTexture(GL_TEXTURE_2D, texId);

	//glColor3d(0.6, 0.6, 0.6);
	//glBegin(GL_QUADS);

	//glNormal3d(0, 0, 1);
	//glTexCoord2d(0, 0);
	//glVertex2dv(A);
	//glTexCoord2d(1, 0);
	//glVertex2dv(B);
	//glTexCoord2d(1, 1);
	//glVertex2dv(C);
	//glTexCoord2d(0, 1);
	//glVertex2dv(D);

	//glEnd();
	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}