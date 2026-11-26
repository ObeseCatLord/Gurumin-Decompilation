// Animated model channels and queued push (EN build). Ported from the matched JP source decomp/src/anim_model.cpp.
#include <math.h>
#include "anim_model.h"
#include "node.h"
#include "engine_memory.h"

// Sets track C's two playback options (playing flag and loop flag).
int AnimModel::SetTrackCOptions(int playing, int looping)
{
    trackC.playing = playing;
    trackC.looping = looping;
    return 0;
}

// Sets the motion channel's two playback options.
int AnimModel::SetMotionOptions(int playing, int looping)
{
    motion.playing = playing;
    motion.looping = looping;
    return 0;
}

// Sets track B's two playback options.
int AnimModel::SetTrackBOptions(int playing, int looping)
{
    trackB.playing = playing;
    trackB.looping = looping;
    return 0;
}

// Adds key `value` at `frame` to track B and widens its frame range. Returns 0. No direct caller.
int AnimModel::AddTrackBKey(int frame, float value)
{
    trackB.keys.Set(frame, value, 0, 0, 0);
    if (trackB.last < frame)
        trackB.last = frame;
    if (trackB.first > frame)
        trackB.first = frame;
    return 0;
}

// Points the model's target heading away from the direction (x, y): the angle of the direction plus a half turn (the
// model then turns towards it). Returns 1. Called by the scene script commands that turn a character.
int AnimModel::FaceDirection(float x, float y)
{
    targetHeading = (float)atan2(x, y) + ANGLE_HALF_TURN;
    return 1;
}

// Adds key (x, y, z) at `frame` to track C and widens its frame range.
int AnimModel::AddTrackCKey(int frame, float x, float y, float z)
{
    trackC.keys.Set(frame, x, y, z, 0);
    if (trackC.last < frame + 1)
        trackC.last = frame + 1;
    if (trackC.first > frame)
        trackC.first = frame;
    return 0;
}

// Adds a motion key at `frame` to the motion channel (flagged as a motion frame) and widens its frame range.
int AnimModel::AddMotionKey(int frame, float value)
{
    motion.keys.flags[motion.keys.Set(frame, value, 0, 0, 0)] = KEY_FLAG_MOTION;
    if (motion.last < frame)
        motion.last = frame;
    if (motion.first > frame)
        motion.first = frame;
    return 0;
}

// Queues a push (x, y, z) with `flag` unless the model is locked.
int AnimModel::SetPendingPush(float x, float y, float z, char flag)
{
    if (locked == 0)
    {
        pushFlag = flag;
        pushVector.x = x;
        pushVector.y = y;
        pushVector.z = z;
    }
    return 0;
}

// Clears track C (no keys, first frame after any key so the first key becomes the start).
int AnimModel::ClearTrackC()
{
    trackC.keys.Clear();
    trackC.last = 0;
    trackC.first = CHANNEL_EMPTY_FIRST_FRAME;
    trackC.frame = 0;
    trackC.playing = 0;
    return 0;
}

// Clears the motion channel.
int AnimModel::ClearMotionKeys()
{
    motion.keys.Clear();
    motion.last = 0;
    motion.first = CHANNEL_EMPTY_FIRST_FRAME;
    motion.frame = 0;
    motion.playing = 0;
    return 0;
}

// Clears track B.
int AnimModel::ClearTrackB()
{
    trackB.keys.Clear();
    trackB.last = 0;
    trackB.first = CHANNEL_EMPTY_FIRST_FRAME;
    trackB.frame = 0;
    trackB.playing = 0;
    return 0;
}

// AnimModel::ApplyMaterialSwap: applies material swap `count` to both swap nodes (key -1 = every subset). Returns 1.
int AnimModel::ApplyMaterialSwap(int count)
{
    if (count > 0)
    {
        if (swapNode1 != 0)
            swapNode1->ReplaceMaterial(-1, count);
        if (swapNode2 != 0)
            swapNode2->ReplaceMaterial(-1, count);
    }
    return 1;
}

// AnimModel::SetMoveStep: sets the model's movement for this frame: x and y scaled by the model's speed scale, z (the
// vertical step) as it is. Called by flying enemies.
int AnimModel::SetMoveStep(float x, float y, float z)
{
    moveStep.x = x * speedScale;
    moveStep.z = z;
    moveStep.y = y * speedScale;
    return 0;
}

// AnimModel::IsPlaybackAtEnd: 1 when the motion playback has reached its end (creates the playback on first use).
int AnimModel::IsPlaybackAtEnd()
{
    if (playback == 0)
    {
        playback = (Playback *)g_engineAlloc(PLAYBACK_SIZE, "");
        playback->Init();
        playback->BindModel(this);
    }
    return playback->endState == PLAYBACK_END_REACHED;
}

// AnimModel::SetLayer: starts layer `layer` on the motion playback with the frame range first..last (creates the
// playback on first use). Returns 1.
int AnimModel::SetLayer(int layer, int a, int first, int last, int b, int c)
{
    if (playback == 0)
    {
        playback = (Playback *)g_engineAlloc(PLAYBACK_SIZE, "");
        playback->Init();
        playback->BindModel(this);
    }
    playback->SetLayer(layer, a, first, last, b, c);
    return 1;
}

