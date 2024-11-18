#include <unittest/unittest.h>
#include <mock/kernel/x86/vgadisp.h>
#include <types.h>
#include <moslimits.h>
#include <kdebug.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <kernel.h>

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

TEST(kearly_snprintf, unsigned_int_bit_format)
{
    INT num = 0xFF01;
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%b", num);
    EQ_SCALAR(ret, 17);
    EQ_STRING(d, "1111111100000001b");
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

TEST(kearly_snprintf, unsigned_int_hex_format_without_base_identifier)
{
    INT num = 0x123def;
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%h", num);
    EQ_SCALAR(ret, 6);
    EQ_STRING(d, "123DEF");
    END();
}

TEST(kearly_snprintf, unsigned_int_hex_format)
{
    INT num = 0x123def;
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%x", num);
    EQ_SCALAR(ret, 7);
    EQ_STRING(d, "123DEFh");
    END();
}

TEST(kearly_snprintf, unsigned_int_octal_format)
{
    INT num = 07210;
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%o", num);
    EQ_SCALAR(ret, 5);
    EQ_STRING(d, "7210o");
    END();
}

TEST(kearly_snprintf, unsigned_long_hex_format)
{
    U32 num = 0xCF010203;
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%lx", num);
    EQ_SCALAR(ret, 9);
    EQ_STRING(d, "CF010203h");
    END();
}
TEST(kearly_snprintf, unsigned_long_long_hex_format)
{
    U64 num = 0xCF010203040506FF;
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%llx", num);
    EQ_SCALAR(ret, 17);
    EQ_STRING(d, "CF010203040506FFh");
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
    EQ_SCALAR(ret, 11);
    EQ_STRING(d, "Hello w");
    END();
}

TEST(kearly_snprintf, limit_check_overflow_string_format)
{
    CHAR *message = "Hello world";
    CHAR d[8];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%s", message);
    EQ_SCALAR(ret, 11);
    EQ_STRING(d, "Hello w");
    END();
}

TEST(kearly_snprintf, limit_check_overflow_unsigned_int)
{
    INT num = 0xFF012EA;
    CHAR d[4];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%x", num);
    EQ_SCALAR(ret, 8);
    EQ_STRING(d, "FF0");
    END();
}

TEST(kearly_snprintf, limit_check_overflow_mixed)
{
    INT num = 0xFF012EA;
    CHAR d[15];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "Hello %s %x",
                                                   "World", num);
    EQ_SCALAR(ret, 20);
    EQ_STRING(d, "Hello World FF");
    END();
}

TEST(kearly_snprintf, percent_symbol)
{
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "100%%");
    EQ_SCALAR(ret, 4);
    EQ_STRING(d, "100%");
    END();
}

TEST(kearly_snprintf, wrong_format)
{
    // %a is not a valid format. snprintf should treat this to be a literal.
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "Hello %arjob");
    EQ_SCALAR(ret, 12);
    EQ_STRING(d, "Hello %arjob");
    END();
}

TEST(kearly_snprintf, string_literal)
{
    CHAR d[MAX_PRINTABLE_STRING_LENGTH];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "Arjob 0x10A");
    EQ_SCALAR(ret, 11);
    EQ_STRING(d, "Arjob 0x10A");
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

TEST(kearly_snprintf, pointer_literal)
{
    USYSINT num = 0xFF012EA;
    CHAR d[10];
    INT ret = kearly_snprintf (d, ARRAY_LENGTH(d), "%px", num);
    EQ_SCALAR(ret, 8);
    EQ_STRING(d, "FF012EAh");
    END();
}

// -------------------------------------------------------------------------------------------------
// Handler functions for fake kdisp_putc function.
// Used in the following tests: printk_test_disp_initialized, printk_test_disp_not_initialized
// -------------------------------------------------------------------------------------------------
static U16 vgab [10];
void kdisp_putc_hander (CHAR c)
{
    static UINT index = 0;
    vgab [index++] = (U16)((0x0FF << 8) | (UINT)c); // attribute of 0xFF (arbitary value)
}
// -------------------------------------------------------------------------------------------------

void reset()
{
    resetVgaDisp();
    memset (vgab, 0, ARRAY_LENGTH (vgab));
}

int main()
{
    no_vargs();
    unsigned_int_hex_format();
    unsigned_int_hex_format_without_base_identifier();
    unsigned_int_decimal_format();
    unsigned_int_octal_format();
    unsigned_long_long_hex_format();
    unsigned_long_hex_format();
    unsigned_int_bit_format();
    percent_symbol();
    string_literal();
    wrong_format();
    limit_check_overflow_literal();
    limit_check_overflow_string_format();
    limit_check_overflow_unsigned_int();
    limit_check_overflow_mixed();
    limit_check_empty_string();
    limit_check_at_edge();
    memory_overlap();
    pointer_literal();
}
