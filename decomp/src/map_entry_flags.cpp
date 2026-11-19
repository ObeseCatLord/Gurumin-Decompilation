// Script flags 0x400, 0x418, 0x43c, 0x453 and 0x454 hold the settings of the map being entered; this table sets
// them for each map number.

extern "C" int DAT_10c62edc[0x1000];      // script flags

// 0x10021380 (Map_SetEntryFlags). Sets script flags 0x400, 0x418, 0x43c, 0x453 and 0x454 for map `map`.
// Returns 1, or 0 for a map without entry flags (only flag 0x454 is cleared then).
extern "C" int FUN_10021380(int map)
{
    switch (map)
    {
    case 0x15:
        DAT_10c62edc[0x400] = 0;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8f0;
        DAT_10c62edc[0x454] = 0;
        return 1;
    case 0x16:
        DAT_10c62edc[0x400] = 0;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8f1;
        DAT_10c62edc[0x454] = 0;
        return 1;
    case 0x17:
        DAT_10c62edc[0x400] = 0;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8f2;
        DAT_10c62edc[0x454] = 0;
        return 1;
    case 0x18:
        DAT_10c62edc[0x400] = 0;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8f3;
        DAT_10c62edc[0x454] = 0;
        return 1;
    case 0x19:
        DAT_10c62edc[0x400] = 0;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8f5;
        DAT_10c62edc[0x454] = 0;
        return 1;
    case 0x1a:
        DAT_10c62edc[0x400] = 0;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8f6;
        DAT_10c62edc[0x454] = 0;
        return 1;
    case 0x1b:
        DAT_10c62edc[0x400] = 0;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8f7;
        DAT_10c62edc[0x454] = 0;
        return 1;
    case 0x66:
        DAT_10c62edc[0x400] = 0x24;
        DAT_10c62edc[0x418] = 3;
        DAT_10c62edc[0x43c] = 0x37;
        DAT_10c62edc[0x453] = 0x8d4;
        DAT_10c62edc[0x454] = 0x4b0;
        return 1;
    case 0x6f:
        DAT_10c62edc[0x400] = 0x1e;
        DAT_10c62edc[0x418] = 1;
        DAT_10c62edc[0x43c] = 0x32;
        DAT_10c62edc[0x453] = 0x8ee;
        DAT_10c62edc[0x454] = 0x4e2;
        return 1;
    case 0x69:
        DAT_10c62edc[0x400] = 1;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8d5;
        DAT_10c62edc[0x454] = 0x4b2;
        return 1;
    case 0xc9:
        DAT_10c62edc[0x400] = 0x19;
        DAT_10c62edc[0x418] = 3;
        DAT_10c62edc[0x43c] = 0x21;
        DAT_10c62edc[0x453] = 0x8d8;
        DAT_10c62edc[0x454] = 0x4ba;
        return 1;
    case 0xcb:
        DAT_10c62edc[0x400] = 0x3a;
        DAT_10c62edc[0x418] = 2;
        DAT_10c62edc[0x43c] = 0x30;
        DAT_10c62edc[0x453] = 0x8e1;
        DAT_10c62edc[0x454] = 0x4bc;
        return 1;
    case 0xce:
        DAT_10c62edc[0x400] = 1;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8d9;
        DAT_10c62edc[0x454] = 0x4be;
        return 1;
    case 0x12d:
        DAT_10c62edc[0x400] = 0x1b;
        DAT_10c62edc[0x418] = 5;
        DAT_10c62edc[0x43c] = 0x12;
        DAT_10c62edc[0x453] = 0x8dc;
        DAT_10c62edc[0x454] = 0x4c6;
        return 1;
    case 0x130:
        DAT_10c62edc[0x400] = 0x2b;
        DAT_10c62edc[0x418] = 2;
        DAT_10c62edc[0x43c] = 0x12;
        DAT_10c62edc[0x453] = 0x8dd;
        DAT_10c62edc[0x454] = 0x4c8;
        return 1;
    case 0x1f5:
        DAT_10c62edc[0x400] = 0x18;
        DAT_10c62edc[0x418] = 1;
        DAT_10c62edc[0x43c] = 0x1b;
        DAT_10c62edc[0x453] = 0x8d6;
        DAT_10c62edc[0x454] = 0x4b4;
        return 1;
    case 0x133:
        DAT_10c62edc[0x400] = 0;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8de;
        DAT_10c62edc[0x454] = 0;
        return 1;
    case 0x132:
        DAT_10c62edc[0x400] = 2;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8f4;
        DAT_10c62edc[0x454] = 0x4ee;
        return 1;
    case 0x1f7:
        DAT_10c62edc[0x400] = 0x1e;
        DAT_10c62edc[0x418] = 1;
        DAT_10c62edc[0x43c] = 0x1c;
        DAT_10c62edc[0x453] = 0x8df;
        DAT_10c62edc[0x454] = 0x4b6;
        return 1;
    case 0x259:
        DAT_10c62edc[0x400] = 0x35;
        DAT_10c62edc[0x418] = 4;
        DAT_10c62edc[0x43c] = 0x25;
        DAT_10c62edc[0x453] = 0x8da;
        DAT_10c62edc[0x454] = 0x4c0;
        return 1;
    case 0x1f9:
        DAT_10c62edc[0x400] = 1;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8d7;
        DAT_10c62edc[0x454] = 0x4b8;
        return 1;
    case 0x25c:
        DAT_10c62edc[0x400] = 0x22;
        DAT_10c62edc[0x418] = 2;
        DAT_10c62edc[0x43c] = 0x2e;
        DAT_10c62edc[0x453] = 0x8e0;
        DAT_10c62edc[0x454] = 0x4c2;
        return 1;
    case 0x44e:
        DAT_10c62edc[0x400] = 0x1c;
        DAT_10c62edc[0x418] = 1;
        DAT_10c62edc[0x43c] = 0x29;
        DAT_10c62edc[0x453] = 0x8e4;
        DAT_10c62edc[0x454] = 0x4cc;
        return 1;
    case 0x3e7:
        DAT_10c62edc[0x400] = 8;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0;
        DAT_10c62edc[0x454] = 0x4de;
        return 1;
    case 0x25e:
        DAT_10c62edc[0x400] = 1;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8db;
        DAT_10c62edc[0x454] = 0x4c4;
        return 1;
    case 0x458:
        DAT_10c62edc[0x400] = 0x1a;
        DAT_10c62edc[0x418] = 3;
        DAT_10c62edc[0x43c] = 0x29;
        DAT_10c62edc[0x453] = 0x8ef;
        DAT_10c62edc[0x454] = 0x4e4;
        return 1;
    case 0x4b5:
        DAT_10c62edc[0x400] = 0x2f;
        DAT_10c62edc[0x418] = 1;
        DAT_10c62edc[0x43c] = 0x21;
        DAT_10c62edc[0x453] = 0x8e7;
        DAT_10c62edc[0x454] = 0x4d2;
        return 1;
    case 0x4b2:
        DAT_10c62edc[0x400] = 0x23;
        DAT_10c62edc[0x418] = 3;
        DAT_10c62edc[0x43c] = 0x1b;
        DAT_10c62edc[0x453] = 0x8e8;
        DAT_10c62edc[0x454] = 0x4d4;
        return 1;
    case 0x516:
        DAT_10c62edc[0x400] = 0x27;
        DAT_10c62edc[0x418] = 7;
        DAT_10c62edc[0x43c] = 9;
        DAT_10c62edc[0x453] = 0x8ec;
        DAT_10c62edc[0x454] = 0x4dc;
        return 1;
    case 0x5de:
        DAT_10c62edc[0x400] = 0x1a;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0x19;
        DAT_10c62edc[0x453] = 0x8e6;
        DAT_10c62edc[0x454] = 0x4d0;
        return 1;
    case 0x51c:
        DAT_10c62edc[0x400] = 1;
        DAT_10c62edc[0x418] = 0;
        DAT_10c62edc[0x43c] = 0;
        DAT_10c62edc[0x453] = 0x8ed;
        DAT_10c62edc[0x454] = 0x4e0;
        return 1;
    case 0x519:
        DAT_10c62edc[0x400] = 0x1b;
        DAT_10c62edc[0x418] = 3;
        DAT_10c62edc[0x43c] = 0xf;
        DAT_10c62edc[0x453] = 0x8eb;
        DAT_10c62edc[0x454] = 0x4da;
        return 1;
    case 0x5e0:
        DAT_10c62edc[0x400] = 0x2e;
        DAT_10c62edc[0x418] = 1;
        DAT_10c62edc[0x43c] = 0x1b;
        DAT_10c62edc[0x453] = 0x8e5;
        DAT_10c62edc[0x454] = 0x4ce;
        return 1;
    case 0x645:
        DAT_10c62edc[0x400] = 0x2e;
        DAT_10c62edc[0x418] = 1;
        DAT_10c62edc[0x43c] = 0x25;
        DAT_10c62edc[0x453] = 0x8e9;
        DAT_10c62edc[0x454] = 0x4d6;
        return 1;
    case 0x643:
        DAT_10c62edc[0x400] = 0x34;
        DAT_10c62edc[0x418] = 5;
        DAT_10c62edc[0x43c] = 0x30;
        DAT_10c62edc[0x453] = 0x8ea;
        DAT_10c62edc[0x454] = 0x4d8;
        return 1;
    }
    DAT_10c62edc[0x454] = 0;
    return 0;
}
