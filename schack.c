#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <GL/gl.h>
#include <sys/ioctl.h>
#include "VectorUtils3.h"
// Några funktioner från GL_utilities.h anropar GLUT. Använd dem inte.
#include "GL_utilities.h"
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "loadobj.h"
#include "LoadTGA.h"

unsigned window_w, window_h;
// Ändra inte, för jag tror att picking-koden antar att det går från -0.5 till 0.5.
#define FRUSTUM_LEFT (-0.5)
#define FRUSTUM_RIGHT (0.5)
#define FRUSTUM_BOTTOM (-0.5)
#define FRUSTUM_TOP (0.5)
#define FRUSTUM_NEAR (1.0)
#define FRUSTUM_FAR (60.0)

#define DEFAULT_W 800
#define DEFAULT_H 600

//Schackbräde
GLfloat vertices[4][3] =
{
	{-0.5, 0.0, -0.5},
	{0.5, 0.0, -0.5},
	{0.5, 0.0, 0.5},
	{-0.5, 0.0, 0.5}
};
GLfloat normals[4][3] = {
	{0.0, 0.58, 0.0},
	{0.0, 0.58, 0.0},
	{0.0, 0.58, 0.0},
	{0.0, 0.58, 0.0}
};
GLuint cubeIndices[6] =
{
	0,1,2, 2,3,0
};
//pedestal
GLfloat ped_vertices[8][3] =
{
	{-0.5,-0.5,-0.5},
	{0.5,-0.5,-0.5},
	{0.5,0.5,-0.5},
	{-0.5,0.5,-0.5},
	{-0.5,-0.5,0.5},
	{0.5,-0.5,0.5},
	{0.5,0.5,0.5},
	{-0.5,0.5,0.5}
};
GLfloat ped_normals[8][3] = {
	{-0.58,-0.58,-0.58},
	{0.58,-0.58,-0.58},
	{0.58,0.58,-0.58},
	{-0.58,0.58,-0.58},
	{-0.58,-0.58,0.58},
	{0.58,-0.58,0.58},
	{0.58,0.58,0.58},
	{-0.58,0.58,0.58}
};
GLubyte ped_cubeIndices[36] =
{
	0,3,2, 0,2,1,
	2,3,7, 2,7,6,
	0,4,7, 0,7,3,
	1,2,6, 1,6,5,
	4,5,6, 4,6,7,
	0,1,5, 0,5,4
};

// Markör
GLfloat marker_vertices[][3] = {
	// Lock
	{ 0.2, 0.4,  0.2}, { 0.2, 0.4, -0.2},
	{-0.2, 0.4,  0.2}, {-0.2, 0.4, -0.2},
	// Sidor
	{ 0.2, 0.4,  0.2}, { 0.2, 0.4, -0.2},
	{-0.2, 0.4,  0.2}, {-0.2, 0.4, -0.2},
	// Botten
	{0.0, -0.2, 0.0}, {0.0, -0.2, 0.0},
	{0.0, -0.2, 0.0}, {0.0, -0.2, 0.0},
};
GLfloat marker_normals[][3] = {
	// Lock (uppåt)
	{0.0, 1.0, 0.0}, {0.0, 1.0, 0.0},
	{0.0, 1.0, 0.0}, {0.0, 1.0, 0.0},
	// Sidor
	{ 0.707, 0.0,  0.707}, { 0.707, 0.0, -0.707},
	{-0.707, 0.0,  0.707}, {-0.707, 0.0, -0.707},
	// Botten
	{ 1.0, 0.0, 0.0}, {0.0, 0.0, -1.0},
	{-1.0, 0.0, 0.0}, {0.0, 0.0,  1.0},
};
GLubyte marker_cubeIndices[] = {
	// Lock
	0, 2, 1,
	2, 3, 1,
	// Sidor
	4, 5, 8,
	5, 7, 9,
	7, 6, 10,
	6, 4, 11,

};

//---------------ny

unsigned int vertexBufferObjID;
unsigned int indexBufferObjID;
unsigned int normalBufferObjID;
unsigned int vertexArrayObjID;
unsigned int vertexArrayObjID2;
unsigned int whiteBufferObjID;

unsigned int pedestalvertexBufferObjID;
unsigned int pedestalindexBufferObjID;
unsigned int pedestalnormalBufferObjID;
unsigned int pedestalvertexArrayObjID;
unsigned int pedestalTexCoordBufferObjID;
//unsigned int pedestalvertexArrayObjID2;

// Markör
unsigned int markervertexBufferObjID;
unsigned int markerindexBufferObjID;
unsigned int markernormalBufferObjID;
unsigned int markervertexArrayObjID;
unsigned int markerTexCoordBufferObjID;

GLuint white_board_program;
GLuint pedestal_program;


unsigned int pieceVertexArrayObjID;
unsigned int pieceVertexBufferObjID;
unsigned int pieceIndexBufferObjID;
unsigned int pieceNormalBufferObjID;
unsigned int pieceTexCoordBufferObjID;

GLuint piece_program;
GLuint moon_program;
GLuint flat_program;
GLuint tex_piece_white;
//GLuint tex_piece_black;
GLuint tex_moon;


Model *m1;
Model *m2;
Model *m3;
Model *m4;
Model *m5;
Model *m6;
Model *m8;

int board[8][8]; //Bräde med pjäser, 0 = tomt, 1 = vit kung, 2 vit dam, ... 11 svart springare, 12 svart bonde

enum { sizePossibleMoves = 512 }; //Teoretiskt antal möjliga drag
enum { sizeMoveInformation = 8 }; //Kan göras större vid behov
int possibleMoves[sizePossibleMoves][sizeMoveInformation]; //0 - pjästyp; 1,2 - nuvarande position; 3,4 - nästa position, 5 - specialdrag (1 - dubbelsteg med bonde, 2 - lång rockad, 3 - kort rockad, 4 - promovering, 5 - en passant); /6 - slagen pjäs//en passant
int possibleMovesCounter; //Pekare på possibleMoves. Ger antal möjliga drag.

int enPassant[3]; //Element 0 anger aktivitet, 1 rad, 2 kolonn

const char pieceReferences[] = { '0', 'K', 'D', 'T', 'L', 'S', 'B'};

int check; //Schack
int whiteMove; //vits drag
int mate; //matt
int tie; //remi


int pieceConstant; //Hör ihop med vems drag det är
int directionConstant; //Samma
int firstRow; // 0 för vit, 7 för svart

enum { sizeKingRays = 8 };
int kingRays[sizeKingRays][2]; //Med strålar avses de riktningar som pjäsen kan gå. Ruta anges i rad, kolonn eller siffra, bokstav

enum { sizeKnightRays = 8 };
int knightRays[sizeKnightRays][2];

enum { sizeRookRays = 4 };
int rookRays[sizeRookRays][2];

enum { sizeBishopRays = 4 };
int bishopRays[sizeBishopRays][2];

void display(); //visar brädet från vits håll

int testInCheck(int row, int column); //testar om  kungen är i schack om den står på row, column

int liftCheck(int oldRow, int oldColumn, int newRow, int newColumn, bool ep); //testar om ett drag häver schacken för egna spelaren; ep = true för en Passant

void init();

char columnCharacter(int column); //Gör om siffran som svarar mot kolonn i systemet till bokstav för utskrift

int ownPieceBlock(int row, int column);

int opposingPieceBlock(int row, int column);

int number(int row) //Samma för raden
{
	return row + 1;
}

void testCastling();

static int program, skybox_program, skybox_texture;
Model *skybox;

