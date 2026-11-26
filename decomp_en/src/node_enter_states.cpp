// Node saved-state entry points (EN build, EN-only: no JP twin found). Each one leaves the other saved states, resets
// the saved block and sets its own state bit; EnterTargetState (scene_node.cpp) is the fourth of the family.
#include "node.h"
#include "node_data.h"

// Node::EnterCollisionSphereState: enters the collision-sphere state (NODE_FLAG_COLLISION_SPHERE) when the node has a
// saved block and is not in it yet: leaves the chasing, two-bone and target-tracking states first. Returns 1.
// Called by the node loading code.
int Node::EnterCollisionSphereState()
{
    if (saved != 0 && (flags & NODE_FLAG_COLLISION_SPHERE) == 0)
    {
        RestoreState1000();
        RestoreState2000();
        if (saved != 0 && (flags & NODE_FLAG_COLLISION_SPHERE) != 0)
        {
            flags &= ~NODE_FLAG_COLLISION_SPHERE;
            ReleaseSavedStates();
        }
        RestoreState400000();
        saved->Reset();
        flags |= NODE_FLAG_COLLISION_SPHERE;
    }
    return 1;
}

// Node::EnterTwoBoneState: the same for the two-bone limb state (NODE_FLAG_TWO_BONE_LIMB). Returns 1. Called by the
// model code.
int Node::EnterTwoBoneState()
{
    if (saved != 0 && (flags & NODE_FLAG_TWO_BONE_LIMB) == 0)
    {
        RestoreState1000();
        RestoreState2000();
        if (saved != 0 && (flags & NODE_FLAG_COLLISION_SPHERE) != 0)
        {
            flags &= ~NODE_FLAG_COLLISION_SPHERE;
            ReleaseSavedStates();
        }
        RestoreState400000();
        saved->Reset();
        flags |= NODE_FLAG_TWO_BONE_LIMB;
    }
    return 1;
}

// Node::EnterChasingState: the same for the chasing state (NODE_FLAG_CHASING). Returns 1.
int Node::EnterChasingState()
{
    if (saved != 0 && (flags & NODE_FLAG_CHASING) == 0)
    {
        RestoreState1000();
        RestoreState2000();
        if (saved != 0 && (flags & NODE_FLAG_COLLISION_SPHERE) != 0)
        {
            flags &= ~NODE_FLAG_COLLISION_SPHERE;
            ReleaseSavedStates();
        }
        RestoreState400000();
        saved->Reset();
        flags |= NODE_FLAG_CHASING;
    }
    return 1;
}
