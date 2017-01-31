//#include <iostream>
//using namespace std;
#include <stdio.h>
#include <SDL.h>
#include <GL/gl.h>
#include <sys/ioctl.h>
#include "VectorUtils3.h"
// Några funktioner från GL_utilities.h anropar GLUT. Använd dem inte.
#include "GL_utilities.h"
#include <sys/time.h>
#include <math.h>
#include "loadobj.h"
#include "LoadTGA.h"

//-----------------START-CLAUDIA-------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
		//Schackbräd
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
/*
GLfloat black[] =
{
	0.0f,0.0f,0.0f,1.0f,
	0.0f,0.0f,0.0f,1.0f,
	0.0f,0.0f,0.0f,1.0f,
	0.0f,0.0f,0.0f,1.0f,
};
GLfloat white[] =
{
	1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,
};
//---------------ny
GLfloat vertices[6][3] =
{
	{-0.5, 0.0, -0.5},
	{0.5, 0.0, -0.5},
	{0.5, 0.0, 0.0},
	{-0.5, 0.0, 0.0},
	{0.5, 0.0, 0.5},
	{-0.5, 0.0, 0.5}
};

GLfloat normals[6][3] = {
	{0.0, 0.58, 0.0},
	{0.0, 0.58, 0.0},
	{0.0, 0.58, 0.0},
	{0.0, 0.58, 0.0},
	{0.0, 0.58, 0.0},
	{0.0, 0.58, 0.0}
};
GLuint cubeIndices[12] =
{
	0,1,2, 2,3,0, 3,2,4, 4,5,3
};
GLfloat black[] =
{
	0.0f,0.0f,0.0f,1.0f,
	0.0f,0.0f,0.0f,1.0f,
	0.5f,0.5f,0.5f,1.0f,
	0.5f,0.5f,0.5f,1.0f,
	1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f
};
*/
//---------------ny

unsigned int vertexBufferObjID;
unsigned int indexBufferObjID;
unsigned int normalBufferObjID;
unsigned int vertexArrayObjID;
unsigned int vertexArrayObjID2;
//unsigned int blackBufferObjID;
unsigned int whiteBufferObjID;

//GLuint black_board_program;
GLuint white_board_program;		
//-----------------END-CLAUDIA-------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//FIXME: Rockad kan trolla fram pjäser som inte finns. Med största sannolikhet fixat

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

int liftCheck(int oldRow, int oldColumn, int newRow, int newColumn); //testar om ett drag häver schacken för egna spelaren

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
int getInput()
{
	while(1) {
		// Har vi fått någon inmatning?
		int bytes;
		ioctl(fileno(stdin), FIONREAD, &bytes);
		if(bytes > 0) {
			int move;
			if(scanf("%d", &move) != 1) return -1;
			return move;
		}

		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {
			if(ev.type == SDL_QUIT) {
				return -1;
			}
			else if(ev.type == SDL_KEYDOWN) {
				if(ev.key.keysym.sym == SDLK_ESCAPE) return -1;
			}
		}

		// Rita nu upp schackbrädet
static float a = 0.0;
a += 0.01;
if(a > 1.0) a = 0.0;
glClearColor(a, .5, a, 0);
		unsigned t = get_time_in_msecs();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		vec3 cam;
		vec3 lookAtPoint;
		mat4 camMatrix;
		mat4 modelView;
		mat4 total;

		// Skybox
		glUseProgram(skybox_program);
		glBindVertexArray(0);
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Beräkna Camera*Modelview för skybox
		cam = (vec3){12.0 * sin(t / 5000.0), 7, 12.0 * cos(t / 5000.0)};
		lookAtPoint = (vec3){0, 0, 0};
		camMatrix = lookAt(cam.x, cam.y, cam.z,
					lookAtPoint.x, lookAtPoint.y, lookAtPoint.z,
					0.0, 1.0, 0.0);
		modelView = IdentityMatrix();
		total = Mult(Mult(camMatrix, modelView), S(20, 20, 20));
		glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, total.m);
		glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
		glDisable(GL_DEPTH_TEST);

		glBindTexture(GL_TEXTURE_2D, skybox_texture);
glDisable(GL_CULL_FACE);
		DrawModel(skybox, skybox_program, "in_Position", NULL, "inTexCoord");

		glEnable(GL_DEPTH_TEST);

		// Värld
		glUseProgram(program);

		// Beräkna Camera*Modelview för värld
		cam = (vec3){12.0 * sin(t / 5000.0), 7, 12.0 * cos(t / 5000.0)};
		lookAtPoint = (vec3){0, 0, 0};
		camMatrix = lookAt(cam.x, cam.y, cam.z,
					lookAtPoint.x, lookAtPoint.y, lookAtPoint.z,
					0.0, 1.0, 0.0);
		modelView = IdentityMatrix();
		total = Mult(camMatrix, modelView);
		glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, total.m);
		glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);


