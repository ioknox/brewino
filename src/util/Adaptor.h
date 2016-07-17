#ifndef __ADAPTOR_H__
#define __ADAPTOR_H__

template <typename TClass, void (TClass::*mf)(), TClass *ptr>
struct Adaptor
{
  static void bind()
  {
    (ptr->*mf)();
  }
};

#endif
