#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sorting_algos.h"

TTF_Font *font = NULL;

typedef struct {
    SDL_Rect rect;
    const char *label;
    Algorithm algo;
} Button;

typedef struct {
    const char *complexity;
    const char *message;
} AlgoInfo;

static const AlgoInfo algoInfo[] = {
    {"Time: O(n^2)", "Bubble sort compares adjacent pairs and keeps swapping."},
    {"Time: O(n^2)", "Insertion sort builds a sorted prefix by shifting larger values."},
    {"Time: O(n^2)", "Selection sort repeatedly selects the smallest remaining element."},
    {"Time: O(n log n)", "Merge sort divides the list and merges sorted halves."},
    {"Time: O(n log n)", "Quick sort partitions around a pivot for fast divides."}
};

void drawText(SDL_Renderer *ren, const char *text, int x, int y, SDL_Color color) {
    if (!font) return;
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect rect = { x, y, surf->w, surf->h };
    SDL_RenderCopy(ren, tex, NULL, &rect);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void drawArray(SDL_Renderer *ren, SortState *state, int h1, int h2) {
    const int topPadding = 180;
    const int bottomPadding = 40;
    int barWidth = state->windowWidth / state->size;
    int drawableHeight = state->windowHeight - (topPadding + bottomPadding);
    if (drawableHeight < 50) drawableHeight = 50;

    for (int i = 0; i < state->size; i++) {
        // Highlight logic with better contrast
        if (i == h1 || i == h2) {
            // vivid orange for active comparison
            SDL_SetRenderDrawColor(ren, 255, 92, 92, 255);
        } 
        else {
            // darker blue-green for normal bars
            SDL_SetRenderDrawColor(ren, 0, 200, 150, 255);
        }

        int barHeight = state->arr[i] * drawableHeight / state->size;
        SDL_Rect rect = { (int)state->pos[i], state->windowHeight - barHeight - bottomPadding, barWidth - 2, barHeight };
        SDL_RenderFillRect(ren, &rect);

        // Optional: highlight outline for active bars
        if (i == h1 || i == h2) {
            SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
            SDL_RenderDrawRect(ren, &rect);
        }
    }
}

void resetMetrics(SortState *state) {
    state->comparisons = 0;
    state->swaps = 0;
}

void reshuffleArray(SortState *state) {
    for (int i = 0; i < state->size; i++) {
        state->arr[i] = rand() % state->size + 1;
        state->pos[i] = i * (state->windowWidth / state->size);
    }
    resetMetrics(state);
    state->sortingStarted = 0;
}

void resetQuickState(QuickState *q) {
    memset(q, 0, sizeof(*q));
    q->top = -1;
    q->highlightPivot = -1;
    q->highlightCurrent = -1;
}

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));
    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        printf("TTF Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Sorting Visualizer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1000, 700, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 24);
    if (!font) {
        printf("Font error: %s\n", TTF_GetError());
        return 1;
    }
    TTF_SetFontHinting(font, TTF_HINTING_LIGHT);

    SortState state = {0};
    state.size = MAX_SIZE;
    state.windowWidth = 1000;
    state.windowHeight = 700;
    state.sortingStarted = 0;
    state.algo = BUBBLE;
    reshuffleArray(&state);

    // Algorithm buttons
    Button buttons[5] = {
        {{120, 15, 120, 40}, "Bubble", BUBBLE},
        {{250, 15, 120, 40}, "Insertion", INSERTION},
        {{380, 15, 120, 40}, "Selection", SELECTION},
        {{510, 15, 120, 40}, "Merge", MERGE},
        {{640, 15, 120, 40}, "Quick", QUICK},
    };

    // Start and Reset buttons
    SDL_Rect startBtn = {780, 15, 100, 40};
    SDL_Rect resetBtn = {890, 15, 100, 40};

    int delay = 50;
    int running = 1;
    SDL_Event e;

    int bubble_i = 0, bubble_j = 0;
    int insertion_i = 1, insertion_j = 0;
    int selection_i = 0, selection_j = 1, selection_min = 0;
    MergeState merge = {0, 0, 0, 0};
    QuickState quick;
    resetQuickState(&quick);

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x = e.button.x, y = e.button.y;

                // Algorithm buttons
                for (int i = 0; i < 5; i++) {
                    if (x >= buttons[i].rect.x && x <= buttons[i].rect.x + buttons[i].rect.w &&
                        y >= buttons[i].rect.y && y <= buttons[i].rect.y + buttons[i].rect.h) {
                        state.algo = buttons[i].algo;
                        state.sortingStarted = 0;
                        bubble_i = bubble_j = 0;
                        insertion_i = 1; insertion_j = 0;
                        selection_i = 0; selection_j = 1; selection_min = 0;
                        merge = (MergeState){0, 0, 0, 0};
                        resetQuickState(&quick);
                        resetMetrics(&state);
                    }
                }

                // Start button
                if (x >= startBtn.x && x <= startBtn.x + startBtn.w &&
                    y >= startBtn.y && y <= startBtn.y + startBtn.h)
                    state.sortingStarted = !state.sortingStarted;

                // Reset button
                if (x >= resetBtn.x && x <= resetBtn.x + resetBtn.w &&
                    y >= resetBtn.y && y <= resetBtn.y + resetBtn.h) {
                    reshuffleArray(&state);
                    bubble_i = bubble_j = 0;
                    insertion_i = 1; insertion_j = 0;
                    selection_i = 0; selection_j = 1; selection_min = 0;
                    merge = (MergeState){0, 0, 0, 0};
                    resetQuickState(&quick);
                }
            }

            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                state.windowWidth = e.window.data1;
                state.windowHeight = e.window.data2;
                for (int i = 0; i < state.size; i++) state.pos[i] = i * (state.windowWidth / state.size);
            }

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_UP && delay > 5) delay -= 5;
                if (e.key.keysym.sym == SDLK_DOWN && delay < 500) delay += 5;
                if ((e.key.keysym.sym == SDLK_EQUALS || e.key.keysym.sym == SDLK_PLUS || e.key.keysym.sym == SDLK_KP_PLUS) && delay > 5) delay -= 5;
                if ((e.key.keysym.sym == SDLK_MINUS || e.key.keysym.sym == SDLK_KP_MINUS) && delay < 500) delay += 5;
            }
        }

        // Perform one step per frame
        if (state.sortingStarted) {
            if (state.algo == BUBBLE) bubbleStep(&state, &bubble_i, &bubble_j);
            else if (state.algo == INSERTION) insertionStep(&state, &insertion_i, &insertion_j);
            else if (state.algo == SELECTION) selectionStep(&state, &selection_i, &selection_j, &selection_min);
            else if (state.algo == MERGE) mergeStep(&state, &merge);
            else if (state.algo == QUICK) quickStep(&state, &quick);
        }

        // ================== RENDER EVERYTHING ==================
        SDL_SetRenderDrawColor(ren, 25, 25, 25, 255);
        SDL_RenderClear(ren);

        // Header background
        SDL_SetRenderDrawColor(ren, 50, 50, 60, 255);
        SDL_Rect header = {0, 0, state.windowWidth, 70};
        SDL_RenderFillRect(ren, &header);

        // Draw bars
        int h1 = -1, h2 = -1;
        if (state.algo == BUBBLE) {
             h1 = bubble_j;
            h2 = bubble_j + 1;
        }
        else if (state.algo == INSERTION) {
            h1 = insertion_j;
            h2 = insertion_j + 1;
        }
        else if (state.algo == SELECTION) {
            h1 = selection_min;
            h2 = selection_j;
        }
        else if (state.algo == MERGE && merge.merging) {
            h1 = merge.mergeLeft;
            h2 = merge.mergeLeft + merge.mergeSize - 1;
        }
        else if (state.algo == QUICK) {
            h1 = quick.highlightPivot;
            h2 = quick.highlightCurrent;
        }
        drawArray(ren, &state, h1, h2);


        SDL_Color btnText = {255, 255, 255, 255};
        SDL_Color infoText = {200, 200, 200, 255};

        // Algorithm buttons
        for (int i = 0; i < 5; i++) {
            SDL_SetRenderDrawColor(ren,
                state.algo == buttons[i].algo ? 0 : 100,
                state.algo == buttons[i].algo ? 180 : 100,
                state.algo == buttons[i].algo ? 255 : 100, 255);
            SDL_RenderFillRect(ren, &buttons[i].rect);
            drawText(ren, buttons[i].label, buttons[i].rect.x + 10, buttons[i].rect.y + 8, btnText);
        }

        // Start/Pause button
        SDL_SetRenderDrawColor(ren, state.sortingStarted ? 0 : 0, state.sortingStarted ? 200 : 200, 0, 255);
        SDL_RenderFillRect(ren, &startBtn);
        drawText(ren, state.sortingStarted ? "Pause" : "Start", startBtn.x + 15, startBtn.y + 8, btnText);

        // Reset button
        SDL_SetRenderDrawColor(ren, 180, 50, 50, 255);
        SDL_RenderFillRect(ren, &resetBtn);
        drawText(ren, "Reset", resetBtn.x + 20, resetBtn.y + 8, btnText);

        // Speed text
        char speedStr[40];
        sprintf(speedStr, "Speed: %dms", delay);
        drawText(ren, speedStr, state.windowWidth - 150, 25, btnText);

        // Complexity and message
        drawText(ren, algoInfo[state.algo].complexity, 20, 80, infoText);
        drawText(ren, algoInfo[state.algo].message, 20, 110, infoText);
        drawText(ren, "Use +/- or =/- keys to adjust speed", 20, 140, infoText);

        // Stats
        char statsStr[80];
        sprintf(statsStr, "Swaps: %d   Comparisons: %d", state.swaps, state.comparisons);
        drawText(ren, statsStr, 20, state.windowHeight - 50, btnText);

        SDL_RenderPresent(ren);
        SDL_Delay(delay);
    }

    // Cleanup
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
