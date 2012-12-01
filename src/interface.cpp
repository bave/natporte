#include "interface.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    pname = argv[0];
    interface::interface iflist;
    int i;
    for (i=0; i<iflist.listsize(); i++) {
        std::cout << iflist.name(i) << std::endl;
        std::cout << iflist.addr(i) << std::endl;
        std::cout << iflist.mask(i) << std::endl;
        std::cout << iflist.family(i) << std::endl;
        std::cout << iflist.broad(i) << std::endl;
        std::cout << std::endl;
    }
    /*
    for (i=0; i<iflist.list4size(); i++) {
        std::cout << iflist.name4(i) << std::endl;
        std::cout << iflist.addr4(i) << std::endl;
        std::cout << iflist.mask4(i) << std::endl;
        std::cout << iflist.family4(i) << std::endl;
        std::cout << std::endl;
    }
    for (i=0; i<iflist.list6size(); i++) {
        std::cout << iflist.name6(i) << std::endl;
        std::cout << iflist.addr6(i) << std::endl;
        std::cout << iflist.mask6(i) << std::endl;
        std::cout << iflist.family6(i) << std::endl;
        std::cout << std::endl;
    }

    interface::interface* ptr_iflist;
    ptr_iflist = new interface::interface();
    for (i=0; i<iflist.listsize(); i++) {
        std::cout << ptr_iflist->name(i) << std::endl;
        std::cout << ptr_iflist->addr(i) << std::endl;
        std::cout << ptr_iflist->mask(i) << std::endl;
        std::cout << ptr_iflist->family(i) << std::endl;
        std::cout << std::endl;
    }
    */
    return 0;
}
