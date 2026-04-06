#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_shell_delimiter(char c) {
    return c == '|' || c == '<' || c == '>' || c == '&';
}

char *get_input(void) {
    char *buffer = NULL;
    int bufsize = 0;
    char chunk[64];
    int read_any = 0;

    while (fgets(chunk, sizeof(chunk), stdin) != NULL) {
        read_any = 1;

        char *newline = strchr(chunk, '\n');
        int addby = (newline != NULL) ? (int)(newline - chunk) : (int)strlen(chunk);

        char *newbuf = (char *)realloc(buffer, bufsize + addby + 1);
        if (!newbuf) {
            free(buffer);
            return NULL;
        }
        buffer = newbuf;

        memcpy(buffer + bufsize, chunk, addby);
        bufsize += addby;

        if (newline != NULL) break;
    }

    if (!read_any) {
        free(buffer);
        return NULL;
    }

    buffer[bufsize] = '\0';
    return buffer;
}

tokenlist *new_tokenlist(void) {
    tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
    tokens->size = 0;
    tokens->items = (char **)malloc(sizeof(char *));
    tokens->items[0] = NULL;
    return tokens;
}

void add_token(tokenlist *tokens, char *item) {
    size_t i = tokens->size;

    tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
    tokens->items[i] = (char *)malloc(strlen(item) + 1);
    tokens->items[i + 1] = NULL;

    strcpy(tokens->items[i], item);
    tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
    tokenlist *tokens = new_tokenlist();
    size_t i = 0;

    while (input[i] != '\0') {
        while (input[i] == ' ' || input[i] == '\t') {
            i++;
        }

        if (input[i] == '\0') {
            break;
        }

        if (is_shell_delimiter(input[i])) {
            char op[2] = {input[i], '\0'};
            add_token(tokens, op);
            i++;
            continue;
        }

        size_t start = i;
        while (input[i] != '\0' &&
               input[i] != ' ' &&
               input[i] != '\t' &&
               !is_shell_delimiter(input[i])) {
            i++;
        }

        size_t length = i - start;
        char *token = (char *)malloc(length + 1);
        if (!token) {
            free_tokens(tokens);
            return NULL;
        }

        memcpy(token, input + start, length);
        token[length] = '\0';
        add_token(tokens, token);
        free(token);
    }
    return tokens;
}

void free_tokens(tokenlist *tokens) {
    for (size_t i = 0; i < tokens->size; i++) {
        free(tokens->items[i]);
    }
    free(tokens->items);
    free(tokens);
}
