#ifndef LEXER_H_
#define LEXER_H_

#include <stddef.h>
#include "./la.h"
#include "./free_glyph.h"
#include "./common.h"

typedef enum {
    TOKEN_END = 0,
    TOKEN_INVALID,
    TOKEN_PREPROC,
    TOKEN_SYMBOL,
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_OPEN_CURLY,
    TOKEN_CLOSE_CURLY,
    TOKEN_SEMICOLON,
    TOKEN_KEYWORD,
    TOKEN_COMMENT,
    TOKEN_STRING,
    TOKEN_COLOR,
    TOKEN_EQUALS,
    TOKEN_NOT_EQUALS,
    TOKEN_EQUALS_EQUALS,
    TOKEN_EXCLAMATION,
    TOKEN_LESS_THAN,
    TOKEN_GREATER_THAN,
    TOKEN_ARROW,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_TRUE,
    TOKEN_FALSE,
  } Token_Kind;

const char *token_kind_name(Token_Kind kind);

typedef struct {
    Token_Kind kind;
    const char *text;
    size_t text_len;
    Vec2f position;
    int nesting_level; // <-- New attribute
} Token;

typedef struct {
    Free_Glyph_Atlas *atlas;
    const char *content;
    size_t content_len;
    size_t cursor;
    size_t line;
    size_t bol;
    float x;
    String_Builder file_path;
} Lexer;

/* Lexer lexer_new(Free_Glyph_Atlas *atlas, const char *content, size_t content_len); */
Lexer lexer_new(Free_Glyph_Atlas *atlas, const char *content, size_t content_len, String_Builder file_path);
Token lexer_next(Lexer *l);

#endif // LEXER_H_
