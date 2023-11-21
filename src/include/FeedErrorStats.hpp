#pragma once

#ifndef __FEEDERRORSTATS__
#define __FEEDERRORSTATS__

#include <stdint.h>
#include <stdio.h>

namespace zeus_core
{
   class FeedErrorStats
   {
   public:
      static FeedErrorStats *instance()
      {
         if (instance_ == 0)
         {
            instance_ = new FeedErrorStats();
         }
         return instance_;
      }

      void init() {}

      void duplicateAdd() { ++duplicate_add_; }
      void tradeMissingOrders() { ++trade_missing_orders_; }
      void badCancel() { ++bad_cancels_; }
      void crossedBook() { ++crossed_book_; }
      void corruptMessage() { ++corrupt_messages_; }
      void invalidQuantity() { ++invalid_qtys_; }
      void invalidPrice() { ++invalid_prices_; }
      void invalidID() { ++invalid_ids_; }
      void invalidModify() { ++bad_modifies_; }
      void goodMessage() { ++good_messages_; }

      void printStatistics()
      {
         fprintf(stderr, "\n[Feed Handler Statistics]\n");
         fprintf(stderr, "   %-30s %10u\n", "Corrupt Messages", corrupt_messages_);
         fprintf(stderr, "   %-30s %10u\n", "Good Messages:", good_messages_);
         fprintf(stderr, "   %-30s %10u\n", "Duplicate Adds:", duplicate_add_);
         fprintf(stderr, "   %-30s %10u\n", "Trades Missing Orders:", trade_missing_orders_);
         fprintf(stderr, "   %-30s %10u\n", "Cancels for Missing ID's:", bad_cancels_);
         fprintf(stderr, "   %-30s %10u\n", "Modifies for Missing ID's:", bad_modifies_);
         fprintf(stderr, "   %-30s %10u\n", "Crossed Book:", crossed_book_);
         fprintf(stderr, "   %-30s %10u\n", "Invalid Quantities:", invalid_qtys_);
         fprintf(stderr, "   %-30s %10u\n", "Invalid Prices:", invalid_prices_);
         fprintf(stderr, "   %-30s %10u\n", "Invalid IDs:", invalid_ids_);
      }

      ~FeedErrorStats()
      {
         delete instance_;
      }

   private:
      FeedErrorStats() {}

      static FeedErrorStats *instance_;

      uint32_t duplicate_add_ = 0;
      uint32_t trade_missing_orders_ = 0;
      uint32_t bad_cancels_ = 0;
      uint32_t bad_modifies_ = 0;
      uint32_t crossed_book_ = 0;

      uint32_t corrupt_messages_ = 0;
      uint32_t invalid_qtys_ = 0;
      uint32_t invalid_prices_ = 0;
      uint32_t invalid_ids_ = 0;
      uint32_t good_messages_ = 0;
   };

}

#endif
