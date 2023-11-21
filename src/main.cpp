#pragma once

#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <fstream>

#include "include/MarketDataHandler.hpp"
#include "include/FeedErrorStats.hpp"
#include "include/HFTimestamp.hpp"
#include "include/PerfMetrics.hpp"
#include "include/Utils.hpp"

using namespace zeus_core;

int main(int argc, char **argv)
{
   if (argc == 1)
   {
      std::cout << "Usage: OrderBookProcessor <filename>" << std::endl;
      return -1;
   }

   FeedErrorStats::instance()->init();

   MarketDataHandler<uint32_t, OrderLevelEntry> feed;
   const std::string filename(argv[1]);

   FILE *pFile;
   try
   {
      pFile = fopen(filename.c_str(), "r");
      if (pFile == NULL)
      {
         throw;
      }
   }
   catch (...)
   {
      std::cout << "Error occured opening file: " << filename << ".  Please check the file and try again." << std::endl;
      return -1;
   }

   uint32_t counter = 0;
   size_t len;
   char *buffer = NULL;
   while (!feof(pFile))
   {
      while (1)
      {
         ssize_t read = getline(&buffer, &len, pFile);
         if (read == -1)
            break;

         feed.processMessage(buffer);

         if (buffer)
            free(buffer);
         buffer = 0;

         ++counter;
         if (counter % 10 == 0)
         {
            feed.printCurrentOrderBook();
         }
      }
   }
   fclose(pFile);

   FeedErrorStats::instance()->printStatistics();
   return 0;
}
