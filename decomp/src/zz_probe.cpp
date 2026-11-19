extern "C" short DAT_10447338[0x44c];
extern "C" int DAT_1032d6a8[0x40];
extern "C" void probe(void)
{
    for (int i = 0; i < 0x44c; i++)
        DAT_10447338[i] = -11;
    for (int j = 0; j < 0x40; j++)
        DAT_1032d6a8[j] = 0;
}
