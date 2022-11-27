#ifndef LEPTJSON_H__
#define LEPTJSON_H__
#include <stdio.h>

typedef enum{ LEPT_NULL, LEPT_TRUE, LEPT_FALSE, LEPT_NUMBER, \
			LEPT_STRING, LEPT_OBJECT, LEPT_ARRAY } lept_type;

#define LEPT_KEY_NOT_EXIST ((size_t)-1)

typedef struct lept_value lept_value;
typedef struct lept_member lept_member;

struct lept_value{
	union{
		struct { lept_member* m; size_t size; }o;	// object
		struct { lept_value* e; size_t size; }a; 	// array
		struct { char *s; size_t len; }s;
		double n;
	}u;
	lept_type type;
};

struct lept_member{
	char* k; size_t klen;	// member key string, key string length
	lept_value v;			// member value
};		// key-value pair

enum{
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_NUMBER_TOO_BIG,
	LEPT_PARSE_MISS_QUOTATION_MARK,			// miss quotation(")
	LEPT_PARSE_INVALID_STRING_ESCAPE,		// illegal escape
	LEPT_PARSE_INVALID_STRING_CHAR,			// illegal char(contral char)
	LEPT_PARSE_INVALID_UNICODE_SURROGATE,	// unicode only have h, or l invalid 
	LEPT_PARSE_INVALID_UNICODE_HEX,			// (\uXXXX)  XXXX is not hex
	LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
	LEPT_PARSE_MISS_KEY,
	LEPT_PARSE_MISS_COLON,
	LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

#define lept_init(v) do{ (v)->type = LEPT_NULL; } while(0)
void lept_free(lept_value* v);
#define lept_set_null(v) lept_free(v);

int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);

const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);

int lept_parse(lept_value *v, const char* json);
char* lept_stringify(const lept_value* v, size_t* length);

void lept_copy(lept_value* dst, const lept_value* src);
void lept_move(lept_value* dst, lept_value* src);
void lept_swap(lept_value* lhs, lept_value* rhs);

lept_type lept_get_type(const lept_value* v);
int lept_is_equal(const lept_value* lhs, const lept_value* rhs);

double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double n);

size_t lept_get_array_size(const lept_value* v);
lept_value* lept_get_array_element(const lept_value* v, size_t index);
void lept_set_array(lept_value* v, size_t capacity);
size_t lept_get_array_capacity(const lept_value* v);
void lept_reserve_array(lept_value* v, size_t capacity);
void lept_shrink_array(lept_value* v);
lept_value* lept_pushback_array_element(lept_value* v);
void lept_popback_array_element(lept_value *v);
lept_value* lept_insert_array_element(lept_value* v, size_t index);
void lept_erase_array_element(lept_value* v, size_t index, size_t count);
void lept_clear_array(lept_value* v);

size_t lept_get_object_size(const lept_value* v);
const char* lept_get_object_key(const lept_value* v, size_t index);
size_t lept_get_object_key_length(const lept_value* v, size_t index);
lept_value* lept_get_object_value(const lept_value* v, size_t index);
size_t lept_find_object_index(const lept_value* v, const char* key, size_t klen);
lept_value* lept_find_object_value(lept_value* v, const char* key, size_t klen);
void lept_set_object(lept_value* v, size_t capacity);
size_t lept_get_object_capacity(const lept_value* v);
void lept_reserve_object(lept_value* v, size_t capacity);
void lept_shrink_object(lept_value* v);
void lept_clear_object(lept_value* v);
lept_value* lept_set_object_value(lept_value* v, const char* key, size_t klen);
void lept_remove_object_value(lept_value* v, size_t index);


#endif