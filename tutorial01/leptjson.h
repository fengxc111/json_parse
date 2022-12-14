#ifndef LEPTJSON_H__
#define LEPTJSON_H__

typedef enum{ LEPT_NULL, LEPT_TRUE, LEPT_FALSE, LEPT_NUMBER, \
			LEPT_STRING, LEPT_OBJECT, LEPT_ARRAY } lept_type;

typedef struct{
	lept_type type;
}lept_value;

enum{
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_INVALID_VALUE
};


int lept_parse(lept_value *v, const char* json);
lept_type lept_get_type(const lept_value *v);


#endif
