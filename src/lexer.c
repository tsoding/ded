#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "lexer.h"

typedef struct {
    Token_Kind kind;
    const char *text;
} Literal_Token;


Literal_Token literal_tokens[] = {
    {.text = "(", .kind = TOKEN_OPEN_PAREN},
    {.text = ")", .kind = TOKEN_CLOSE_PAREN},
    {.text = "{", .kind = TOKEN_OPEN_CURLY},
    {.text = "}", .kind = TOKEN_CLOSE_CURLY},
    {.text = ";", .kind = TOKEN_SEMICOLON},
    {.text = "=",  .kind = TOKEN_EQUALS},
    {.text = "!=", .kind = TOKEN_NOT_EQUALS},
    {.text = "==", .kind = TOKEN_EQUALS_EQUALS},
    {.text = "!",  .kind = TOKEN_EXCLAMATION},
};
#define literal_tokens_count (sizeof(literal_tokens)/sizeof(literal_tokens[0]))

const char *cKeywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
    "else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register",
    "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef",
    "union", "unsigned", "void", "volatile", "while", "alignas", "alignof", "and",
    "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "bitand",
    "bitor", "bool", "catch", "char16_t", "char32_t", "char8_t", "class", "co_await",
    "co_return", "co_yield", "compl", "concept", "const_cast", "consteval", "constexpr",
    "constinit", "decltype", "delete", "dynamic_cast", "explicit", "export", "false",
    "friend", "inline", "mutable", "namespace", "new", "noexcept", "not", "not_eq",
    "nullptr", "operator", "or", "or_eq", "private", "protected", "public", "reflexpr",
    "reinterpret_cast", "requires", "static_assert", "static_cast", "synchronized",
    "template", "this", "thread_local", "throw", "true", "try", "typeid", "typename",
    "using", "virtual", "wchar_t", "xor", "xor_eq",
};

/* #define keywords_count (sizeof(keywords)/sizeof(keywords[0])) */
#define cKeywords_count (sizeof(cKeywords)/sizeof(cKeywords[0]))


const char *token_kind_name(Token_Kind kind)
{
    switch (kind) {
    case TOKEN_END:
        return "end of content";
    case TOKEN_INVALID:
        return "invalid token";
    case TOKEN_PREPROC:
        return "preprocessor directive";
    case TOKEN_SYMBOL:
        return "symbol";
    case TOKEN_OPEN_PAREN:
        return "open paren";
    case TOKEN_CLOSE_PAREN:
        return "close paren";
    case TOKEN_OPEN_CURLY:
        return "open curly";
    case TOKEN_COLOR:
        return "color";
    case TOKEN_CLOSE_CURLY:
        return "close curly";
    case TOKEN_SEMICOLON:
        return "semicolon";
    case TOKEN_KEYWORD:
        return "keyword";
    case TOKEN_EQUALS:
        return "=";
    case TOKEN_NOT_EQUALS:
        return "!=";
    case TOKEN_EQUALS_EQUALS:
        return "==";
    case TOKEN_EXCLAMATION:
        return "!";
    case TOKEN_ARROW:
        return "->";
    case TOKEN_MINUS:
        return "-";
    case TOKEN_PLUS:
        return "+";
    case TOKEN_TRUE:
        return "true";
    case TOKEN_FALSE:
        return "false";
    case TOKEN_ARRAY_CONTENT:
        return "array_content";
    case TOKEN_OPEN_SQUARE:
        return "open_square";
    case TOKEN_CLOSE_SQUARE:
        return "close_square";
    default:
        UNREACHABLE("token_kind_name");
    }
    return NULL;
}

Lexer lexer_new(Free_Glyph_Atlas *atlas, const char *content, size_t content_len)
{
    Lexer l = {0};
    l.atlas = atlas;
    l.content = content;
    l.content_len = content_len;
    return l;
}

bool lexer_starts_with(Lexer *l, const char *prefix)
{
    size_t prefix_len = strlen(prefix);
    if (prefix_len == 0) {
        return true;
    }
    if (l->cursor + prefix_len - 1 >= l->content_len) {
        return false;
    }
    for (size_t i = 0; i < prefix_len; ++i) {
        if (prefix[i] != l->content[l->cursor + i]) {
            return false;
        }
    }
    return true;
}

