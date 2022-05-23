#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define FPS 60
bool run = true;
int winw = 1280, winh = 720;
int cell_size = 10;

// typedef struct
// {
	// int8_t alive;
// } CELL;

typedef struct
{
	int p, size;
	int* cells;
} STACK;


typedef struct
{
	int x, y, offset_x, offset_y;
	int w, h;
	STACK stack;
	bool run, show_grid;
} CANVAS;


void undo(STACK* stack)
{
	if (stack->p > 0)
		stack->p--;
}

void redo(STACK* stack)
{
	stack->p++;
}

void push(STACK* stack, int data)
{	
	bool ok = true;

	for (int i = 0; i < stack->p; i++) {
		if (stack->cells[i] == data) {
			ok = false;
			break;
		}
	}
	
	if(ok) {
		SDL_Log("stack pointer: %d %d", stack->p, data);
		stack->cells[stack->p++] = data;
	}
}

void cap_fps(int start_time)
{
	if (1000/FPS > SDL_GetTicks() - start_time) {
		SDL_Delay(1000/FPS - (SDL_GetTicks() - start_time));
	}
}

void draw_grid(SDL_Renderer* renderer, CANVAS* canvas, int size)
{
	for (int i = 0; i < size; i+=cell_size) {
		SDL_RenderDrawLine(renderer, canvas->x+i, canvas->y, canvas->x+i, canvas->y+size);
		SDL_RenderDrawLine(renderer, canvas->x, canvas->y+i, canvas->x+size, canvas->y+i);
	}
	for (int y = 0; y < canvas->h; y += cell_size) {
		for (int x = 0; x < canvas->w; x+=cell_size) {
			SDL_RenderDrawLine(renderer, canvas->x+x, canvas->y, canvas->x+x, canvas->h);
			SDL_RenderDrawLine(renderer, canvas->x, canvas->y+y, canvas->w, canvas->y+y);
		}
	}
}

void render(SDL_Renderer* renderer, CANVAS* canvas)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 200, 0, 140, 255);

	SDL_Rect r = (SDL_Rect) {
		0,
		0,
		cell_size,
		cell_size
	};

	int size = fmax(canvas->w, canvas->h);
	if (canvas->show_grid)
		draw_grid(renderer, canvas, size);

	// int i = (canvas->offset_y/cell_size*canvas->w)+canvas->offset_x/cell_size;
	// for (int y = canvas->y; y < canvas->y+size; y += cell_size) {
		// for (int x = canvas->x; x < canvas->x+size; x+=cell_size) {
			// if (canvas->stack.cells[i] == 1) {
				// r.x = x;
				// r.y = y;
				// SDL_RenderDrawRect(renderer, &r);
				// SDL_RenderFillRect(renderer, &r);
			// }
			// i++;
		// }
		// i += (canvas->w-canvas->w/cell_size);
	// }

	int x, y;
	for (int i = 0; i < canvas->stack.p; i++) {
		if (canvas->stack.cells[i] != NULL) {
			if (canvas->stack.cells[i] < canvas->w) {
				x = canvas->x+canvas->stack.cells[i] * cell_size;
				y = canvas->y;
			}
			
			else {
				x = canvas->x+(canvas->stack.cells[i] % canvas->w) * cell_size;
				y = canvas->y+floor(canvas->stack.cells[i] / canvas->w) * cell_size;
			}


			r.x = x;
			r.y = y;
			SDL_RenderDrawRect(renderer, &r);
			SDL_RenderFillRect(renderer, &r);
		}
	}

	SDL_RenderPresent(renderer);
	
}


void canvas_init(CANVAS* canvas, int x, int y, int w, int h)
{
	canvas->x = x;
	canvas->y = y;
	canvas->w = w;
	canvas->h = h;
	canvas->stack.p = 0;
	canvas->stack.size = canvas->w*canvas->h;
	int* cells = calloc(canvas->stack.size, sizeof(int));
	canvas->stack.cells = cells;
	canvas->run = false;
}


int get_neighbor_count(CANVAS canvas, int index)
{
	int count = 0;
	int lr = index-canvas.w;
	int nr = index+canvas.w;
	for (int i = lr-1; i <= lr+1; i++) {
		if (i < 0) {continue;}
		if (canvas.stack.cells[i] != 0) {
			count++;
		}
	}

	for (int i = index-1; i <= index+1; i+=2) {
		if (i > canvas.w) {continue;}
		if (canvas.stack.cells[i] != 0) {
			count++;
		}
	}

	for (int i = nr-1; i <= nr+1; i++) {
		if (i > canvas.w) {continue;}
		if (canvas.stack.cells[i] != 0) {
			count++;
		}
	}

	SDL_Log("count: %d, %d", index, count);
	return count;
}

