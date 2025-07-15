struct MyStruct {
  int value;
};

void printvalue_mystruct(const struct MyStruct *m)
    __attribute__((language_name("MyStruct.printValue(self:)")));
