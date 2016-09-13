#include <stdlib.h>

#include "jsmn.h"

/**
 * Allocates a fresh unused token from the token pull.
 */
static jsmntok_t *jsmn_alloc_token(jsmn_parser *parser, jsmntok_t *tokens,
                                   size_t num_tokens) {
    jsmntok_t *tok;
    if (parser->toknext >= num_tokens) {
        return NULL;
    }
    tok = &tokens[parser->toknext++];
    tok->start = tok->end = -1;
    tok->size = 0;
#ifdef JSMN_PARENT_LINKS
    tok->parent = -1;
#ifdef JSMN_POSITION_INSIDE_PARENT
    tok->posinparent = -1;
#endif
#endif
#ifdef JSMN_DISABLED_ITEMS
    tok->enabled = true;
    tok->_in_comment = false;
#endif
    return tok;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(jsmntok_t *token, jsmntype_t type, int start,
                            int end) {
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static jsmnerr_t jsmn_parse_primitive(jsmn_parser *parser,
                                      CFStringInlineBuffer* buf,
                                      jsmntok_t *tokens, size_t num_tokens,
                                      long len,
#ifdef JSMN_DISABLED_ITEMS
    bool enabled
#endif
) {
    jsmntok_t *token;
    int start;

    start = parser->pos;

    for (; parser->pos < len; parser->pos++) {
        UniChar c = CFStringGetCharacterFromInlineBuffer(buf, parser->pos);
        switch (c) {
#ifndef JSMN_STRICT
            /* In strict mode primitive must be followed by "," or "}" or "]" */
            case ':':
#endif
            case '\t' : case '\r' : case '\n' : case ' ' :
            case ','  : case ']'  : case '}' :
            case 0xa0 /* non breaking-space */ :
#ifdef JSMN_DISABLED_ITEMS
            case '/' : case '*' :
#endif
                goto found;
        }
        if (c < 32 || c >= 127) {
            parser->pos = start;
            return JSMN_ERROR_INVAL;
        }
    }
#ifdef JSMN_STRICT
    /* In strict mode primitive must be followed by a comma/object/array */
    parser->pos = start;
    return JSMN_ERROR_PART;
#endif

found:
    token = jsmn_alloc_token(parser, tokens, num_tokens);
    if (token == NULL) {
        parser->pos = start;
        return JSMN_ERROR_NOMEM;
    }
    jsmn_fill_token(token, JSMN_PRIMITIVE, start, parser->pos);
#ifdef JSMN_PARENT_LINKS
    if (parser->toksuper != -1) {
        token->parent = parser->toksuper;
#ifdef JSMN_POSITION_INSIDE_PARENT
        token->posinparent = tokens[parser->toksuper].size;
#endif
    }
#endif
#ifdef JSMN_DISABLED_ITEMS
    token->enabled = enabled;
#endif
    parser->pos--;
    return JSMN_SUCCESS;
}

/**
 * Fills next token with JSON string.
 */
static jsmnerr_t jsmn_parse_string(jsmn_parser *parser,
                                   CFStringInlineBuffer* buf, jsmntok_t *tokens,
                                   size_t num_tokens, CFIndex len,
#ifdef JSMN_DISABLED_ITEMS
    bool enabled
#endif
) {
    jsmntok_t *token;

    int start = parser->pos;

    parser->pos++;

    /* Skip starting quote */
    for (; parser->pos < len; parser->pos++) {
        UniChar c = CFStringGetCharacterFromInlineBuffer(buf, parser->pos);

        /* Quote: end of string */
        if (c == '\"') {
            token = jsmn_alloc_token(parser, tokens, num_tokens);
            if (token == NULL) {
                parser->pos = start;
                return JSMN_ERROR_NOMEM;
            }
            jsmn_fill_token(token, JSMN_STRING, start+1, parser->pos);
#ifdef JSMN_PARENT_LINKS
            if (parser->toksuper != -1) {
                token->parent = parser->toksuper;
#ifdef JSMN_POSITION_INSIDE_PARENT
                token->posinparent = tokens[parser->toksuper].size;
#endif
            }
#endif
#ifdef JSMN_DISABLED_ITEMS
            token->enabled = enabled;
#endif
            return JSMN_SUCCESS;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\') {
            parser->pos++;
            switch (CFStringGetCharacterFromInlineBuffer(buf, parser->pos)) {
                /* Allowed escaped symbols */
                case '\"': case '/' : case '\\' : case 'b' :
                case 'f' : case 'r' : case 'n'  : case 't' :
                    break;
                /* Allows escaped symbol \uXXXX */
                case 'u':
                    /* TODO */
                    break;
                /* Unexpected symbol */
                default:
                    parser->pos = start;
                    return JSMN_ERROR_INVAL;
            }
        }
    }
    parser->pos = start;
    return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
jsmnerr_t jsmn_parse(jsmn_parser *parser, CFStringInlineBuffer* buf,
                     CFIndex len, jsmntok_t *tokens, unsigned int num_tokens) {
    jsmnerr_t r;
    int i;
    jsmntok_t *token;

    for (; parser->pos < len; parser->pos++) {
        UniChar c = CFStringGetCharacterFromInlineBuffer(buf, parser->pos);
        jsmntype_t type;

        switch (c) {
            case '{': case '[':
                token = jsmn_alloc_token(parser, tokens, num_tokens);
                if (token == NULL)
                    return JSMN_ERROR_NOMEM;
                if (parser->toksuper != -1) {
                    tokens[parser->toksuper].size++;
#ifdef JSMN_PARENT_LINKS
                    token->parent = parser->toksuper;
#ifdef JSMN_POSITION_INSIDE_PARENT
                    token->posinparent = tokens[parser->toksuper].size - 1;
#endif
#endif
#ifdef JSMN_DISABLED_ITEMS
                    token->enabled = !(tokens[parser->toksuper]._in_comment);
#endif
                }
                token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
                token->start = parser->pos;
                parser->toksuper = parser->toknext - 1;
                break;
            case '}': case ']':
                type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
                if (parser->toknext < 1) {
                    return JSMN_ERROR_INVAL;
                }
                token = &tokens[parser->toknext - 1];
                for (;;) {
                    if (token->start != -1 && token->end == -1) {
                        if (token->type != type) {
                            return JSMN_ERROR_INVAL;
                        }
                        token->end = parser->pos + 1;
                        parser->toksuper = token->parent;
                        break;
                    }
                    if (token->parent == -1) {
                        break;
                    }
                    token = &tokens[token->parent];
                }
#else
                for (i = parser->toknext - 1; i >= 0; i--) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        if (token->type != type) {
                            return JSMN_ERROR_INVAL;
                        }
                        parser->toksuper = -1;
                        token->end = parser->pos + 1;
                        break;
                    }
                }
                /* Error if unmatched closing bracket */
                if (i == -1) return JSMN_ERROR_INVAL;
                for (; i >= 0; i--) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        parser->toksuper = i;
                        break;
                    }
                }
#endif
                break;
            case '\"':
#ifdef JSMN_DISABLED_ITEMS
                r = jsmn_parse_string(parser, buf, tokens, num_tokens, len, parser->toksuper >= 0 ? !(tokens[parser->toksuper]._in_comment) : true);
#else
                r = jsmn_parse_string(parser, buf, tokens, num_tokens, len);
#endif
                if (r < 0) return r;
                if (parser->toksuper != -1)
                    tokens[parser->toksuper].size++;
                break;
            case '\t' : case '\r' : case '\n' : case ':' : case ',': case ' ':
            case 0xa0 /* non breaking-space */ :
                break;
#ifdef JSMN_DISABLED_ITEMS
            case '/':
                if (CFStringGetCharacterFromInlineBuffer(buf, parser->pos + 1) != '*') {
                    return JSMN_ERROR_INVAL;
                }
                if (parser->toksuper != -1)
                    tokens[parser->toksuper]._in_comment = true;
                parser->pos++;
                break;
            case '*':
                if (CFStringGetCharacterFromInlineBuffer(buf, parser->pos + 1) != '/') {
                    return JSMN_ERROR_INVAL;
                }
                if (parser->toksuper != -1)
                    tokens[parser->toksuper]._in_comment = false;
                parser->pos++;
                break;
#endif
#ifdef JSMN_STRICT
            /* In strict mode primitives are: numbers and booleans */
            case '-': case '0': case '1' : case '2': case '3' : case '4':
            case '5': case '6': case '7' : case '8': case '9':
            case 't': case 'f': case 'n' :
#else
            /* In non-strict mode every unquoted value is a primitive */
            default:
#endif
#ifdef JSMN_DISABLED_ITEMS
                r = jsmn_parse_primitive(parser, buf, tokens, num_tokens, len, parser->toksuper >= 0 ? !(tokens[parser->toksuper]._in_comment) : true);
#else
                r = jsmn_parse_primitive(parser, buf, tokens, num_tokens, len);
#endif
                if (r < 0) return r;
                if (parser->toksuper != -1)
                    tokens[parser->toksuper].size++;
                break;

#ifdef JSMN_STRICT
            /* Unexpected char in strict mode */
            default:
                return JSMN_ERROR_INVAL;
#endif

        }
    }

    for (i = parser->toknext - 1; i >= 0; i--) {
        /* Unmatched opened object or array */
        if (tokens[i].start != -1 && tokens[i].end == -1) {
            return JSMN_ERROR_PART;
        }
    }

    return JSMN_SUCCESS;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens 
 * available.
 */
void jsmn_init(jsmn_parser *parser) {
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
}
