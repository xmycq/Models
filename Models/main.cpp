#include "Angel.h"
#include "mat.h"
#include "vec.h"
#include <fstream>
#include<vector>
#include<iostream>
using namespace std;
#pragma comment(lib, "glew32.lib")

// 三角面片中的顶点序列
typedef struct vIndex {
	unsigned int a, b, c;
	vIndex(int ia, int ib, int ic) : a(ia), b(ib), c(ic) {}
} vec3i;

const int X_AXIS = 0;
const int Y_AXIS = 1;
const int Z_AXIS = 2;

const int TRANSFORM_SCALE = 0;
const int TRANSFORM_ROTATE = 1;
const int TRANSFORM_TRANSLATE = 2;
const int ROTATE_X = 3;
const int ROTATE_Y = 4;
const int ROTATE_Z = 5;

const double DELTA_DELTA = 0.1;    // Delta的变化率
const double DEFAULT_DELTA = 0.3;    // 默认的Delta值

double scaleDelta = DEFAULT_DELTA;
double rotateDelta = DEFAULT_DELTA;
double translateDelta = DEFAULT_DELTA;
int nVertices = 0;
int nFaces = 0;
int nEdges = 0;
int ROTATE_XYZ = X_AXIS;

vec3 scaleTheta(1.0, 1.0, 1.0);    // 缩放控制变量
vec3 rotateTheta(0.0, 0.0, 0.0);    // 旋转控制变量
vec3 translateTheta(0.0, 0.0, 0.0);    // 平移控制变量

GLint matrixLocation;
int currentTransform = TRANSFORM_TRANSLATE;    // 设置当前变换
int mainWindow;
const int NUM_VERTICES = 8;

std::vector<vec3i> faces;
std::vector<vec3> vertices;
std::vector<vec3> colors;

// 通过文件读取物体的顶点位置，及片元索引
void read_off(const std::string filename)
{
	if (filename.empty()) {
		return;
	}
	std::ifstream fin;
	fin.open(filename);
	string str;
	fin >> str;
	fin >> nVertices >> nFaces >> nEdges;
	double x, y, z;
	for (int i = 0; i < nVertices; ++i) {
		fin >> x >> y >> z;
		vertices.push_back(vec3(x, y, z));
	}
	int d, d1, d2, d3;
	for (int i = 0; i < nFaces; i++) {
		fin >> d >> d1 >> d2 >> d3;
		faces.push_back(vec3i(d1, d2, d3));
	}
}

// 生成每个顶点的位置，颜色，索引
void generateCube()
{
	vertices.clear();
	faces.clear();

	read_off("cow.off");
	
}

void init()
{
	generateCube();

	// 创建顶点数组对象
	GLuint vao[1];
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);

	// 创建并初始化顶点缓存对象
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3) + colors.size() * sizeof(vec3), NULL, GL_STATIC_DRAW);

	// 分别用输入顶点及颜色信息
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vec3), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), colors.size() * sizeof(vec3), colors.data());

	// 创建并初始化顶点索引缓存对象
	GLuint vertexIndexBuffer;
	glGenBuffers(1, &vertexIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(vec3i), faces.data(), GL_STATIC_DRAW);

	// 读取着色器并使用
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// 从顶点着色器中初始化顶点的位置
	GLuint pLocation = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(pLocation);
	glVertexAttribPointer(pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	// 从片元着色器中初始化顶点的颜色
	GLuint cLocation = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(cLocation);
	glVertexAttribPointer(cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertices.size() * sizeof(vec3)));

	// 获得矩阵存储位置
	matrixLocation = glGetUniformLocation(program, "matrix");

	// 白色背景
	glClearColor(1.0, 1.0, 1.0, 1.0);
}
void display()
{
	// 清理窗口
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 生成变换矩阵
	mat4 m(1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0);

	// @TODO: 在此处修改函数，根据scaleTheta，rotateTheta，translateTheta，计算变换矩阵
	// 可使用Scale(),Translate(),RotateX(),RotateY(),RotateZ()等函数。函数定义在mat.h
	m = Translate(translateTheta) * RotateZ(rotateTheta.z) * RotateY(rotateTheta.y) * RotateX(rotateTheta.x) * Scale(scaleTheta);

	switch (currentTransform) {
	case TRANSFORM_SCALE:
		m *= Scale(scaleTheta);
		break;
	case TRANSFORM_ROTATE:
		m *= RotateX(rotateTheta.x);
		m *= RotateY(rotateTheta.y);
		m *= RotateZ(rotateTheta.z);
		break;
	case TRANSFORM_TRANSLATE:
		m *= Translate(translateTheta);
		break;
	}

	// 从指定位置matrixLocation中传入变换矩阵m
	glUniformMatrix4fv(matrixLocation, 1, GL_TRUE, m);
	// 绘制立方体中的各个三角形
	glDrawElements(GL_TRIANGLES, int(faces.size() * 3), GL_UNSIGNED_INT, BUFFER_OFFSET(0));

	glutSwapBuffers();
}

