#pragma once

#ifndef __ORDERBOOK__
#define __ORDERBOOK__

#include <assert.h>
#include <stdio.h>

#include <map>
#include <unordered_map>

#include "OrderDLList.hpp"
#include "FeedErrorStats.hpp"
#include "Logger.hpp"
#include "Parser.hpp"
#include "Utils.hpp"

namespace zeus_core
{
   template <typename ORDERIDTYPE, typename ORDERTYPE>
   class Book
   {
   public:
      Book() : logger_(), recent_trade_price_(0), recent_trade_qty_(0) {}

      ~Book()
      {
         for (auto it = buy_book_map_.begin(); it != buy_book_map_.end(); ++it)
         {
            it->second.clearLevel();
         }
         for (auto it = sell_book_map_.begin(); it != sell_book_map_.end(); ++it)
         {
            it->second.clearLevel();
         }
         buy_book_map_.clear();
         sell_book_map_.clear();
         while (!orders_.empty())
         {
            auto it = orders_.begin();
            delete it->second;
            orders_.erase(it);
         }
      }

      typedef std::map<unsigned long long, CountedOrderList<ORDERTYPE>> OrderListMap;
      typedef std::unordered_map<ORDERIDTYPE, ORDERTYPE *> OrderHash;

      Logger &getLoggerReference()
      {
         return logger_;
      }

      void printMidpoint()
      {
         if (sell_book_map_.empty() || buy_book_map_.empty())
         {
            logger_.print("NAN\n");
            return;
         }

         unsigned long long sell_min = sell_book_map_.begin()->first;
         unsigned long long buy_max = buy_book_map_.rbegin()->first;
         double midpoint = (sell_min + buy_max) / 200.;
         char buffer[20];
         sprintf(buffer, "%.2f\n", midpoint);
         logger_.print(buffer);
      }

      void printBook()
      {
         int max_buffer = 500;
         char *buffer = (char *)calloc(max_buffer, sizeof(char));
         int index = 0;

         for (auto it = sell_book_map_.rbegin(); it != sell_book_map_.rend(); ++it)
         {
            if (it->second.getQuantity() != 0)
            {
               if (index + 100 > max_buffer)
                  growBuffer(buffer, max_buffer);
               index += sprintf(&buffer[index], "%.2f ", it->first / 100.);

               it->second.printLevel('S', buffer, index, max_buffer);
               if (index + 10 > max_buffer)
                  growBuffer(buffer, max_buffer);
               index += sprintf(&buffer[index], "\n");
            }
         }
         safeCopyToBuffer(buffer, "\n", index, max_buffer);
         for (auto it = buy_book_map_.rbegin(); it != buy_book_map_.rend(); ++it)
         {
            if (it->second.getQuantity() != 0)
            {
               if (index + 100 > max_buffer)
                  growBuffer(buffer, max_buffer);
               index += sprintf(&buffer[index], "%.2f ", it->first / 100.);

               it->second.printLevel('B', buffer, index, max_buffer);

               if (index + 10 > max_buffer)
                  growBuffer(buffer, max_buffer);
               index += sprintf(&buffer[index], "\n");
            }
         }
         if (index + 10 > max_buffer)
            growBuffer(buffer, max_buffer);
         index += sprintf(&buffer[index], "\n");

         logger_.print(buffer);
         free(buffer);
      }

      void checkCross() const
      {
         if (sell_book_map_.empty() || buy_book_map_.empty())
         {
            return;
         }
         auto sit = sell_book_map_.begin();
         auto bit = buy_book_map_.end();
         --bit;
         if (sit->first <= bit->first)
         {
            FeedErrorStats::instance()->crossedBook();
         }
      }

      void addOrder(ORDERTYPE *ole)
      {
         checkCross();

         auto ooit = orders_.find(ole->order_id_);
         if (ooit != orders_.end())
         {
            FeedErrorStats::instance()->duplicateAdd();
            delete ole;
            return;
         }

         OrderListMap &map = ole->order_side_ == eS_Buy ? buy_book_map_ : sell_book_map_;

         std::pair<typename OrderListMap::iterator, bool> ret;
         ret = map.insert(
             std::pair<unsigned long long, CountedOrderList<ORDERTYPE>>(
                 ole->order_price_, CountedOrderList<ORDERTYPE>()));
         ret.first->second.addNode(ole);
         orders_[ole->order_id_] = ole;
      }

