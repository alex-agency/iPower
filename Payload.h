
#ifndef __PAYLOAD_H__
#define __PAYLOAD_H__

const uint8_t capacity = 5;

class Payload
{
public:
  void put(uint8_t key, int value) {
    if(contains(key)) {
      values[indexOf(key)] = value;
    } else
    if(next_empty_index < capacity) {
      keys[next_empty_index] = key;
      values[next_empty_index] = value;
    } else {
      next_empty_index = 0;
      keys[next_empty_index] = key;
      values[next_empty_index] = value;
    }
    next_empty_index++;
    
    //printf_P(PSTR("index %d put %s=%d\n\r"), next_empty_index, keys[next_empty_index-1], values[next_empty_index-1]);
  };
  
  int get(uint8_t key) {
    if(contains(key)) {
      return values[indexOf(key)];
    }
    return 0;
  };
  
  int indexOf(uint8_t key) {
    for(int index=0; index<capacity; index++) {
      if(keys[index] == key) {
        return index;
      }
    }
    return -1;
  };
  
  bool contains(uint8_t key) {
    return indexOf(key) != -1;
  };

  void clear() {
    for(int index=0; index<capacity; index++) {
      keys[index]=0;
      values[index]=0;
    }
  };

  void print() {
    printf_P(PSTR("{"));
    for(int index=0; index<capacity; index++) {
      printf_P(PSTR("%d=%d"), keys[index], values[index]);
      if(index<capacity-1)
          printf_P(PSTR(", "));
        else
          printf_P(PSTR("}"));
    }
  };
  
private:
  uint8_t keys[capacity];
  int values[capacity];
  uint8_t next_empty_index;
};

#endif // __PAYLOAD_H__
