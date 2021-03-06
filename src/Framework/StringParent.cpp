#include "StdHdr.h"
#include "StringParent.h"
#include "FrameworkUtils.h"
#include "FileUtils.h"

bool StringParent::loadFromFile(FILE * f)
{
  Clear();
  union
  {
    char c[2];
    wchar_t uc;
  } sign;
  size_t read = fread(&sign, 1, sizeof(sign), f);
  bool unicode = (read == 2) && (sign.uc == 0xFEFF || sign.uc == 0xFFFE);
  bool inv = (unicode) && (sign.uc == 0xFFFE);
  const size_t bsize = DEFAULT_SECTOR_SIZE, ssize = DEFAULT_SECTOR_SIZE;
  if (unicode)
  {
    const wchar_t CR = L'\r', LF = L'\n';
    wchar_t buffer[bsize + 1], string[ssize + 1];
    size_t bpos = 0, spos = 0;
    while (1)
    {
      wchar_t oldc = 0;
      read = fread(buffer + bpos, sizeof(wchar_t), bsize - bpos, f);
      if (read == 0)
        break;
      for (size_t Index = 0; Index < read + bpos; Index++)
      {
        if (inv)
          buffer[Index] = ((buffer[Index] & 0x00FF) << 8) | ((buffer[Index] & 0xFF00) >> 8);
        if (buffer[Index] == CR || (buffer[Index] == LF && oldc != CR))
        {
          string[spos] = 0;
          AddString(string);
          spos = 0;
        }
        else
        {
          if (buffer[Index] != CR && buffer[Index] != LF && spos < ssize)
          {
            string[spos++] = buffer[Index];
          }
        }
        oldc = buffer[Index];
      }
      bpos = 0;
    }
    if (spos)
    {
      string[spos] = 0;
      AddString(string);
    }
  }
  else
  {
    const char CR = '\r', LF = '\n';
    char buffer[bsize + 1], string[ssize + 1];
    size_t bpos = 0, spos = 0;
    if (read >= 1)
      buffer[0] = sign.c[0];
    if (read >= 2)
      buffer[1] = sign.c[1];
    bpos = read;
    while (1)
    {
      char oldc = 0;
      read = fread(buffer + bpos, sizeof(char), bsize - bpos, f);
      if (read == 0)
        break;
      for (size_t Index = 0; Index < read + bpos; Index++)
      {
        if (buffer[Index] == CR || (buffer[Index] == LF && oldc != CR))
        {
          string[spos] = 0;
          AddString(String(string));
          spos = 0;
        }
        else
        {
          if (buffer[Index] != CR && buffer[Index] != LF && spos < ssize)
          {
            string[spos++] = buffer[Index];
          }
        }
        oldc = buffer[Index];
      }
      bpos = 0;
    }
    if (spos)
    {
      string[spos] = 0;
      AddString(String(string));
    }
  }
  return true;
}

bool StringParent::loadFromFile(const String & fn)
{
  FILE * f = nullptr;
  _wfopen_s(&f, fn.ptr(), L"rb");
  if (!f)
  {
    return false;
  }
  bool res = loadFromFile(f);
  fclose(f);
  return res;
}

bool StringParent::saveToFile(FILE * f, TextFormat tf) const
{
  if (tf == tfUnicode)
  {
    uint16_t sign = 0xFEFF;
    fwrite(&sign, sizeof(sign), 1, f);
  }
  else if (tf == tfUnicodeBE)
  {
    uint16_t sign = 0xFFFE;
    fwrite(&sign, sizeof(sign), 1, f);
  }
  for (size_t Index = 0; Index < Count(); Index++)
  {
    const String &s = (*this)[Index];
    const int ssize = DEFAULT_SECTOR_SIZE;
    if (tf != tfOEM)
    {
      wchar_t buf[ssize];
      wcscpy_s(buf, ssize, s.c_str());
      if (tf == tfUnicodeBE)
      {
        for (size_t j = 0; j < wcslen(buf); j++)
        {
          buf[j] = ((buf[j] & 0x00FF) << 8) | ((buf[j] & 0xFF00) >> 8);
        }
      }
      fwrite(buf, sizeof(wchar_t), wcslen(buf), f);
      wcscpy_s(buf, ssize, L"\r\n");
      if (tf == tfUnicodeBE)
      {
        for (size_t j = 0; j < wcslen(buf); j++)
        {
          buf[j] = ((buf[j] & 0x00FF) << 8) | ((buf[j] & 0xFF00) >> 8);
        }
      }
      fwrite(buf, sizeof(wchar_t), 2, f);
    }
    else
    {
      std::string buf;

      int sizeRequired = ::WideCharToMultiByte(CP_OEMCP, 0, s.c_str(), (int)s.len(),
        nullptr, 0, nullptr, nullptr);
      if (sizeRequired > 0)
      {
        buf.resize(sizeRequired);
        ::WideCharToMultiByte(CP_OEMCP, 0, s.c_str(), (int)s.len(),
          &(*buf.begin()), sizeRequired, nullptr, nullptr);
      }

      fwrite(buf.c_str(), sizeof(char), buf.size(), f);
      fwrite("\r\n", sizeof(char), 2, f);
    }
  }
  return true;
}

bool StringParent::saveToFile(const String & fn, TextFormat tf)
{
  DWORD attr = ::GetFileAttributes(fn.ptr());
  ::SetFileAttributes(fn.ptr(), FILE_ATTRIBUTE_NORMAL);
  FILE * f = nullptr;
  _wfopen_s(&f, fn.ptr(), L"wb");
  bool res = false;
  if (f)
  {
    res = saveToFile(f, tf);
    fclose(f);
  }
  if (attr != INVALID_FILE_ATTRIBUTES)
  {
    ::SetFileAttributes(fn.ptr(), attr);
  }
  return res;
}

void StringParent::loadFromString(const wchar_t * s, wchar_t delim)
{
  Clear();
  wchar_t * p = (wchar_t *)s;
  wchar_t * pp = p;
  wchar_t buf[DEFAULT_SECTOR_SIZE];
  do
  {
    if (!*p || *p == delim)
    {
      size_t len = __min((size_t)(p - pp), _countof(buf) - 1);
      wcsncpy_s(buf, _countof(buf), pp, len);
      buf[len] = 0;
      pp = p + 1;
      AddString(buf);
    }
  }
  while (*p++);
}

void StringParent::loadFromString(const String & s, wchar_t delim)
{
  loadFromString(s.c_str(), delim);
}
