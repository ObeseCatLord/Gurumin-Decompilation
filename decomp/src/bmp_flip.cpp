extern "C" int FUN_10161200(unsigned char *pixels, int width, int height)
{
    unsigned char *top = pixels;
    unsigned char *bottom = pixels + (height - 1) * width * 3;
    for (int y = 0; y < height / 2; y++)
    {
        for (int x = 0; x < width * 3; x += 3)
        {
            unsigned char g = top[x + 1];
            unsigned char b = top[x];
            unsigned char r = top[x + 2];
            top[x] = bottom[x];
            top[x + 1] = bottom[x + 1];
            top[x + 2] = bottom[x + 2];
            bottom[x] = b;
            bottom[x + 1] = g;
            bottom[x + 2] = r;
        }
        top += width * 3;
        bottom -= width * 3;
    }
    return 1;
}
