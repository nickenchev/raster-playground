#include <SDL3/SDL.h>
#include "sdl-demos.h"

using namespace std;

int main()
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* win = SDL_CreateWindow("SDL Demos", 2160, 1440, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(win, nullptr);

	float logW = 320;
	float logH = 200;

	SDL_SetRenderLogicalPresentation(renderer, logW, logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	bool running = true;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				running = false;
			}
		}

		// render screen to ourun sky color
		SDL_SetRenderDrawColor(renderer, 1, 146, 250, 255);
		SDL_RenderClear(renderer);

		float horizon = logH / 4;
		float cameraDepth = logH * 0.9f;

		// draw the ground
		for (int i = logH; i >= horizon; --i)
		{
			if (i % 2 == 0)
			{
				SDL_SetRenderDrawColor(renderer, 234, 219, 202, 255);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 226, 210, 195, 255);
			}
			SDL_RenderLine(renderer, 0, i, logW, i);
		}

		// draw the road
		float roadWidth = 200;

		for (int y = logH - 1; y >= horizon; --y)
		{
			float perspective = cameraDepth / (y - horizon);
			float xWidth = roadWidth * perspective;
			float xOffset = (logW - xWidth) / 2;

			if ((y / 4) % 2 == 0)
			{
				SDL_SetRenderDrawColor(renderer, 154, 155, 154, 255);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 146, 146, 147, 255);
			}
			SDL_RenderLine(renderer, xOffset, y, logW - xOffset, y);
		}

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