static unsigned get_time_in_msecs(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

mat4 projectionMatrix;

struct matrix_info {
	mat4 skybox_matrix, world_matrix;
	vec3 cam;
	vec3 lookAtPoint;
	vec3 cameraUp;
};

static int cam_height = 3;

//static void get_matrices(mat4 *skybox_matrix_out, mat4 *world_matrix_out, int mouse_x, int mouse_y, int *row_out, int *column_out)
static void get_matrices(double camera_angle, struct matrix_info *info_out, int mouse_x, int mouse_y)
{
	mat4 camMatrix;
	mat4 modelView;
	mat4 total;
	info_out->cameraUp = (vec3){ 0.0, 1.0, 0.0 };

	// Beräkna Camera*Modelview för skybox
	info_out->cam = (vec3){12.0 * sin(camera_angle), 4 + cam_height, 12.0 * cos(camera_angle)};
	info_out->lookAtPoint = (vec3){0, 0, 0};
	camMatrix = lookAt(info_out->cam.x, info_out->cam.y, info_out->cam.z,
				info_out->lookAtPoint.x, info_out->lookAtPoint.y, info_out->lookAtPoint.z,
				0.0, 1.0, 0.0);
	modelView = IdentityMatrix();
	total = Mult(Mult(camMatrix, modelView), S(20, 20, 20));

	info_out->skybox_matrix = total;

	// Beräkna Camera*Modelview för värld
	camMatrix = lookAt(info_out->cam.x, info_out->cam.y, info_out->cam.z,
				info_out->lookAtPoint.x, info_out->lookAtPoint.y, info_out->lookAtPoint.z,
				info_out->cameraUp.x, info_out->cameraUp.y, info_out->cameraUp.z);
	modelView = IdentityMatrix();
	total = Mult(camMatrix, modelView);

	info_out->world_matrix = total;
}

static void trace_pointer(struct matrix_info *info, int mouse_x, int mouse_y, int *row_out, int *col_out)
{
	// h och v är vektorer i x- samt y-riktning i världen
	vec3 view = Normalize(VectorSub(info->lookAtPoint, info->cam));
	vec3 h = Normalize(CrossProduct(view, info->cameraUp));
	vec3 v = Normalize(CrossProduct(h, view));

	// Muskoordinater från -0.5 till 0.5.
	double x = ((mouse_x - (double)window_w / 2.0)/(double)window_w);
	double y = ((window_h - mouse_y - 1 - (double)window_h / 2.0)/(double)window_h);

	// Kombinera för att få den punkt i Near-planet där musen ligger.
	vec3 camera_to_mouse_point = VectorAdd(ScalarMult(view, FRUSTUM_NEAR), VectorAdd(ScalarMult(h, x), ScalarMult(v, y)));

	// Strålen går genom kamerans position och den punkten
	vec3 ray_point = VectorAdd(info->cam, ScalarMult(camera_to_mouse_point, 4.0));

	// Stålen går cam + camera_to_mouse_point * t
	// Utnyttjar att brädet är i y = 0.

	// (cam + camera_to_mouse_point * t).y = 0 =>
	// cam.y + camera_to_mouse_point.y * t = 0 =>
	// t = -cam.y / camera_to_mouse_point.y
	double r = -info->cam.y / camera_to_mouse_point.y;
	vec3 intersect = VectorAdd(info->cam, ScalarMult(camera_to_mouse_point, r));

	int board_column = (int)floor(intersect.x + 4.0);
	int board_row = (int)floor(intersect.z + 4.0);
	if(board_column >= 0 && board_column < 8 && board_row >= 0 && board_row < 8) {
		*row_out = board_row;
		*col_out = board_column;
	}
	else {
		*row_out = -1;
		*col_out = -1;
	}
}

static void draw_skybox(mat4 skybox_matrix)
{
	// Skybox
	glUseProgram(skybox_program);
	glBindVertexArray(0);

	glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, skybox_matrix.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glDisable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, skybox_texture);
	glDisable(GL_CULL_FACE);
	DrawModel(skybox, skybox_program, "in_Position", NULL, "inTexCoord");

	glEnable(GL_DEPTH_TEST);

}

static void draw_board(mat4 total)
{
	//Draw board
	
	int x,z;
	float fieldsize = 1.0;
	mat4 translMatrix;
	
      glUseProgram(white_board_program);
	for(z=-3; z<5; z++){
	  for(x=-3; x<5; x++){
	    //Translation
	    translMatrix = Mult(T(fieldsize*(float)x - 0.5*fieldsize, 0.0, fieldsize*(float)z - 0.5*fieldsize), S(fieldsize,fieldsize,fieldsize));
	
	    //draw either black or white
	    
	    if(((z==-3.0 || z==-1.0 || z==1.0 || z==3.0) && (x==-2.0 || x==0.0 || x==2.0 || x==4.0))||((z==-2.0 || z==0.0 || z==2.0 || z==4.0) && (x==-3.0 || x==-1.0 || x==1.0 || x==3.0))){
	      glBindVertexArray(vertexArrayObjID2);
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "translMatrix"), 1, GL_TRUE, translMatrix.m);	
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "mdlMatrix"), 1, GL_TRUE, total.m);
	      GLfloat white[] = {0.25, 0.13, 0.0, 0.7};
	      glUniform4fv(glGetUniformLocation(white_board_program, "color"), 1, white);
	      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	      
	    }else{
	      glBindVertexArray(vertexArrayObjID2);
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "translMatrix"), 1, GL_TRUE, translMatrix.m);	
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "mdlMatrix"), 1, GL_TRUE, total.m);
	      GLfloat white[] = {0.9, 0.85, 0.2, 0.2};
	      glUniform4fv(glGetUniformLocation(white_board_program, "color"), 1, white);
	      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	    }		
	  }
	}
}

static void draw_board_pedestal(mat4 world_matrix)
{
	mat4 translMatrix;
	
	//wooden pedestal
	translMatrix = Mult(S(8.5, 0.3, 8.5), T(0.0, -0.6001, 0.0));
	
	glUseProgram(pedestal_program);
	glBindVertexArray(pedestalvertexArrayObjID);
	glUniformMatrix4fv(glGetUniformLocation(pedestal_program, "translMatrix"), 1, GL_TRUE, translMatrix.m);	
	glUniformMatrix4fv(glGetUniformLocation(pedestal_program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(pedestal_program, "mdlMatrix"), 1, GL_TRUE, world_matrix.m);
	
	GLfloat wood[] = {0.2, 0.2, 0.2, 0.0};
	glUniform4fv(glGetUniformLocation(pedestal_program, "color"), 1, wood);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, NULL);
	
}

