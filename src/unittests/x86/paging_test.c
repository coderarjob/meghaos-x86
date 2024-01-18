#include <unittest/unittest.h>
#include <mock/kernel/x86/paging.h>
#include <kerror.h>

KernelErrorCodes k_errorNumber;

void reset()
{
    resetPagingFake();
}

int main()
{
    return 0;
}
