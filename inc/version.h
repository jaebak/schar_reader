#ifndef __emu_reader_version_h__
#define __emu_reader_version_h__

#include "config/PackageInfo.h"

namespace emudaqreader {
	const std::string project     = "emu";
	const std::string package     = "emudaqreader";
	const std::string versions    = "15.00.00";
	const std::string description = "Emu (CSC) DAQ readers";

	const std::string summary     = "emu/ldaq/reader";
	const std::string authors     = "Karoly Banicz";
	const std::string link        = "";

	config::PackageInfo getPackageInfo();
	void checkPackageDependencies()
			throw (config::PackageInfo::VersionException);
	std::set<std::string, std::less<std::string> > getPackageDependencies();
};

#endif