void simulate(CANVAS* canvas)
{
	int c = 0;
	int* cells = calloc(canvas->stack.size, sizeof(int));
	for (int i = 0; i < canvas->stack.size; i++) {
		cells[i] = canvas->stack.cells[i];

		if (cells[i] != 0) {
			c = get_neighbor_count(*canvas, cells[i]);
			SDL_Log("ok: %d, %d", cells[i], c);
			if (c > 3 || c < 2) {
				SDL_Log("fuck: %d, %d", cells[i], c);
				cells[i] = 0;
			}
		}
	}
	free(canvas->stack.cells);
	canvas->stack.cells = cells;
}

int main() {
	printf("c to start clear mode\nh to (un)hide grid\nreturn/enter to simulate one generation\np to start|stop simulation\ns to stop simulation\n\n");
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_Window* window = SDL_CreateWindow("test", SDL_WINDOWPOS_UNDEFINED,
		 SDL_WINDOWPOS_UNDEFINED, winw, winh,
		 SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI
	);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Event event;
	int ticks = SDL_GetTicks();

	CANVAS canvas;
	canvas_init(&canvas, 200, 0, 1280, 720);
	
	int initial_x = 0, initial_y = 0;
	int m_x = 0, m_y = 0;
	bool moving = false;
	bool clear = false;
	char c = 0;
	int multiplier = 1;

	while (run) {
		cap_fps(ticks);
		ticks = SDL_GetTicks();

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					run = false;
					break;


				case SDL_MOUSEWHEEL:
					if (event.wheel.y > 0) {
						cell_size++;
					}

					else {
						cell_size--;
					}

					if (cell_size <= 1) {
						cell_size = 2;
					}
					break;

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEMOTION:
					if (event.button.button == SDL_BUTTON_RIGHT) {
						SDL_GetMouseState(&initial_x, &initial_y);
						moving = true;
					}

					int distance_x = abs(m_x - initial_x);
					int distance_y = abs(m_x - initial_x);

					if (moving) {
						SDL_GetMouseState(&m_x, &m_y);
						if (distance_x > 20) {
							if (m_x > initial_x) {
								canvas.offset_x = -(int)distance_x;
								if (canvas.offset_x < 0) {
									canvas.offset_x = 0;
								}
							}
	
							else {
								canvas.offset_x = (int)distance_x;
								if (canvas.offset_x > canvas.w-canvas.w/cell_size) {
									canvas.offset_x = canvas.w-canvas.w/cell_size;
								}
							}
						}

						if (distance_y > 50) {
							if (m_y > initial_y) {
								canvas.offset_y = -(int)distance_y-50;
								if (canvas.offset_y < 0) {
									canvas.offset_y = 0;
								}
							}
							
	
							else {
								canvas.offset_y = (int)distance_y+50;
								if (canvas.offset_y > canvas.h-canvas.h/cell_size) {
									canvas.offset_y = canvas.h-canvas.h/cell_size;
								}
							}
						}
						// SDL_Log("AFTER! canvas offset x: %d | y: %d", canvas.offset_x, canvas.offset_y);
					}

					if (event.button.button == SDL_BUTTON_LEFT && 
						event.button.x > canvas.x && event.button.x < canvas.x+canvas.w &&
						event.button.y > canvas.y && event.button.y < canvas.y+canvas.h
					) {

						// int f = (int)((r.x)+(r.y*(winw)));
						int x = event.button.x-(event.button.x%cell_size);
						int y = event.button.y-(event.button.y%cell_size);
						x = (canvas.offset_x+x-canvas.x)/cell_size;
						y = ((canvas.offset_y+y-canvas.y)/cell_size); // *(canvas.w/cell_size)
						int f = x+y*canvas.w;
						// if (!canvas.cells[f]) {
						// canvas.cells[f] = !clear;
						push(&canvas.stack, f);
						// }
					}
					break;

				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_RIGHT) {
						moving = false;
					}
					break;

				case SDL_KEYDOWN:
					c = event.key.keysym.sym;
					switch (c) {
						case SDLK_RETURN:
							simulate(&canvas);
						break;

						case SDLK_c:
							clear = !clear;
						break;

						case SDLK_p:
							canvas.run = !canvas.run;
						break;

						case SDLK_s:
							canvas.run = false;
						break;
						
						case SDLK_h:
							canvas.show_grid = !canvas.show_grid;
						break;

						case SDLK_z:
							for (int i = 0; i < multiplier; i++) {
								undo(&canvas.stack);
							}
							multiplier++;
						break;

						case SDLK_y:
							for (int i = 0; i < multiplier; i++) {
								redo(&canvas.stack);
							}
							multiplier++;
						break;

						case SDLK_ESCAPE:
							run = false;
						break;
					}
					break;

				case SDL_KEYUP:
					multiplier = 1;
					break;
					
			}
		}
		if (canvas.run) {
			simulate(&canvas);
		}
		render(renderer, &canvas);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	

	SDL_Quit();
	return 0;
}