static void draw_marker(mat4 world_matrix, int row, int column, GLfloat *color, double t)
{
	if(row == -1) return;

	mat4 rotMatrix = Mult(T((double)column - 3.5, 2.2, (double)row - 3.5),S(.9, .9, .9));
	rotMatrix = Mult(rotMatrix, Ry(t/(double)300));
	
	glUseProgram(flat_program);

	glUniform4fv(glGetUniformLocation(flat_program, "color"), 1, color);
	glUniformMatrix4fv(glGetUniformLocation(flat_program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(flat_program, "mdlMatrix"), 1, GL_TRUE, world_matrix.m);
	glUniformMatrix4fv(glGetUniformLocation(flat_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);

	glBindVertexArray(markervertexArrayObjID);

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glFrontFace(GL_CW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawElements(GL_TRIANGLES, sizeof marker_cubeIndices / sizeof marker_cubeIndices[0], GL_UNSIGNED_BYTE, NULL);

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}

static void draw_scene(mat4 world_matrix, vec3 cam, int row, int column, int primary_color, int secondary_row, int secondary_col, int secondary_color, double secondary_stop_time)
{
	GLfloat light[] = {-0.18, 0.58, 0.58};

	//Rita upp pjäser
	glUseProgram(piece_program);
	glUniform3fv(glGetUniformLocation(piece_program, "light"), 1, light);
	int i,j;
	for(i=0;i<8;++i){
		for(j=0;j<8;++j){
			mat4 rotMatrix = Mult(T(i - 3.5, 0.0, j - 3.5),S(0.05, 0.05, 0.05));
			mat4 tmp = rotMatrix;

			//white--------------------------
			GLfloat dark[] = {0.9, 0.8, 0.6, 0.0};
			glUniform4fv(glGetUniformLocation(piece_program, "color"), 1, dark);

			glUniformMatrix4fv(glGetUniformLocation(piece_program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
			glUniformMatrix4fv(glGetUniformLocation(piece_program, "mdlMatrix"), 1, GL_TRUE, world_matrix.m);

			rotMatrix = Mult(rotMatrix, Ry(M_PI));

			//rook	
			if(board[i][j] == 3) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m1, piece_program, "in_Position", "in_Normal", NULL);
			}

			//knight
			if(board[i][j] == 5) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m2, piece_program, "in_Position", "in_Normal", NULL);
			}

			//bishop
			if(board[i][j] == 4) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m3, piece_program, "in_Position", "in_Normal", NULL);
			}

			//queen
			if(board[i][j] == 2) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m5, piece_program, "in_Position", "in_Normal", NULL);
			}

			//king
			if(board[i][j] == 1) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m4, piece_program, "in_Position", "in_Normal", NULL);
			}

			//pawn
			if(board[i][j] == 6) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m6, piece_program, "in_Position", "in_Normal", NULL);
			}

			//black--------------------------
			GLfloat bright[] = {0.4, 0.2, 0.1, 0.0};
				glUniform4fv(glGetUniformLocation(piece_program, "color"), 1, bright);
			rotMatrix = tmp;

			//rooks
			if(board[i][j] == 9) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m1, piece_program, "in_Position", "in_Normal", NULL);
			}

			//knight
			if(board[i][j] == 11) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m2, piece_program, "in_Position", "in_Normal", NULL);
			}

			//bishop
			if(board[i][j] == 10) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m3, piece_program, "in_Position", "in_Normal", NULL);
			}

			//king
			if(board[i][j] == 7) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m4, piece_program, "in_Position", "in_Normal", NULL);
			}

			//queen
			if(board[i][j] == 8) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m5, piece_program, "in_Position", "in_Normal", NULL);
			}

			//pawn
			if(board[i][j] == 12) {
				glUniformMatrix4fv(glGetUniformLocation(piece_program, "rotMatrix"), 1, GL_TRUE, rotMatrix.m);
				DrawModel(m6, piece_program, "in_Position", "in_Normal", NULL);
			}
		}
	}

	glUseProgram(moon_program);

	GLfloat dark[] = {0.9, 0.8, 0.6, 0.0};
	glUniform4fv(glGetUniformLocation(moon_program, "color"), 1, dark);
	glUniformMatrix4fv(glGetUniformLocation(moon_program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(moon_program, "mdlMatrix"), 1, GL_TRUE, world_matrix.m);

	glUniform3fv(glGetUniformLocation(moon_program, "light"), 1, light);

	double t = get_time_in_msecs();
	mat4 matrix1 = Mult(Mult(T(20.0 * sin(t*0.0005), 5.0, 20.0 * cos(t*0.0005)),S(0.05, 0.05, 0.05)), Ry(t*0.0005));
	glUniformMatrix4fv(glGetUniformLocation(moon_program, "rotMatrix"), 1, GL_TRUE, matrix1.m);

	glBindTexture(GL_TEXTURE_2D, tex_moon);
	DrawModel(m8, moon_program, "in_Position", "in_Normal", "in_TexCoord");

	// Markör
	GLfloat marker_color[][4] = {
		{1.0, 1.0, 0.4, 0.7}, // Kan flytta denna pjäs
		{0.4, 0.4, 1.0, 0.7}, // Pjäs som flyttas
		{0.9, 0.8, 0.6, 0.7}, // Det är vits drag
		{0.4, 0.2, 0.1, 0.7}, // Det är svarts drag
	};

	// Alpha sorting between 2 markers
	if(Norm(VectorSub(cam, SetVector((double)column - 3.5, 2.2, (double)row - 3.5))) > Norm(VectorSub(cam, SetVector((double)secondary_col - 3.5, 2.2, (double)secondary_row - 3.5)))) {
		// Draw primary marker first
		if(primary_color != -1) draw_marker(world_matrix, row, column, marker_color[primary_color], t);
		if(secondary_color != -1) draw_marker(world_matrix, secondary_row, secondary_col, marker_color[secondary_color], secondary_stop_time);
	}
	else {
		// Draw secondary marker first
		if(secondary_color != -1) draw_marker(world_matrix, secondary_row, secondary_col, marker_color[secondary_color], secondary_stop_time);
		if(primary_color != -1) draw_marker(world_matrix, row, column, marker_color[primary_color], t);
	}
}

static void draw_the_whole_thing(struct matrix_info *info, int row, int col, int primary_color, int secondary_row, int secondary_col, int secondary_color, double secondary_stop_time)
{
	// Rita nu upp schackbrädet
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* Rita skybox (utan depth test). */
	draw_skybox(info->skybox_matrix);

	/* Rita bräde in i stencilbuffer (utan djup- eller färgbuffer). */
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
	draw_board(info->world_matrix);

	/* På med stencil test. */
	glStencilFunc(GL_EQUAL, 1, 0xffffffff); /* Draw if stencil == 1. */
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	/* Rita reflekterad skybox */
	mat4 skybox_reflection = Mult(info->skybox_matrix, S(1, -1 , 1));
	draw_skybox(skybox_reflection);

	/* Rita reflekterad scen (scen betyder pjäser, måne och markörer, men inget som är under spegeln) */
	mat4 world_reflection = Mult(info->world_matrix, S(1, -1 , 1));
	glEnable(GL_DEPTH_TEST);
	draw_scene(world_reflection, info->cam, row, col, primary_color, secondary_row, secondary_col, secondary_color, secondary_stop_time);

	/* Stäng av stencilbuffern. */
	glDisable(GL_STENCIL_TEST);

	/* Rita bräde normalt med blending. */
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	draw_board(info->world_matrix);
	glDisable(GL_BLEND);

#if 1
	/* Rita pedestalen. */
	draw_board_pedestal(info->world_matrix);

	/* Rita resten. */
	draw_scene(info->world_matrix, info->cam, row, col, primary_color, secondary_row, secondary_col, secondary_color, secondary_stop_time);
#endif

	SDL_GL_SwapBuffers();
}

static int handle_wm_events(SDL_Event *ev)
{
	if(ev->type == SDL_QUIT) {
		return -1;
	}
	else if(ev->type == SDL_KEYDOWN) {
		if(ev->key.keysym.sym == SDLK_ESCAPE) return -1;
	}
	else if(ev->type == SDL_VIDEORESIZE) {
		window_w = ev->resize.w;
		window_h = ev->resize.h;
		SDL_SetVideoMode(window_w, window_h, 32, SDL_OPENGL | SDL_RESIZABLE);
		glViewport(0, 0, window_w, window_h);
	}
	return 0;
}

static unsigned piece_can_move(int row, int col)
{
	unsigned i;
	if(row == -1) return 0;
	for(i = 0; i < possibleMovesCounter; ++i) {
		if(possibleMoves[i][1] == col && possibleMoves[i][2] == row) {
			return 1;
		}
	}
	return 0;
}

static unsigned is_possible_destination_of(int dst_row, int dst_col, int row, int col, int *move)
{
	unsigned i;
	if(dst_row == -1) return 0;
	if(row == -1) return 0;
	for(i = 0; i < possibleMovesCounter; ++i) {
		if(possibleMoves[i][1] == col && possibleMoves[i][2] == row && possibleMoves[i][3] == dst_col && possibleMoves[i][4] == dst_row) {
			if(move) *move = i;
			return 1;
		}
	}
	return 0;
}

// "from", "to" och returvärde i radianer.
static double cw_interpolate(double from, double to, unsigned time)
{
	if(to < from) to += 2.0 * M_PI;

	return remainder(to - exp(-(double)time / 900.0) * (to - from), 2.0 * M_PI);
}


