/// @file mssplit.cc
///
/// @brief
///
/// @copyright (c) 2012 CSIRO
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

// Package level header file
#include "askap_synthesis.h"

// System includes
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "askap/AskapUtil.h"
#include "askap/Log4cxxLogSink.h"
#include "boost/shared_ptr.hpp"
#include "boost/regex.hpp"
#include "Common/ParameterSet.h"
#include "CommandLineParser.h"
#include "casa/OS/Timer.h"
#include "casa/OS/File.h"
#include "casa/aips.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "tables/Tables/TableDesc.h"
#include "tables/Tables/SetupNewTab.h"
#include "tables/Tables/IncrementalStMan.h"
#include "tables/Tables/StandardStMan.h"
#include "tables/Tables/TiledShapeStMan.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"

ASKAP_LOGGER(logger, ".msmerge2");

using namespace askap;
using namespace casa;

boost::shared_ptr<casa::MeasurementSet> create(const std::string& filename)
{
    // Get configuration first to ensure all parameters are present
    casa::uInt bucketSize =  128 * 1024;
    casa::uInt tileNcorr = 4;
    casa::uInt tileNchan = 1;

    if (bucketSize < 8192) {
        bucketSize = 8192;
    }

    if (tileNcorr < 1) {
        tileNcorr = 1;
    }

    if (tileNchan < 1) {
        tileNchan = 1;
    }

    ASKAPLOG_DEBUG_STR(logger, "Creating dataset " << filename);

    // Make MS with standard columns
    TableDesc msDesc(MS::requiredTableDesc());

    // Add the DATA column.
    MS::addColumnToDesc(msDesc, MS::DATA, 2);

    SetupNewTable newMS(filename, msDesc, Table::New);

    // Set the default Storage Manager to be the Incr one
    {
        IncrementalStMan incrStMan("ismdata", bucketSize);
        newMS.bindAll(incrStMan, True);
    }

    // Bind ANTENNA1, and ANTENNA2 to the standardStMan
    // as they may change sufficiently frequently to make the
    // incremental storage manager inefficient for these columns.
    {
        StandardStMan ssm("ssmdata", bucketSize);
        newMS.bindColumn(MS::columnName(MS::ANTENNA1), ssm);
        newMS.bindColumn(MS::columnName(MS::ANTENNA2), ssm);
        newMS.bindColumn(MS::columnName(MS::UVW), ssm);
    }

    // These columns contain the bulk of the data so save them in a tiled way
    {
        // Get nr of rows in a tile.
        const int nrowTile = std::max(1u, bucketSize / (8 * tileNcorr * tileNchan));
        TiledShapeStMan dataMan("TiledData",
                                IPosition(3, tileNcorr, tileNchan, nrowTile));
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::DATA),
                         dataMan);
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::FLAG),
                         dataMan);
    }
    {
        const int nrowTile = std::max(1u, bucketSize / (4 * 8));
        TiledShapeStMan dataMan("TiledWeight",
                                IPosition(2, 4, nrowTile));
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::SIGMA),
                         dataMan);
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::WEIGHT),
                         dataMan);
    }

    // Now we can create the MeasurementSet and add the (empty) subtables
    boost::shared_ptr<casa::MeasurementSet> ms(new MeasurementSet(newMS, 0));
    ms->createDefaultSubtables(Table::New);
    ms->flush();

    // Set the TableInfo
    {
        TableInfo& info(ms->tableInfo());
        info.setType(TableInfo::type(TableInfo::MEASUREMENTSET));
        info.setSubType(String(""));
        info.readmeAddLine("This is a MeasurementSet Table holding simulated astronomical observations");
    }
    return ms;
}

void copyAntenna(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSAntennaColumns& sc = srcMsc.antenna();

    MSColumns destMsc(dest);
    MSAntennaColumns& dc = destMsc.antenna();

    // Add new rows to the destination and copy the data
    dest.antenna().addRow(sc.nrow());

    dc.name().putColumn(sc.name());
    dc.station().putColumn(sc.station());
    dc.type().putColumn(sc.type());
    dc.mount().putColumn(sc.mount());
    dc.position().putColumn(sc.position());
    dc.dishDiameter().putColumn(sc.dishDiameter());
    dc.flagRow().putColumn(sc.flagRow());
}

