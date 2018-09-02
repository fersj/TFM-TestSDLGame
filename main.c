//
//  main.c
//  TFM-SDLGame
//
//  Created by Fernando Suárez Jiménez on 9/3/18.
//  Copyright © 2018 Fernando Suárez Jiménez. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <time.h>

//Key press surfaces constants
enum KeyPressSurfaces {
    KEY_PRESS_SURFACE_DEFAULT,
    KEY_PRESS_SURFACE_UP,
    KEY_PRESS_SURFACE_DOWN,
    KEY_PRESS_SURFACE_LEFT,
    KEY_PRESS_SURFACE_RIGHT,
    KEY_PRESS_SURFACE_TOTAL
};

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

SDL_Window *window;
SDL_Surface *screen;
SDL_Surface *background;
SDL_Surface *shipSurface[KEY_PRESS_SURFACE_TOTAL], *shipCurrent;
SDL_Surface *laserSurface;

typedef struct Ship Ship;
typedef struct Shot Shot;

struct Ship {
    int x, y;
    int vx, vy;
    const int VEL;
    float sw, sh;
    Shot *shots;
};

struct Shot {
    int x, y;
    int vx, vy;
    float sw, sh;
    Shot *next;
};

SDL_bool Init (void);
SDL_bool LoadMedia(void);
void End (void);
void addShot (Ship *ship);
void drawShots (Ship *ship, SDL_Surface *s);
void deleteOutShots (Shot *shots, SDL_Surface *s );
void HandleShipMovement (SDL_Event *e);
void MoveShip (Ship *s);

Ship s1 = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 0, 0, 2, 0.5, 0.5, NULL};

int main () {
    //Start up SDL and create window
    if(!Init()){
        printf( "Failed to initialize!\n" );
        exit(1);
    }
    
    //Load media
    if(!LoadMedia()) {
        printf( "Failed to load media!\n" );
        exit(1);
    }
    
    SDL_bool quit = SDL_FALSE;
    SDL_Event *e = malloc(sizeof(SDL_Event));
    //const uint8_t *keyboardState = SDL_GetKeyboardState(NULL);
    shipCurrent = shipSurface[KEY_PRESS_SURFACE_DEFAULT];
    
    clock_t t_ini, t_end;
    int cnt = 250;
    t_ini = clock();
    while (!quit) {
        while (SDL_PollEvent(e) != 0) {
            //User requests quit
            if (e->type == SDL_QUIT) {
                quit = SDL_TRUE;
            }
            if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_SPACE)
                addShot(&s1);
            HandleShipMovement(e);
        }
        MoveShip(&s1);
        //Apply the background surface
        SDL_BlitSurface(background, NULL, screen, NULL);
        //Apply the ship surface
        SDL_Rect shipRect = {s1.x, s1.y, shipCurrent->w*s1.sw, shipCurrent->h*s1.sh};
        SDL_BlitScaled(shipCurrent, NULL, screen, &shipRect);
        //Apply the shot sorfaces
        drawShots(&s1, laserSurface);
        //Update the surface
        SDL_UpdateWindowSurface(window);
        if (cnt == 0) {
            t_end = clock();
            double secs = (double)(t_end - t_ini) / CLOCKS_PER_SEC;
            printf("FPS: %f\n", 250/secs);
            cnt = 250;
            t_ini = clock();
        }
        cnt--;
    }
    
    //Free resources and close SDL
    End();
    
    return 0;
}

SDL_bool Init () {
    SDL_bool success = SDL_TRUE;
    //int i = SDLK_UP;
    
    // SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_CDROM)
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = SDL_FALSE;
    } else {
        //Create window
        window = SDL_CreateWindow("Space TFM", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if(window == NULL) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = SDL_FALSE;
        } else {
            // Get window surface
            screen = SDL_GetWindowSurface(window);
        }
    }
    
    return success;
}