void lexer_chop_char(Lexer *l, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        // TODO: get rid of this assert by checking the length of the choped prefix upfront
        assert(l->cursor < l->content_len);
        char x = l->content[l->cursor];
        l->cursor += 1;
        if (x == '\n') {
            l->line += 1;
            l->bol = l->cursor;
            l->x = 0;
        } else {
            if (l->atlas) {
                size_t glyph_index = x;
                // TODO: support for glyphs outside of ASCII range
                if (glyph_index >= GLYPH_METRICS_CAPACITY) {
                    glyph_index = '?';
                }
                Glyph_Metric metric = l->atlas->metrics[glyph_index];
                l->x += metric.ax;
            }
        }
    }
}

void lexer_trim_left(Lexer *l)
{
    while (l->cursor < l->content_len && isspace(l->content[l->cursor])) {
        lexer_chop_char(l, 1);
    }
}

bool is_symbol_start(char x)
{
    return isalpha(x) || x == '_';
}

bool is_symbol(char x)
{
    return isalnum(x) || x == '_';
}

Token lexer_next(Lexer *l)
{
    lexer_trim_left(l);

    Token token = {
        .text = &l->content[l->cursor],
    };

    token.position.x = l->x;
    token.position.y = -(float)l->line * FREE_GLYPH_FONT_SIZE;

    if (l->cursor >= l->content_len) return token;

    // Check for specific operators (e.g., "=", "!", "!=", "==", "<", ">", "<=", ">=")
    if (l->cursor < l->content_len) {
        char current_char = l->content[l->cursor];
        char next_char = (l->cursor + 1 < l->content_len) ? l->content[l->cursor + 1] : '\0';

        switch (current_char) {
        case '=':
            if (next_char == '=') {
                token.kind = TOKEN_EQUALS_EQUALS;
                token.text_len = 2;
                lexer_chop_char(l, 2);
            } else {
                token.kind = TOKEN_EQUALS;
                token.text_len = 1;
                lexer_chop_char(l, 1);
            }
            return token;

        case '!':
            if (next_char == '=') {
                token.kind = TOKEN_NOT_EQUALS;
                token.text_len = 2;
                lexer_chop_char(l, 2);
            } else {
                token.kind = TOKEN_EXCLAMATION;
                token.text_len = 1;
                lexer_chop_char(l, 1);
            }
            return token;

        case '<':
            token.kind = TOKEN_LESS_THAN;
            token.text_len = 1;
            lexer_chop_char(l, 1);
            return token;

        case '>':
            token.kind = TOKEN_GREATER_THAN;
            token.text_len = 1;
            lexer_chop_char(l, 1);
            return token;

        case '-':
            if (next_char == '>') {
                token.kind = TOKEN_ARROW;
                token.text_len = 2;
                lexer_chop_char(l, 2);
            } else {
                token.kind = TOKEN_MINUS;
                token.text_len = 1;
                lexer_chop_char(l, 1);
            }
            return token;

        case '+':
            token.kind = TOKEN_PLUS;
            token.text_len = 1;
            lexer_chop_char(l, 1);
            return token;

        }
    }

    // Check for arrays
    if (l->cursor < l->content_len) {
        char current_char = l->content[l->cursor];

        // If the current character is the start of an array
        if (current_char == '[') {
            token.kind = TOKEN_OPEN_SQUARE;
            token.text_len = 1;
            lexer_chop_char(l, 1);
            l->in_array = true;  // Set the flag indicating we are inside an array
            return token;
        }
        else if (current_char == ']' && l->in_array) {
            token.kind = TOKEN_CLOSE_SQUARE;
            token.text_len = 1;
            lexer_chop_char(l, 1);
            l->in_array = false;  // Reset the flag indicating we are no longer inside an array
            return token;
        }
    }

    // Check for array content, but only if we are inside an array
    if (l->in_array && l->cursor < l->content_len) {
        size_t potential_length = 0;

        while (l->cursor + potential_length < l->content_len && l->content[l->cursor + potential_length] != ']') {
            potential_length++;
        }

        // If potential array content was detected and not empty
        if (potential_length > 0) {
            token.kind = TOKEN_ARRAY_CONTENT;
            token.text_len = potential_length;
            lexer_chop_char(l, potential_length);
            return token;
        }
    }

    // Check for boolean literals "true" and "false"
    if ((l->cursor + 3 < l->content_len) &&
        (strncmp(&l->content[l->cursor], "true", 4) == 0) &&
        ((l->cursor + 4 == l->content_len) || !isalnum(l->content[l->cursor + 4]))) {

        lexer_chop_char(l, 4); // Skip the entire "true" token
        token.kind = TOKEN_TRUE;
        token.text_len = 4;
        return token;

    } else if ((l->cursor + 4 < l->content_len) &&
               (strncmp(&l->content[l->cursor], "false", 5) == 0) &&
               ((l->cursor + 5 == l->content_len) || !isalnum(l->content[l->cursor + 5]))) {

        lexer_chop_char(l, 5); // Skip the entire "false" token
        token.kind = TOKEN_FALSE;
        token.text_len = 5;
        return token;
    }


    // Check for color-like format (e.g., 0xf38ba8FF)
    if (l->content[l->cursor] == '0' &&
        (l->cursor + 1 < l->content_len) &&
        (l->content[l->cursor + 1] == 'x' || l->content[l->cursor + 1] == 'X')) {

        size_t start_cursor = l->cursor;
        size_t potential_length = 0;

        // Count the potential hex digits
        while ((start_cursor + 2 + potential_length) < l->content_len && isxdigit(l->content[start_cursor + 2 + potential_length])) {
            potential_length++;
        }

        // Check if the length is 8, meaning it's a full color
        if (potential_length == 8) {
            lexer_chop_char(l, 10); // Skip the entire color token including '0x'
            token.kind = TOKEN_COLOR;
            token.text_len = 10; // Including the '0x' prefix
            return token;
        }
    }

    if (l->content[l->cursor] == '"') {
        // TODO: TOKEN_STRING should also handle escape sequences
        token.kind = TOKEN_STRING;
        lexer_chop_char(l, 1);
        while (l->cursor < l->content_len && l->content[l->cursor] != '"' && l->content[l->cursor] != '\n') {
            lexer_chop_char(l, 1);
        }
        if (l->cursor < l->content_len) {
            lexer_chop_char(l, 1);
        }
        token.text_len = &l->content[l->cursor] - token.text;
        return token;
    }

    if (l->content[l->cursor] == '#') {
        if (l->cursor + 6 < l->content_len && is_hex_digit(l->content[l->cursor + 1])
            && is_hex_digit(l->content[l->cursor + 2])
            && is_hex_digit(l->content[l->cursor + 3])
            && is_hex_digit(l->content[l->cursor + 4])
            && is_hex_digit(l->content[l->cursor + 5])
            && is_hex_digit(l->content[l->cursor + 6])) {
            token.kind = TOKEN_PREPROC;
            lexer_chop_char(l, 7); // Chop # and the 6 characters
            token.text_len = &l->content[l->cursor] - token.text;
            return token;
        } else {
            // Your existing handling for # as a preprocessor directive
            token.kind = TOKEN_PREPROC;
            while (l->cursor < l->content_len && l->content[l->cursor] != '\n') {
                lexer_chop_char(l, 1);
            }
            if (l->cursor < l->content_len) {
                lexer_chop_char(l, 1);
            }
            token.text_len = &l->content[l->cursor] - token.text;
            return token;
        }
    }

    if (lexer_starts_with(l, "//")) {
        token.kind = TOKEN_COMMENT;
        while (l->cursor < l->content_len && l->content[l->cursor] != '\n') {
            lexer_chop_char(l, 1);
        }
        if (l->cursor < l->content_len) {
            lexer_chop_char(l, 1);
        }
        token.text_len = &l->content[l->cursor] - token.text;
        return token;
    }

     for (size_t i = 0; i < literal_tokens_count; ++i) {
        if (lexer_starts_with(l, literal_tokens[i].text)) {
            // NOTE: this code assumes that there is no newlines in literal_tokens[i].text
            size_t text_len = strlen(literal_tokens[i].text);
            token.kind = literal_tokens[i].kind;
            token.text_len = text_len;
            lexer_chop_char(l, text_len);
            return token;
        }
    }

    if (is_symbol_start(l->content[l->cursor])) {
        token.kind = TOKEN_SYMBOL;
        while (l->cursor < l->content_len && is_symbol(l->content[l->cursor])) {
            lexer_chop_char(l, 1);
            token.text_len += 1;
        }

        for (size_t i = 0; i < cKeywords_count; ++i) {
            size_t keyword_len = strlen(cKeywords[i]);
            if (keyword_len == token.text_len && memcmp(cKeywords[i], token.text, keyword_len) == 0) {
                token.kind = TOKEN_KEYWORD;
                break;
            }
        }

        return token;
    }

    lexer_chop_char(l, 1);
    token.kind = TOKEN_INVALID;
    token.text_len = 1;
    return token;
}
