#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <math.h>

float base = 1.0f;
float altura = 3.0f;
float movex = 0.0f;
float movey = 0.0f;
float movez = 0.0f;
float angulo = 0.0f;
float eixox = 0.0f;
float eixoy = 1.0f;
float eixoz = 0.0f;
float scalex = 1.0f;
float scaley = 1.0f;
float scalez = 1.0f;

void drawPyramid() {
    glBegin(GL_TRIANGLES);

    // Face 1 (frente)
    glColor3f(1.0f, 0.0f, 0.0f);   // vermelho
    glVertex3f( 0.0f, altura, 0.0f); // cimo
    glVertex3f(-base, 0.0f, base); // base esquerda
    glVertex3f( base, 0.0f, base); // base direita

    // Face 2 (direita)
    glColor3f(0.0f, 1.0f, 0.0f);   // verde
    glVertex3f( 0.0f, altura, 0.0f); // cimo
    glVertex3f( base, 0.0f, base); // base frente direita
    glVertex3f( base, 0.0f,-base); // base trás direita

    // Face 3 (trás)
    glColor3f(0.0f, 0.0f, 1.0f);   // azul
    glVertex3f( 0.0f, altura, 0.0f); // cimo
    glVertex3f( base, 0.0f,-base); // base trás direita
    glVertex3f(-base, 0.0f,-base); // base trás esquerda

    // Face 4 (esquerda)
    glColor3f(1.0f, 1.0f, 0.0f);   // amarelo
    glVertex3f( 0.0f, altura, 0.0f); // cimo
    glVertex3f(-base, 0.0f,-base); // base trás esquerda
    glVertex3f(-base, 0.0f, base); // base frente esquerda

    glEnd();

    // Base (quadrado em dois triângulos)
    glBegin(GL_TRIANGLES);

    glColor3f(1.0f, 0.0f, 1.0f);   // roxo
    glVertex3f(-base, 0.0f, base); // base frente esquerda
    glVertex3f( base, 0.0f, -base); // base frente direita
    glVertex3f( base, 0.0f,base); // base trás direita

	glColor3f(0.0f, 1.0f, 1.0f);   // ciano
    glVertex3f(-base, 0.0f, base); // base frente esquerda
    glVertex3f(-base, 0.0f,-base); // base trás direita
    glVertex3f(base, 0.0f,-base); // base trás esquerda

    glEnd();
}

void drawAxis() {
    glBegin(GL_LINES);
	// X axis in red
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-100.0f, 0.0f, 0.0f);
	glVertex3f( 100.0f, 0.0f, 0.0f);
	// Y Axis in Green
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, -100.0f, 0.0f);
	glVertex3f(0.0f, 100.0f, 0.0f);
	// Z Axis in Blue
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, -100.0f);
	glVertex3f(0.0f, 0.0f, 100.0f);
	glEnd();
}



void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if(h == 0)
		h = 1;

	// compute window's aspect ratio 
	float ratio = w * 1.0 / h;

	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load Identity Matrix
	glLoadIdentity();
	
	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Set perspective
	gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}


void renderScene(void) {

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();
	gluLookAt(5.0,5.0,5.0, 
		      0.0,0.0,0.0,
			  0.0f,1.0f,0.0f);

// put axis drawing in here

	drawAxis();

// put the geometric transformations here

	glTranslatef(movex, movey, movez);	
	glRotatef(angulo, eixox, eixoy, eixoz);
	glScalef(scalex, scaley, scalez);


// put pyramid drawing instructions here

	drawPyramid();

	// End of frame
	glutSwapBuffers();
}



// write function to process keyboard events

void processKeys(unsigned char key, int xx, int yy) {
	switch (key) {
		case 'w':
			// move forward
			movez -= 0.1f;
			break;
		case 's':
			movez += 0.1f;
			break;
		case 'a':
			movex -= 0.1f;
			break;
		case 'd':
			movex += 0.1f;
			break;
		case 'q':
    		angulo -= 5.0f;
    		break;
		case 'e':
    		angulo += 5.0f;
    		break;
		case '+':   // ou 'r'
    		scaley += 0.1f;
    		break;
		case '-':   // ou 'f'
    		if (scaley > 0.1f) scaley -= 0.1f;
    		break;

	}
	glutPostRedisplay();
}

void processSpecial(int key, int xx, int yy) {
	switch (key) { 

	}
	glutPostRedisplay();
}




int main(int argc, char **argv) {

// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("CG@DI-UM");
		
// Required callback registry 
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	
// put here the registration of the keyboard callbacks

    glutKeyboardFunc(processKeys);
	glutSpecialFunc(processSpecial);


//  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
// enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}
