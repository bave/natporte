#include "log.hpp"
#include <boost/scoped_ptr.hpp>

log* s;

int main(int argc, char** argv)
{
    //boost::scoped_ptr<log> s(new log(argv[0], LOG_LOCAL0, LOG_DEBUG));
    s = new log::log(argv[0], LOG_LOCAL0, LOG_DEBUG);
    s->output("test");
    return 0;
}
