#include <iostream>
#include <vector>
#include <array>
#include <deque>
#include <string>
#include <algorithm>

#include <SDL2/SDL.h>

using namespace std;

const int WIDTH = 420, HEIGHT = 600;

const int GRID_SIZE = 50;
const int X_CELLS   = 3;
const int Y_CELLS   = 3;

int gameboard_x = X_CELLS * GRID_SIZE + 1;
int gameboard_y = Y_CELLS * GRID_SIZE + 1;

struct Vect2D
{
  Vect2D() { x = 0; y = 0;}
  Vect2D(char _x, char _y) { x = _x; y = _y;}

  bool operator==(Vect2D const & obj)
  {
    return obj.x == x && obj.y == y;
  }

  char x ,y;
};

class CustomColor
{
  public:
    CustomColor()
    {
      r = 0;
      g = 0;
      b = 0;
    }

    CustomColor(int color)
    {
      b = color & 0xFF;
      g = (color >> 8) & 0xFF;
      r = (color >> 16) & 0xFF;
    }
      
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

vector<CustomColor> color_pallete = {
  0xF2FFE9,
  0xA6CF98,
  0x557C55,
  0xFA7070
};

void SetColor(SDL_Renderer * renderer, CustomColor color)
{
  SDL_SetRenderDrawColor(renderer,  color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
}

void SetColorAlpha(SDL_Renderer * renderer, CustomColor color, char alpha)
{
  SDL_SetRenderDrawColor(renderer,  color.r, color.g, color.b, alpha);
}


class Snake
{
  public:
    Snake(SDL_Renderer * render)
    {
      SDL_RenderGetViewport(render, &view_port);
      SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);
      
      body.clear();
      body.push_back({static_cast<char>(view_port.w / GRID_SIZE / 2), static_cast<char>(view_port.h / GRID_SIZE / 2)});
      direction.x = 1;
      direction.y = 0;
      grow = false;
      renderer = render;
    }

    void SetDirection(Vect2D dir)
    {
      if( (dir.x == -1 && direction.x ==  1) || // move right
          (dir.x ==  1 && direction.x == -1) || // move left
          (dir.y == -1 && direction.y ==  1) || // move up
          (dir.y ==  1 && direction.y == -1))   // move down
        return;

      direction = dir;
      return;
    }

    void SetGrow()
    {
      grow = true;
    }

    void GetPos(int *x_, int *y_)
    {
      *x_ = body[0].x;
      *y_ = body[0].y;
    }

    int GetSize()
    {
      return body.size();
    }

    deque<Vect2D> GetBodyPos()
    {
      deque<Vect2D> body_pos(body);
      return body_pos;
    }

    void Update()
    {
      // new position
      int x = body[0].x + direction.x;
      int y = body[0].y + direction.y;
      
      // check borders
      if(x > static_cast<int>(view_port.w / GRID_SIZE - 1))
        x = 0;
      else if( x < 0)
        x = static_cast<int>(view_port.w / GRID_SIZE - 1);

      if(y > static_cast<int>(view_port.h / GRID_SIZE - 1))
        y = 0;
      else if( y < 0)
        y = static_cast<int>(view_port.h / GRID_SIZE - 1);

      body.push_front(Vect2D(x, y));
      
      if(!grow)
        body.pop_back();
      else
        grow = false;
    }

    void Draw()
    {
      SDL_Rect rect;

      if(renderer != nullptr)
      {
        for(int i = 0; i < body.size(); i++)
        {
          char al = 0xFF - i * 5 < 40 ? 40 : 0xFF - i * 5 ;
          SetColorAlpha(renderer, color_pallete[2], al);
          rect.x = body[i].x * GRID_SIZE;
          rect.y = body[i].y * GRID_SIZE;
          rect.w = GRID_SIZE;
          rect.h = GRID_SIZE;

          SDL_RenderFillRect(renderer, &rect);
        }
      }
      else
      {
        cout << "La has liado con la snake y en renderer" << endl;
      }
    }

  
  private:
    bool            grow;
    deque<Vect2D>   body;
    Vect2D          direction;
    SDL_Renderer *  renderer = nullptr;
    SDL_Rect        view_port;
};

class Food
{
  public:
    Food(SDL_Renderer * render)
    {
      renderer = render;
      GenRandom();
    };

    void Draw()
    {
      SDL_Rect rect;
      rect.x = x * GRID_SIZE;
      rect.y = y * GRID_SIZE;
      rect.w = GRID_SIZE;
      rect.h = GRID_SIZE;

      SetColor(renderer, color_pallete[3]);
      
      SDL_RenderFillRect(renderer, &rect);
    }

    void GetPos(int *x_, int *y_)
    {
      *x_ = x;
      *y_ = y;
    }

