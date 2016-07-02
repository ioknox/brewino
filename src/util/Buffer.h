#ifndef __BUFFER_H__
#define __BUFFER_H__

template <typename TValue>
class Buffer
{
  public:
    Buffer(int length);
    virtual void set(TValue value);
    virtual TValue avg();

  private:
    TValue *_content;
    int _length;
    int _count;
    int _index;
};

template <typename TValue>
Buffer<TValue>::Buffer(int length)
{
  _count = 0;
  _index = 0;
  _length = length;
  _content = new TValue[_length];
}

template <typename TValue>
void Buffer<TValue>::set(TValue value)
{
  _content[_index] = value;
  _index = (_index + 1) % _length;

  if (_count < _length)
  {
    _count++;
  }
}

template <typename TValue>
TValue Buffer<TValue>::avg()
{
  TValue sum;

  if (_count <= 0)
  {
    return sum;
  }
  
  for (int i = 0; i < _count; i++)
  {
    int realIndex = _index - i;
    if (realIndex < 0)
    {
      realIndex = _count + realIndex;
    }
    else
    {
      realIndex = realIndex % 10;
    }

    sum += _content[realIndex];
  }

  return sum / _count;
}

#endif
