/*!\file window.c
 *
 * \brief Utilisation de la SDL2/GL4Dummies et d'OpenGL 3+
 *
 * \author Farès BELHADJ, amsi@ai.univ-paris8.fr
 * \date April 24 2014
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
//#include <mobile.h>
#include <GL4D/gl4du.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <objdetect.hpp>
#include <SDL/SDL_image.h>
#include <SDL.h>
#include <SDL/SDL.h>



using namespace cv;
using namespace std;

/*
 * Prototypes des fonctions statiques contenues dans ce fichier C
 */


//////////////////////////////////////////////////////////////////////////////
// les prototypes des fonctions et variables globales récupéré des fichiers //
// 												mobiles.c et mobiles.h													  //
//////////////////////////////////////////////////////////////////////////////

typedef struct mobile_t mobile_t;
struct mobile_t 
{
  float x, y, vx, vy; // les coordonnées
  float r; // La taille 
  GLfloat c[4];// couleurs RGBA
};

extern void mobile_init(int n);
extern void mobile_move(void);
extern void mobile_up(float dy);
extern void mobile_draw(GLuint pId);
extern void mobile_delete(void);
static void printFPS(void) ;
int getLastDigit(float number);
void MyFilledCircle( Mat img, Point center,int i,int r );

static int _nmobiles = 30;	//nb éléments
static mobile_t *  _mobiles = new mobile_t[ _nmobiles] ;

static GLuint _vao = 0;
static GLuint _vbo = 0;
static GLuint _tex = 0;

SDL_Surface * t = NULL;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


static SDL_Window * initWindow(int w, int h, SDL_GLContext * poglContext);
static void initGL(SDL_Window * win);
static void initData(void);
static void resizeGL(SDL_Window * win);
static void loop(SDL_Window * win);
static void manageEvents(SDL_Window * win);
static void draw(void);
static void quit(void);

/*!\brief dimensions de la fenêtre */
static GLfloat _dim[] = {1024, 768};//{640, 480};

/*!\brief pointeur vers la (future) fenêtre SDL */
static SDL_Window * _win = NULL;

/*!\brief pointeur vers le (futur) contexte OpenGL */
static SDL_GLContext _oglContext = NULL;

/*!\brief identifiant du (futur) buffer de data */
static GLuint _buffer = 0;

/*!\brief identifiants des (futurs) GLSL programs */
static GLuint _pId = 0;
static GLuint _pId2 = 0;

/*!\brief identifiant de la texture chargée */
static GLuint _tId = 0;

/*!\brief device de capture vidéo */
static VideoCapture * _cap = NULL;






/*!\brief La fonction principale initialise la bibliothèque SDL2,
 * demande la création de la fenêtre SDL et du contexte OpenGL par
 * l'appel à \ref initWindow, initialise OpenGL avec \ref initGL et
 * lance la boucle (infinie) principale.
 */
int main(int argc, char ** argv) 
{
/*
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Erreur lors de l'initialisation de SDL :  %s", SDL_GetError());
    return -1;
  }
*/
  atexit(SDL_Quit);
  if((_win = initWindow(_dim[0], _dim[1], &_oglContext))) {
    atexit(quit);
    gl4duInit(argc, argv);
    initGL(_win);
    _pId = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
    _pId2 = gl4duCreateProgram("<vs>shaders/basic2.vs", "<fs>shaders/basic2.fs", NULL);
    initData();
    loop(_win);
  }
  return 0;
}

/*!\brief Cette fonction créé la fenêtre SDL de largeur \a w et de
 *  hauteur \a h, le contexte OpenGL \ref et stocke le pointeur dans
 *  poglContext. Elle retourne le pointeur vers la fenêtre SDL.
 *
 * Le contexte OpenGL créé est en version 3 pour
 * SDL_GL_CONTEXT_MAJOR_VERSION, en version 2 pour
 * SDL_GL_CONTEXT_MINOR_VERSION et en SDL_GL_CONTEXT_PROFILE_CORE
 * concernant le profile. Le double buffer est activé et le buffer de
 * profondeur est en 24 bits.
 *
 * \param w la largeur de la fenêtre à créer.
 * \param h la hauteur de la fenêtre à créer.
 * \param poglContext le pointeur vers la case où sera référencé le
 * contexte OpenGL créé.
 * \return le pointeur vers la fenêtre SDL si tout se passe comme
 * prévu, NULL sinon.
 */