void copyDataDescription(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSDataDescColumns& sc = srcMsc.dataDescription();

    MSColumns destMsc(dest);
    MSDataDescColumns& dc = destMsc.dataDescription();

    // Add new rows to the destination and copy the data
    dest.dataDescription().addRow(sc.nrow());

    dc.flagRow().putColumn(sc.flagRow());
    dc.spectralWindowId().putColumn(sc.spectralWindowId());
    dc.polarizationId().putColumn(sc.polarizationId());
}

void copyFeed(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSFeedColumns& sc = srcMsc.feed();

    MSColumns destMsc(dest);
    MSFeedColumns& dc = destMsc.feed();

    // Add new rows to the destination and copy the data
    dest.feed().addRow(sc.nrow());

    dc.antennaId().putColumn(sc.antennaId());
    dc.feedId().putColumn(sc.feedId());
    dc.spectralWindowId().putColumn(sc.spectralWindowId());
    dc.beamId().putColumn(sc.beamId());
    dc.numReceptors().putColumn(sc.numReceptors());
    dc.position().putColumn(sc.position());
    dc.beamOffset().putColumn(sc.beamOffset());
    dc.polarizationType().putColumn(sc.polarizationType());
    dc.polResponse().putColumn(sc.polResponse());
    dc.receptorAngle().putColumn(sc.receptorAngle());
    dc.time().putColumn(sc.time());
    dc.interval().putColumn(sc.interval());
}

void copyField(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSFieldColumns& sc = srcMsc.field();

    MSColumns destMsc(dest);
    MSFieldColumns& dc = destMsc.field();

    // Add new rows to the destination and copy the data
    dest.field().addRow(sc.nrow());

    dc.name().putColumn(sc.name());
    dc.code().putColumn(sc.code());
    dc.time().putColumn(sc.time());
    dc.numPoly().putColumn(sc.numPoly());
    dc.sourceId().putColumn(sc.sourceId());
    dc.delayDir().putColumn(sc.delayDir());
    dc.phaseDir().putColumn(sc.phaseDir());
    dc.referenceDir().putColumn(sc.referenceDir());
}

void copyObservation(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSObservationColumns& sc = srcMsc.observation();

    MSColumns destMsc(dest);
    MSObservationColumns& dc = destMsc.observation();

    // Add new rows to the destination and copy the data
    dest.observation().addRow(sc.nrow());

    dc.timeRange().putColumn(sc.timeRange());
    //dc.log().putColumn(sc.log());
    //dc.schedule().putColumn(sc.schedule());
    dc.flagRow().putColumn(sc.flagRow());
    dc.observer().putColumn(sc.observer());
    dc.telescopeName().putColumn(sc.telescopeName());
    dc.project().putColumn(sc.project());
    dc.releaseDate().putColumn(sc.releaseDate());
    dc.scheduleType().putColumn(sc.scheduleType());
}

void copyPointing(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSPointingColumns& sc = srcMsc.pointing();

    MSColumns destMsc(dest);
    MSPointingColumns& dc = destMsc.pointing();

    // Add new rows to the destination and copy the data
    dest.pointing().addRow(sc.nrow());

    // TODO: The first two left out because adding "target" hangs the split (or at least
    // gets it stuch in some long/infinite loop). Maybe need to handle these
    // MeasCol differently
    //dc.direction().putColumn(sc.direction());
    //dc.target().putColumn(sc.target());
    dc.antennaId().putColumn(sc.antennaId());
    dc.interval().putColumn(sc.interval());
    dc.name().putColumn(sc.name());
    dc.numPoly().putColumn(sc.numPoly());
    dc.time().putColumn(sc.time());
    dc.timeOrigin().putColumn(sc.timeOrigin());
    dc.tracking().putColumn(sc.tracking());
}

void copyPolarization(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSPolarizationColumns& sc = srcMsc.polarization();

    MSColumns destMsc(dest);
    MSPolarizationColumns& dc = destMsc.polarization();

    // Add new rows to the destination and copy the data
    dest.polarization().addRow(sc.nrow());

    dc.flagRow().putColumn(sc.flagRow());
    dc.numCorr().putColumn(sc.numCorr());
    dc.corrType().putColumn(sc.corrType());
    dc.corrProduct().putColumn(sc.corrProduct());
}

void appendToVector(const casa::Vector<double>& src, std::vector<double>& dest)
{
    std::copy(src.begin(), src.end(), std::back_inserter(dest));
}

void splitSpectralWindow(const casa::MeasurementSet& source,
                         casa::MeasurementSet& dest,
                         const unsigned int startChan,
                         const unsigned int endChan,
                         const unsigned int width)
{
}

