#include "main.h"

const int SCREEN_WIDTH = 452;
const int SCREEN_HEIGHT = 465;
int intervalChange = 1, intDots = 240, intSuperDots = 4;
uint64_t intScore = 0;
double degree;
bool enabled_Spook = false, enabled_Debug = false, quit = false;

struct xy
{
    int x;
    int y;
} coords[255];

struct tile
{

}gTile;

std::string itos(uint64_t i)
{
    std::stringstream ss;
    ss<<i;
    return ss.str();
}

void controls()
{
    std::cout <<
        "Controls:" << std::endl <<
        "[W] - Move the ManPac forward" << std::endl <<
        "[A] - Move the ManPac backward" << std::endl <<
        "[S] - Move the ManPac left" << std::endl <<
        "[D] - Move the ManPac right" << std::endl <<
        std::endl <<
        "[X] - Increase the movement amount" << std::endl <<
        "[Z] - Decrease the movement amount" << std::endl <<
        std::endl <<
        "[R] - Resets the ManPac's location, speed, and rotation" << std::endl <<
        std::endl <<
        "[PRNT SCRN] - Toggles printing the ManPac's coordinates to this debug console" << std::endl <<
        std::endl <<
        "[C] - Toggles on the ManPac 'Spook mode'" << std::endl <<
        std::endl <<
        "[TAB] - Prints these controls to the console" <<std::endl <<
        std::endl;
    return;
}

bool init(); //Starts up SDL and creates window
bool loadMedia(); //Loads media
void close(); //Frees media and shuts down SDL

//Our custom window
LWindow gWindow;

SDL_Renderer* gRenderer = NULL; //The window renderer
//Globally used font
TTF_Font *gFont = NULL;

//The music that will be played
Mix_Music *gMusic = NULL;

//The sound effects that will be used
Mix_Chunk *gNom = NULL, *gStart = NULL;

//Rendered texture
LTexture gTextScoreTexture, gTextTitleTexture;

//Scene textures
LTexture gManPacTexture, gGhostsTexture, gDotsTexture, gSuperDotsTexture[4];

LTexture gBound[40], gBackGround;

LTexture::LTexture() {mTexture = NULL;mWidth = 0;mHeight = 0;} //Initialize
LTexture::~LTexture() {free();}//Deallocate
bool LTexture::loadFromFile(std::string path)
{
	free(); //Get rid of preexisting texture
	SDL_Texture* newTexture = NULL; //The final texture
	SDL_Surface* loadedSurface = IMG_Load(path.c_str()); //Load image at specified path
	if(loadedSurface == NULL) {printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());}
	else
	{
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0, 0)); //Color key image
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface); //Create texture from surface pixels
		if(newTexture == NULL) {printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());}
		else {mWidth = loadedSurface->w;mHeight = loadedSurface->h;} //Get image dimensions
		SDL_FreeSurface(loadedSurface); //Get rid of old loaded surface
	}
	mTexture = newTexture;
	return mTexture != NULL; //Return success
}
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();
	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
	if(textSurface == NULL) {printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());}
	else
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if(mTexture == NULL) {printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}
		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}
	//Return success
	return mTexture != NULL;
}
void LTexture::free() {if(mTexture != NULL) {SDL_DestroyTexture(mTexture); mTexture = NULL; mWidth = 0; mHeight = 0;}} //Free texture if it exists}
void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue) {SDL_SetTextureColorMod(mTexture, red, green, blue);}//Modulate texture rgb
void LTexture::setBlendMode(SDL_BlendMode blending) {SDL_SetTextureBlendMode(mTexture, blending);}//Set blending function
void LTexture::setAlpha(Uint8 alpha) {SDL_SetTextureAlphaMod(mTexture, alpha);}//Modulate texture alpha
void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	SDL_Rect renderQuad = {x, y, mWidth, mHeight};//Set rendering space and render to screen
	if(clip != NULL) {renderQuad.w = clip->w; renderQuad.h = clip->h;}//Set clip rendering dimensions
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);//Render to screen
}
int LTexture::getWidth() {return mWidth;}
int LTexture::getHeight() {return mHeight;}

ManPac::ManPac()
{
    //Initialize the offsets
    mPosX = ((SCREEN_WIDTH - gManPacTexture.getWidth())/2);
    mPosY = ((SCREEN_HEIGHT)/2);

    //Initialize the velocity
    mVelX = 0; mVelY = 0;
    mRotL = 0; mRotR = 0;
}