//-----------------START-CLAUDIA-------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
	
	//Draw board
	
	int x,z;
	float fieldsize = 1.0;
	mat4 translMatrix;
	
	for(z=0; z<8; z++){
	  for(x=0; x<8; x++){
	    //Translation
	    translMatrix = Mult(T(fieldsize*(float)x - 4*fieldsize, 0.0, fieldsize*(float)z - 4*fieldsize), S(fieldsize,fieldsize,fieldsize));
	
	    //draw either black or white
	    
	    if(((z==0.0 || z==2.0 || z==4.0 || z==6.0) && (x==1.0 || x==3.0 || x==5.0 || x==7.0))||((z==1.0 || z==3.0 || z==5.0 || z==7.0) && (x==0.0 || x==2.0 || x==4.0 || x==6.0))){
	      glUseProgram(white_board_program);
	      glBindVertexArray(vertexArrayObjID2);
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "translMatrix"), 1, GL_TRUE, translMatrix.m);	
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "mdlMatrix"), 1, GL_TRUE, total.m);
	      GLfloat white[] = {0.25, 0.13, 0.0, 1.0};
	      glUniform4fv(glGetUniformLocation(white_board_program, "color"), 1, white);
	      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	      
	    }else{
	      glUseProgram(white_board_program);
	      glBindVertexArray(vertexArrayObjID2);
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "translMatrix"), 1, GL_TRUE, translMatrix.m);	
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	      glUniformMatrix4fv(glGetUniformLocation(white_board_program, "mdlMatrix"), 1, GL_TRUE, total.m);
	      GLfloat white[] = {0.9, 0.85, 0.7, 0.0};
	      glUniform4fv(glGetUniformLocation(white_board_program, "color"), 1, white);
	      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	      /*
	      glUseProgram(black_board_program);
	      glBindVertexArray(vertexArrayObjID);
	      glUniformMatrix4fv(glGetUniformLocation(black_board_program, "translMatrix"), 1, GL_TRUE, translMatrix.m);	
	      glUniformMatrix4fv(glGetUniformLocation(black_board_program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	      glUniformMatrix4fv(glGetUniformLocation(black_board_program, "mdlMatrix"), 1, GL_TRUE, total.m);
	      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	      */
	    }		
	  }
	}
	
//-----------------END-CLAUDIA-------------------------------------
//-----------------------------------------------------------------
//----------------------------------------------------------------------------

		SDL_GL_SwapBuffers();
	}
}

int castlingPossibility[2][2]; // Om elementen på följande platser är 1, har inte de eventuellt rockerande pjäserna flyttats: 0,0 - Vit kan rockera på damsidan; 0,1 - Vit kan rockera på kungssidan; 1,0 - Svart kan rockera på damsidan; 1,1 - Svart kan rockera på kungssidan

static void init_gl(void)
{
	glViewport(0, 0, 640, 480);
	glClearColor(1, 0, 0, 0);

	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 50.0);

	// Load and compile shader
	skybox_program = loadShaders("skybox.vert", "skybox.frag");
//-----------------START-CLAUDIA-------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
	//black_board_program = loadShaders("board_black.vert", "board_black.frag");
	white_board_program = loadShaders("board_white.vert", "board_white.frag");
//-----------------END-CLAUDIA-------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
	program = loadShaders("schack.vert", "schack.frag");
	printError("Shaders");

	skybox = LoadModelPlus("skybox.obj");

	LoadTGATextureSimple("nebula_tut10.tga", &skybox_texture);
	
	
//-----------------START-CLAUDIA-------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
	glGenBuffers(1, &vertexBufferObjID);
	glGenBuffers(1, &indexBufferObjID);
	glGenBuffers(1, &normalBufferObjID);
	//glGenBuffers(1, &blackBufferObjID);
	glGenBuffers(1, &whiteBufferObjID);
	
	glGenVertexArrays(1, &vertexArrayObjID);
	glBindVertexArray(vertexArrayObjID);
	/*
	//black board
	glUseProgram(black_board_program);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(black_board_program, "in_Position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(black_board_program, "in_Position"));
	
	glBindBuffer(GL_ARRAY_BUFFER, normalBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(GLfloat), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(black_board_program, "in_Normal"), 3, GL_FLOAT, GL_FALSE, 0,0);
	glEnableVertexAttribArray(glGetAttribLocation(black_board_program, "in_Normal"));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObjID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLuint), cubeIndices, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, blackBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 16*sizeof(GLfloat), black, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(black_board_program, "in_colors_black"), 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(glGetAttribLocation(black_board_program, "in_colors_black"));
	*/
	
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
	/*
	glBindBuffer(GL_ARRAY_BUFFER, whiteBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 16*sizeof(GLfloat), white, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(white_board_program, "in_colors_white"), 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(glGetAttribLocation(white_board_program, "in_colors_white"));
	*/
