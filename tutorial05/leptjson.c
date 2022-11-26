#include "leptjson.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++;} while(0)
#define ISDIGIT(ch)     ((ch) >= '0' && (ch) <= '9')
#define IS1TO9(ch)      ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch) do { *(char*)lept_context_push(c, sizeof(char)) = (ch); } while(0)
#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)


typedef struct {
    const char* json;
    char* stack;
    size_t size, top;
}lept_context;

static int lept_parse_value(lept_context* c, lept_value* v);


static void* lept_context_push(lept_context* c, size_t size){
    void* ret;
    assert(size > 0);
    if (c->top + size >= c->size){
        if (c->size == 0)
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        while (c->top + size >= c->size)
            c->size += c->size >> 1;
            // c->size *= 1.5;
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

static void* lept_context_pop(lept_context* c, size_t size){
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}


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

    v->u.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}

static const char* lept_parse_hex4(const char* p, unsigned int* u){
    int i = 0;
    *u = 0;
    for(i = 0; i < 4; i++){
        char ch = *p++;
        *u <<= 4;
        if (ch >= '0' && ch <= '9') *u |= ch-'0';
        else if (ch >= 'a' && ch <= 'f') *u |= ch-'a'+10;
        else if (ch >= 'A' && ch <= 'F') *u |= ch-'A'+10;
        else return NULL;
    }
    return p;
}

static void lept_encode_utf8(lept_context* c, unsigned int u){
    if(u <= 0x007f)
        PUTC(c, u & 0xff);
    else if(u <= 0x07ff){
        // PUTC(c, ((u & 0xfc0)>>6)|0xc0); woshisb
        // PUTC(c, (u>>6)&0xff | 0xc0);
        PUTC(c, (u>>6) | 0xc0);
        PUTC(c, (u&0x3f) | 0x80);
    }else if(u <= 0xffff){
        PUTC(c, (u>>12) | 0xe0);
        PUTC(c, ((u>>6)&0x3f) | 0x80);
        PUTC(c, (u&0x3f) | 0x80);
    }else{
        assert(u <= 0x10ffff);
        PUTC(c, (u>>18) | 0xf0);
        PUTC(c, ((u>>12)&0x3f) | 0x80);
        PUTC(c, ((u>>6)&0x3f) | 0x80);
        PUTC(c, (u&0x3f) | 0x80);
    }
}


static int lept_parse_string(lept_context* c, lept_value* v){
    size_t head = c->top, len;
    unsigned int u1, u2;
    const char* p;
    EXPECT(c, '\"');
    p = c->json;
    for (;;){
        char ch = *p++;
        switch (ch){
            case '\"':
                len = c->top - head;
                lept_set_string(v, (const char*)lept_context_pop(c, len), len);
                c->json = p;
                return LEPT_PARSE_OK;
            case '\\':
                // ch = *p++;
                // PUTC(c, ch);
                switch (*p++) {
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c, '\\'); break;
                    case '/':  PUTC(c, '/' ); break;
                    case 'b':  PUTC(c, '\b'); break;
                    case 'f':  PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                    case 'u':  
                        if (!(p = lept_parse_hex4(p, &u1)))
                            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                        if (u1 >= 0xd800 && u1 <= 0xdbff){
                            if (*p++ != '\\')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(p = lept_parse_hex4(p, &u2)))
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (u2 < 0xdc00 || u2 > 0xdfff)
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            u1 = (( (u1-0xd800) << 10) | (u2-0xdc00)) + 0x10000;
                        }
                        lept_encode_utf8(c, u1);
                        break;
                    default:
                        STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            case '\0':
                STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
            default:
                if ((unsigned char)ch < 0x20)
                    STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
                PUTC(c, ch);
        }
    }
}

