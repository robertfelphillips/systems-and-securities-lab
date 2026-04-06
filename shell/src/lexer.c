#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    char *buf = (char *)malloc(strlen(input) + 1);
    strcpy(buf, input);

    tokenlist *tokens = new_tokenlist();

    char *tok = strtok(buf, " ");
    while (tok != NULL) {
        add_token(tokens, tok);
        tok = strtok(NULL, " ");
    }

    free(buf);
    return tokens;
}

void free_tokens(tokenlist *tokens) {
    for (size_t i = 0; i < tokens->size; i++) {
        free(tokens->items[i]);
    }
    free(tokens->items);
    free(tokens);
}