int getInput(double *camera_direction)
{
	unsigned start_time = get_time_in_msecs();
	int x, y;
	enum { HOVER, DRAG, INVALID_DRAG } mouse_state = HOVER;
	char buf[16];
	unsigned buf_p = 0;

	int move_start_row, move_start_col;
	double secondary_stop_time;
	double start_camera = *camera_direction;

	while(1) {
		// Så vi vet var på brädet musen pekar.
		struct matrix_info info;
		int row, col;

		*camera_direction = cw_interpolate(start_camera, whiteMove ? -M_PI/2.0 : M_PI/2.0, get_time_in_msecs() - start_time);
		get_matrices(*camera_direction, &info, x, y);
		if(x != -1) {
			trace_pointer(&info, x, y, &row, &col);
		}
		else {
			row = -1;
			col = -1;
		}

		// Har vi fått någon inmatning i terminalen?
		while(read(0, buf + buf_p, 1) > 0) {
			if(buf[buf_p] == '\n') {
				char *end;
				long move = strtol(buf, &end, 10);
				if(buf_p != 0 && end == buf + buf_p) {
					// Vi har fått ett nummer.
					if(move >= 0 && move < possibleMovesCounter) {
						// Giltigt
						return move;
					}
				}
				// Fick nåt ogiltigt. Bara att läsa en ny rad.
				buf_p = 0;
			}
			else {
				++buf_p;
				if(buf_p == 16) buf_p = 0;
			}
		}

		// Har vi fått någon inmatning i fönstret?
		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {
			if(handle_wm_events(&ev) < 0) return -1;
			else if(ev.type == SDL_MOUSEMOTION) {
				x = ev.motion.x;
				y = ev.motion.y;
				trace_pointer(&info, x, y, &row, &col);
			}
			else if(ev.type == SDL_ACTIVEEVENT && ev.active.gain == 0) {
				x = -1;
				row = -1;
				col = -1;
			}
			else if(ev.button.button == SDL_BUTTON_LEFT && ev.type == SDL_MOUSEBUTTONDOWN) {
				unsigned i;
				x = ev.button.x;
				y = ev.button.y;
				trace_pointer(&info, x, y, &row, &col);
				if(piece_can_move(row, col)) {
					mouse_state = DRAG;
					move_start_row = row;
					move_start_col = col;
					secondary_stop_time = get_time_in_msecs();
				}
				else {
					mouse_state = INVALID_DRAG;
				}
			}
			else if(ev.button.button == SDL_BUTTON_LEFT && ev.type == SDL_MOUSEBUTTONUP) {
				int move;
				if(mouse_state == DRAG) {
					x = ev.button.x;
					y = ev.button.y;
					trace_pointer(&info, x, y, &row, &col);
					if(is_possible_destination_of(row, col, move_start_row, move_start_col, &move)) return move;
				}
				mouse_state = HOVER;
			}
			else if(ev.button.button == SDL_BUTTON_WHEELUP && ev.type == SDL_MOUSEBUTTONDOWN) {
				cam_height += 1;
				if(cam_height > 10) cam_height = 10;
			}
			else if(ev.button.button == SDL_BUTTON_WHEELDOWN && ev.type == SDL_MOUSEBUTTONDOWN) {
				cam_height -= 1;
				if(cam_height < 1) cam_height = 1;
			}
		}

		// All inmatning behandlad. Klura ut markörernas position och rita upp.
		int secondary_row;
		int secondary_col;

		int primary_color = -1;
		int secondary_color = -1;

		if(mouse_state == HOVER) {
			if(piece_can_move(row, col)) primary_color = 0;
			else primary_color = whiteMove ? 2 : 3;
		}
		else if(mouse_state == DRAG) {
			primary_color = 0;
			if(row == move_start_row && col == move_start_col) {
				primary_color = -1;
			}
			else {
				if(is_possible_destination_of(row, col, move_start_row, move_start_col, NULL)) {
					primary_color = 0;
				}
				else {
					primary_color = whiteMove ? 2 : 3;
				}
			}
			secondary_row = move_start_row;
			secondary_col = move_start_col;
			secondary_color = 1;
		}

		draw_the_whole_thing(&info, row, col, primary_color, secondary_row, secondary_col, secondary_color, secondary_stop_time);
	}
}

static int get_piece(double *camera_direction)
{
	int board_backup[8][8];
	memcpy(board_backup, board, sizeof board);

	static const int white_pieces[8][8] = {
		[4][2] = 2,
		[4][3] = 3,
		[4][4] = 4,
		[4][5] = 5,
	};
	static const int black_pieces[8][8] = {
		[3][2] = 8,
		[3][3] = 9,
		[3][4] = 10,
		[3][5] = 11,
	};
	memcpy(board, whiteMove ? white_pieces : black_pieces, sizeof board);

	unsigned start_time = get_time_in_msecs();
	double start_camera = *camera_direction;
	int x, y;
retry:	printf("Valj ny pjäs\n");
	for (int i = 2; i <= 5; i++)
	{
		printf("%c\n", pieceReferences[i]);
	}

	while(1) {
		struct matrix_info info;
		int row, col;

		*camera_direction = cw_interpolate(start_camera, whiteMove ? -M_PI/2.0 : M_PI/2.0, get_time_in_msecs() - start_time);
		get_matrices(*camera_direction, &info, x, y);
		if(x != -1) {
			trace_pointer(&info, x, y, &row, &col);
		}
		else {
			row = -1;
			col = -1;
		}

		// Har vi fått någon inmatning?
		int bytes;
		ioctl(fileno(stdin), FIONREAD, &bytes);
		if(bytes > 0) {
			char move;
			if(scanf("%c", &move) != 1) return -1;

			for (int i = 2; i <= 5; i++)
			{
				if (pieceReferences[i] == move) {
					memcpy(board, board_backup, sizeof board);
					return pieceConstant + i;
				}
			}
			goto retry;
		}

		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {
			if(handle_wm_events(&ev) < 0) return -1;
			else if(ev.type == SDL_MOUSEMOTION) {
				x = ev.motion.x;
				y = ev.motion.y;
				trace_pointer(&info, x, y, &row, &col);
			}
			else if(ev.type == SDL_ACTIVEEVENT && ev.active.gain == 0) {
				x = -1;
				row = -1;
				col = -1;
			}
			else if(ev.button.button == SDL_BUTTON_LEFT && ev.type == SDL_MOUSEBUTTONDOWN) {
				if(row != -1 && col != -1 && board[col][row] != 0) {
					int new_piece = board[col][row];
					memcpy(board, board_backup, sizeof board);
					return new_piece;
				}
			}
		}

		// All inmatning behandlad.
		int color = 0;
		if(row == -1 || col == -1) color = -1;
		if(board[col][row] == 0) color = -1;
		draw_the_whole_thing(&info, row, col, color, -1, -1, -1, 0.0);
	}

}

static void on_checkmate(double *camera_direction)
{
	unsigned start_time = get_time_in_msecs();
	double start_camera = *camera_direction;
	while(1) {
		// Har vi fått någon inmatning?
		int bytes;
		ioctl(fileno(stdin), FIONREAD, &bytes);
		if(bytes > 0) {
			return;
		}

		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {
			if(handle_wm_events(&ev) < 0) return;
			if(ev.type == SDL_KEYDOWN) return;
		}

		// All inmatning behandlad.
		struct matrix_info info;
		*camera_direction = remainder(start_camera + (get_time_in_msecs() - start_time) / 5000.0, 2.0 * M_PI);
		get_matrices(*camera_direction, &info, -1, -1);
		draw_the_whole_thing(&info, -1, -1, -1, -1, -1, -1, 0.0);
	}
}

static int read_ch(int pgn_file) {
	char c;
	if(read(pgn_file, &c, 1) < 1) return -1;
	return c;
}
static int replay_promote_to;
static int interpret_replay_move(int pgn_file)
{
	int c = read_ch(pgn_file);
	while(1) {
		// Hoppa över mellanslag
		if(strchr(" \t\n\r", c)) {
			c = read_ch(pgn_file);
			continue;
		}
		// Hoppa över kommentarer
		if(c == '[' || c == '{') {
			int first_bracket = c;
			while(1) {
				c = read_ch(pgn_file);
				if(c < 0) abort();
				if(c == (first_bracket == '[' ? ']' : '}')) break;
			}
			c = read_ch(pgn_file);
			continue;
		}
		// Läs till mellanslag.
		char move[16] = {};
		unsigned i = 0;
		while(1) {
			if(c < 0) abort();
			if(strchr(" \t\n\r", c)) break;
			if(c == '.') {
				// Ta bort allt innan punkt.
				memset(move, 0, sizeof move);
				i = 0;
				c = read_ch(pgn_file);
				continue;
			}
			move[i++] = c;
			if(i == 15) abort();
			c = read_ch(pgn_file);
		}
		// Tolka drag
		unsigned len = strlen(move);
		if(len < 2) abort();

		if(move[len - 2] == '=') {
			// Befodra bonde //Promovera heter det /Mikael
			if(!strchr("QRBN", move[len - 1])) abort();
			char pieces[] = " KQRBNP";
			int piece = strchr(pieces, move[len - 1]) - pieces;
			if(!whiteMove) piece += 6;
			replay_promote_to = piece;
			len -= 2;
		}

		int piece_type = 'P', dst_row = -1, dst_col = -1, src_row = -1, src_col = -1;
		if(!strcmp(move, "O-O") || !strcmp(move, "O-O-O")) {
			src_col = 4;
			if(!strcmp(move, "O-O")) dst_col = 6;
			else dst_col = 1;
			if(whiteMove) {
				src_row = 0;
				dst_row = 0;
			}
			else {
				src_row = 7;
				dst_row = 7;
			}
			piece_type = 'K';
			goto find_move;
		}

		if(strchr(move, '-')) {
			// Det är en poängräkning vi har vilket innebär att vår spelare gett up.
			return -2;
		}

		// Strunta i + och #
		if(move[len - 1] == '+' || move[len - 1] == '#') {
			move[len - 1] = '\0';
			--len;
		}
		// Destination
		if(!strchr("abcdefgh", move[len - 2])) abort();
		dst_col = move[len - 2] - 'a';
		if(!strchr("12345678", move[len - 1])) abort();
		dst_row = move[len - 1] - '1';

		len -= 2;
		if(len) {
			// Pjäs?
			if(strchr("KQRBN", move[0])) {
				piece_type = move[0];
				memmove(move, move + 1, 15);
				--len;
			}
		}

		if(len) {
			// Strunta i x
			if(move[len - 1] == 'x') --len;

			unsigned has_col = 0, has_row = 0;
			if(len == 2) {
				// Exakt ruta
				has_col = 1;
				has_row = 1;
			}
			else if(len == 1) {
				// Rad eller kolumn
				if(strchr("12345678", move[len - 1])) has_row = 1;
				else has_col = 1;
			}

			if(has_row) {
				if(!strchr("12345678", move[len - 1])) abort();
				src_row = move[--len] - '1';
			}
			if(has_col) {
				if(!strchr("abcdefgh", move[len - 1])) abort();
				src_col = move[--len] - 'a';
			}
			if(len != 0) abort();
		}

find_move:;
		char pieces[] = " KQRBNP";
		int piece = strchr(pieces, piece_type) - pieces;
		if(!whiteMove) piece += 6;
		unsigned j;
		for(j = 0; j < possibleMovesCounter; ++j) {
			unsigned possible = 1;
			// Exkludera felaktiga drag baserat på den information vi fått.
			if(possibleMoves[j][0] != piece) possible = 0;
			if(src_row != -1 && possibleMoves[j][1] != src_row) possible = 0;
			if(src_col != -1 && possibleMoves[j][2] != src_col) possible = 0;
			if(possibleMoves[j][3] != dst_row) possible = 0;
			if(possibleMoves[j][4] != dst_col) possible = 0;
			if(possible) {
				// Rätt drag funnet
				return j;
			}
		}
		abort();
	}
}

