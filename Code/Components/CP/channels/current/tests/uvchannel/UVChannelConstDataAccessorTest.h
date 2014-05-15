/// @file UVChannelConstDataAccessorTest.h
///
/// @copyright (c) 2011 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "casa/aipstype.h"
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"

// Classes to test
#include "uvchannel/uvdataaccess/UVChannelConstDataAccessor.h"

// Using
using namespace std;

namespace askap {
namespace cp {
namespace channels {

class UVChannelConstDataAccessorTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(UVChannelConstDataAccessorTest);
        CPPUNIT_TEST(testConstructor);
        CPPUNIT_TEST_SUITE_END();

    public:

        void setUp()
        {
        };

        void tearDown()
        {
        }

        void testConstructor()
        {
            const casa::uInt nRow = 21*36;
            const casa::uInt nChannel = 16416;
            const casa::uInt nPol = 4;

            boost::shared_ptr<askap::cp::common::VisChunk> chunk(
                    new askap::cp::common::VisChunk(nRow, nChannel, nPol));
            UVChannelConstDataAccessor acc(chunk);

            CPPUNIT_ASSERT_EQUAL(nRow, acc.nRow());
            CPPUNIT_ASSERT_EQUAL(nChannel, acc.nChannel());
            CPPUNIT_ASSERT_EQUAL(nPol, acc.nPol());

            CPPUNIT_ASSERT(acc.visibility().shape() == acc.flag().shape());
            CPPUNIT_ASSERT(acc.visibility().shape() == acc.noise().shape());

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nRow), acc.antenna1().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nRow), acc.antenna2().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nRow), acc.feed1().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nRow), acc.feed2().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nRow), acc.feed1PA().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nRow), acc.feed2PA().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nRow), acc.pointingDir1().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nRow), acc.pointingDir2().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nRow), acc.dishPointing1().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nRow), acc.dishPointing2().size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nRow), acc.uvw().size());

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nPol), acc.stokes().size());

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nChannel), acc.frequency().size());
        }
};

}   // End namespace channels
}   // End namespace cp
}   // End namespace askap