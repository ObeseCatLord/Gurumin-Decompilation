// Tests of where two segments meet (EN build, EN-only: no JP twin found).
#include "vec4.h"

// How close, per axis, the meeting point has to be to an end point for that end point to count as the meeting point.
const float SEGMENT_END_TOLERANCE = 0.018f;

// Works out where the line through `first` and `second` meets the line through `third` and `fourth`; the point goes to
// `meeting`. Returns non-zero when they meet. EN 0x775700 (not ported yet).
extern "C" int Segments_MeetingPoint(Vec4 *meeting, const Vec4 *first, const Vec4 *second, const Vec4 *third,
                                     const Vec4 *fourth);

// Segments_MeetAwayFromEnds: 1 when the segments first-second and third-fourth meet and the meeting point is not (within
// SEGMENT_END_TOLERANCE on every axis) the point `first` when `excludeFirst` is set, nor the point `second` when
// `excludeSecond` is set; 0 otherwise. No direct caller recovered.
extern "C" int Segments_MeetAwayFromEnds(const Vec4 *first, const Vec4 *second, const Vec4 *third, const Vec4 *fourth,
                                         int excludeFirst, int excludeSecond)
{
    Vec4 meeting;
    if (Segments_MeetingPoint(&meeting, first, second, third, fourth) != 0)
    {
        bool atFirst = excludeFirst != 0 && first->x >= meeting.x - SEGMENT_END_TOLERANCE &&
                       meeting.x + SEGMENT_END_TOLERANCE >= first->x && first->y >= meeting.y - SEGMENT_END_TOLERANCE &&
                       meeting.y + SEGMENT_END_TOLERANCE >= first->y && first->z >= meeting.z - SEGMENT_END_TOLERANCE &&
                       meeting.z + SEGMENT_END_TOLERANCE >= first->z;
        bool atSecond = excludeSecond != 0 && second->x >= meeting.x - SEGMENT_END_TOLERANCE &&
                        meeting.x + SEGMENT_END_TOLERANCE >= second->x &&
                        second->y >= meeting.y - SEGMENT_END_TOLERANCE &&
                        meeting.y + SEGMENT_END_TOLERANCE >= second->y &&
                        second->z >= meeting.z - SEGMENT_END_TOLERANCE && meeting.z + SEGMENT_END_TOLERANCE >= second->z;
        if (!atFirst && !atSecond)
            return 1;
    }
    return 0;
}
