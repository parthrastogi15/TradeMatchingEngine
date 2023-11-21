#pragma once

#ifndef __MARKETDATAHANDLER__
#define __MARKETDATAHANDLER__

#include <string>
#include <vector>

#include "HFTimestamp.hpp"
#include "PerfMetrics.hpp"
#include "Book.hpp"
#include "Parser.hpp"

#ifdef ENABLE_PROFILING
#define START()       \
   {                  \
      timer_.start(); \
   }
#define STOP(y)             \
   {                        \
      y.add(timer_.stop()); \
   }
#else
#define START()
#define STOP(y)
#endif

namespace zeus_core
{
   template <typename ORDERIDTYPE, typename ORDERTYPE>
   class MarketDataHandler
   {
   public:
#ifdef ENABLE_PROFILING
      MarketDataHandler()
          : order_book_(), parser_(), timer_(), add_("AddOrder"), modify_("ModifyOrder"), remove_("RemoveOrder"), trade_("Trade"), midquote_("MidQuote Print"), book_print_("Book Print")
      {
      }

      ~MarketDataHandler()
      {
         order_book_.getLoggerReference().stopLogger();

         add_.print();
         modify_.print();
         remove_.print();
         trade_.print();
         midquote_.print();
         book_print_.print();
      }
#endif

      void processMessage(char *line)
      {
         MessageType mt = parser_.getMessageType(line);
         bool valid_message = false;
         if (mt == eMT_Unknown)
         {
            FeedErrorStats::instance()->corruptMessage();
            return;
         }
         else if (mt == eMT_Trade)
         {
            START();
            TradeMessage tm;
            parser_.parseTrade(line, tm);
            if (tm.trade_price_ != 0)
            {
               valid_message = true;
               order_book_.handleTrade(tm);
            }
            STOP(trade_);
         }
         else
         {
            START();
            ORDERTYPE *ole = new ORDERTYPE();
            parser_.parseOrder(line, *ole);
            if (ole->order_side_ == eS_Unknown)
            {
               delete ole;
            }
            else
            {
               switch (mt)
               {
               case eMT_Add:
                  order_book_.addOrder(ole);
                  STOP(add_);
                  valid_message = true;
                  break;
               case eMT_Modify:
                  order_book_.modifyOrder(ole);
                  STOP(modify_);
                  valid_message = true;
                  break;
               case eMT_Remove:
                  order_book_.removeOrder(ole);
                  STOP(remove_);
                  valid_message = true;
                  break;
               default:
                  fprintf(stderr, "Unknown order type hit in switch.  This should not happen.\n");
                  delete ole;
                  FAILASSERT();
                  break;
               }
            }
         }
         if (valid_message)
         {
            START();
            order_book_.printMidpoint();
            STOP(midquote_);
         }
      }

      void printCurrentOrderBook()
      {
         START()
         order_book_.printBook();
         STOP(book_print_);
      }

   private:
      Book<ORDERIDTYPE, ORDERTYPE> order_book_;
      Parser parser_;

#ifdef ENABLE_PROFILING
      HFTimestamp timer_;
      PerfMetrics add_;
      PerfMetrics modify_;
      PerfMetrics remove_;
      PerfMetrics trade_;
      PerfMetrics midquote_;
      PerfMetrics book_print_;
#endif
   };

}

#endif
