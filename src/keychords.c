#include "keychords.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <string.h>

#include "file_browser.h"

bool file_browser = false;

Keychord keychords[] = {
    {SEQ(SDLK_SPACE, SDLK_d, SDLK_j), NORMAL, ACTION_BOOL_SET, .action.boolSet = {&file_browser, true}},
    // ... other sequences ...
};

void capture_sequence(SDL_Event *event, KeySequence *currentSequence) {
    if (currentSequence->length < MAX_SEQUENCE_LENGTH) {
        currentSequence->keys[currentSequence->length++] = event->key.keysym.sym;
    }
}

void clear_sequence(KeySequence *currentSequence) {
    currentSequence->length = 0;
    memset(currentSequence->keys, 0, sizeof(currentSequence->keys));
}

bool sequence_matches(const KeySequence *seq1, const KeySequence *seq2) {
    if (seq1->length != seq2->length) {
        return false;
    }
    for (int i = 0; i < seq1->length; i++) {
        if (seq1->keys[i] != seq2->keys[i]) {
            return false;
        }
    }
    return true;
}

bool check_for_keychord(EvilMode mode, SDL_Event *event, KeySequence *currentSequence, Editor *editor) {
    (void)editor;  // To silence the unused parameter warning, if editor is not used in this context
    capture_sequence(event, currentSequence);

    for (size_t i = 0; i < sizeof(keychords) / sizeof(Keychord); i++) {
        if (mode == keychords[i].mode && sequence_matches(&keychords[i].sequence, currentSequence)) {
            // Execute the action of the matching keychord
            switch (keychords[i].type) {
                case ACTION_FUNC:
                    keychords[i].action.func();
                    break;
                case ACTION_BOOL_SET:
                    *(keychords[i].action.boolSet.var) = keychords[i].action.boolSet.value;
                    break;
            }
            clear_sequence(currentSequence);
            return true;
        }
    }

    // If you've reached here, no sequence matched. But don't clear sequence unless it's of maximum length.
    if (currentSequence->length == MAX_SEQUENCE_LENGTH) {
        clear_sequence(currentSequence);
    }
    return false;
}
