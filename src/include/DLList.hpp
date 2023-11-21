#pragma once

#ifndef __DLLIST__
#define __DLLIST__

#include <stdio.h>

namespace zeus_core
{
   template <typename NODE>
   class DLList
   {
   public:
      DLList() : head_(0), tail_(0) {}

      virtual ~DLList() {}

      void addNode(NODE *input)
      {
         if (head_ == 0)
         {
            head_ = input;
            tail_ = head_;
            return;
         }

         head_->next_ = input;
         input->previous_ = head_;
         head_ = input;
      }

      void removeNode(NODE *target)
      {
         if (target->previous_ != 0 && target->next_ != 0)
         {
            target->previous_->next_ = target->next_;
            target->next_->previous_ = target->previous_;
         }
         else if (target == tail_ && target->next_ != 0)
         {
            tail_ = target->next_;
            tail_->previous_ = 0;
         }
         else if (target == head_ && target->previous_ != 0)
         {
            head_ = target->previous_;
            head_->next_ = 0;
         }
         else
         {
            head_ = 0;
            tail_ = 0;
            return;
         }
      }

      NODE *getHead() const { return head_; }
      NODE *getTail() const { return tail_; }

   private:
      NODE *head_;
      NODE *tail_;
   };

}

#endif