static int replay(double *camera_direction, unsigned *start_time_p, int pgn_file)
{
	unsigned start_time = *start_time_p;
	double start_camera = *camera_direction;
	unsigned paused = 0;
	while(1) {
		struct matrix_info info;
		if(!paused) *start_time_p = get_time_in_msecs();
		*camera_direction = remainder(start_camera + (*start_time_p - start_time) / 5000.0, 2.0 * M_PI);
		get_matrices(*camera_direction, &info, -1, -1);
		draw_the_whole_thing(&info, -1, -1, -1, -1, -1, -1, 0.0);

		// Drag utförs med 1 sekunds intevall.
		if(!paused && get_time_in_msecs() - start_time >= 1000) {
			return interpret_replay_move(pgn_file);
		}

		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {
			if(handle_wm_events(&ev) < 0) return -1;
			else if(ev.type == SDL_MOUSEBUTTONDOWN) {
				paused = 1;
			}
			else if(ev.type == SDL_MOUSEBUTTONUP) {
				paused = 0;
				start_time = get_time_in_msecs();
				start_camera = *camera_direction;
			}
		}
	}
}

int castlingPossibility[2][2]; // Om elementen på följande platser är 1, har inte de eventuellt rockerande pjäserna flyttats: 0,0 - Vit kan rockera på damsidan; 0,1 - Vit kan rockera på kungssidan; 1,0 - Svart kan rockera på damsidan; 1,1 - Svart kan rockera på kungssidan

static void init_gl(void)
{
	glViewport(0, 0, DEFAULT_W, DEFAULT_H);
	glClearColor(1, 0, 0, 0);

	projectionMatrix = frustum(FRUSTUM_LEFT, FRUSTUM_RIGHT, FRUSTUM_BOTTOM, FRUSTUM_TOP, FRUSTUM_NEAR, FRUSTUM_FAR);

	// Shaders
	skybox_program = loadShaders("skybox.vert", "skybox.frag");
	white_board_program = loadShaders("board_white.vert", "board_white.frag");
	pedestal_program = loadShaders("pedestal.vert", "pedestal.frag");
	
	piece_program = loadShaders("piece.vert", "piece.frag");
	moon_program = loadShaders("moon.vert", "moon.frag");
	flat_program = loadShaders("flat.vert", "flat.frag");
	
	
	m1 = LoadModelPlus("rook.obj");
	m2 = LoadModelPlus("knight.obj");
	m3 = LoadModelPlus("bishop.obj");
	m4 = LoadModelPlus("king.obj");
	m5 = LoadModelPlus("queen.obj");
	m6 = LoadModelPlus("pawn.obj");

	m8 = LoadModelPlus("moon.obj");
	LoadTGATextureSimple("moon.tga", &tex_moon);

	glGenVertexArrays(1, &pieceVertexArrayObjID);
	glGenBuffers(1, &pieceVertexBufferObjID);
	glGenBuffers(1, &pieceIndexBufferObjID);
	glGenBuffers(1, &pieceNormalBufferObjID);
	
	glBindVertexArray(pieceVertexArrayObjID);
    
	
	program = loadShaders("schack.vert", "schack.frag");
	printError("Shaders");

	skybox = LoadModelPlus("skybox.obj");

	LoadTGATextureSimple("nebula_tut10.tga", &skybox_texture);
	
	glGenBuffers(1, &vertexBufferObjID);
	glGenBuffers(1, &indexBufferObjID);
	glGenBuffers(1, &normalBufferObjID);
	//glGenBuffers(1, &blackBufferObjID);
	glGenBuffers(1, &whiteBufferObjID);
	
	glGenVertexArrays(1, &vertexArrayObjID);
	glBindVertexArray(vertexArrayObjID);
	
	//white board
	glGenVertexArrays(1, &vertexArrayObjID2);
	glBindVertexArray(vertexArrayObjID2);

	glUseProgram(white_board_program);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(white_board_program, "in_Position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(white_board_program, "in_Position"));
	
	glBindBuffer(GL_ARRAY_BUFFER, normalBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(GLfloat), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(white_board_program, "in_Normal"), 3, GL_FLOAT, GL_FALSE, 0,0);
	glEnableVertexAttribArray(glGetAttribLocation(white_board_program, "in_Normal"));
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObjID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLuint), cubeIndices, GL_STATIC_DRAW);

	//pedestal
	glGenBuffers(1, &pedestalvertexBufferObjID);
	glGenBuffers(1, &pedestalindexBufferObjID);
	glGenBuffers(1, &pedestalnormalBufferObjID);
	glGenBuffers(1, &pedestalTexCoordBufferObjID);
	
	glGenVertexArrays(1, &pedestalvertexArrayObjID);
	glBindVertexArray(pedestalvertexArrayObjID);

	glUseProgram(pedestal_program);

	glBindBuffer(GL_ARRAY_BUFFER, pedestalvertexBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 8*3*sizeof(GLfloat), ped_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(pedestal_program, "in_Position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(pedestal_program, "in_Position"));
	
	glBindBuffer(GL_ARRAY_BUFFER, pedestalnormalBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 8*3*sizeof(GLfloat), ped_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(pedestal_program, "in_Normal"), 3, GL_FLOAT, GL_FALSE, 0,0);
	glEnableVertexAttribArray(glGetAttribLocation(pedestal_program, "in_Normal"));
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pedestalindexBufferObjID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24*sizeof(GLuint), ped_cubeIndices, GL_STATIC_DRAW);
	
	//Markör
	glGenBuffers(1, &markervertexBufferObjID);
	glGenBuffers(1, &markerindexBufferObjID);
	glGenBuffers(1, &markernormalBufferObjID);
	
	glGenVertexArrays(1, &markervertexArrayObjID);
	glBindVertexArray(markervertexArrayObjID);

	glUseProgram(flat_program);

	glBindBuffer(GL_ARRAY_BUFFER, markervertexBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, sizeof marker_vertices, marker_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(flat_program, "in_Position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(flat_program, "in_Position"));
	
	glBindBuffer(GL_ARRAY_BUFFER, markernormalBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, sizeof marker_normals, marker_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(flat_program, "in_Normal"), 3, GL_FLOAT, GL_FALSE, 0,0);
	glEnableVertexAttribArray(glGetAttribLocation(flat_program, "in_Normal"));
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, markerindexBufferObjID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof marker_cubeIndices, marker_cubeIndices, GL_STATIC_DRAW);

	// Måne
	glUseProgram(moon_program);
	glUniform1i(glGetUniformLocation(moon_program, "texUnit"), 0); // Texture unit 0
	//Skybox
	glUseProgram(skybox_program);
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit"), 0); // Texture unit 0
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "tex"), 0); // Texture unit 0

	printError("end of init");
}

