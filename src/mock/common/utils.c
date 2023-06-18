#include <mock/common/utils.h>
#include <unittest/fake.h>

DEFINE_FUNC_1(UINT, power_of_two, UINT);

void resetUtilsFake()
{
    RESET_FAKE(power_of_two);
}
