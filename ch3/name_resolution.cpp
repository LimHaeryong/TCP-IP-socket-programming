#include "../common.h"

#define TEST_NAME "www.google.com"

bool get_ip_addr(const char* name, struct in_addr* addr)
{
    struct hostent* ptr = gethostbyname(name);
    if(ptr == NULL)
    {
        error_display("gethostbyname()");
        return false;
    }
    if(ptr->h_addrtype != AF_INET)
        return false;
    
    memcpy(addr, &ptr->h_addr[0], ptr->h_length);
    return true;
}

bool get_domain_name(struct in_addr addr, char* name, int name_len)
{
    struct hostent* ptr = gethostbyaddr((const char*)&addr, sizeof(addr), AF_INET);
    if(ptr == NULL)
    {
        error_display("gethostbyaddr()");
        return false;
    }
    if(ptr->h_addrtype != AF_INET)
        return false;
    
    strncpy(name, ptr->h_name, name_len);
    return true;
}

int main(int argc, char* argv[])
{
    

    printf("domain name : %s\n", TEST_NAME);
    struct in_addr addr;
    if(get_ip_addr(TEST_NAME, &addr))
    {
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr, str, sizeof(str));
        printf("ip addr = %s\n", str);

        char name[256];
        if(get_domain_name(addr, name, sizeof(name)))
        {
            printf("domain name = %s\n", name);
        }
    }

    return 0;
}