static int lept_parse_array(lept_context* c, lept_value* v){
    size_t size = 0;
    int ret;
    EXPECT(c, '[');
    lept_parse_whitespace(c);
    if (*c->json == ']'){
        c->json++;
        v->type = LEPT_ARRAY;
        v->u.a.size = 0;
        v->u.a.e = NULL;
        return LEPT_PARSE_OK;
    }
    for (;;){
        // lept_parse_whitespace(c);
        lept_value e;
        lept_init(&e);
        if ((ret = lept_parse_value(c, &e)) != LEPT_PARSE_OK)
            return ret;
        memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value));
        // lept_free(&e);
        size++;
        lept_parse_whitespace(c);
        if (*c->json == ','){
            c->json++;
            lept_parse_whitespace(c);
        }else if (*(c->json) == ']'){
            c->json++;
            v->type = LEPT_ARRAY;
            v->u.a.size = size;
            size *= sizeof(lept_value);
            v->u.a.e = (lept_value*)malloc(size);
            memcpy(v->u.a.e, lept_context_pop(c, size), size);
            return LEPT_PARSE_OK;
        }else{
            ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    size_t i;
    for (i = 0; i < size; i++)
        lept_free((lept_value*)lept_context_pop(c, sizeof(lept_value)));
}

static int lept_parse_value(lept_context* c, lept_value* v){
    switch (*c->json){
        case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
        case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case '\"': return lept_parse_string(c, v);
        case '[': return lept_parse_array(c, v);
        default: return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json){
    lept_context c;
    int t;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = 0;
    c.top = 0;
    lept_init(v);
    lept_parse_whitespace(&c);
    t = lept_parse_value(&c, v);
    if (t == LEPT_PARSE_OK){
        lept_parse_whitespace(&c);
        if (*(c.json) != '\0'){
            t = LEPT_PARSE_ROOT_NOT_SINGULAR;
            v->type = LEPT_NULL;
        }
    }
    assert(c.top == 0);     // make sure, stack is empty.
    free(c.stack);
    return t;
}

lept_type lept_get_type(const lept_value* v){
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v){
    assert(v != NULL);
    assert(v->type == LEPT_NUMBER);
    return v->u.n;
}
void lept_set_number(lept_value* v, double n){
    assert(v != NULL);
    lept_free(v);
    v->type = LEPT_NUMBER;
    v->u.n = n;
}

void lept_free(lept_value* v){
    size_t i;
    assert(v != NULL);
    if (v->type == LEPT_STRING){
        free(v->u.s.s);
    }
    else if (v->type == LEPT_ARRAY){
        for (i = 0; i < v->u.a.size; i++)
            lept_free(&v->u.a.e[i]);
        free(v->u.a.e);
    }
    v->type = LEPT_NULL;
}

size_t lept_get_string_length(const lept_value* v){
    assert(v != NULL);
    assert(v->type == LEPT_STRING);
    return v->u.s.len;
}
const char* lept_get_string(const lept_value* v){
    assert(v != NULL);
    assert(v->type == LEPT_STRING);
    return v->u.s.s;
}
void lept_set_string(lept_value* v, const char* s, size_t len){
    assert(v != NULL);
    assert(s != NULL || len == 0);
    lept_free(v);
    v->u.s.s = (char*)malloc(len+1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = LEPT_STRING;
}

void lept_set_boolean(lept_value* v, int b){
    // assert(v != NULL);
    lept_free(v);
    if (b == 0)
        v->type = LEPT_FALSE;
    else
        v->type = LEPT_TRUE;
}
int lept_get_boolean(const lept_value* v){
    assert(v != NULL);
    assert(v->type == LEPT_TRUE || v->type == LEPT_FALSE);
    return v->type == LEPT_TRUE;
}

size_t lept_get_array_size(const lept_value* v){
    assert(v != NULL);
    assert(v->type == LEPT_ARRAY);
    return v->u.a.size;
}

lept_value* lept_get_array_element(const lept_value* v, size_t index){
    assert(v != NULL);
    assert(v->type == LEPT_ARRAY);
    assert(index < v->u.a.size);
    return &v->u.a.e[index];
}