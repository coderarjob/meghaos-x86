#include <unittest/unittest.h>
#include <kernel.h>
#include <string.h>
#include <stdio.h>

TEST(kearly_snprintf, no_vargs)
{
    char *message = "Hello world";
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    int ret = kearly_snprintf (d, ARRAY_LENGTH(d), message);
    EQ_SCALAR(ret, strlen(message));
    EQ_STRING(d, message);
    END();
}

TEST(kearly_snprintf, unsigned_init_decimal_format)
{
    int num = 1045;
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    int ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%u", num);
    EQ_SCALAR(ret, 4);
    EQ_STRING(d, "1045");
    END();
}

TEST(kearly_snprintf, unsigned_int_hex_format)
{
    int num = 0x123def;
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    int ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%x", num);
    EQ_SCALAR(ret, 6);
    EQ_STRING(d, "123DEF");
    END();
}

TEST(kearly_snprintf, limit_check)
{
    CHAR *message = "Hello world";
    CHAR d[8];
    int ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%s", message);
    EQ_SCALAR(ret, strlen(message));
    EQ_STRING(d, "Hello w");

    ret = snprintf (d, ARRAY_LENGTH(d), "%s", message);
    EQ_SCALAR(ret, strlen(message));
    EQ_STRING(d, "Hello w");

    END();
}

int main()
{
    no_vargs();
    unsigned_int_hex_format();
    unsigned_init_decimal_format();
    limit_check();
}
