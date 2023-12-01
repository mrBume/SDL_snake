#include <iostream>
#include <vector>
#include <array>
#include <deque>
#include <string>
#include <cstring>
#include <algorithm>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

const int WIDTH = 550, HEIGHT = 550;

const int GRID_SIZE = 50;
const int X_CELLS   = 10;
const int Y_CELLS   = 10;

int gameboard_x = X_CELLS * GRID_SIZE;
int gameboard_y = Y_CELLS * GRID_SIZE;

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

struct CustomColor
{
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
    Snake(SDL_Renderer * render, SDL_Rect game_area)
    {
      view_port = game_area;
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

    bool CheckPosInBody(Vect2D pos, int offset = 0)
    {
      deque<Vect2D>::iterator it = find(body.begin() + offset, body.end(), pos);
      
      return it != body.end();
    }

    Vect2D GetHeadPos()
    {
      return body[0];
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
      
      int lim_x = static_cast<int>(view_port.w / GRID_SIZE - 1);
      int lim_y = static_cast<int>(view_port.h / GRID_SIZE - 1);
      // check borders
      if(x > lim_x)
        x = 0;
      else if( x < 0)
        x = lim_x;

      if(y > lim_y)
        y = 0;
      else if( y < 0)
        y = lim_y;

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
          rect.x = view_port.x + body[i].x * GRID_SIZE;
          rect.y = view_port.y + body[i].y * GRID_SIZE;
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
    Food(SDL_Renderer * render, SDL_Rect game_area)
    {
      renderer = render;
      view_port = game_area;
      GenRandom();
    };

    void Draw()
    {
      SDL_Rect rect;
      rect.x = view_port.x + pos.x * GRID_SIZE;
      rect.y = view_port.y + pos.y * GRID_SIZE;
      rect.w = GRID_SIZE;
      rect.h = GRID_SIZE;

      SetColor(renderer, color_pallete[3]);
      
      SDL_RenderFillRect(renderer, &rect);
    }

    Vect2D GetPos()
    {
      return pos;
    }

    void GenRandom()
    {
      pos.x = rand() % X_CELLS;
      pos.y = rand() % Y_CELLS;   
    }

    void SetDefault()
    {
      pos = {-1, -1};
    }


  private:
    Vect2D pos;
    SDL_Renderer * renderer = nullptr;
    bool draw_allowed = true;
    SDL_Rect view_port;
};

class Game
{
  public:
    Game(SDL_Window * window)
    {
      int x_center = static_cast<int>((WIDTH - gameboard_x) / 2);
      int y_center = static_cast<int>((HEIGHT - gameboard_y) / 2);

      game_area = {x_center, y_center, gameboard_x, gameboard_y};
      info_area = {x_center, y_center - 20, gameboard_x, 20};
      game_render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
      
      // SDL_RenderSetViewport(game_render, &game_area);

      snake = new Snake(game_render, game_area);
      food = new Food(game_render, game_area);
      score = 0;
      ticks = snake_time;
      move = {1,0};
      running = true;

      normal_font = TTF_OpenFont("../example/font.ttf", 24);
      small_font = TTF_OpenFont("../example/font.ttf", 14);
      if (!normal_font || !small_font)
      {
        cout << "Error loading font: " << TTF_GetError() << endl;
      }
    }
    
    void Reset()
    {
      delete(snake);
      delete(food);
      snake = new Snake(game_render, game_area);
      food = new Food(game_render, game_area);
      score = 0;
      move = {1,0};
      running = true;
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
      DrawBase();
      
      if(running)
      {
        CheckSnakeColission();
        CheckFoodColission();
        if(CheckTime() && (snake->GetSize() < (X_CELLS * Y_CELLS)))
        {
          snake->SetDirection(move);
          snake->Update();
        }
      }

      food->Draw();
      snake->Draw();
      DrawInfo();

      if(!running)
      {
        endScreen();
      }

      SDL_RenderPresent(game_render);
    }

    void endScreen()
    {
      string text;
      SDL_Rect rect;
      SDL_Rect render_rect;
      SDL_GetRendererOutputSize(game_render, &render_rect.w, &render_rect.h);

      if(snake->GetSize() == (X_CELLS * Y_CELLS))
      {
        text = "Ganador!";
        TTF_SizeText(normal_font, text.c_str(), &rect.w, &rect.h);
        rect.x = (render_rect.w - rect.w) / 2;
        rect.y = (render_rect.h - rect.h) / 2;
      }
      else
      {
        text = "Perdedor!";
        TTF_SizeText(normal_font, text.c_str(), &rect.w, &rect.h);
        rect.x = (render_rect.w - rect.w) / 2;
        rect.y = (render_rect.h - rect.h) / 2;
      }

      renderText(text, rect, normal_font);
      rect.x -= 20;
      rect.y += 20;
      renderText("(R) reset, (Q) quit", rect, small_font);
    }

    void InputHandler(SDL_Keycode key)
    {
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
      if(snake->GetSize() >= (X_CELLS * Y_CELLS))
      {
        // food->SetDefault();
        GameOver();
      }
      else
      {
        if(snake->GetHeadPos() == food->GetPos())
        {
          snake->SetGrow();
          do
          {
            food->GenRandom();
          } while (snake->CheckPosInBody(food->GetPos()));

          score += 10;
        }
      }
    }

    void CheckSnakeColission()
    {
      if(snake->CheckPosInBody(snake->GetHeadPos(), 1))
      {
        GameOver();
      }
    }

    void GameOver()
    {
      running = false;
    }

    void DrawBase()
    {
      // draw background
      SetColor(game_render, color_pallete[1]);
      SDL_RenderClear(game_render);
      
      // draw grid
      SetColor(game_render, color_pallete[2]);

      for(int i = 0; i <= static_cast<int>(game_area.w / GRID_SIZE); i++)
      {
        SDL_RenderDrawLine(game_render, game_area.x + i * GRID_SIZE, game_area.y, game_area.x + i * GRID_SIZE, game_area.y + game_area.h);
      }

      for(int i = 0; i <= static_cast<int>(game_area.h / GRID_SIZE); i++)
      {
        SDL_RenderDrawLine(game_render, game_area.x, game_area.y + i * GRID_SIZE, game_area.x + game_area.w, game_area.y + i * GRID_SIZE);
      }
    }

    void DrawInfo()
    {
      string text = "Score: " + to_string(score);
      renderText(text.c_str(), info_area, small_font);
    }

    void renderText(string text, SDL_Rect dest, TTF_Font * font)
    {
      SDL_Color fg = {color_pallete[0].r, color_pallete[0].g, color_pallete[0].b};
      SDL_Surface *surf = TTF_RenderText_Solid(font, text.c_str(), fg);

      dest.w = surf->w;
      dest.h = surf->h;

      SDL_Texture *tex = SDL_CreateTextureFromSurface(game_render, surf);

      SDL_RenderCopy(game_render, tex, NULL, &dest);
      SDL_DestroyTexture(tex);
      SDL_FreeSurface(surf);
    }

  private:
    Snake * snake;
    Food * food;
    unsigned int score;
    SDL_Renderer * game_render;
    int ticks;
    int snake_time = 200;
    Vect2D move;
    SDL_Rect game_area;
    SDL_Rect info_area;
    bool running;

    TTF_Font * normal_font;
    TTF_Font * small_font;
};

int main(int argc, char *argv[])
{
  bool isRunning = true;

  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    cout << "Error at initilializatiion" << endl;
  }

  if (TTF_Init() < 0)
	{
		cout << "Error initializing SDL_ttf: " << TTF_GetError() << endl;
		return false;
	}

  SDL_Window *window = SDL_CreateWindow("Simple Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
  
  if (NULL == window)
  {
    cout << "Could not create window: " << SDL_GetError() << endl;
    return 1;
  }
    
  Game game(window);
  
  while (isRunning)
  {
    SDL_Event windowEvent;

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
    
    SDL_Delay(1000u / 60);
  }

  SDL_Quit();

  return EXIT_SUCCESS;
}