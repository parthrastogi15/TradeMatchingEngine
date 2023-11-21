#pragma once

#ifndef __PARSER__
#define __PARSER__

#include <stdint.h>

#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_real.hpp>

#include "FeedErrorStats.hpp"
#include "Utils.hpp"

using namespace boost::spirit;

namespace zeus_core
{
   enum ParseStatus
   {
      ePS_Good,
      ePS_CorruptMessage,
      ePS_BadQuantity,
      ePS_BadPrice,
      ePS_BadID,
      ePS_GenericBadValue
   };

   class Parser
   {
   public:
      Parser()
      {
      }
      ~Parser() {}

      inline MessageType getMessageType(char *message);

      inline void parseOrder(char *tk_msg, OrderLevelEntry &ole);
      inline void parseTrade(char *tk_msg, TradeMessage &tm);

   private:
      inline ParseStatus tokenizeAndConvertToUint(char *tk_msg, uint32_t &dest);
      inline ParseStatus tokenizeAndConvertToDouble(char *tk_msg, double &dest);

      inline void reportStatus(ParseStatus status);
      inline void failOrderParse(OrderLevelEntry &ole, ParseStatus status);
      inline void failTradeParse(TradeMessage &tm, ParseStatus status);
   };

   inline MessageType Parser::getMessageType(char *tk_msg)
   {
      uint32_t len = strlen(tk_msg);
      if (len == 0 || len > MESSAGELENMAX)
      {
         FeedErrorStats::instance()->corruptMessage();
         return eMT_Unknown;
      }

      tk_msg = strtok(tk_msg, ",");
      switch (tk_msg[0])
      {
      case 'A':
         return eMT_Add;
         break;
      case 'M':
         return eMT_Modify;
         break;
      case 'X':
         return eMT_Remove;
         break;
      case 'T':
         return eMT_Trade;
         break;
      default:
         return eMT_Unknown;
         break;
      }
   }

   inline ParseStatus Parser::tokenizeAndConvertToUint(char *tk_msg, uint32_t &dest)
   {
      tk_msg = strtok(NULL, ",");
      if (tk_msg == NULL)
      {
         return ePS_CorruptMessage;
      }
      else if (tk_msg[0] == '-')
      {
         return ePS_GenericBadValue;
      }
      else if (!qi::parse(tk_msg, &tk_msg[strlen(tk_msg)], uint_, dest))
      {
         return ePS_GenericBadValue;
      }
      return ePS_Good;
   }

   inline ParseStatus Parser::tokenizeAndConvertToDouble(char *tk_msg, double &dest)
   {
      tk_msg = strtok(NULL, ",");
      if (tk_msg == NULL)
      {
         return ePS_CorruptMessage;
      }
      else if (tk_msg[0] == '-')
      {
         return ePS_GenericBadValue;
      }
      else if (!qi::parse(tk_msg, &tk_msg[strlen(tk_msg)], double_, dest))
      {
         return ePS_GenericBadValue;
      }
      return ePS_Good;
   }

   inline void Parser::reportStatus(ParseStatus status)
   {
      switch (status)
      {
      case ePS_Good:
         FeedErrorStats::instance()->goodMessage();
         break;
      case ePS_CorruptMessage:
         FeedErrorStats::instance()->corruptMessage();
         break;
      case ePS_BadQuantity:
         FeedErrorStats::instance()->invalidQuantity();
         break;
      case ePS_BadPrice:
         FeedErrorStats::instance()->invalidPrice();
         break;
      case ePS_BadID:
         FeedErrorStats::instance()->invalidID();
         break;
      default:
         fprintf(stderr, "Unknown parsing error occurred.  Skipping report of error.\n");
         FAILASSERT();
         break;
      }
   }

   inline void Parser::failOrderParse(OrderLevelEntry &ole, ParseStatus status)
   {
      ole.order_side_ = eS_Unknown;
      return reportStatus(status);
   }

   inline void Parser::parseOrder(char *tk_msg, OrderLevelEntry &ole)
   {
      ParseStatus result = tokenizeAndConvertToUint(tk_msg, ole.order_id_);
      if (result != ePS_Good)
      {
         if (result == ePS_CorruptMessage)
            return failOrderParse(ole, result);
         return failOrderParse(ole, ePS_BadID);
      }

      tk_msg = strtok(NULL, ",");
      if (tk_msg == NULL)
      {
         return failOrderParse(ole, ePS_CorruptMessage);
      }
      else
      {
         switch (tk_msg[0])
         {
         case 'B':
            ole.order_side_ = eS_Buy;
            break;
         case 'S':
            ole.order_side_ = eS_Sell;
            break;
         default:
            return failOrderParse(ole, ePS_CorruptMessage);
            break;
         }
      }

      result = tokenizeAndConvertToUint(tk_msg, ole.order_qty_);
      if (result != ePS_Good)
      {
         if (result == ePS_CorruptMessage)
            return failOrderParse(ole, result);
         return failOrderParse(ole, ePS_BadQuantity);
      }
      if (ole.order_qty_ == 0)
         return failOrderParse(ole, ePS_BadQuantity);

      double price = 0;
      result = tokenizeAndConvertToDouble(tk_msg, price);
      if (result != ePS_Good)
      {
         if (result == ePS_CorruptMessage)
            return failOrderParse(ole, result);
         return failOrderParse(ole, ePS_BadPrice);
      }
      if (static_cast<unsigned long long>(price * 100) == 0)
         return failOrderParse(ole, ePS_BadPrice);
      if (price > ULLONG_MAX)
      {
         return failOrderParse(ole, ePS_BadPrice);
      }

      if ((double)(price * 100) - static_cast<unsigned long long>(price * 100) != 0)
      {
         return failOrderParse(ole, ePS_BadPrice);
      }
      ole.order_price_ = static_cast<unsigned long long>(price * 100);

      FeedErrorStats::instance()->goodMessage();
   }

   inline void Parser::failTradeParse(TradeMessage &tm, ParseStatus status)
   {
      tm.trade_qty_ = 0;
      tm.trade_price_ = 0;
      reportStatus(status);
   }

   inline void Parser::parseTrade(char *tk_msg, TradeMessage &tm)
   {
      ParseStatus result = tokenizeAndConvertToUint(tk_msg, tm.trade_qty_);
      if (result != ePS_Good)
      {
         if (result == ePS_CorruptMessage)
            return failTradeParse(tm, result);
         ;
         return failTradeParse(tm, ePS_BadQuantity);
      }
      if (tm.trade_qty_ == 0)
         return failTradeParse(tm, ePS_BadPrice);

      double price;
      result = tokenizeAndConvertToDouble(tk_msg, price);
      if (result != ePS_Good)
      {
         if (result == ePS_CorruptMessage)
            return failTradeParse(tm, result);
         return failTradeParse(tm, ePS_BadPrice);
      }
      if (price > ULLONG_MAX)
      {
         return failTradeParse(tm, ePS_BadPrice);
      }

      if ((double)(price * 100) - static_cast<unsigned long long>(price * 100) != 0)
      {
         return failTradeParse(tm, ePS_BadPrice);
      }
      if (static_cast<unsigned long long>(price * 100) == 0)
         return failTradeParse(tm, ePS_BadPrice);

      tm.trade_price_ = static_cast<unsigned long long>(price * 100);
      if (tm.trade_price_ > MAXPRICE - 1)
      {
         return failTradeParse(tm, ePS_BadPrice);
      }

      FeedErrorStats::instance()->goodMessage();
   }

}

#endif