int main(int argc, char **argv)
{
	int pgn_fd = -1;
	unsigned player_resigned = 0;

	fcntl(0, F_SETFL, O_NONBLOCK);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
//	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
//	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	window_w = DEFAULT_W;
	window_h = DEFAULT_H;
	SDL_SetVideoMode(window_w, window_h, 32, SDL_OPENGL | SDL_RESIZABLE);

	init_gl();

	init();

	if(argc == 2) {
		// Ska spela replay.
		pgn_fd = open(argv[1], O_RDONLY);
		if(pgn_fd < 0) {
			printf("Ingen sådan fil.\n");
			return 1;
		}
	}
	else if(argc != 1) {
		return 1;
	}

	//starta spelet
	//Återställ en passant-data

	double camera_direction = M_PI;
	unsigned start_time = get_time_in_msecs();

	while(mate == 0 && tie == 0)
	{
		if(whiteMove)
		{
			pieceConstant = 0;
			directionConstant = 1;
			firstRow = 0;
		}
		else
		{
			pieceConstant = 6;
			directionConstant = -1;
			firstRow = 7;
		}

		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				if (board[i][j] == 1 + pieceConstant) //egen kung i schack?
				{
					check = check || testInCheck(i,j);
				}
			}
		}

		if (check)
		{
			printf("Schack!\n");
		}


		int testRow; //Rörliga variabler för att testa
		int testColumn;

		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				if (board[i][j] == 0)
				{
					continue;
				}

				if (board[i][j] == pieceConstant + 6) //bonde
				{
					testRow = i + directionConstant;
					testColumn = j;

					if (board[testRow][testColumn] == 0) //tom ruta
					{
						if (liftCheck(i, j, testRow, testColumn, false))
						{
							//Här kan promovering ske
							possibleMoves[possibleMovesCounter][0] = board[i][j];
							possibleMoves[possibleMovesCounter][1] = i; //nuvarande rad
							possibleMoves[possibleMovesCounter][2] = j; //nuvarande kolonn
							possibleMoves[possibleMovesCounter][3] = testRow; //ny rad
							possibleMoves[possibleMovesCounter][4] = testColumn; //ny kolonn

							possibleMovesCounter++;
						}

						if (i == firstRow + directionConstant) //orörd pjäs
						{
							testRow += directionConstant; //dubbelsteg

							if(board[testRow][testColumn] == 0) //tom ruta
							{

								if (liftCheck(i, j, testRow, testColumn, false))
								{
									possibleMoves[possibleMovesCounter][0] = board[i][j];
									possibleMoves[possibleMovesCounter][1] = i; //nuvarande rad
									possibleMoves[possibleMovesCounter][2] = j; //nuvarande kolonn
									possibleMoves[possibleMovesCounter][3] = testRow; //ny rad
									possibleMoves[possibleMovesCounter][4] = testColumn; //ny kolonn
									possibleMoves[possibleMovesCounter][5] = 1; //dubbelsteg

									possibleMovesCounter++;
								}
							}
						}
					}

					testRow = i + directionConstant;
					for (int l = -1; l < 3; l += 2)
					{
						testColumn = j + l;
						if (testColumn >= 0 && testColumn <= 7)
						{
							if (opposingPieceBlock(testRow, testColumn)) //För slag, måste det finnas en motståndarpjäs
							{
								if (liftCheck(i, j, testRow, testColumn, false))
								{
									possibleMoves[possibleMovesCounter][0] = board[i][j];
									possibleMoves[possibleMovesCounter][1] = i; //nuvarande rad
									possibleMoves[possibleMovesCounter][2] = j; //nuvarande kolonn
									possibleMoves[possibleMovesCounter][3] = testRow; //ny rad
									possibleMoves[possibleMovesCounter][4] = testColumn; //ny kolonn

									possibleMovesCounter++;
								}
							}
						}
					}

					if (enPassant[0] == 1 && enPassant[1] == i && (enPassant[2] == j + 1 || enPassant[2] == j - 1)) //En passant - samma rad, kolonnen bredvid
					{
						if (liftCheck(i, j, i + directionConstant, enPassant[2], true))
						{
							possibleMoves[possibleMovesCounter][0] = board[i][j];
							possibleMoves[possibleMovesCounter][1] = i; //nuvarande rad
							possibleMoves[possibleMovesCounter][2] = j; //nuvarande kolonn
							possibleMoves[possibleMovesCounter][3] = i + directionConstant; //ny rad
							possibleMoves[possibleMovesCounter][4] = enPassant[2]; //ny kolonn
							possibleMoves[possibleMovesCounter][5] = 5;

							possibleMovesCounter++;
						}
					}
				}
				

				if (board[i][j] == pieceConstant + 1)	//kung?
				{
					for (int k = 0; k < sizeKingRays; k++)
					{
						testRow = i + kingRays[k][0];
						testColumn = j + kingRays[k][1];

						if(testRow > 7 || testRow < 0 || testColumn > 7 || testColumn < 0) //utanför brädet?
						{
							continue;
						}
						
						if(ownPieceBlock(testRow,testColumn))
						{
							continue;
						}
						
						if(testInCheck(testRow, testColumn)) //flytta in i schack?
						{
							continue;
						}

						possibleMoves[possibleMovesCounter][0] = board[i][j]; //kung
						possibleMoves[possibleMovesCounter][1] = i; //nuvarande rad
						possibleMoves[possibleMovesCounter][2] = j; //nuvarande kolonn
						possibleMoves[possibleMovesCounter][3] = testRow; //ny rad
						possibleMoves[possibleMovesCounter][4] = testColumn; //ny kolonn

						possibleMovesCounter++;
					}
					testCastling();
				}
				else if (board[i][j] == pieceConstant + 5) //Springare
				{
					for (int k = 0; k < sizeKnightRays; k++)
					{
						testRow = i + knightRays[k][0];
						testColumn = j + knightRays[k][1];

						if (testRow > 7 || testRow < 0 || testColumn > 7 || testColumn < 0)
						{
							continue;
						}
						if(ownPieceBlock(testRow,testColumn)) //Står egen pjäs i vägen?
						{
							continue;
						}
						if (!liftCheck(i,j,testRow,testColumn,false)) //Orsakar den/Häver den ej/ schack?
						{
							continue;
						}

						possibleMoves[possibleMovesCounter][0] = board[i][j];
						possibleMoves[possibleMovesCounter][1] = i; //nuvarande rad
						possibleMoves[possibleMovesCounter][2] = j; //nuvarande kolonn
						possibleMoves[possibleMovesCounter][3] = testRow; //ny rad
						possibleMoves[possibleMovesCounter][4] = testColumn; //ny kolonn

						possibleMovesCounter++;
					}
				}
				else if (board[i][j] == pieceConstant + 3) //Torn
				{
					int block;
					for (int k = 0; k < sizeRookRays; k++)
					{
						testRow = i;
						testColumn = j;
						block = 0;
						while (!block)
						{
							testRow += rookRays[k][0];
							testColumn += rookRays[k][1];

							//cout << "testpos " << columnCharacter(testColumn + 1) << number(testRow) << endl;

							if (testRow > 7 || testRow < 0 || testColumn > 7 || testColumn < 0)
							{
								break;
							}
							if(ownPieceBlock(testRow,testColumn)) //Står egen pjäs i vägen?
							{
								break;
							}
							if(opposingPieceBlock(testRow, testColumn))
							{
								block = 1; //Detta drag ska sparas, men inga fler drag kan göras längs strålen
							}
							if (!liftCheck(i,j,testRow,testColumn,false)) //Orsakar den/Häver den ej/ schack?
							{
								continue;
							}

							possibleMoves[possibleMovesCounter][0] = board[i][j];
							possibleMoves[possibleMovesCounter][1] = i; //nuvarande rad
							possibleMoves[possibleMovesCounter][2] = j; //nuvarande kolonn
							possibleMoves[possibleMovesCounter][3] = testRow; //ny rad
							possibleMoves[possibleMovesCounter][4] = testColumn; //ny kolonn

							possibleMovesCounter++;
						}
					}
				}
				else if (board[i][j] == pieceConstant + 4) //Löpare
				{
					int block;
					for (int k = 0; k < sizeBishopRays; k++)
					{
						testRow = i;
						testColumn = j;
						block = 0;
						while (!block)
						{
							testRow += bishopRays[k][0];
							testColumn += bishopRays[k][1];

							if (testRow > 7 || testRow < 0 || testColumn > 7 || testColumn < 0)
							{
								break;
							}
							if(ownPieceBlock(testRow,testColumn)) //Står egen pjäs i vägen?
							{
								break;
							}
							if(opposingPieceBlock(testRow, testColumn))
							{
								block = 1; //Detta drag ska sparas, men inga fler drag kan göras längs strålen
							}
							if (!liftCheck(i,j,testRow,testColumn,false)) //Orsakar den/Häver den ej/ schack?
							{
								continue;
							}

							possibleMoves[possibleMovesCounter][0] = board[i][j];
							possibleMoves[possibleMovesCounter][1] = i; //nuvarande rad
							possibleMoves[possibleMovesCounter][2] = j; //nuvarande kolonn
							possibleMoves[possibleMovesCounter][3] = testRow; //ny rad
							possibleMoves[possibleMovesCounter][4] = testColumn; //ny kolonn

							possibleMovesCounter++;
						}
					}
				}
				else if (board[i][j] == pieceConstant + 2) //Dam
				{
					int block;
					for (int k = 0; k < sizeKingRays; k++) //Kung- och damstrålar samma
					{
						testRow = i;
						testColumn = j;
						block = 0;
						while (!block)
						{
							testRow += kingRays[k][0];
							testColumn += kingRays[k][1];

							if (testRow > 7 || testRow < 0 || testColumn > 7 || testColumn < 0)
							{
								break;
							}
							if(ownPieceBlock(testRow,testColumn)) //Står egen pjäs i vägen?
							{
								break;
							}
							if(opposingPieceBlock(testRow, testColumn))
							{
								block = 1; //Detta drag ska sparas, men inga fler drag kan göras längs strålen
							}
							if (!liftCheck(i,j,testRow,testColumn,false)) //Orsakar den/Häver den ej/ schack?
							{
								continue;
							}

							possibleMoves[possibleMovesCounter][0] = board[i][j];
							possibleMoves[possibleMovesCounter][1] = i; //nuvarande rad
							possibleMoves[possibleMovesCounter][2] = j; //nuvarande kolonn
							possibleMoves[possibleMovesCounter][3] = testRow; //ny rad
							possibleMoves[possibleMovesCounter][4] = testColumn; //ny kolonn

							possibleMovesCounter++;
						}
					}
				}
			}
		}

		//Här vet vi om det finns drag att göra. Om det inte gör det, är det schackmatt eller patt Alltså kan vi häva schack-inten
		if (possibleMovesCounter == 0)
		{
			if (check)
			{
				mate = 1;
			}
			else
			{
				tie = 1;
			}
		}
		else
		{
			check = 0;
			enPassant[0] = enPassant[1] = enPassant[2] = 0; //rensa e.p.

			for (int i = 0; i < sizePossibleMoves; i++) //Skriv ut alla möjliga drag
			{
				if(i == possibleMovesCounter)
				{
					break;
				}

				printf("%d\t%d %c%d %c%d\n", i, possibleMoves[i][0], columnCharacter(possibleMoves[i][2] + 1), number(possibleMoves[i][1]), columnCharacter(possibleMoves[i][4] + 1), number(possibleMoves[i][3]));
			}

			//Gör drag

			int myMove = 0;
			myMove = pgn_fd == -1 ? getInput(&camera_direction) : replay(&camera_direction, &start_time, pgn_fd);

			// Spelare gav upp (bara i replays)
			if(myMove == -2) {
				player_resigned = 1;
				goto game_over;
			}

			// Användare stängde ner
			if(myMove < 0) return 0;

			if(possibleMoves[myMove][5] == 2) //lång rockad, flytta tornet
			{
				board[firstRow][0] = 0;
				board[firstRow][3] = 3 + pieceConstant;
			}
			else if (possibleMoves[myMove][5] == 3) //kort rockad, flytta tornet
			{
				board[firstRow][7] = 0;
				board[firstRow][5] = 3 + pieceConstant;
			}
			else if (possibleMoves[myMove][5] == 1) //dubbelsteg
			{
				enPassant[0] = 1;
				enPassant[1] = possibleMoves[myMove][3];
				enPassant[2] = possibleMoves[myMove][4];
			}
			else if (possibleMoves[myMove][5] == 5) //en passant
			{
				board[possibleMoves[myMove][3] - directionConstant][possibleMoves[myMove][4]] = 0; //ta bort slagen pjäs
			}

			if (possibleMoves[myMove][0] != 0) //om det inte är ett nulldrag, gör draget
			{
				board[possibleMoves[myMove][3]][possibleMoves[myMove][4]] = possibleMoves[myMove][0];
				board[possibleMoves[myMove][1]][possibleMoves[myMove][2]] = 0;
			}

			if (possibleMoves[myMove][0] == pieceConstant + 6 && possibleMoves[myMove][3] == 7 - firstRow) //bonde på sista raden => promovering
			{
				int piece;
				if(pgn_fd >= 0) {
					piece = replay_promote_to;
				}
				else {
					piece = get_piece(&camera_direction);
					if(piece < 0) return 0;
				}
				board[possibleMoves[myMove][3]][possibleMoves[myMove][4]] = piece;
			}

			//Hantera framtida rockader
			if (board[0][0] != 3 || board[0][4] == 0)
			{
				printf("ingen rockad pa vits damsida\n");
				castlingPossibility[0][0] = 0;
			}
			if (board[0][7] != 3 || board[0][4] == 0)
			{
				printf("ingen rockad pa vits kungssida\n");
				castlingPossibility[0][1] = 0;
			}
			if (board[7][0] != 9 || board[7][4] == 0)
			{
				printf("ingen rockad pa svarts damsida\n");
				castlingPossibility[1][0] = 0;
			}
			if (board[7][7] != 9 || board[7][4] == 0)
			{
				printf("ingen rockad pa svarts kungssida\n");
				castlingPossibility[1][1] = 0;
			}

			//Close

			for (int i = 0; i <= possibleMovesCounter; i++)
			{
				for (int j = 0; j < sizeMoveInformation; j++)
				{
					possibleMoves[i][j] = 0;
				}
			}

			possibleMovesCounter = 0;

			whiteMove = !whiteMove;

			display();
		}
	}

