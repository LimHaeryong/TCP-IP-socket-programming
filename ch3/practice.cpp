#include "../common.h"

#include <sys/types.h>

#define TEST_NAME "www.facebook.com"

bool get_ip_info(const char* name, struct in_addr* addr)
{
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(name, "https", &hints, &result);
    if(ret != 0)
    {
        error_display("getaddrinfo()");
        return false;
    }

    struct sockaddr_in* ipv4 = (struct sockaddr_in*)result->ai_addr;
    *addr = ipv4->sin_addr;
    return true;
}

bool get_domain_info(struct in_addr addr, char* name, int name_len)
{
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_addr = addr;

    int ret = getnameinfo((struct sockaddr*)&sa, sizeof(struct sockaddr_in), name, name_len, NULL, 0, 0);
    if(ret != 0)
    {
        error_display("getnameinfo()");
        return false;
    }
    return true;
}

bool is_little_endian()
{
    uint32_t i = 1;
    uint8_t* i_uchar = (uint8_t*)&i;
    return (*i_uchar == 1);
}

bool is_big_endian()
{
    return !is_little_endian();
}

void print_aliases_and_ip_addrs(struct hostent* host)
{

    if(host->h_addrtype != AF_INET)
        return;
    
    if(host->h_aliases != NULL)
    {
        char** alias_list = host->h_aliases;
        while((*alias_list) != NULL)
        {
            printf("Alias : %s\n", *alias_list);
            ++alias_list;
        }
    }

    struct in_addr addr;
    char str[INET_ADDRSTRLEN];
    if(host->h_addr_list != NULL)
    {
        struct in_addr** addr_list = (struct in_addr**)host->h_addr_list;
        while((*addr_list) != NULL)
        {
            memcpy(&addr, *addr_list, host->h_length);
            inet_ntop(AF_INET, &addr, str, sizeof(str));
            printf("Addr : %s\n", str);
            ++addr_list;
        }
    }
}

void print_from_domain_name(const char* domain_name)
{
    struct hostent* host = gethostbyname(domain_name);
    if(host == NULL)
    {
        error_display("gethostbyname()");
        return;
    }
    print_aliases_and_ip_addrs(host);
}

void print_from_ip_addr(struct in_addr addr)
{
    struct hostent* host = gethostbyaddr((const char*)&addr, sizeof(addr), AF_INET);
    if(host == NULL)
    {
        error_display("gethostbyaddr()");
        return;
    }
    print_aliases_and_ip_addrs(host);
}

int main(int argc, char* argv[])
{
    if(is_little_endian())
        printf("little endian\n");
    else
        printf("big endian\n");

    printf("print from domain name\n");
    print_from_domain_name(TEST_NAME);
    struct in_addr addr;
    get_ip_info(TEST_NAME, &addr);
    printf("print from ip addr\n");
    print_from_ip_addr(addr);

    char name[256];
    if(get_domain_info(addr, name, sizeof(name)))
    {
        printf("domain name = %s\n", name);
    }
    return 0;
}