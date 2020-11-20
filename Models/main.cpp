#include "Angel.h"
#include "mat.h"
#include "vec.h"
#include <fstream>
#include<vector>
#include<iostream>
using namespace std;
#pragma comment(lib, "glew32.lib")

// ������Ƭ�еĶ�������
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

const double DELTA_DELTA = 0.1;    // Delta�ı仯��
const double DEFAULT_DELTA = 0.3;    // Ĭ�ϵ�Deltaֵ

double scaleDelta = DEFAULT_DELTA;
double rotateDelta = DEFAULT_DELTA;
double translateDelta = DEFAULT_DELTA;
int nVertices = 0;
int nFaces = 0;
int nEdges = 0;
int ROTATE_XYZ = X_AXIS;

vec3 scaleTheta(1.0, 1.0, 1.0);    // ���ſ��Ʊ���
vec3 rotateTheta(0.0, 0.0, 0.0);    // ��ת���Ʊ���
vec3 translateTheta(0.0, 0.0, 0.0);    // ƽ�ƿ��Ʊ���

GLint matrixLocation;
int currentTransform = TRANSFORM_TRANSLATE;    // ���õ�ǰ�任
int mainWindow;
const int NUM_VERTICES = 8;

std::vector<vec3i> faces;
std::vector<vec3> vertices;
std::vector<vec3> colors;

// ͨ���ļ���ȡ����Ķ���λ�ã���ƬԪ����
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

// ����ÿ�������λ�ã���ɫ������
void generateCube()
{
	vertices.clear();
	faces.clear();

	read_off("cow.off");
	
}

void init()
{
	generateCube();

	// ���������������
	GLuint vao[1];
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);

	// ��������ʼ�����㻺�����
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3) + colors.size() * sizeof(vec3), NULL, GL_STATIC_DRAW);

	// �ֱ������붥�㼰��ɫ��Ϣ
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vec3), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), colors.size() * sizeof(vec3), colors.data());

	// ��������ʼ�����������������
	GLuint vertexIndexBuffer;
	glGenBuffers(1, &vertexIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(vec3i), faces.data(), GL_STATIC_DRAW);

	// ��ȡ��ɫ����ʹ��
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// �Ӷ�����ɫ���г�ʼ�������λ��
	GLuint pLocation = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(pLocation);
	glVertexAttribPointer(pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	// ��ƬԪ��ɫ���г�ʼ���������ɫ
	GLuint cLocation = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(cLocation);
	glVertexAttribPointer(cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertices.size() * sizeof(vec3)));

	// ��þ���洢λ��
	matrixLocation = glGetUniformLocation(program, "matrix");

	// ��ɫ����
	glClearColor(1.0, 1.0, 1.0, 1.0);
}
void display()
{
	// ������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ���ɱ任����
	mat4 m(1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0);

	// @TODO: �ڴ˴��޸ĺ���������scaleTheta��rotateTheta��translateTheta������任����
	// ��ʹ��Scale(),Translate(),RotateX(),RotateY(),RotateZ()�Ⱥ���������������mat.h
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

	// ��ָ��λ��matrixLocation�д���任����m
	glUniformMatrix4fv(matrixLocation, 1, GL_TRUE, m);
	// �����������еĸ���������
	glDrawElements(GL_TRIANGLES, int(faces.size() * 3), GL_UNSIGNED_INT, BUFFER_OFFSET(0));

	glutSwapBuffers();
}

// ͨ��Deltaֵ����Theta
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

// ��ԭTheta��Delta
void resetTheta()
{
	scaleTheta = vec3(1.0, 1.0, 1.0);
	rotateTheta = vec3(0.0, 0.0, 0.0);
	translateTheta = vec3(0.0, 0.0, 0.0);
	scaleDelta = DEFAULT_DELTA;
	rotateDelta = DEFAULT_DELTA;
	translateDelta = DEFAULT_DELTA;
}

// ���±仯Deltaֵ
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

// ���лص�������ʵ����ĳ������ת
void idle() {
	// �ı���ת�ĽǶ�,ROTATE_XYZ������0,1,2
	// �ֱ����x,y,z��
	rotateTheta[ROTATE_XYZ] += DELTA_DELTA;
	// ���»���
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
		// Esc����
		exit(EXIT_SUCCESS);
		break;
	}
	glutPostWindowRedisplay(mainWindow);
}

// �����Ӧ�¼�
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
	// �����ĳһ������ת
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
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);    // ����֧��˫�ػ������Ȳ���
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	mainWindow = glutCreateWindow("����Ң2018152062");

	glewExperimental = GL_TRUE;
	glewInit();

	init();
	setupMenu();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	// ���������Ϣ
	printHelp();
	// ������Ȳ���
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
	return 0;
}