#include <unittest/unittest.h>
#include <kernel.h>
#include <string.h>
#include <stdio.h>


TEST(kearly_snprintf, no_vargs)
{
#define MESSAGE "Hello world"
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), MESSAGE);
    EQ_SCALAR(ret, 11);
    EQ_SCALAR(ret, strlen(MESSAGE));
    EQ_STRING(d, MESSAGE);
    END();
}

TEST(kearly_snprintf, unsigned_int_decimal_format)
{
    INT num = 1045;
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%u", num);
    EQ_SCALAR(ret, 4);
    EQ_STRING(d, "1045");
    END();
}

TEST(kearly_snprintf, unsigned_int_hex_format)
{
    INT num = 0x123def;
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%x", num);
    EQ_SCALAR(ret, 6);
    EQ_STRING(d, "123DEF");
    END();
}

TEST(kearly_snprintf, unsigned_int_octal_format)
{
    INT num = 07210;
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%o", num);
    EQ_SCALAR(ret, 4);
    EQ_STRING(d, "7210");
    END();
}

TEST(kearly_snprintf, limit_check_empty_string)
{
    CHAR *message = "";
    CHAR d[1];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%s", message);
    EQ_SCALAR(ret, 0);
    EQ_STRING(d, "");
    END();
}

TEST(kearly_snprintf, limit_check_at_edge)
{
    CHAR *message = "aaa"; // 3 'a's. NULL at index 3.
    CHAR d[4];

    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%s", message);
    EQ_SCALAR(ret, 3);
    EQ_STRING(d, message);
    END();
}

TEST(kearly_snprintf, limit_check_overflow_literal)
{
#define MESSAGE "Hello world"
    CHAR d[8];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), MESSAGE);
    EQ_SCALAR(ret, ERR_OVERFLOW);
    EQ_STRING(d, "Hello w");
    END();
}

TEST(kearly_snprintf, limit_check_overflow_string_format)
{
    CHAR *message = "Hello world";
    CHAR d[8];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%s", message);
    EQ_SCALAR(ret, ERR_OVERFLOW);
    EQ_STRING(d, "Hello w");
    END();
}

TEST(kearly_snprintf, limit_check_overflow_unsigned_int)
{
    INT num = 0xFF012EA;
    CHAR d[4];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%x", num);
    EQ_SCALAR(ret, ERR_OVERFLOW);
    EQ_STRING(d, "FF0");
    END();
}

TEST(kearly_snprintf, limit_check_overflow_mixed)
{
    INT num = 0xFF012EA;
    CHAR d[15];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "Hello %s %x",
                                                   "World", num);
    EQ_SCALAR(ret, ERR_OVERFLOW);
    EQ_STRING(d, "Hello World FF");
    END();
}

TEST(kearly_snprintf, memory_overlap)
{
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret1 = kearly_snprintf (d, ARRAY_LENGTH(d), "Hello");
    INT ret2 = kearly_snprintf (d, ARRAY_LENGTH(d), "%s World", d);
    EQ_SCALAR(ret1, 5);
    EQ_SCALAR(ret2, 5 + 1 + 5);
    EQ_STRING(d, "Hello World");
    END();
}

int main()
{
    no_vargs();
    unsigned_int_hex_format();
    unsigned_int_decimal_format();
    unsigned_int_octal_format();
    limit_check_overflow_literal();
    limit_check_overflow_string_format();
    limit_check_overflow_unsigned_int();
    limit_check_overflow_mixed();
    limit_check_empty_string();
    limit_check_at_edge();
    memory_overlap();
}
