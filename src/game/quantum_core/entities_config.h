// entities_config.h - Quantum.Core.FrameBase.EntitiesConfig: how a frame sizes its entity
// table and component blocks, and when it applies deferred removals.
//
// Declared at namespace level so ComponentDataBuffer can use it without the whole frame;
// FrameBase exposes it as FrameBase::EntitiesConfig.
#pragma once

#include <string>
#include <vector>

namespace Quantum {

// When a frame applies the component removals and entity destructions requested while it
// was simulated.
enum class CommitCommandsModes : int {
    OnFrameSimulationBegin = 0,
    OnFrameSimulationEnd = 1,
    InBetweenSystems = 2,
    Manual = 3,
};

namespace Core {

struct EntitiesConfig {
    // Block size override for one component type, matched by C# type name.
    struct ComponentBufferConfig {
        std::string Name;
        int ComponentBlockCapacity = 0;
    };

    CommitCommandsModes CommitCommandsMode = CommitCommandsModes::OnFrameSimulationBegin;
    // 0x7d1da18 - The defaults are 1024 entities and 128 instances per block.
    int DefaultEntityCapacity = 1024;
    int DefaultComponentBlockCapacity = 128;
    std::vector<ComponentBufferConfig> ComponentTypeConfiguration;

    // 0x7d1d8c4 - Instances per block for the component named `typeName`: the first
    // matching override, else the default; never less than 1.
    int GetComponentBlockCapacity(const std::string& typeName) const {
        int capacity = DefaultComponentBlockCapacity;
        for (const ComponentBufferConfig& entry : ComponentTypeConfiguration) {
            // 0x7d1da2c (the lambda comparing names)
            if (entry.Name == typeName) {
                capacity = entry.ComponentBlockCapacity;
                break;
            }
        }
        return capacity > 1 ? capacity : 1;
    }
};

} // namespace Core
} // namespace Quantum