    void GenRandom(deque<Vect2D> snake_body = deque<Vect2D>()) // default empty
    {
      deque<Vect2D>::iterator it = snake_body.begin();
      int _x, _y;
      if(snake_body.size() < (X_CELLS * Y_CELLS))
      {
        do
        {
          _x = rand() % X_CELLS;
          _y = rand() % Y_CELLS;

        }while((find(snake_body.begin(), snake_body.end(), Vect2D(_x, _y)) != snake_body.end()) || // new element not in snake
                (_x == x && _y == y)); // not in previous position
        
        x = _x;
        y = _y;
      }
      else
      {
        x = -1;
        y = -1;
      }     
    }


  private:
    int x, y;
    SDL_Renderer * renderer = nullptr;
    bool draw_allowed = true;
};

class Game
{
  public:
    Game(SDL_Renderer * render)
    {
      snake = new Snake(render);
      food = new Food(render);
      score = 0;
      game_render = render;
      ticks = snake_time;
      move = {1,0};
    }
    
    void Reset()
    {
      delete(snake);
      delete(food);
      snake = new Snake(game_render);
      food = new Food(game_render);
      score = 0;
    } 

    bool CheckTime()
    {
      int ticks_now = SDL_GetTicks();
      if(ticks_now - ticks > snake_time)
      {
        ticks = ticks_now;
        return true;
      }
      return false;
    }

    void Update()
    {
      CheckFoodColission();
      if(CheckTime() && (snake->GetSize() < (X_CELLS * Y_CELLS)))
      {
        snake->SetDirection(move);
        snake->Update();
      }

      food->Draw();
      snake->Draw();

    }

    void InputHandler(SDL_Keycode key)
    {
      cout << "key: " << key << endl;
      switch (key)
      {
      case SDLK_w:
        move = {0, -1};
        break;
      case SDLK_s:
        move = {0, 1};
        break;
      case SDLK_a:
        move = {-1, 0};
        break;
      case SDLK_d:
        move = {1, 0};
        break;
      case SDLK_r:
        Reset();
        break;
      default:
        break;
      }
    }

    void CheckFoodColission()
    {
      int fx, fy, sx, sy;
      food->GetPos(&fx, &fy);
      snake->GetPos(&sx, &sy);

      if((fx == sx) && (fy == sy))
      {
        snake->SetGrow();
        food->GenRandom(snake->GetBodyPos());
        score += 10;
        cout << "score: " << score << endl;
      }
    }

  private:
    Snake * snake;
    Food * food;
    unsigned int score;
    SDL_Renderer * game_render;
    int ticks;
    int snake_time = 200;
    Vect2D move;
};

int main(int argc, char *argv[])
{
  bool isRunning = true;

  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    cout << "Error at initilializatiion" << endl;
  }

  SDL_Window *window = SDL_CreateWindow("Simple Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);

  if (NULL == window)
  {
    cout << "Could not create window: " << SDL_GetError() << endl;
    return 1;
  }

  SDL_Renderer * rendered = nullptr;
  rendered = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  int x_center = static_cast<int>((WIDTH - gameboard_x) / 2);
  int y_center = static_cast<int>((HEIGHT - gameboard_y) / 2);

  SDL_Rect rect = {x_center, y_center, gameboard_x, gameboard_y};

  SDL_RenderSetViewport(rendered, &rect);
  Game game(rendered);
  
  int counter = 0;

  while (isRunning)
  {
    SDL_Event windowEvent;
    
    // draw background
    SetColor(rendered, color_pallete[1]);
    SDL_RenderClear(rendered);
    
    // draw grid
    SetColor(rendered, color_pallete[2]);
    
    for(int i = 0; i <= static_cast<int>(rect.w / GRID_SIZE); i++)
    {
      SDL_RenderDrawLine(rendered, i * GRID_SIZE, 0, i * GRID_SIZE, rect.h);
    }

    for(int i = 0; i <= static_cast<int>(rect.h / GRID_SIZE); i++)
    {
      SDL_RenderDrawLine(rendered, 0, i * GRID_SIZE, rect.w, i * GRID_SIZE);
    }

    while(SDL_PollEvent(&windowEvent))
    {
      switch(windowEvent.type)
      {
        case SDL_KEYDOWN:
          switch (windowEvent.key.keysym.sym)
          {
          case SDLK_w:
          case SDLK_a:
          case SDLK_s:
          case SDLK_d:
          case SDLK_r:
            game.InputHandler(windowEvent.key.keysym.sym);
            break;
          case SDLK_q:
            isRunning = false;
            break;
          default:
            break;
          }
        break;
        case SDL_QUIT:
          isRunning = false;
        break;
      }
    }

    game.Update();

    SDL_RenderPresent(rendered);
    SDL_Delay(1000u / 60);
  }

  SDL_Quit();

  return EXIT_SUCCESS;
}