SDL_bool LoadMedia () {
    SDL_bool success = SDL_TRUE;
    char *shipImgDefaultPath = "resources/img/ship1.png";
    char *backgroundImgPath = "resources/img/space_bg.png";
    char *laserImgPath = "resources/img/ray.png";
    
    //Load background image
    SDL_Surface *backgroundAux = IMG_Load(backgroundImgPath);
    if (backgroundAux == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", backgroundImgPath, IMG_GetError());
        success = SDL_FALSE;
    } else {
        background = SDL_ConvertSurface(backgroundAux, screen->format, 0);
        if (background == NULL) {
            printf("Unable to optimize image %s! SDL Error: %s\n", backgroundImgPath, SDL_GetError());
        }
        SDL_FreeSurface(backgroundAux);
    }
    
    //Load spaceship image
    SDL_Surface *shipSurfaceAux = IMG_Load(shipImgDefaultPath);
    if(shipSurfaceAux == NULL) {
        printf( "Unable to load image %s! SDL_image Error: %s\n", shipImgDefaultPath, IMG_GetError() );
        success = SDL_FALSE;
    } else {
        shipSurface[KEY_PRESS_SURFACE_DEFAULT] = SDL_ConvertSurface(shipSurfaceAux, screen->format, 0);
        if (shipSurface[KEY_PRESS_SURFACE_DEFAULT] == NULL) {
            printf("Unable to optimize image %s! SDL Error: %s\n", shipImgDefaultPath, SDL_GetError());
        }
        SDL_FreeSurface(shipSurfaceAux);
    }
    
    //Load laser ray
    SDL_Surface *laserSurfaceAux = IMG_Load(laserImgPath);
    if(laserSurfaceAux == NULL) {
        printf( "Unable to load image %s! SDL_image Error: %s\n", laserImgPath, IMG_GetError() );
        success = SDL_FALSE;
    } else {
        laserSurface = SDL_ConvertSurface(laserSurfaceAux, screen->format, 0);
        if (laserSurface == NULL) {
            printf("Unable to optimize image %s! SDL Error: %s\n", laserImgPath, SDL_GetError());
        }
        SDL_FreeSurface(laserSurfaceAux);
    }
    
    return success;
}

void End () {
    //Deallocate surfaces
    for (int i=0; i<KEY_PRESS_SURFACE_TOTAL; ++i) {
        SDL_FreeSurface(shipSurface[i]);
        shipSurface[i] = NULL;
    }
    SDL_FreeSurface(background);
    background = NULL;
    
    //Destroy window
    SDL_DestroyWindow(window);
    window = NULL;
    
    //Quit SDL subsystems
    SDL_Quit();
}

void addShot (Ship *ship) {
    Shot *shot = SDL_malloc(sizeof(Shot));
    shot->sw = shot->sh = 0.1;
    shot->x = (ship->x + shipCurrent->w*ship->sw/2) - laserSurface->w/2*shot->sw;
    shot->y = ship->y;
    shot->vx = 0;
    shot->vy = 5;
    shot->sw = shot->sh = 0.1;
    shot->next = NULL;
    
    if (ship->shots == NULL)
        ship->shots = shot;
    else {
        Shot *i = ship->shots;
        while (i->next != NULL) {
            i = i->next;
        }
        i->next = shot;
    }
}

void drawShots (Ship *ship, SDL_Surface *s) {
    Shot *i = ship->shots;
    while (i != NULL) {
        i->x -= i->vx;
        i->y -= i->vy;
        SDL_Rect r = {i->x, i->y, s->w*i->sw, s->h*i->sh};
        SDL_BlitScaled(s, NULL, screen, &r);
        i = i->next;
    }
    
    deleteOutShots(ship->shots, s);
}

void deleteOutShots (Shot *shots, SDL_Surface *s) {
    if (shots == NULL)
        return;
    Shot *i=shots, *aux;
    while (i->next != NULL) {
        if (i->next->y < -s->h*i->next->sh) {
            aux = i->next->next;
            SDL_free(i->next);
            i->next = aux;
        } else {
            i = i->next;
        }
    }
}

void HandleShipMovement (SDL_Event *e) {
    if (e->type == SDL_KEYDOWN && e->key.repeat == 0) {
        //Adjust the velocity
        switch (e->key.keysym.sym) {
            case SDLK_UP: s1.vy -= s1.VEL; break;
            case SDLK_DOWN: s1.vy += s1.VEL; break;
            case SDLK_LEFT: s1.vx -= s1.VEL; break;
            case SDLK_RIGHT: s1.vx += s1.VEL; break;
        }
    }
    //If a key was released
    else if (e->type == SDL_KEYUP && e->key.repeat == 0) {
        //Adjust the velocity
        switch (e->key.keysym.sym) {
            case SDLK_UP: s1.vy += s1.VEL; break;
            case SDLK_DOWN: s1.vy -= s1.VEL; break;
            case SDLK_LEFT: s1.vx += s1.VEL; break;
            case SDLK_RIGHT: s1.vx -= s1.VEL; break;
        }
    }
    //printf("Vx=%d, Vy=%d\n",s1.vx, s1.vy);
}

void MoveShip (Ship *s) {
    //Move the ship left or right
    s->x += s->vx;
    
    if ((s->x < 0) || (s->x + shipCurrent->w*s->sw > SCREEN_WIDTH)) {
        s->x -= s->vx;
    }
    
    //Move the ship up or down
    s->y += s->vy;
    
    if ((s->y < 0) || (s->y + shipCurrent->h*s->sh > SCREEN_HEIGHT)) {
        s->y -= s->vy;
    }
}