      void modifyOrder(ORDERTYPE *ole)
      {
         checkCross();

         auto ooit = orders_.find(ole->order_id_);
         if (ooit == orders_.end())
         {
            FeedErrorStats::instance()->invalidModify();
            delete ole;
            return;
         }

         OrderListMap &map = ooit->second->order_side_ == eS_Buy ? buy_book_map_ : sell_book_map_;

         if (ole->order_price_ == ooit->second->order_price_)
         {
            if (ole->order_qty_ <= ooit->second->order_qty_)
            {
               typename OrderListMap::iterator pit = map.find(ooit->second->order_price_);
               pit->second.changeNodeQuantity(ooit->second, ole->order_qty_);
               delete ole;
            }
            else
            {
               typename OrderListMap::iterator pit = map.find(ooit->second->order_price_);
               pit->second.removeNode(ooit->second);
               delete ooit->second;
               orders_.erase(ooit);

               pit->second.addNode(ole);
               orders_[ole->order_id_] = ole;
            }
         }
         else
         {
            typename OrderListMap::iterator opit = map.find(ooit->second->order_price_);
            opit->second.removeNode(ooit->second);
            delete ooit->second;
            orders_.erase(ooit);

            if (opit->second.getQuantity() == 0)
            {
               map.erase(opit);
            }

            std::pair<typename OrderListMap::iterator, bool> ret;
            ret = map.insert(
                std::pair<unsigned long long, CountedOrderList<ORDERTYPE>>(
                    ole->order_price_, CountedOrderList<ORDERTYPE>()));
            ret.first->second.addNode(ole);
            orders_[ole->order_id_] = ole;
         }
      }

      void removeOrder(ORDERTYPE *ole)
      {
         checkCross();
         auto ooit = orders_.find(ole->order_id_);
         if (ooit == orders_.end())
         {
            FeedErrorStats::instance()->badCancel();
            delete ole;
            return;
         }

         OrderListMap &map = ooit->second->order_side_ == eS_Buy ? buy_book_map_ : sell_book_map_;

         typename OrderListMap::iterator pit = map.find(ooit->second->order_price_);
         if (pit == map.end())
         {
            fprintf(stderr, "Cancel for order on price level (%llu) not found.\n", ooit->second->order_price_);
            delete ole;
            return;
         }
         pit->second.removeNode(ooit->second);
         delete ooit->second;
         orders_.erase(ooit);
         delete ole;

         if (pit->second.getQuantity() == 0)
         {
            map.erase(pit);
         }
      }

      void handleTrade(TradeMessage &tm)
      {
         if (buy_book_map_.empty() || sell_book_map_.empty())
         {
            FeedErrorStats::instance()->tradeMissingOrders();
            return;
         }
         auto bit = buy_book_map_.end();
         --bit;
         if (bit->first < tm.trade_price_)
         {
            FeedErrorStats::instance()->tradeMissingOrders();
            return;
         }
         auto sit = sell_book_map_.find(tm.trade_price_);
         if (sit == sell_book_map_.end())
         {
            FeedErrorStats::instance()->tradeMissingOrders();
            return;
         }

         if (bit->second.getQuantity() < tm.trade_qty_ ||
             sit->second.getQuantity() < tm.trade_qty_)
         {
            FeedErrorStats::instance()->tradeMissingOrders();
            return;
         }

         uint32_t trade_qty = tm.trade_qty_;
         while (trade_qty > 0)
         {
            ORDERTYPE *tail = bit->second.getTail();
            if (tail->order_qty_ > trade_qty)
            {
               trade_qty = 0;
            }
            else
            {
               trade_qty -= tail->order_qty_;
            }

            if (tm.trade_qty_ >= tail->order_qty_)
            {
               bit->second.removeNode(tail);
               auto odit = orders_.find(tail->order_id_);
               delete odit->second;
               orders_.erase(odit);
            }
            else
            {
               bit->second.changeNodeQuantity(tail, tail->order_qty_ - tm.trade_qty_);
            }
         }
         if (bit->second.getQuantity() == 0)
         {
            buy_book_map_.erase(bit);
         }

         trade_qty = tm.trade_qty_;
         while (trade_qty > 0)
         {
            ORDERTYPE *tail = sit->second.getTail();
            if (tail->order_qty_ > trade_qty)
            {
               trade_qty = 0;
            }
            else
            {
               trade_qty -= tail->order_qty_;
            }

            if (tm.trade_qty_ >= tail->order_qty_)
            {
               sit->second.removeNode(tail);
               auto odit = orders_.find(tail->order_id_);
               delete odit->second;
               orders_.erase(odit);
            }
            else
            {
               sit->second.changeNodeQuantity(tail, tail->order_qty_ - tm.trade_qty_);
            }
         }
         if (sit->second.getQuantity() == 0)
         {
            sell_book_map_.erase(sit);
         }

         if (tm.trade_price_ != recent_trade_price_)
         {
            recent_trade_price_ = tm.trade_price_;
            recent_trade_qty_ = 0;
         }
         recent_trade_qty_ += tm.trade_qty_;

         char trade_buffer[25];
         sprintf(trade_buffer, "%u@%.2f\n", recent_trade_qty_, recent_trade_price_ / 100.);
         logger_.print(trade_buffer);

         checkCross();
      }

   private:
      Logger logger_;

      OrderListMap buy_book_map_;
      OrderListMap sell_book_map_;

      OrderHash orders_;

      unsigned long long recent_trade_price_;
      uint32_t recent_trade_qty_;
   };

}

#endif