game_over:
	if (tie)
	{
		printf("Remi!");
	}
	else if (whiteMove)
	{
		if(player_resigned) printf("Vit gav upp. Svart vinner!\n");
		else printf("Schackmatt. Svart vinner!\n");
	}
	else
	{
		if(player_resigned) printf("Svart gav upp. Vit vinner!\n");
		else printf("Schackmatt. Vit vinner!\n");
	}

	on_checkmate(&camera_direction);

	return 0;
}

void init()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			board[i][j] = 0;
		}
	}

	kingRays[0][0] = -1;
	kingRays[0][1] = -1;

	kingRays[1][0] = -1;
	kingRays[1][1] = 1;

	kingRays[2][0] = 1;
	kingRays[2][1] = -1;

	kingRays[3][0] = 1;
	kingRays[3][1] = 1;

	kingRays[4][0] = -1;
	kingRays[4][1] = 0;

	kingRays[5][0] = 1;
	kingRays[5][1] = 0;

	kingRays[6][0] = 0;
	kingRays[6][1] = -1;

	kingRays[7][0] = 0;
	kingRays[7][1] = 1;


	knightRays[0][0] = -2;
	knightRays[0][1] = -1;

	knightRays[1][0] = -2;
	knightRays[1][1] = 1;

	knightRays[2][0] = -1;
	knightRays[2][1] = -2;

	knightRays[3][0] = -1;
	knightRays[3][1] = 2;

	knightRays[4][0] = 1;
	knightRays[4][1] = -2;

	knightRays[5][0] = 1;
	knightRays[5][1] = 2;

	knightRays[6][0] = 2;
	knightRays[6][1] = -1;

	knightRays[7][0] = 2;
	knightRays[7][1] = 1;


	rookRays[0][0] = -1;
	rookRays[0][1] = 0;

	rookRays[1][0] = 1;
	rookRays[1][1] = 0;

	rookRays[2][0] = 0;
	rookRays[2][1] = -1;

	rookRays[3][0] = 0;
	rookRays[3][1] = 1;


	bishopRays[0][0] = -1;
	bishopRays[0][1] = -1;

	bishopRays[1][0] = -1;
	bishopRays[1][1] = 1;

	bishopRays[2][0] = 1;
	bishopRays[2][1] = -1;

	bishopRays[3][0] = 1;
	bishopRays[3][1] = 1;

	board[0][4] = 1; //Vit kung på e1
	board[7][4] = 7; //Svart kung på e8
	board[0][1] = 5; //Vit springare på b1
	board[7][1] = 11; //Svart springare på b8
	board[0][6] = 5;
	board[7][6] = 11;
	board[0][0] = 3; //Vitt torn på a1
	board[0][7] = 3;
	board[7][7] = 9; //Svart torn på h8
	board[7][0] = 9;
	board[0][2] = 4; //Löpare på c1
	board[0][5] = 4;
	board[7][2] = 10;
	board[7][5] = 10;
	board[0][3] = 2; //Dam på d1
	board[7][3] = 8;
	board[1][0] = 6;
	board[1][1] = 6;
	board[1][2] = 6;
	board[1][3] = 6;
	board[1][4] = 6;
	board[1][5] = 6;
	board[1][6] = 6;
	board[1][7] = 6;
	board[6][0] = 12;
	board[6][1] = 12;
	board[6][2] = 12;
	board[6][3] = 12;
	board[6][4] = 12;
	board[6][5] = 12;
	board[6][6] = 12;
	board[6][7] = 12;
	

	possibleMovesCounter = 0;

	for (int i = 0; i < sizePossibleMoves; i++)
	{
		for (int j = 0; j < sizeMoveInformation; j++)
		{
			possibleMoves[i][j] = 0;
		}
	}

	pieceConstant = 0; //0 för vit, 6 för svart
	directionConstant = 1; //1 för vit, -1 för svart

	check = 0;
	whiteMove = 1;
	mate = 0; //matt
	tie = 0; //remi

	castlingPossibility[0][0] = castlingPossibility[0][1] = castlingPossibility[1][0] = castlingPossibility[1][1] = 1;

	enPassant[0] = enPassant[1] = enPassant[2] = 0;

	display();
}

