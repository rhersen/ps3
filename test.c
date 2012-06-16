#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>

#include "process_status.h"
#include "processes.h"

BOOST_AUTO_TEST_CASE(nonExistingDirectoryShouldGiveEmptyProcessList)
{
    processes test_object("qwerty");

    BOOST_CHECK(!test_object.size());
}
