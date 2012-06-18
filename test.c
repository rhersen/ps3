#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>

#include "process_status.h"
#include "processes.h"

BOOST_AUTO_TEST_CASE(nonExistingDirectoryShouldGiveEmptyProcessList)
{
    processes target("qwerty");

    BOOST_CHECK(!target.size());
}

BOOST_AUTO_TEST_CASE(single)
{
    processes target("fixtures/single");

    BOOST_CHECK(target.size() == 1);
}

BOOST_AUTO_TEST_CASE(diffEmpty)
{
    processes target("fixtures/empty");
    processes that("fixtures/empty");

    BOOST_CHECK(target.size() == 0);
    target.diff(that, 1);
    BOOST_CHECK(target.size() == 0);
}