void splitMainTable(const casa::MeasurementSet& source,
                    casa::MeasurementSet& dest,
                    const unsigned int startChan,
                    const unsigned int endChan,
                    const unsigned int width)
{
}

void split(const std::string& invis, const std::string& outvis,
           const unsigned int startChan,
           const unsigned int endChan,
           const unsigned int width)
{
    // Open the input measurement set
    const casa::MeasurementSet in(invis);

    // Create the output measurement set
    ASKAPCHECK(!casa::File(outvis).exists(), "File or table "
                   << outvis << " already exists!");
    boost::shared_ptr<casa::MeasurementSet> out(create(outvis));

    // Copy ANTENNA
    ASKAPLOG_INFO_STR(logger,  "Copying ANTENNA table");
    copyAntenna(in, *out);

    // Copy DATA_DESCRIPTION
    ASKAPLOG_INFO_STR(logger,  "Copying DATA_DESCRIPTION table");
    copyDataDescription(in, *out);

    // Copy FEED
    ASKAPLOG_INFO_STR(logger,  "Copying FEED table");
    copyFeed(in, *out);

    // Copy FIELD
    ASKAPLOG_INFO_STR(logger,  "Copying FIELD table");
    copyField(in, *out);

    // Copy OBSERVATION
    ASKAPLOG_INFO_STR(logger,  "Copying OBSERVATION table");
    copyObservation(in, *out);

    // Copy POINTING
    ASKAPLOG_INFO_STR(logger,  "Copying POINTING table");
    copyPointing(in, *out);

    // Copy POLARIZATION
    ASKAPLOG_INFO_STR(logger,  "Copying POLARIZATION table");
    copyPolarization(in, *out);

    // Split SPECTRAL_WINDOW
    ASKAPLOG_INFO_STR(logger,  "Splitting SPECTRAL_WINDOW table");
    splitSpectralWindow(in, *out, startChan, endChan, width);

    // Split main table
    ASKAPLOG_INFO_STR(logger,  "Splitting main table");
    splitMainTable(in, *out, startChan, endChan, width);
}

std::vector<unsigned int> parseRange(const LOFAR::ParameterSet parset)
{
    std::vector<unsigned int> results;
    const std::string raw = parset.getString("channel");

    // These are the two possible patterns, either a single integer, or an
    // integer separated by a dash (potentially surrounded by whitespace)
    const boost::regex e1("[\\d]+");
    const boost::regex e2("([\\d]+)\\s*-\\s*([\\d]+)");

    boost::smatch what;

    if (regex_match(raw, what, e1)) {
        results.push_back(utility::fromString<unsigned int>(raw));
    } else if (regex_match(raw, e2)) {
        // Now extract the first and second integer
        const boost::regex digits("[\\d]+");
        boost::sregex_iterator it(raw.begin(), raw.end(), digits);
        boost::sregex_iterator end;

        for (; it != end; ++it) {
            results.push_back(utility::fromString<unsigned int>(it->str()));
        }

        ASKAPCHECK(results.size() == 2,
                   "Internal error: expected two integers in result vector");
    } else {
        ASKAPLOG_ERROR_STR(logger, "Invalid format 'channel' parameter");
    }

    return results;
}

// Main function
int main(int argc, const char** argv)
{
    std::ostringstream ss;
    ss << argv[0] << ".log_cfg";
    ASKAPLOG_INIT(ss.str().c_str());

    // Ensure that CASA log messages are captured
    casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
    casa::LogSink::globalSink(globalSink);

    try {
        casa::Timer timer;
        timer.mark();

        // Command line parser
        cmdlineparser::Parser parser;

        // Command line parameter
        cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs", "mssplit.in");

        // Throw an exception if the parameter is not present
        parser.add(inputsPar, cmdlineparser::Parser::throw_exception);
        parser.process(argc, const_cast<char**>(argv));

        // Create a parset
        LOFAR::ParameterSet parset(inputsPar);

        // Get the parameters to split
        const std::string invis = parset.getString("vis");
        const std::string outvis = parset.getString("outputvis");
        const std::vector<unsigned int> range = parseRange(parset);

        if (range.empty()) {
            return 1;
        }
        const unsigned int width = parset.getUint32("width", 1);
        split(invis, outvis, range[0], range[1], width);

        ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user() << " system: " << timer.system()
                              << " real:   " << timer.real());
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        ASKAPLOG_FATAL_STR(logger, "Usage: " << argv[0] << " -o output.ms inMS1 ... inMSn");
        return 1;
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        return 1;
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        return 1;
    }

    return 0;
}
