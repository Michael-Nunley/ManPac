#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <stdio.h>
#include <string>
#include <cmath>
#include <BaseTsd.h>
#include <random>
#include <iostream>
#include <vector>

#include <ostream>

#include <sstream>



class LWindow
{
	public:
		//Intializes internals
		LWindow();

		//Creates window
		bool init();

		//Creates renderer from internal window
		SDL_Renderer* createRenderer();

		//Handles window events
		void handleEvent( SDL_Event& e );

		//Deallocates internals
		void free();

		//Window dimensions
		int getWidth();
		int getHeight();

		//Window focii
		bool hasMouseFocus();
		bool hasKeyboardFocus();
		bool isMinimized();

	private:
		//Window data
		SDL_Window* mWindow;

		//Window dimensions
		int mWidth;
		int mHeight;

		//Window focus
		bool mMouseFocus;
		bool mKeyboardFocus;
		bool mFullScreen;
		bool mMinimized;
};


//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile(std::string path);

		//#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
		//#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor(Uint8 red, Uint8 green, Uint8 blue);

		//Set blending
		void setBlendMode(SDL_BlendMode blending);

		//Set alpha modulation
		void setAlpha(Uint8 alpha);

		//Renders texture at given point
		void render(
              int x,
              int y,
              SDL_Rect* clip = NULL,
              double angle = 0.0,
              SDL_Point* center = NULL,
              SDL_RendererFlip flip = SDL_FLIP_NONE
                    );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//The application time based timer
class LTimer
{
    public:
		//Initializes variables
		LTimer();

		//The various clock actions
		void start();
		void stop();
		void pause();
		void unpause();

		//Gets the timer's time
		Uint32 getTicks();

		//Checks the status of the timer
		bool isStarted();
		bool isPaused();

    private:
		//The clock time when the timer started
		Uint32 mStartTicks;

		//The ticks stored when the timer was paused
		Uint32 mPausedTicks;

		//The timer status
		bool mPaused;
		bool mStarted;
};

//The ManPac that will move around on the screen
class ManPac
{
    public:
		//The dimensions of the ManPac
		static const int ManPac_WIDTH = 32;
		static const int ManPac_HEIGHT = 32;

		//Initializes the variables
		ManPac();

		//Takes key presses and adjusts the ManPac's velocity
		void handleEvent(SDL_Event& e);

		//Moves the ManPac
		void move();

		//Shows the ManPac on the screen
		void render();

   // private:
		//The X and Y offsets of the ManPac
		int mPosX, mPosY;

		//The velocity of the ManPac
		int mVelX, mVelY;

		int mRotL, mRotR;
};

class Dots
{
    public:
		void render();
		int mPosX, mPosY;
};

Dots SmallDots[240];