static SDL_Window * initWindow(int w, int h, SDL_GLContext * poglContext) {
  SDL_Window * win = NULL;
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  if( (win = SDL_CreateWindow("GLSLExample", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
			      w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | 
			      SDL_WINDOW_SHOWN)) == NULL )
    return NULL;
  if( (*poglContext = SDL_GL_CreateContext(win)) == NULL ) {
    SDL_DestroyWindow(win);
    return NULL;
  }
  fprintf(stderr, "Version d'OpenGL : %s\n", glGetString(GL_VERSION));
  fprintf(stderr, "Version de shaders supportes : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));  
  return win;
}

/*!\brief Cette fonction initialise les paramètres OpenGL.
 *
 * \param win le pointeur vers la fenêtre SDL pour laquelle nous avons
 * attaché le contexte OpenGL.
 */
static void initGL(SDL_Window * win) 
{
  glClearColor(1.0f, 0.2f, 0.2f, 0.0f);
	
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
  gl4duGenMatrix(GL_FLOAT, "modelviewMatrix");
  gl4duBindMatrix("modelviewMatrix");
	
  gl4duLoadIdentityf();
  /* placer les objets en -n, soit bien après le plan near  */
  gl4duTranslatef(0, 0, -2);  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  resizeGL(win);
}




/*!\brief Cette fonction paramétrela vue (viewPort) OpenGL en fonction
 * des dimensions de la fenêtre SDL pointée par \a win.
 *
 * \param win le pointeur vers la fenêtre SDL pour laquelle nous avons
 * attaché le contexte OpenGL.
 */
static void resizeGL(SDL_Window * win) 
{
  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  glViewport(0, 0, w, h);
  gl4duBindMatrix("projectionMatrix");
  gl4duLoadIdentityf();
  gl4duFrustumf(-1.0f, 1.0f, -h / (GLfloat)w, h / (GLfloat)w, 2.0f, 1000.0f);
}

static void initData(void) 
{

  GLuint vert = 0xFFFFFFFF;

  GLfloat data[] = {
    /* 4 coordonnées de sommets */
    -1.f, -1.f, 0.f, 1.f, -1.f, 0.f,
    -1.f,  1.f, 0.f, 1.f,  1.f, 0.f,
    /* 2 coordonnées de texture par sommet */
    1.0f, 1.0f, 0.0f, 1.0f, 
    1.0f, 0.0f, 0.0f, 0.0f
  };
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1); 
  glGenTextures(1, &_tId);



  glGenBuffers(1, &_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, _buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof data, data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);  
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const void *)((4 * 3) * sizeof *data));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glBindTexture(GL_TEXTURE_2D, _tId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &vert);
  glBindTexture(GL_TEXTURE_2D, 0);
 

  glGenTextures(1, &_tId);
  glBindTexture(GL_TEXTURE_2D, _tId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  _cap = new VideoCapture(0);
  if(!_cap || !_cap->isOpened()) {
    delete _cap;
    _cap = new VideoCapture(CV_CAP_ANY);
  }
  _cap->set(CV_CAP_PROP_FRAME_WIDTH,  (int)_dim[0]);
  _cap->set(CV_CAP_PROP_FRAME_HEIGHT, (int)_dim[1]);
  
	mobile_init(_nmobiles);
}

/*!\brief Boucle infinie principale : gère les évènements, dessine,
 * imprime le FPS et swap les buffers.
 *
 * \param win le pointeur vers la fenêtre SDL pour laquelle nous avons
 * attaché le contexte OpenGL.
 */
static void loop(SDL_Window * win) 
{
  SDL_GL_SetSwapInterval(1);
  for(;;) {

    manageEvents(win);
		mobile_move();
    draw();
		printFPS(); 
    gl4duPrintFPS(stderr);
    SDL_GL_SwapWindow(win);
    gl4duUpdateShaders();
  }
}

/*!\brief Cette fonction permet de gérer les évènements clavier et
 * souris via la bibliothèque SDL et pour la fenêtre pointée par \a
 * win.
 *
 * \param win le pointeur vers la fenêtre SDL pour laquelle nous avons
 * attaché le contexte OpenGL.
 */