void ManPac::handleEvent(SDL_Event& e)
{
    //if a key was pressed
	if(e.type == SDL_KEYDOWN && e.key.repeat == 0)
    {
        //Adjust the velocity
        switch(e.key.keysym.sym)
        {
            case SDLK_UP:
            case SDLK_w: mVelY -= intervalChange; break;
            case SDLK_DOWN:
            case SDLK_s: mVelY += intervalChange; break;
            case SDLK_LEFT:
            case SDLK_a: mVelX -= intervalChange; mRotL += intervalChange; break;
            case SDLK_RIGHT:
            case SDLK_d: mVelX += intervalChange; mRotR += intervalChange; break;
            case SDLK_TAB: controls(); break;
            case SDLK_c: enabled_Spook = !enabled_Spook; break;
            case SDLK_r:
                mPosX = ((SCREEN_WIDTH - gManPacTexture.getWidth())/2);
                mPosY = ((SCREEN_HEIGHT - gManPacTexture.getHeight())/2);
                degree = 0; intervalChange = 1; enabled_Spook = false;
                mVelX = 0; mVelY = 0; mRotR = 0; mRotL = 0;
                break;
            case SDLK_PRINTSCREEN: enabled_Debug = !enabled_Debug; break;

            case SDLK_z: intervalChange -= 1; std::cout << intervalChange << " "; break;
            case SDLK_x: intervalChange += 1; std::cout << intervalChange << " "; break;
        }
    }
    else if(e.type == SDL_KEYDOWN)
    {
        switch(e.key.keysym.sym)
        {
            //case SDLK_PAGEUP: (intScore = (intScore + 1)*2); break;
            //case SDLK_PAGEDOWN: (intScore = (intScore - 1)/2); break;

            case SDLK_z: intervalChange -= 1; std::cout << intervalChange << " "; break;
            case SDLK_x: intervalChange += 1; std::cout << intervalChange << " "; break;
        }
    }

    //if a key was released
    else if(e.type == SDL_KEYUP && e.key.repeat == 0)
    {
        //Adjust the velocity
        switch(e.key.keysym.sym)
        {
            case SDLK_UP:
            case SDLK_w: mVelY += intervalChange; break;
            case SDLK_DOWN:
            case SDLK_s: mVelY -= intervalChange; break;
            case SDLK_LEFT:
            case SDLK_a: mVelX += intervalChange; mRotL -= intervalChange; break;
            case SDLK_RIGHT:
            case SDLK_d: mVelX -= intervalChange; mRotR -= intervalChange; break;
        }
    }
}

void ManPac::move()
{
    //Move ManPac left or right
    mPosX += mVelX;
    degree -= mRotL;

    //if ManPac went too far to the left or right
    if((mPosX < 16) || (mPosX + ManPac_WIDTH > SCREEN_WIDTH))
    {
        mPosX -= mVelX;
    }//Move back

    //Move ManPac up or down
    mPosY += mVelY;
    degree += mRotR;

    //if ManPac went too far up or down
    if((mPosY < 32) || (mPosY + ManPac_HEIGHT > SCREEN_HEIGHT))
    {
        mPosY -= mVelY;
    }//Move back
}

void ManPac::render() {gManPacTexture.render(mPosX, mPosY,  NULL, degree);}//Show ManPac
void Dots::render() {gDotsTexture.render(mPosX, mPosY);}//Show Dots

LWindow::LWindow()
{
	//Initialize non-existant window
	mWindow = NULL;
	mMouseFocus = false;
	mKeyboardFocus = false;
	mFullScreen = false;
	mMinimized = false;
	mWidth = 0;
	mHeight = 0;
}

