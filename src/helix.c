#include <stdlib.h>
#include <time.h>
#include "helix.h"
#include "editor.h"
#include "theme.h"

void helix_mode() {
    if (current_mode != HELIX) {
        current_mode = HELIX;
        switch_to_theme(&currentThemeIndex, 7);
        targetModelineHeight = 21.0f;
        targetMinibufferHeight = 0.0f;
    } else {
        current_mode = NORMAL;
        targetModelineHeight = 35.0f;
        targetMinibufferHeight = 21.0f;
        srand(time(NULL));

        int randomThemeIndex;
        do {
            randomThemeIndex = rand() % 8;
        } while (randomThemeIndex == 7); // Ensure the random theme is not Helix

        switch_to_theme(&currentThemeIndex, randomThemeIndex);
    }

    minibufferAnimationProgress = 0.0f;
    isModelineAnimating = true;
    isMinibufferAnimating = true;
}