// 通过Delta值更新Theta
void updateTheta(int axis, int sign) {
	switch (currentTransform) {
	case TRANSFORM_SCALE:
		scaleTheta[axis] += sign * scaleDelta;
		break;
	case TRANSFORM_ROTATE:
		rotateTheta[axis] += sign * rotateDelta;
		break;
	case TRANSFORM_TRANSLATE:
		translateTheta[axis] += sign * translateDelta;
		break;
	}
}

// 复原Theta和Delta
void resetTheta()
{
	scaleTheta = vec3(1.0, 1.0, 1.0);
	rotateTheta = vec3(0.0, 0.0, 0.0);
	translateTheta = vec3(0.0, 0.0, 0.0);
	scaleDelta = DEFAULT_DELTA;
	rotateDelta = DEFAULT_DELTA;
	translateDelta = DEFAULT_DELTA;
}

// 更新变化Delta值
void updateDelta(int sign)
{
	switch (currentTransform) {
	case TRANSFORM_SCALE:
		scaleDelta += sign * DELTA_DELTA;
		break;
	case TRANSFORM_ROTATE:
		rotateDelta += sign * DELTA_DELTA;
		break;
	case TRANSFORM_TRANSLATE:
		translateDelta += sign * DELTA_DELTA;
		break;
	}
}

// 空闲回调函数，实现绕某个轴旋转
void idle() {
	// 改变旋转的角度,ROTATE_XYZ可以是0,1,2
	// 分别代表x,y,z轴
	rotateTheta[ROTATE_XYZ] += DELTA_DELTA;
	// 重新绘制
	glutPostWindowRedisplay(mainWindow);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':
		updateTheta(X_AXIS, 1);
		break;
	case 'a':
		updateTheta(X_AXIS, -1);
		break;
	case 'w':
		updateTheta(Y_AXIS, 1);
		break;
	case 's':
		updateTheta(Y_AXIS, -1);
		break;
	case 'e':
		updateTheta(Z_AXIS, 1);
		break;
	case 'd':
		updateTheta(Z_AXIS, -1);
		break;
	case 'r':
		updateDelta(1);
		break;
	case 'f':
		updateDelta(-1);
		break;
	case 't':
		resetTheta();
		break;
	case 033:
		// Esc按键
		exit(EXIT_SUCCESS);
		break;
	}
	glutPostWindowRedisplay(mainWindow);
}

// 鼠标响应事件
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		glutIdleFunc(idle);
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		glutIdleFunc(NULL);
	}
}

void menuEvents(int menuChoice)
{
	if (menuChoice < 3) {
		currentTransform = menuChoice;
	}
	else {
		ROTATE_XYZ = menuChoice - 3;
	}
}

void setupMenu()
{
	glutCreateMenu(menuEvents);
	glutAddMenuEntry("Scale", TRANSFORM_SCALE);
	glutAddMenuEntry("Rotate", TRANSFORM_ROTATE);
	glutAddMenuEntry("Translate", TRANSFORM_TRANSLATE);
	// 添加绕某一个轴旋转
	glutAddMenuEntry("X", ROTATE_X);
	glutAddMenuEntry("Y", ROTATE_Y);
	glutAddMenuEntry("Z", ROTATE_Z);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void printHelp() {
	printf("%s\n\n", "3D Transfomations");
	printf("Keyboard options:\n");
	printf("q: Increase x\n");
	printf("a: Decrease x\n");
	printf("w: Increase y\n");
	printf("s: Decrease y\n");
	printf("e: Increase z\n");
	printf("d: Decrease z\n");
	printf("r: Increase delta of currently selected transform\n");
	printf("f: Decrease delta of currently selected transform\n");
	printf("t: Reset all transformations and deltas\n");
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);    // 窗口支持双重缓冲和深度测试
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	mainWindow = glutCreateWindow("徐明尧2018152062");

	glewExperimental = GL_TRUE;
	glewInit();

	init();
	setupMenu();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	// 输出帮助信息
	printHelp();
	// 启用深度测试
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
	return 0;
}