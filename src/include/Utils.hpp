#pragma once

#ifndef __UTILS__
#define __UTILS__

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MESSAGELENMIN 5
#define MESSAGELENMAX 36
#define MAXPRICE 100000 * 100

#define FAILASSERT()  \
   {                  \
      assert(0 == 1); \
   }

#define LIKELY(expr) (__builtin_expect(!!(expr), 1))
#define UNLIKELY(expr) (__builtin_expect(!!(expr), 0))

void growBuffer(char *&buf, int &max_buffer)
{
   int old_max = max_buffer;
   max_buffer *= 2;
   buf = (char *)realloc(buf, max_buffer * sizeof(char));
   memset(&buf[old_max], '\0', max_buffer - old_max);
}

void safeCopyToBuffer(char *&dest_root, const char *source, int &index, int &max_buffer)
{
   int len = strlen(source);
   if (len == 0)
      return;

   while (index + len >= max_buffer)
   {
      int old_max = max_buffer;
      max_buffer *= 2;
      dest_root = (char *)realloc(dest_root, max_buffer * sizeof(char));

      memset(&dest_root[old_max], '\0', max_buffer - old_max);
   }

   strncpy(&dest_root[index], source, len);
   index += len;
}

enum MessageType
{
   eMT_Unknown,
   eMT_Add,
   eMT_Remove,
   eMT_Modify,
   eMT_Trade
};

enum Side
{
   eS_Unknown,
   eS_Buy,
   eS_Sell
};

struct OrderLevelEntry
{
   OrderLevelEntry()
       : order_id_(0), order_price_(0), order_qty_(0), order_side_(eS_Unknown), next_(0), previous_(0)
   {
   }

   uint32_t order_id_;
   unsigned long long order_price_;
   uint32_t order_qty_;
   Side order_side_;

   OrderLevelEntry *next_;
   OrderLevelEntry *previous_;
};

struct TradeMessage
{
   uint32_t trade_qty_;
   unsigned long long trade_price_;
};

#endif
