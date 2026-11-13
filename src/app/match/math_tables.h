// math_tables.h - loads FPMath's lookup tables from the user's game files (PC glue).
//
// The six large tables are TextAssets in the player's Resources ("LUT/FPSinCos"...); the two
// small ones are array initialisers in global-metadata.dat (PhotonDeterministic.dll's
// <PrivateImplementationDetails>). A folder of exported "<Name>.bytes" files can stand in
// for the Resources ones. With `allowApproximation`, tables that can't be found are
// computed from <cmath> instead: the match runs, but not bit-exactly like the original.
#pragma once

#include "rr/assets/player_resources.h"

#include <filesystem>
#include <string>
#include <vector>

namespace rr::app {

struct MathTablesReport {
    bool loaded = false;        // FPLut is ready
    bool exact = false;         // every table came from the game files
    std::vector<std::string> fromGame;
    std::vector<std::string> approximated;
    std::vector<std::string> problems; // why a table couldn't be read
};

MathTablesReport LoadMathTables(assets::PlayerResources* resources, const std::filesystem::path& metadataFile,
                                const std::filesystem::path& lutDir, bool allowApproximation);

} // namespace rr::app