// AnimModel::GetPlaybackLayer: the layer the motion playback plays now (creates the playback on first use).
int AnimModel::GetPlaybackLayer()
{
    if (playback == 0)
    {
        playback = (Playback *)g_engineAlloc(PLAYBACK_SIZE, "");
        playback->Init();
        playback->BindModel(this);
    }
    return playback->layer;
}

// AnimModel::PlayMotion: starts motion `layer` on the playback (creates the playback on first use). `a` and `b` are
// the playback options (loop mode, blend frames). A model flagged as important raises g_importantMotionChange while
// the motion starts, so blending and playback code can tell.
void AnimModel::PlayMotion(int layer, int a, int b, float speed)
{
    if (playback == 0)
    {
        playback = (Playback *)g_engineAlloc(PLAYBACK_SIZE, "");
        playback->Init();
        playback->BindModel(this);
    }
    if (important != 0)
        g_importantMotionChange = 1;
    playback->Play(layer, a, b, speed);
    g_importantMotionChange = 0;
}

// Name searched for the biped root when a model file is loaded.
static const char BIPED_ROOT_NAME[] = "Bip01";

// AnimModel::LoadModel: loads model file `path` into this model's existing node tree. Counts the loads (up to 3),
// looks up the biped root (the result is unused in the shipped code), clears the event camera height and rebinds the
// motion playback to the new model. Returns 0 when the model has no node tree yet, else 1.
int AnimModel::LoadModel(const char *path)
{
    if (root == 0)
        return 0;
    root->PreScanFile(path, 0);
    if (loadCount < 3)
        loadCount++;
    root->Find(BIPED_ROOT_NAME, 0);
    Vec4 unused1;
    Vec4 unused2;
    eventCameraHeight = 0;
    if (playback != 0)
        playback->BindModel(this);
    return 1;
}

// AnimModel::UpdateCollision: releases the saved matrix states of the node tree, clears the collision result and
// runs the proximity pass over the tree with the model's world matrix. Returns 0. Called by the model update.
int AnimModel::UpdateCollision()
{
    Matrix4 work;
    if (root != 0)
        root->ReleaseSavedStates();
    collisionResult = 0;
    root->StartProximityPass(matrix.m, &work, 0, motionFrame, 0);
    return 0;
}

// Largest slot and layer numbers SetSlotSource accepts.
const int PLAYBACK_LAST_SLOT_SOURCE = 0xf;
const int PLAYBACK_LAST_SOURCE_LAYER = 0x8f;

// AnimModel::SetSlotSource: sets the motion source and layer of playback slot `slot` (creates the playback on first
// use). Slots above 15, layers above 0x8f and an empty source are ignored. Returns 1.
int AnimModel::SetSlotSource(void *source, int slot, int layer)
{
    if (playback == 0)
    {
        playback = (Playback *)g_engineAlloc(PLAYBACK_SIZE, "");
        playback->Init();
        playback->BindModel(this);
    }
    Playback *state = playback;
    if (layer <= PLAYBACK_LAST_SOURCE_LAYER && slot <= PLAYBACK_LAST_SLOT_SOURCE && source != 0)
    {
        state->slotSource[slot] = source;
        state->slotLayer[slot] = layer;
    }
    return 1;
}

// AnimModel::SetFollowUp: sets follow-up entry `index` of layer `layer` of the motion playback (creates the playback on
// first use). Layers above PLAYBACK_LAYER_COUNT - 1 and entries above PLAYBACK_FOLLOWUP_COUNT - 1 are ignored. Returns 1.
// Called by the scene script command DIS.
int AnimModel::SetFollowUp(int layer, int index, int value)
{
    if (playback == 0)
    {
        playback = (Playback *)g_engineAlloc(PLAYBACK_SIZE, "");
        playback->Init();
        playback->BindModel(this);
    }
    if (layer <= PLAYBACK_LAYER_COUNT - 1 && index <= PLAYBACK_FOLLOWUP_COUNT - 1)
        playback->layerParams[layer][index] = value;
    return 1;
}

// Adds key (x, y, z) at `frame` to track C with key flag `flag` and widens its frame range.
int AnimModel::AddTrackCKeyFlagged(int frame, float x, float y, float z, char flag)
{
    trackC.keys.flags[trackC.keys.Set(frame, x, y, z, 0)] = flag;
    if (trackC.last < frame + 1)
        trackC.last = frame + 1;
    if (trackC.first > frame)
        trackC.first = frame;
    return 0;
}

// Advances the motion channel by one frame while it plays: samples the motion frame for the current channel frame,
// shows it (also on the playback object) and moves on. At the end the channel loops back to its first frame, or stops
// on the last one. Returns 0.
int AnimModel::StepMotion()
{
    if (motion.playing != 0)
    {
        float frameValue, y, z, w;
        motion.keys.Sample(motion.frame, &frameValue, &y, &z, &w, 0);
        motionFrame = (int)frameValue;
        if (playback != 0)
            playback->frame = motionFrame;
        if (++motion.frame >= motion.last)
        {
            if (motion.looping != 0)
            {
                motion.frame = motion.first;
                return 0;
            }
            motion.playing = 0;
            motion.frame--;
        }
    }
    return 0;
}
