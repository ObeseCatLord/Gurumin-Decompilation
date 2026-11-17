#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ListBank
{
    char pad_0000[0xc];
    char m_000c[8][0x80][0x10];
    char m_400c[8][0x80];
    int m_440c[8][0x100];
    char pad_640c[0x8400];
    char m_e80c[8][0x80];
    char pad_ec0c[0x1b9d0];
    int m_2a5dc[8];
    char pad_2a5fc[0xbd30];
    int m_3632c;
    int pad_36330;
    char m_36334[0x10][0x20];

    int M_10004680(int group);
    int M_100046d0(const char *text, int group, int value, char flag);
    int M_100048f0(const char *name, int *index);
    int M_10004950(char *buffer, void *value, char type);
    int M_100049e0(const char *text);
    int M_10004a30(const char *text, int *value);
    int M_10004aa0(const char *text, float *value);
};

int ListBank::M_10004680(int group)
{
    m_2a5dc[group] = 0;
    for (int i = 0; i < 0x80; i++)
    {
        if (m_e80c[group][i] == 0)
        {
            m_440c[group][i] = 0;
            m_e80c[group][i] = 0;
        }
    }
    return 1;
}

int ListBank::M_100046d0(const char *text, int group, int value, char flag)
{
    m_e80c[group][m_2a5dc[group]] = 1;
    strcpy(m_000c[group][m_2a5dc[group]], text);
    m_440c[group][m_2a5dc[group]] = value;
    m_400c[group][m_2a5dc[group]] = flag;
    m_2a5dc[group]++;
    return 1;
}

int ListBank::M_100048f0(const char *name, int *index)
{
    for (int i = 0; i < 0x10; i++)
    {
        if (m_36334[i][0] != 0 && strncmp(name, m_36334[i], strlen(m_36334[i])) == 0)
        {
            *index = i;
            return 1;
        }
    }
    return 0;
}

int ListBank::M_10004950(char *buffer, void *value, char type)
{
    if (type == 0)
        sprintf(buffer, "%d", *(int *)value);
    else if (type == 1)
        sprintf(buffer, "%f", *(float *)value);
    else if (type == 2)
        sprintf(buffer, "%s", (char *)value);
    buffer[0xff] = 0;
    return 1;
}

int ListBank::M_100049e0(const char *text)
{
    int length = strlen(text);
    for (int i = 0; i < length; i++)
    {
        if (m_3632c != 0)
            return 0;
        if (text[i] == '(' || text[i] == ')')
            return 1;
    }
    return 0;
}

int ListBank::M_10004a30(const char *text, int *value)
{
    int length = strlen(text);
    for (int i = 0; i < length; i++)
    {
        if (m_3632c != 0)
            return 0;
        if (text[i] != '+' && text[i] != '-' && (text[i] < '0' || text[i] > '9'))
            return 0;
    }
    *value = atol(text);
    return 1;
}

int ListBank::M_10004aa0(const char *text, float *value)
{
    int length = strlen(text);
    for (int i = 0; i < length; i++)
    {
        if (m_3632c != 0)
            return 0;
        if (text[i] != '+' && text[i] != '-' && text[i] != '.' && (text[i] < '0' || text[i] > '9'))
            return 0;
    }
    *value = (float)atof(text);
    return 1;
}
