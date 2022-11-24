#include "leptjson.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++;} while(0)
#define ISDIGIT(ch)     ((ch) >= '0' && (ch) <= '9')
#define IS1TO9(ch)      ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c){
    const char* p = c->json;
    while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r')
        p++;
    c->json = p;
}

/*
 *  replace lept_parse_null/true/false 
 */
static int lept_parse_literal(lept_context* c, lept_value* v, const char* expect_s, lept_type expect_t){
    int i = 0;
    EXPECT(c, expect_s[0]);
    for(i = 0; expect_s[i+1]; i++){
        if (c->json[i] != expect_s[i+1])
            return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += i;
    v->type = expect_t;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v){
    const char* p = c->json;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else{
        if (!IS1TO9(*p))    return LEPT_PARSE_INVALID_VALUE;
        p++;
        for(; ISDIGIT(*p); p++);
    }
    if (*p == '.'){
        p++;
        if(!ISDIGIT(*p))    return LEPT_PARSE_INVALID_VALUE;
        p++;
        for(; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E'){
        p++;
        if(*p == '+' || *p == '-') p++;
        if(!ISDIGIT(*p))    return LEPT_PARSE_INVALID_VALUE;
        p++;
        for(; ISDIGIT(*p); p++);
    }

    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}


static int lept_parse_value(lept_context* c, lept_value* v){
    switch (*c->json){
        case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
        case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
        default: return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}



int lept_parse(lept_value* v, const char* json){
    lept_context c;
    int t;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    t = lept_parse_value(&c, v);
    if (t == LEPT_PARSE_OK){
        lept_parse_whitespace(&c);
        if (*(c.json) != '\0'){
            t = LEPT_PARSE_ROOT_NOT_SINGULAR;
            v->type = LEPT_NULL;
        }
    }
    return t;
}

lept_type lept_get_type(const lept_value* v){
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v){
    assert(v != NULL);
    assert(v->type == LEPT_NUMBER);
    return v->n;
}