void display()
{
	for (int i = 7; i >= 0; i--) //varning för unsigned int
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%02d ", board[i][j]);
		}
		printf("\n");
	}
}

int testInCheck(int row, int column)
{
	int testRow = row;
	int testColumn = column;

	for (int i = -1; i < 3; i += 2) //Hotande Bonde?
	{
		testRow = row + directionConstant;
		testColumn = column + i;
		if (!(testRow > 7 || testRow < 0 || testColumn > 7 || testColumn < 0))
		{
			if (board[testRow][testColumn] == 6 - pieceConstant + 6)
			{

				return 1;
			}
		}
	}
	
	for(int i = 0; i < sizeKingRays; i++) //Hotande kung?
	{
		testRow = row + kingRays[i][0];
		testColumn = column + kingRays[i][1];

		if(testRow > 7 || testRow < 0 || testColumn > 7 || testColumn < 0)
		{
			continue;
		}

		if(board[testRow][testColumn] == (7 - pieceConstant)) 
		{
			return 1;
		}
		
	}

	for(int i = 0; i < sizeKnightRays; i++) //Hotande springare?
	{
		testRow = row + knightRays[i][0];
		testColumn = column + knightRays[i][1];

		if(testRow > 7 || testRow < 0 || testColumn > 7 || testColumn < 0)
		{
			continue;
		}

		if(board[testRow][testColumn] == (6 - pieceConstant + 5)) 
		{
			return 1;
		}
		
	}

	for(int i = 0; i < sizeRookRays; i++) //Hotande torn eller dam?
	{
		testRow = row;
		testColumn = column;
		int block = 0;

		while (!block)
		{
			testRow += rookRays[i][0];
			testColumn += rookRays[i][1];

			if(testRow > 7 || testRow < 0 || testColumn > 7 || testColumn < 0)
			{
				break;
			}
			if ((board[testRow][testColumn] == 6 - pieceConstant + 3) || (board[testRow][testColumn] == 6 - pieceConstant + 2))
			{
				return 1;
			}
			else if (board[testRow][testColumn] != 0 && board[testRow][testColumn] != pieceConstant + 1) //Om inte tom och inte innehåller egen kung
			{
				block = 1;
			}
		}
	}

	for(int i = 0; i < sizeBishopRays; i++) //Hotande löpare eller dam?
	{
		testRow = row;
		testColumn = column;
		int block = 0;

		while (!block)
		{
			testRow += bishopRays[i][0];
			testColumn += bishopRays[i][1];

			if(testRow > 7 || testRow < 0 || testColumn > 7 || testColumn < 0)
			{
				break;
			}
			if ((board[testRow][testColumn] == 6 - pieceConstant + 4) || (board[testRow][testColumn] == 6 - pieceConstant + 2))
			{
				return 1;
			}
			else if (board[testRow][testColumn] != 0 && board[testRow][testColumn] != pieceConstant + 1) //Om inte tom och inte innehåller egen kung
			{
				block = 1;
			}
		}
	}

	
	return 0;
}

char columnCharacter(int column)
{
	char character;
	character = (char)(column + 'a' - 1);
	return character;
}

int liftCheck(int oldRow, int oldColumn, int newRow, int newColumn, bool ep)
{
	int piece = board[oldRow][oldColumn];
	board[oldRow][oldColumn] = 0; //Lämnar egen position tom
	int possibleCapturedPiece;

	if(ep)
	{
		possibleCapturedPiece = board[enPassant[1]][enPassant[2]]; //Bonde slagen i en passant
		board[enPassant[1]][enPassant[2]] = 0;
	}
	else
	{
		possibleCapturedPiece = board[newRow][newColumn]; //Oftast räcker det med att anta att den slagna pjäsen faktiskt står på rutan som den anfallande pjäsen går till
	}

	board[newRow][newColumn] = piece; //ny position	

	int possibleCheck = 0;

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (board[i][j] == 1 + pieceConstant) //egen kung i schack?
			{
				possibleCheck = possibleCheck || testInCheck(i,j);
			}
		}
	}

	board[oldRow][oldColumn] = piece;

	if(ep)
	{
		board[enPassant[1]][enPassant[2]] = possibleCapturedPiece;
		board[newRow][newColumn] = 0;
	}
	else
	{
		board[newRow][newColumn] = possibleCapturedPiece;
	}

	return !possibleCheck;
}

int ownPieceBlock(int row, int column)
{
	return ((board[row][column] > 0) && (board[row][column] - pieceConstant >= 1) && (board[row][column] - pieceConstant <= 6));
}

int opposingPieceBlock(int row, int column)
{
	return ((board[row][column] > 0) && (board[row][column] - (6 - pieceConstant) >= 1) && (board[row][column] - (6 - pieceConstant) <= 6)); //6 - pieceConstant ger andra färgens pieceConstant
}

void testCastling()
{
	//damsidan
	if (board[firstRow][1] == 0 && board[firstRow][2] == 0 && board[firstRow][3] == 0 && !check && board[firstRow][0]) //Tomt mellan kung och torn och ej rockera ur schack, torn finns
	{
		if (!testInCheck(firstRow,3) && !testInCheck(firstRow,2)) //Inte passera hotad ruta
		{
			if (castlingPossibility[firstRow / 7][0]) //Om varken kung eller torn flyttats
			{
				possibleMoves[possibleMovesCounter][0] = board[firstRow][4]; //kung
				possibleMoves[possibleMovesCounter][1] = firstRow; //nuvarande rad
				possibleMoves[possibleMovesCounter][2] = 4; //nuvarande kolonn
				possibleMoves[possibleMovesCounter][3] = firstRow; //ny rad
				possibleMoves[possibleMovesCounter][4] = 2; //ny kolonn
				possibleMoves[possibleMovesCounter][5] = 2; // lång rockad

				possibleMovesCounter++;
			}
		}
	}

	if (board[firstRow][5] == 0 && board[firstRow][6] == 0 && !check) //Tomt mellan kung och torn och ej rockera ur schack, torn finns
	{
		if (!testInCheck(firstRow,5) && !testInCheck(firstRow,6)) //Inte passera hotad ruta
		{
			if (castlingPossibility[firstRow / 7][1]) //Om varken kung eller torn flyttats
			{
				possibleMoves[possibleMovesCounter][0] = board[firstRow][4]; //kung
				possibleMoves[possibleMovesCounter][1] = firstRow; //nuvarande rad
				possibleMoves[possibleMovesCounter][2] = 4; //nuvarande kolonn
				possibleMoves[possibleMovesCounter][3] = firstRow; //ny rad
				possibleMoves[possibleMovesCounter][4] = 6; //ny kolonn
				possibleMoves[possibleMovesCounter][5] = 3; //kort rockad

				possibleMovesCounter++;
			}
		}
	}
}
