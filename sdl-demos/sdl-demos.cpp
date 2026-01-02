#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <array>
#include <vector>
#include "sdl-demos.h"

using namespace std;

struct RoadSegment
{
	float elevation;
};

float logW = 320;
float logH = 224;
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

void initialize()
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("SDL Demos", 1920, 1080, 0);
	renderer = SDL_CreateRenderer(window, nullptr);
	SDL_SetRenderLogicalPresentation(renderer, logW, logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

SDL_Texture *skylineTex = nullptr;
SDL_Texture *plant1Tex = nullptr;
SDL_Texture *carForwardTex = nullptr;

void load()
{
	skylineTex = IMG_LoadTexture(renderer, "data/skyline.png");
	SDL_SetTextureScaleMode(skylineTex, SDL_SCALEMODE_NEAREST);
	plant1Tex = IMG_LoadTexture(renderer, "data/plant1.png");
	SDL_SetTextureScaleMode(plant1Tex, SDL_SCALEMODE_NEAREST);
	carForwardTex = IMG_LoadTexture(renderer, "data/car.png");
	SDL_SetTextureScaleMode(carForwardTex, SDL_SCALEMODE_NEAREST);
}

void cleanup()
{
	SDL_DestroyTexture(carForwardTex);
	SDL_DestroyTexture(skylineTex);
	SDL_DestroyTexture(plant1Tex);
}

int main()
{
	initialize();
	load();

	const float metersPerSegment = 2.0f;
	const float metersPerUnit = 1.0f;

	std::array<RoadSegment, 500> segments;
	int i = 0;
	for (auto &seg : segments)
	{
		seg.elevation = sin(i / 2.0f) * 2;
		i++;
	}

	// 1 z-unit = 1 meter
	float camY = 25.0f;
	float camZ = 0; // distance from the screen
	const bool *keys = SDL_GetKeyboardState(nullptr);

	// car speed control
	const float maxSpeed = 15;
	const float speedMultFactor = 6; // multiply speed by this (pretend we're moving faster), to avoid wagonwheel-effect
	const float acceleration = 2.0f;
	float velocity = 0;
	float carX = 0;

	uint64_t prevTime = SDL_GetTicks();
	float accumulator = 0;
	const float fixedStep = 1 / 30.0f;
	double globalTime = 0;

	bool running = true;
	while (running)
	{
		uint64_t nowTime = SDL_GetTicks();
		const float dt = (nowTime - prevTime) / 1000.0f;
		prevTime = nowTime;
		accumulator += dt;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				running = false;
			}
			else if (event.type == SDL_EVENT_KEY_UP)
			{
				if (event.key.scancode == SDL_SCANCODE_F9)
				{
					SDL_Surface *surface = SDL_RenderReadPixels(renderer, nullptr);
					SDL_SaveBMP(surface, "screenshot.bmp");
				}
			}
		}

		if (accumulator >= fixedStep)
		{
			accumulator -= fixedStep;
			globalTime += fixedStep;

			float directionZ = 0;
			if (keys[SDL_SCANCODE_W])
			{
				directionZ += 1;
			}
			if (keys[SDL_SCANCODE_S])
			{
				directionZ += -1;
			}

			float directionX = 0;
			if (keys[SDL_SCANCODE_A])
			{
				directionX += -1;
			}
			if (keys[SDL_SCANCODE_D])
			{
				directionX += 1;
			}
			carX += directionX * 30.0f * fixedStep;

			// will use this to accel slower at high speeds
			// and draw things more densely at high speeds later
			// to correct for high speed optical illusion
			const float speedFactor = velocity / maxSpeed;
			if (directionZ)
			{
				float accelSpeed = acceleration * (1.0f - speedFactor); // acceleration changes as speed increases

				// accelerating
				velocity += directionZ * accelSpeed * fixedStep;
				if (velocity > maxSpeed)
				{
					velocity = maxSpeed;
				}
			}
			else
			{
				// decelerating
				const float deceleration = -2.0f;
				velocity += deceleration * fixedStep;
				if (velocity < 0.01f)
				{
					velocity = 0;
				}
			}
			camZ += velocity * fixedStep;

			// render screen to ourun sky color
			SDL_SetRenderDrawColor(renderer, 1, 146, 250, 255);
			SDL_RenderClear(renderer);

			float horizon = logH - 80;
			float roadWidth = 250.0f;

			SDL_FRect skylineSrc
			{
				.x = camZ,
				.y = 0,
				.w = logW,
				.h = logH
			};

			SDL_FRect skylineDst{
				.x = 0,
				.y = horizon - skylineTex->h + 1,
				.w = logW,
				.h = logH
			};
			//SDL_RenderTexture(renderer, skylineTex, &skylineSrc, &skylineDst);

			struct Object
			{
				float x;
				float y;
				float scale;
			};
			std::vector<Object> objects;

			int plantInterval = 0;

			for (int y = logH - 1; y > horizon; --y)
			{
				float scanlineZ = camY / (y - horizon); // scanlint -> Z distance
				//if (scanlineZ > 9.5f) continue;

				const float worldZ = camZ + scanlineZ;
				const float zMeters = worldZ / metersPerUnit;
				const unsigned int segmentIndex = static_cast<unsigned int>(zMeters / metersPerSegment);

				RoadSegment &seg = segments[segmentIndex];
				//const float camElevationDiff = camY - seg.elevation;
				//const float camElevationDiff = camY - sin(worldZ / 2);
				const float camElevationDiff = camY - 0;

				const float z = camElevationDiff / (y - horizon); // elevation adjusted z
				float scale = 1.0f / z; // needs to be inverted in order to be used as scale
				float xWidth = roadWidth * scale;
				const float roadMiddle = logW / 2;

				// draw ground
				if (static_cast<int>(zMeters / 0.5f) % 2)
				{
					SDL_SetRenderDrawColor(renderer, 234, 219, 202, 255);
				}
				else
				{
					SDL_SetRenderDrawColor(renderer, 226, 210, 195, 255);
				}
				SDL_RenderLine(renderer, 0, y, logW, y);

				// draw road
				if (static_cast<int>(zMeters / 0.5f) % 2)
				{
					SDL_SetRenderDrawColor(renderer, 154, 155, 154, 255);
				}
				else
				{
					SDL_SetRenderDrawColor(renderer, 146, 146, 147, 255);
				}
				SDL_RenderLine(renderer, roadMiddle - xWidth / 2, y, roadMiddle + xWidth / 2, y);

				// middle strip
				if (static_cast<int>(worldZ / 0.5f) % 2)
				{
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

					const float stripeWidth = 3.0f * scale;
					const float stripeStartL = roadMiddle - stripeWidth - 20.0f * scale;
					const float stripeStartR = roadMiddle + 20.0f * scale;
					SDL_RenderLine(renderer, stripeStartL, y, stripeStartL + stripeWidth, y);
					SDL_RenderLine(renderer, stripeStartR, y, stripeStartR + stripeWidth, y);
				}

				// side strips
				const float stripeWidth = 15.0f * scale;
				float leftStart = roadMiddle - xWidth / 2;
				float rightStart = roadMiddle + xWidth / 2 - stripeWidth;

				if (static_cast<int>(worldZ / 0.7f) % 2)
				{
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					SDL_RenderLine(renderer, leftStart, y, leftStart + stripeWidth, y);
					SDL_RenderLine(renderer, rightStart, y, rightStart + stripeWidth, y);
				}
				else
				{
					SDL_SetRenderDrawColor(renderer, 170, 0, 0, 255);
					SDL_RenderLine(renderer, leftStart, y, leftStart + stripeWidth, y);
					SDL_RenderLine(renderer, rightStart, y, rightStart + stripeWidth, y);
				}

				// plants
				int interval = segmentIndex;
				if (interval != plantInterval)
				{
					plantInterval = interval;
					float objY = static_cast<float>(y) - 16 * scale;
					objects.push_back(
						Object{
							.x = roadMiddle - xWidth / 2 - 16 * scale,
							.y = static_cast<float>(y) - 16 * scale,
							.scale = scale
						}
					);

					objects.push_back(
						Object{
							.x = roadMiddle + xWidth / 2 + 16 * scale,
							.y = static_cast<float>(y) - 16 * scale,
							.scale = scale
						}
					);
				}
			}

			for (Object &obj : objects)
			{
				SDL_FRect src{
					.x = 0, .y = 0,
					.w = static_cast<float>(plant1Tex->w),
					.h = static_cast<float>(plant1Tex->h)
				};
				SDL_FRect dst{
					.x = obj.x,
					.y = obj.y,
					.w = plant1Tex->w * obj.scale,
					.h = plant1Tex->h * obj.scale
				};
				SDL_RenderTexture(renderer, plant1Tex, &src, &dst);
			}

			SDL_FRect dst{
				.x = logW / 2 - carForwardTex->w / 2 + carX,
				.y = logH - 80 + sinf(globalTime * 60) * 0.2f,
				.w = carForwardTex->w * 1.0f,
				.h = carForwardTex->h * 1.0f
			};
			SDL_RenderTexture(renderer, carForwardTex, nullptr, &dst);

			int kmPerHour = (velocity * 3600) / (1000 / metersPerUnit) * speedMultFactor;
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			char buffer[255];
			sprintf(buffer, "S: %d, Z: %.2f", kmPerHour, camZ);
			SDL_RenderDebugText(renderer, 5, 5, buffer);

			SDL_RenderPresent(renderer);

		}
	}

	cleanup();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
