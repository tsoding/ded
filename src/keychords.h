#pragma once

#include <SDL2/SDL.h>
#include "editor.h"

typedef void (*ActionFunction)();

typedef enum {
    ACTION_BOOL_SET,
    ACTION_FUNC
} ActionType;

#define MAX_SEQUENCE_LENGTH 10

typedef struct {
    SDL_Keycode keys[MAX_SEQUENCE_LENGTH];
    int length;
} KeySequence;

typedef struct {
    KeySequence sequence;
    EvilMode mode;
    ActionType type;
    union {
        ActionFunction func;
        struct {
            bool *var;
            bool value;
        } boolSet;
    } action;
} Keychord;

#define SEQ(...) { .keys = {__VA_ARGS__}, .length = sizeof((SDL_Keycode[]){__VA_ARGS__}) / sizeof(SDL_Keycode) }

extern Keychord keychords[];  // Declaration of keychords array without definition

bool check_for_keychord(EvilMode mode, SDL_Event *event, KeySequence *currentSequence, Editor *editor);
void capture_sequence(SDL_Event *event, KeySequence *currentSequence);
void clear_sequence(KeySequence *currentSequence);
bool sequence_matches(const KeySequence *seq1, const KeySequence *seq2);
