// team.cpp - Quantum.Team's team membership helpers.
#include "quantum_code/frame.h"
#include "quantum_core/component_filter_struct.h"

namespace Quantum {

namespace {
// Quantum.Team.PlayerTeamFilter
struct PlayerTeamFilter {
    EntityRef entity;
    Team* team = nullptr;
    PlayerActor* playerActor = nullptr;
    static constexpr auto FilterEntity = &PlayerTeamFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&PlayerTeamFilter::team, &PlayerTeamFilter::playerActor);
};
} // namespace

void Team::JoinTeam(Frame* f, EntityRef newTeamMember, std::int32_t teamID) {
    Team* team = nullptr;
    if (f->Add<Team>(newTeamMember, Team{}, team) != AddResult::ComponentAdded &&
        !f->TryGetPointer<Team>(newTeamMember, team)) {
        return;
    }
    team->TeamID = teamID;
    EntityRef leader;
    team->isLeader = !TryGetPlayerTeamLeader(f, teamID, leader);
}

bool Team::TryGetPlayerTeamLeader(Frame* f, std::int32_t teamId, EntityRef& leader) {
    auto filter = FilterStruct<PlayerTeamFilter>(f);
    PlayerTeamFilter current;
    while (filter.Next(current)) {
        if (current.team->TeamID == teamId && current.team->isLeader) {
            leader = current.entity;
            return true;
        }
    }
    leader = EntityRef{};
    return false;
}

} // namespace Quantum
