// CameraPoint: declared here once (map_records.h; first used by src/map_movie_select.cpp).
#ifndef GURUMIN_CAMERA_POINT_H
#define GURUMIN_CAMERA_POINT_H

// A camera point of the map (a 0x100-byte map record): the frame of the camera path at which it lies and its
// movie.
struct CameraPoint
{
    int m_0000;
    int frame;                                       // 0x04: frame of the camera path
    int m_0008;
    int movie;                                       // 0x0c: movie number (index into DAT_10875e48), -1 = none
    char pad_0010[0xf0];
};

#endif
