#pragma once

class object_reference {
  public:
    __inline object_reference();
    __inline virtual ~object_reference();

    __inline void Connect();
    __inline void Disconnect();

  protected:
    int count;
};

object_reference::object_reference() {
    count = 0;
}

object_reference::~object_reference() {

}

void object_reference::Connect() {
    count++;
}

void object_reference::Disconnect() {
    count--;

    if (count < 1) {
        delete this;
    }
}




