// Choosing the background movie of the map (JP twin: decomp/src/map_movie_select.cpp).

#include "map_movie.h"
#include "map_records.h"
#include "mmv_player.h"
#include "node.h"
#include "node_data.h"
#include "node_fade.h"
#include "collide_regions.h"
#include "scene_start.h"
#include "texture_mgr.h"
#include "vec4.h"

// Picks the camera point nearest to the player (within 100 units, else record 0x5c) and, when its movie differs from
// the one playing, finishes decoding the old one's first 0x30 frames, opens the new movie, decodes as many frames as
// were shown of the old one (mirrored past 0x30) and renders it into the movie texture. Returns 0. Called by the map
// screen update.
int Map_SelectMovie()
{
    Vec4 point;
    Vec4 player;
    float best = MAP_MOVIE_NEAR_DISTANCE_SQUARED;
    int nearest = -1;
    player.Copy(g_viewerMatrix.GetTranslation());
    for (int i = 0; i < g_mapEntryCount; i++)
    {
        point.Copy(g_markerNode->extra->AnimatedMatrix(g_mapRecords[i].camera.frame).GetTranslation());
        float distance = (player - point).LengthSquared3();
        if (distance < best)
        {
            nearest = i;
            best = distance;
        }
    }
    if (nearest == -1 || best > MAP_MOVIE_NEAR_DISTANCE_SQUARED)
        nearest = MAP_MOVIE_DEFAULT_ENTRY;
    int playing = g_playingMovie;
    int shown = g_movieFramesDecoded;
    g_currentMapEntry = nearest;
    int movie = g_mapRecords[nearest].camera.movie;
    if (movie != playing)
    {
        if (shown != 0 && shown < MAP_MOVIE_FRAMES_FORWARD)
        {
            for (int frame = shown; frame < MAP_MOVIE_FRAMES_FORWARD - 1; frame += 2)
                g_moviePlayer.DecodeNextFrame();
            movie = g_mapRecords[g_currentMapEntry].camera.movie;
            shown = MAP_MOVIE_FRAMES_FORWARD;
            g_movieFramesDecoded = shown;
            playing = movie;
            g_playingMovie = playing;
        }
        if (movie != playing)
        {
            g_playingMovie = movie;
            if (movie >= 0 && movie < MAP_MOVIE_COUNT)
            {
                Movie_PlayToTexture(g_movieNames[movie], 1);
                g_mapIconTextureSlot = Map_IconTexture();
                if (shown != 0)
                    g_movieFramesDecoded = shown;
                if (g_movieFramesDecoded > MAP_MOVIE_FRAMES_FORWARD - 1)
                {
                    g_movieFramesDecoded = MAP_MOVIE_MIRROR_FRAME - g_movieFramesDecoded;
                    if (g_movieFramesDecoded <= 1)
                        g_movieFramesDecoded = 1;
                    for (int frame = 1; frame < g_movieFramesDecoded; frame += 2)
                        g_moviePlayer.DecodeNextFrame();
                    g_moviePlayer.DrawToTexture(&g_textures[g_movieTextureSlot], 1);
                }
            }
        }
    }
    g_mapScreenEntry = g_currentMapEntry;
    return 0;
}