bool LWindow::init()
{
	//Create window
	mWindow = SDL_CreateWindow("ManPac", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if(mWindow != NULL)
	{
		mMouseFocus = true;
		mKeyboardFocus = true;
		mWidth = SCREEN_WIDTH;
		mHeight = SCREEN_HEIGHT;
	}

	return mWindow != NULL;
}

SDL_Renderer* LWindow::createRenderer()
{
	return SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

void LWindow::handleEvent(SDL_Event& e)
{
	//Window event occured
	if(e.type == SDL_WINDOWEVENT)
	{
		//Caption update flag
		bool updateCaption = false;

		switch(e.window.event)
		{
			//Get new dimensions and repaint on window size change
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			mWidth = e.window.data1;
			mHeight = e.window.data2;
			SDL_RenderPresent(gRenderer);
			break;

			//Repaint on exposure
			case SDL_WINDOWEVENT_EXPOSED:
			SDL_RenderPresent(gRenderer);
			break;

			//Mouse entered window
			case SDL_WINDOWEVENT_ENTER:
			mMouseFocus = true;
			updateCaption = true;
			break;

			//Mouse left window
			case SDL_WINDOWEVENT_LEAVE:
			mMouseFocus = false;
			updateCaption = true;
			break;

			//Window has keyboard focus
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			mKeyboardFocus = true;
			updateCaption = true;
			break;

			//Window lost keyboard focus
			case SDL_WINDOWEVENT_FOCUS_LOST:
			mKeyboardFocus = false;
			updateCaption = true;
			break;

			//Window minimized
			case SDL_WINDOWEVENT_MINIMIZED:
            mMinimized = true;
            break;

			//Window maxized
			case SDL_WINDOWEVENT_MAXIMIZED:
			mMinimized = false;
            break;

			//Window restored
			case SDL_WINDOWEVENT_RESTORED:
			mMinimized = false;
            break;
		}

		//Update window caption with new data
		if(updateCaption)
		{
			std::stringstream caption;
			caption << "Man - MouseFocus:" << ( ( mMouseFocus ) ? "On" : "Off" ) << " KeyboardFocus:" << ( ( mKeyboardFocus ) ? "On" : "Off" );
			SDL_SetWindowTitle( mWindow, caption.str().c_str() );
		}
	}
	//Enter exit full screen on return key
	else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
	{
		if(mFullScreen)
		{
			SDL_SetWindowFullscreen(mWindow, SDL_FALSE);
			mFullScreen = false;
		}
		else
		{
			SDL_SetWindowFullscreen(mWindow, SDL_TRUE);
			mFullScreen = true;
			mMinimized = false;
		}
	}
}

void LWindow::free()
{
	if(mWindow != NULL)
	{
		SDL_DestroyWindow(mWindow);
	}

	mMouseFocus = false;
	mKeyboardFocus = false;
	mWidth = 0;
	mHeight = 0;
}

int LWindow::getWidth()
{
	return mWidth;
}

int LWindow::getHeight()
{
	return mHeight;
}

bool LWindow::hasMouseFocus()
{
	return mMouseFocus;
}

bool LWindow::hasKeyboardFocus()
{
	return mKeyboardFocus;
}

bool LWindow::isMinimized()
{
	return mMinimized;
}


bool init()
{
	//Initialization flag
	bool success = true;

	system("@echo off");
    system("title ManPac - Debug");

	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		if(!gWindow.init())
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = gWindow.createRenderer();
			if(gRenderer == NULL)
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if(!(IMG_Init(imgFlags) &imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
				 //Initialize SDL_ttf
				if(TTF_Init() == -1)
				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}
			}
            //Initialize SDL_mixer
            if(Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
            {
                printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                success = false;
            }
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;
    gFont = TTF_OpenFont("lazy.ttf", 32);

	//Load ManPac texture
	if(!gManPacTexture.loadFromFile("sprites/ManPac.bmp"))
	{
		printf("Failed to load ManPac texture!\n");
		success = false;
	}
    if(!gGhostsTexture.loadFromFile("sprites/Ghosts.bmp"))
	{
		printf("Failed to load Ghosts texture!\n");
		success = false;
	}

    if(!gDotsTexture.loadFromFile("sprites/Dot.bmp"))
    {
        printf("Failed to load Dot texture!\n");
        success = false;
    }
    if(!gBackGround.loadFromFile("sprites/background2_coords.png"))
    {
        printf("Failed to load Dot texture!\n");
        success = false;
    }
    for (int i = 0; i < intSuperDots; i++)
    {
        if(!gSuperDotsTexture[i].loadFromFile("sprites/SuperDot.bmp"))
        {
            printf("Failed to load Dot texture!\n");
            success = false;
        }
    }
	if(gFont == NULL)
	{
		printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false;
	}
	else
	{
		//Render text
		SDL_Color textColor = {255, 255, 255};

		if(!gTextScoreTexture.loadFromRenderedText("Score:", textColor))
		{
			printf("Failed to render text texture!\n");
			success = false;
		}
		if(!gTextTitleTexture.loadFromRenderedText("ManPac", textColor))
		{
			printf("Failed to render text texture!\n");
			success = false;
		}
	}
	gNom = Mix_LoadWAV( "audio/Nom.wav" );
	if( gNom == NULL )
	{
		printf( "Failed to load Nom sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}
	gStart = Mix_LoadWAV( "audio/Start.wav" );
	if( gStart == NULL )
	{
		printf( "Failed to load Start sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}
	return success;
}

void close()
{
    //Free text textures
	gTextScoreTexture.free();
	gTextTitleTexture.free();
	gBackGround.free();

	//Free global font
	TTF_CloseFont(gFont);
	gFont = NULL;

	//Free loaded images
    gDotsTexture.free();

    for (int i = 0; i < intSuperDots; i++)
    {
        gSuperDotsTexture[i].free();
    }
	gManPacTexture.free();

	//Destroy window
	SDL_DestroyRenderer(gRenderer);
	gWindow.free();
	gRenderer = NULL;

    Mix_FreeChunk(gNom);
    Mix_FreeChunk(gStart);
	gNom = NULL;
	gStart = NULL;

	//Free the music
	Mix_FreeMusic(gMusic);
	gMusic = NULL;


	//Quit SDL subsystems
    TTF_Quit();
    Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

void initDots()
{
    int i = 0;
    for (int y = 0; y < 16 ; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            if (i < intDots)
            {
                SmallDots[i].mPosX = (16*(x));
                SmallDots[i].mPosY = (16*(y+2));
                i++;
            }
        }
    }
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if(!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if(!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
            controls();

			//Event handler
			SDL_Event e;

			//The ManPac that will be moving around on the screen
			ManPac ManPac;

			Mix_PlayChannel(-1, gStart, 0);

			initDots();

			//While application is running
			while(!quit)
			{
			    if (enabled_Debug) {std::cout << ManPac.mPosX << " " << ManPac.mPosY << std::endl;
			    std::cout << (ManPac.mPosY/32) << ((SmallDots[0].mPosY)/32) << (ManPac.mPosX/32) << ((SmallDots[0].mPosX)/32) << std::endl;}
                if (enabled_Spook)
                {
                    if (intervalChange != 0)
                    {
                        degree +=  (rand() % std::abs(intervalChange)); degree -=  (rand() % std::abs(intervalChange));
                        ManPac.mPosX += rand() % std::abs(intervalChange); ManPac.mPosY += rand() % std::abs(intervalChange);
                        ManPac.mPosX -= rand() % std::abs(intervalChange); ManPac.mPosY -= rand() % std::abs(intervalChange);
                        for (int i = 0; i < intDots; i++)
                        {
                            SmallDots[i].mPosX += rand() % std::abs(intervalChange); SmallDots[i].mPosY += rand() % std::abs(intervalChange);
                            SmallDots[i].mPosX -= rand() % std::abs(intervalChange); SmallDots[i].mPosY -= rand() % std::abs(intervalChange);
                        }
                    }
                }

				//Handle events on queue
				while(SDL_PollEvent(&e) != 0)
				{
					if(e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE) {quit = true;} //User requests quit
					ManPac.handleEvent(e); //Handle input for the ManPac
				}

				//Move the ManPac
				ManPac.move();

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
				SDL_RenderClear(gRenderer);
				gBackGround.render(0,32);

                SDL_Color textColor = {255, 255, 255};
				//Render objects
				std::string strScore;
				strScore += "Score: "; strScore += itos(intScore);
                gTextScoreTexture.loadFromRenderedText(strScore , textColor);
                gTextScoreTexture.render(0,0);

                gTextTitleTexture.loadFromRenderedText("ManPac" , textColor);
                gTextTitleTexture.render((gWindow.getWidth()-gTextTitleTexture.getWidth()),0);
                for (int i = 0; i < intDots; i++)
                {
                    SmallDots[i].render();
                }


                for (int i = 0; i < intDots; i++)
                {
                    if ((((ManPac.mPosY+16)/32) == ((SmallDots[i].mPosY)/32)) && (((ManPac.mPosX+16)/32) == ((SmallDots[i].mPosX)/32)))
                    {
                        SmallDots[i].mPosY = -999; SmallDots[i].mPosX = -999;
                        SmallDots[i].render();
                        Mix_PlayChannel(-1, gNom, 0);
                        intScore += 10;
                    }
                }


                //ManPac, must be last to render ontop of everything
				ManPac.render();

				//Update screen
				SDL_RenderPresent(gRenderer);
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
