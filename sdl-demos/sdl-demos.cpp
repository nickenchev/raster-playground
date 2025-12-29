#include <SDL3/SDL.h>
#include "sdl-demos.h"

using namespace std;

struct RoadSegment
{
};

float logW = 320;
float logH = 200;
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

void initialize()
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("SDL Demos", 1280, 800, 0);
	renderer = SDL_CreateRenderer(window, nullptr);
	SDL_SetRenderLogicalPresentation(renderer, logW, logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

int main()
{
	initialize();

	const float metersPerSegment = 5.0f;
	const float metersPerUnit = 1.0f;
	RoadSegment segments[200];

	// 1 z-unit = 1 meter
	float camY = 10.0f;
	float camZ = 0; // distance from the screen
	const bool *keys = SDL_GetKeyboardState(nullptr);
	float velocity = 0;
	float acceleration = 3.0f;

	bool running = true;
	uint64_t prevTime = SDL_GetTicks();
	while (running)
	{
		uint64_t nowTime = SDL_GetTicks();
		const float dt = (nowTime - prevTime) / 1000.0f;
		prevTime = nowTime;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				running = false;
			}
		}

		float directionZ = 0;
		if (keys[SDL_SCANCODE_W])
		{
			directionZ += 1;
		}
		if (keys[SDL_SCANCODE_S])
		{
			directionZ += -1;
		}

		velocity += acceleration * dt;
		camZ += directionZ * velocity * dt;

		// render screen to ourun sky color
		SDL_SetRenderDrawColor(renderer, 1, 146, 250, 255);
		SDL_RenderClear(renderer);

		float horizon = logH / 2;
		// draw the road
		float roadWidth = 100;

		// draw the ground
		for (int y = logH - 1; y > horizon; --y)
		{
			float z = camY / (y - horizon); // z distance from the camera
			float scale = 1.0f / z; // needs to be inverted in order to be used as scale
			const float zMeters = z / metersPerUnit;

		}

		for (int y = logH - 1; y > horizon; --y)
		{
			float z = camY / (y - horizon); // z distance from the camera
			float scale = 1.0f / z; // needs to be inverted in order to be used as scale
			float xWidth = roadWidth * scale;
			float xOffset = (logW - xWidth) / 2;
			const float worldZ = camZ + z;

			const float zMeters = worldZ / metersPerUnit;
			if (static_cast<int>(zMeters / 3.0f) % 2)
			{
				SDL_SetRenderDrawColor(renderer, 234, 219, 202, 255);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 226, 210, 195, 255);
			}
			SDL_RenderLine(renderer, 0, y, logW, y);

			if (static_cast<int>(zMeters / 2.0f) % 2)
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
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
