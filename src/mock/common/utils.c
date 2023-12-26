#include <mock/common/utils.h>
#include <unittest/fake.h>

DEFINE_FUNC(UINT, power_of_two, UINT);

void resetUtilsFake()
{
    RESET_FAKE(power_of_two);
}