//-----------------END-CLAUDIA-------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


	glUseProgram(skybox_program);
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit"), 0); // Texture unit 0
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "tex"), 0); // Texture unit 0

	printError("end of init");
}

int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetVideoMode(640, 480, 32, SDL_OPENGL);

	init_gl();

	init();

	//starta spelet
	//Återställ en passant-data

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
//			cout << "Schack!" << endl;
			printf("Schack!\n");
		}

//		system("PAUSE"); //Bra ställe att stanna

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
						if (liftCheck(i, j, testRow, testColumn))
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

							if (liftCheck(i, j, testRow, testColumn))
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

					testRow = i + directionConstant;
					for (int l = -1; l < 3; l += 2)
					{
						testColumn = j + l;
						if (testColumn >= 0 && testColumn <= 7)
						{
							if (opposingPieceBlock(testRow, testColumn)) //För slag, måste det finnas en motståndarpjäs
							{
								if (liftCheck(i, j, testRow, testColumn))
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
						if (liftCheck(i, j, i + directionConstant, enPassant[2]))
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
						if (!liftCheck(i,j,testRow,testColumn)) //Orsakar den/Häver den ej/ schack?
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
							if (!liftCheck(i,j,testRow,testColumn)) //Orsakar den/Häver den ej/ schack?
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
							if (!liftCheck(i,j,testRow,testColumn)) //Orsakar den/Häver den ej/ schack?
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
							if (!liftCheck(i,j,testRow,testColumn)) //Orsakar den/Häver den ej/ schack?
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

#if 0
				cout << i << "   ";

				cout << possibleMoves[i][0] << " ";
				cout << columnCharacter(possibleMoves[i][2] + 1);
				cout << number(possibleMoves[i][1]) << " ";
				cout << columnCharacter(possibleMoves[i][4] + 1);
				cout << number(possibleMoves[i][3]) << " ";			
				cout << endl;
#else
				printf("%d\t%d %c%d %c%d\n", i, possibleMoves[i][0], columnCharacter(possibleMoves[i][2] + 1), number(possibleMoves[i][1]), columnCharacter(possibleMoves[i][4] + 1), number(possibleMoves[i][3]));
#endif
			}

			//Gör drag

			int myMove = 0;
//			cin >> myMove;
			myMove = getInput();
			if(myMove < 0) break;
//			if(scanf("%d", &myMove) != 1) {
//				break;
//			}
			
			


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
//				cout << "Valj ny pjas" << endl << endl;
				printf("Valj ny pjas\n");

				for (int i = 2; i <= 5; i++)
				{
//					cout << pieceReferences[i] << endl;
					printf("%c\n", pieceReferences[i]);
				}
				char myPiece;
//				cin >> myPiece;
				scanf("%d\n", &myPiece);
				for (int i = 2; i <= 5; i++)
				{
					if (pieceReferences[i] == myPiece)
					{
						board[possibleMoves[myMove][3]][possibleMoves[myMove][4]] = pieceConstant + i;
					}
				}
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

	if (tie)
	{
//		cout << "Remi!" << endl;
		printf("Remi!");
	}
	else if (whiteMove)
	{
//		cout << "Schackmatt! Svart har vunnit!";
		printf("Schackmatt! Svart har vunnit!");
	}
	else
	{
//		cout << "Schackmatt! Vit har vunnit!";
		printf("Schackmatt! Vit har vunnit!");
	}

	printf("\n");
//	cout << endl;
//	system("PAUSE");
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

	/*for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			cout << kingRays[i][j];
		}

		cout << endl;
	}*/

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
#if 0
			if(board[i][j] >= 10)
			{
//				cout << board[i][j] << ' ';
			}
			else
			{
				cout <<'0' << board[i][j] << ' ';
			}
#endif
		}
//		cout << endl;
		printf("\n");
	}
}

int testInCheck(int row, int column)
{
	int testRow = row;
	int testColumn = column;

	for (int i = -1; i < 3; i += 2)
	{
		testRow = row + directionConstant;
		testColumn = column + i;
		if (!(testRow > 7 || testRow < 0 || testColumn > 7 || testColumn < 0))
		{
			if (board[testRow][testColumn] == 6 - pieceConstant + 6)
			{
//				cout << "something wrong " << testRow << testColumn << endl;
				printf("something wrong %d %d\n", testRow, testColumn);
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
			else if (board[testRow][testColumn] != 0)
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
			else if (board[testRow][testColumn] != 0)
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

int liftCheck(int oldRow, int oldColumn, int newRow, int newColumn)
{
	int piece = board[oldRow][oldColumn];
	int possibleCapturedPiece = board[newRow][newColumn];

	board[oldRow][oldColumn] = 0; //Lämnar egen position tom
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
	board[newRow][newColumn] = possibleCapturedPiece;

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
