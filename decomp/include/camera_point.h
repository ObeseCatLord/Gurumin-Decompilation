// CameraPoint: declared here once (first used by src/map_movie_select.cpp).
#ifndef GURUMIN_CAMERA_POINT_H
#define GURUMIN_CAMERA_POINT_H

// A camera point of the map (0x100 bytes): the frame of the camera path at which it lies and its movie.
struct CameraPoint
{
    int frame;
    int m_0004;
    int movie;                                       // 0x08: movie number (index into DAT_10875e48), -1 = none
    char pad_000c[0xf4];
};

#endif
