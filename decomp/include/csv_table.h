// CsvTable: declared here once (first used by src/text_fields.cpp).
#ifndef GURUMIN_CSV_TABLE_H
#define GURUMIN_CSV_TABLE_H

// Holds 256-byte text lines at offset 0x50d30 (loaded from a comma-separated data file).
struct CsvTable
{
    char pad_0000[0x50d30];
    char lines[1][0x100];
    int M_101672c0(int line, int field, char *out);
};

#endif