static void manageEvents(SDL_Window * win) {
  SDL_Event event;
  while(SDL_PollEvent(&event)) 
    switch (event.type) {
    case SDL_KEYDOWN:
      switch(event.key.keysym.sym) {
      case SDLK_ESCAPE:
      case 'q':
	exit(0);
      default:
	fprintf(stderr, "La touche %s a ete pressee\n",
		SDL_GetKeyName(event.key.keysym.sym));
	break;
      }
      break;
    case SDL_KEYUP:
      break;
    case SDL_WINDOWEVENT:
      if(event.window.windowID == SDL_GetWindowID(win)) {
	switch (event.window.event)  {
	case SDL_WINDOWEVENT_RESIZED:
	  resizeGL(win);
	  break;
	case SDL_WINDOWEVENT_CLOSE:
	  event.type = SDL_QUIT;
	  SDL_PushEvent(&event);
	  break;
	}
      }
      break;
    case SDL_QUIT:
      exit(0);
    }
}





static void draw(void) 
{

  Mat ci;
  const GLfloat blanc[] = {1.0f, 1.0f, 1.0f, 1.0f};
  const GLfloat bleu[]  = {0.5f, 0.5f, 1.0f, 1.0f};


  *_cap >> ci;
  
  glUseProgram(_pId);
  glBindTexture(GL_TEXTURE_2D, _tId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ci.cols, ci.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, ci.data);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(_pId);
  glDisable(GL_DEPTH_TEST);
  glUniform1i(glGetUniformLocation(_pId, "myTexture"), 0);
  glUniform1f(glGetUniformLocation(_pId, "width"), _dim[0]);
  glUniform1f(glGetUniformLocation(_pId, "height"), _dim[1]);

  // streaming au fond 

  gl4duBindMatrix("modelviewMatrix");
  gl4duSendMatrices(); // envoyer les matrices 
  glUniform4fv(glGetUniformLocation(_pId, "couleur"), 1, blanc); // envoyer une couleur 
  glBindVertexArray(_vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // dessiner le streaming sur le lointain (ici perspective liée à projection) plan tournant 
	
  mobile_draw(_pId2);


}


void mobile_draw(GLuint pId) 
{
  int i;static int acc=0;	

 

  glUseProgram(pId);
  glBindVertexArray(_vao);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _tex);
  glUniform1i(glGetUniformLocation(pId, "myTexture"), 0);




  for(i = 0; i < _nmobiles; i++) 
  {
    glUniform1f(glGetUniformLocation(pId, "ball_x"), _mobiles[i].x);
    glUniform1f(glGetUniformLocation(pId, "ball_y"), _mobiles[i].y);
    glUniform1f(glGetUniformLocation(pId, "scale"), 2.0 * _mobiles[i].r); 
    glUniform4f(glGetUniformLocation(pId, "couleur"), _mobiles[i].c[0], _mobiles[i].c[1], _mobiles[i].c[2], _mobiles[i].c[3]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
printf("-loop-> %d-_mobiles x= %f y=%f r=%f \n ",i,_mobiles[i].vx,_mobiles[i].vy,_mobiles[i].r);
	
}

/*!\brief Cette fonction est appelée au moment de sortir du programme
 *  (atexit), elle libère la fenêtre SDL \ref _win et le contexte
 *  OpenGL \ref _oglContext.
 */
static void quit(void) {
  if(_cap) {
    delete _cap;
    _cap = NULL;
  }
  if(_vao)
    glDeleteVertexArrays(1, &_vao);
  if(_buffer)
    glDeleteBuffers(1, &_buffer);
  if(_tId)
    glDeleteTextures(1, &_tId);
  if(_oglContext)
    SDL_GL_DeleteContext(_oglContext);
  if(_win)
    SDL_DestroyWindow(_win);
  gl4duClean(GL4DU_ALL);
}










/////////////////////////////////////////////////////
// Les fonctions récupérées du fichiers mobile.c  //
////////////////////////////////////////////////////




void mobile_init(int nb_ball) 
{
  int i, j;
  static int firstTime = 1;


  if(firstTime) 
  {
    glBindTexture(GL_TEXTURE_2D, _tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    if( (t = IMG_Load("bull.png")) != NULL ) 
	{
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, t->pixels);
     SDL_FreeSurface(t);
    } 


	else 
	{
      fprintf(stderr, "Erreur lors du chargement de la texture\n");
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  _nmobiles = nb_ball;

	// initialiser les positions depuis lequelles les ballons se jetteront 

  for(i = 0; i < _nmobiles; i++) {
    _mobiles[i].x = 2.0 * (rand() / (RAND_MAX + 1.0)) - 1.0;
    _mobiles[i].y = (rand() / (RAND_MAX + 1.0));
    _mobiles[i].vx = (rand() / (RAND_MAX + 1.0)) - 0.5;
    _mobiles[i].vy = (rand() / (RAND_MAX + 1.0)) - 0.5;
    _mobiles[i].r = 0.025 + 0.035 * (rand() / (RAND_MAX + 1.0));   
    for(j = 0 ; j < 3; j++)
      _mobiles[i].c[j] = rand() / ((GLfloat)RAND_MAX);
    _mobiles[i].c[3] = 1.0;
  }

}



static void frottements(int i, float kx, float ky, float dt) 
{
  _mobiles[i].vx = SIGN(_mobiles[i].vx) * MAX((fabs(_mobiles[i].vx) - kx * _mobiles[i].vx * _mobiles[i].vx * dt * dt), 0.0);
  _mobiles[i].vy = SIGN(_mobiles[i].vy) * MAX((fabs(_mobiles[i].vy) - ky * _mobiles[i].vy * _mobiles[i].vy * dt * dt), 0.0);
}





void mobile_move(void) 
{

 
  static int t0 = 0;
  static float k = 10.0;

  int t, i, j, collision;

  float dt, dx, dy, d, vx, vy, de, ndx, ndy, nde;

  t = SDL_GetTicks();

  dt = (t - t0) / 9000.0;  // gravité 

  t0 = t;


  for(i = 0; i < _nmobiles; i++) 
  {
		

    for(j = i + 1; j < _nmobiles; j++) 
	{
        dx = _mobiles[i].x - _mobiles[j].x;

        dy = _mobiles[i].y - _mobiles[j].y;

        d = _mobiles[i].r + _mobiles[j].r;
        
        if((de = dx * dx + dy * dy) < d * d) 
		    {
				
				  ndx = _mobiles[i].x + _mobiles[j].vx * dt - (_mobiles[j].x + _mobiles[i].vx * dt);
				  ndy = _mobiles[i].y + _mobiles[j].vy * dt - (_mobiles[j].y + _mobiles[i].vy * dt);
				  nde = ndx * ndx + ndy * ndy;

			
				if(nde < de) 
						continue;

				vx = _mobiles[i].vx;

				vy = _mobiles[i].vy;

				_mobiles[i].vx = _mobiles[j].vx;
				_mobiles[i].vy = _mobiles[j].vy;
				_mobiles[j].vx = vx;
				_mobiles[j].vy = vy;
				frottements(i, 5 * k, 5 * k, dt);
         }
    }

  }

  for(i = 0; i < _nmobiles; i++) 
  {

  
    collision = 0;

  // collision sur les cotés
    if((_mobiles[i].x + _mobiles[i].r >  1.0 && _mobiles[i].vx > 0.0) || 
       (_mobiles[i].x - _mobiles[i].r < -1.0 && _mobiles[i].vx < 0.0) ) 
		{
        _mobiles[i].vx = -_mobiles[i].vx;
        frottements(i, 20 * k, 50 * k, dt);
        
        collision = 1;
    }


    // collision bas et haut
    if((_mobiles[i].y  + 	_mobiles[i].r >  1.0 && _mobiles[i].vy > 0.0) || 
       (_mobiles[i].y  - 	_mobiles[i].r < -1.0 && _mobiles[i].vy < 0.0) ) 
    {
       _mobiles[i].vy   = -_mobiles[i].vy;

        frottements(i, 30 * k, 0 * k, dt); 
      collision = 1;
    }

    if(!collision)
      _mobiles[i].vy += -0.58 * dt;

    // déplacement horizontal
    _mobiles[i].x  += _mobiles[i].vx * dt;

    //déplacement vertical
    _mobiles[i].y  += _mobiles[i].vy * dt;


    // empêcher que le balon continue de tomber en sortant de l'écrant une fois qu'il ne bombe plus
    if (_mobiles[i].y < -1)
        _mobiles[i].y = -1;
    frottements(i, 0.5 * k, 0.5 * k, dt);

  }
}





void mobile_delete(void) 
{
  if(_mobiles) 
	{
    //free(_mobiles);
		delete [] _mobiles;
    _mobiles = NULL;
  }
  if(_vao)
    glDeleteVertexArrays(1, &_vao);
  if(_vbo)
    glDeleteBuffers(1, &_vbo);
  if(_tex)
    glDeleteTextures(1, &_tex);
}

static void printFPS(void) 
{
  Uint32 t;
  static Uint32 t0 = 0, f = 0;
  f++;
  t = SDL_GetTicks();
  if(t - t0 > 2000) 
	{
    fprintf(stderr, "%8.2f\n", (1000.0 * f / (t - t0)));
    t0 = t;
    f  = 0;
  }
}

