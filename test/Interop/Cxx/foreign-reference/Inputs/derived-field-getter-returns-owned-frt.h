#pragma once

#define FRT                                       \
  __attribute__((language_attr("import_reference"))) \
  __attribute__((language_attr("retain:immortal")))  \
  __attribute__((language_attr("release:immortal")))

int &getLiveRefCountedCounter() {
    static int counter = 0;
    return counter;
}

class RefCounted {
public:
    RefCounted() { getLiveRefCountedCounter()++; }
    ~RefCounted() {
        getLiveRefCountedCounter()--;
    }

    void retain() {
        ++refCount;
    }
    void release() {
        --refCount;
        if (refCount == 0)
            delete this;
    }

    int testVal = 1;
private:
    int refCount = 1;
}   __attribute__((language_attr("import_reference")))
__attribute__((language_attr("retain:retainRefCounted")))
__attribute__((language_attr("release:releaseRefCounted")));

RefCounted * _Nonnull createRefCounted() {
    return new RefCounted;
}

void retainRefCounted(RefCounted *r) {
    if (r)
        r->retain();
}
void releaseRefCounted(RefCounted *r) {
    if (r)
        r->release();
}

class BaseFieldFRT {
public:
    BaseFieldFRT(): value(new RefCounted) {}
    BaseFieldFRT(const BaseFieldFRT &other): value(other.value) {
        value->retain();
    }
    ~BaseFieldFRT() {
        value->release();
    }

    RefCounted * _Nonnull value;
};

class DerivedFieldFRT : public BaseFieldFRT {
};

class NonEmptyBase {
public:
    int getY() const {
        return y;
    }
private:
    int y = 11;
};

class DerivedDerivedFieldFRT : public NonEmptyBase, public DerivedFieldFRT {
};
