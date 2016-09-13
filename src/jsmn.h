#ifndef __JSMN_H_
#define __JSMN_H_

#import <CoreFoundation/CoreFoundation.h>

#define JSMN_PARENT_LINKS
#define JSMN_POSITION_INSIDE_PARENT
#define JSMN_STRICT
#define JSMN_DISABLED_ITEMS

/**
 * JSON type identifier. Basic types are:
 *  o Object
 *  o Array
 *  o String
 *  o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
    JSMN_PRIMITIVE = 0,
    JSMN_OBJECT = 1,
    JSMN_ARRAY = 2,
    JSMN_STRING = 3
} jsmntype_t;

typedef enum {
    /* Not enough tokens were provided */
    JSMN_ERROR_NOMEM = -1,
    /* Invalid character inside JSON string */
    JSMN_ERROR_INVAL = -2,
    /* The string is not a full JSON packet, more bytes expected */
    JSMN_ERROR_PART = -3,
    /* Everything was fine */
    JSMN_SUCCESS = 0
} jsmnerr_t;

/**
 * JSON token description.
 * @param       type    type (object, array, string etc.)
 * @param       start   start position in JSON data string
 * @param       end     end position in JSON data string
 */
typedef struct {
    jsmntype_t type;
    int start;
    int end;
    int size;
#ifdef JSMN_PARENT_LINKS
    int parent;
#ifdef JSMN_POSITION_INSIDE_PARENT
    int posinparent;
#endif
#endif
#ifdef JSMN_DISABLED_ITEMS
    bool enabled;
    bool _in_comment;
#endif
} jsmntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct {
    unsigned int pos; /* offset in the JSON string */
    int toknext; /* next token to allocate */
    int toksuper; /* superior token node, e.g parent object or array */
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 * @param parser Target parser
 * @param buffer Input string buffer
 * @param tokens Tokens buffer
 * @param num_tokens Number of tokens allowed in the buffer
 * @return Error number, JSMN_SUCCESS for success
 */
jsmnerr_t jsmn_parse(jsmn_parser *parser, CFStringInlineBuffer* buf,
                     CFIndex len, jsmntok_t *tokens, unsigned int num_tokens);

#endif /* __JSMN_H_ */
