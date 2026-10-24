/* Development-only probe of the exact companion libraries and reference bytes.
 * This executable is not installed with the game or used by production code. */
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>

static int resource_path(char *output, size_t capacity, const char *root,
                         const char *relative) {
    int size = SDL_snprintf(output, capacity, "%s/%s", root, relative);
    return size >= 0 && (size_t)size < capacity;
}

int main(int argc, char **argv) {
    const char *names[] = {
        "spaceShip/stitchedFiles/spaceships.png",
        "spaceShip/stitchedFiles/projectiles.png",
        "background1.png", "background2.png", "background3.png", "background4.png",
        "healthBar.png", "health.png"
    };
    const int widths[] = {640, 1500, 500, 500, 500, 500, 128, 128};
    const int heights[] = {256, 1000, 500, 500, 500, 500, 16, 16};
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *font = NULL;
    Mix_Chunk *shot = NULL;
    char path[4096];
    int exit_code = 1;
    int index;

    if (argc != 2) {
        fprintf(stderr, "Usage: starfall_resource_probe <reference-resource-root>\n");
        return 2;
    }
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) goto failed;
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) goto failed;
    if (TTF_Init() < 0) goto failed;
    if ((Mix_Init(MIX_INIT_FLAC) & MIX_INIT_FLAC) == 0) goto failed;

    window = SDL_CreateWindow("Resource probe", 0, 0, 128, 128, SDL_WINDOW_HIDDEN);
    if (!window) goto failed;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) goto failed;

    for (index = 0; index < 8; ++index) {
        SDL_Texture *texture;
        int width = 0, height = 0;
        if (!resource_path(path, sizeof(path), argv[1], names[index])) goto failed;
        texture = IMG_LoadTexture(renderer, path);
        if (!texture) goto failed;
        if (SDL_QueryTexture(texture, NULL, NULL, &width, &height) < 0 ||
            width != widths[index] || height != heights[index]) {
            SDL_DestroyTexture(texture);
            SDL_SetError("Unexpected decoded size for %s", names[index]);
            goto failed;
        }
        printf("PNG texture: %s = %d x %d\n", names[index], width, height);
        SDL_DestroyTexture(texture);
    }

    if (!resource_path(path, sizeof(path), argv[1], "CommodorePixeled.ttf")) goto failed;
    font = TTF_OpenFont(path, 16);
    if (!font) goto failed;
    {
        SDL_Color ink = {226, 240, 252, 255};
        SDL_Surface *text = TTF_RenderUTF8_Blended(font, "PILOT 0123456789", ink);
        if (!text) goto failed;
        if (text->w <= 0 || text->h <= 0) {
            SDL_FreeSurface(text);
            SDL_SetError("Font did not produce readable text dimensions");
            goto failed;
        }
        printf("TrueType rendered: %s = %d x %d\n", TTF_FontFaceFamilyName(font), text->w, text->h);
        SDL_FreeSurface(text);
    }

    if (Mix_OpenAudio(48000, AUDIO_S16SYS, 2, 512) < 0) goto failed;
    if (!resource_path(path, sizeof(path), argv[1], "shoot.flac")) goto failed;
    shot = Mix_LoadWAV(path);
    if (!shot || shot->alen == 0) goto failed;
    if (Mix_PlayChannel(-1, shot, 0) < 0) goto failed;
    SDL_Delay(40);
    printf("FLAC decoded and mixed: %u PCM bytes\n", (unsigned)shot->alen);
    {
        const SDL_version *image = IMG_Linked_Version();
        const SDL_version *ttf = TTF_Linked_Version();
        const SDL_version *mixer = Mix_Linked_Version();
        printf("Versions: SDL_image %u.%u.%u; SDL_ttf %u.%u.%u; SDL_mixer %u.%u.%u\n",
               image->major, image->minor, image->patch,
               ttf->major, ttf->minor, ttf->patch,
               mixer->major, mixer->minor, mixer->patch);
    }
    printf("PASS: real PNG textures, TrueType rasterization, FLAC decoding and mixer playback\n");
    exit_code = 0;
    goto cleanup;

failed:
    fprintf(stderr, "Resource probe failed: %s\n", SDL_GetError());
cleanup:
    Mix_HaltChannel(-1);
    if (shot) Mix_FreeChunk(shot);
    Mix_CloseAudio();
    Mix_Quit();
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return exit_code;
}
