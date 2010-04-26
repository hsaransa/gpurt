#ifndef _dn_zorder_hpp_
#define _dn_zorder_hpp_

#include "dndefs.hpp"

namespace dn
{
    class ZOrder
    {
    public:
        ZOrder(int w, int h);
        ~ZOrder();

        HostMemory* get_to_index();
        HostMemory* get_to_coord();

    private:
        HostMemory* to_index;
        HostMemory* to_coord;
    };
}

#endif
