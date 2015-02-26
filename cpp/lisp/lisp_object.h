#ifndef _MONSTER_AVENGERS_LISP_OBJ_
#define _MONSTER_AVENGERS_LISP_OBJ_

#include <vector>
#include <unordered_map>

namespace monster_avengers {

  struct LispObject {
    typedef std::unique_ptr<void, std::function<void(void*)>> DataHolder;
    typedef std::unordered_map<std::string, LispObject> ObjectHolder;

    enum LispObjectType {
      LISP_NUM = 0,
      LISP_STR = 1,
      LISP_OBJ = 2,
      LISP_LIST = 3,
    };
    LispObjectType type;
    DataHolder data;

  LispObject() : data(nullptr), type(LISP_OBJ) {}

    LispObject(LispObject &&other) {
      type = other.type;
      data = std::move(other.data);
    }

    LispObject(int x) :
      type(LISP_NUM), data(new int(x), [](void * content) {
	  delete static_cast<int*>(content);}) {}

    LispObject(const std::wstring &x) :
      type(LISP_STR), data(new std::wstring(x), [](void * content) {
	  delete static_cast<std::wstring*>(content);}) {}
    
    const LispObject &operator=(LispObject &&other) {
      type = other.type;
      data = std::move(other.data);
      return (*this);
    }
    
    static LispObject Number(int x = 0) {
      return LispObject(x);
    }

    static LispObject String(const std::wstring &x) {
      return LispObject(x);
    }

    static LispObject Object() {
      LispObject result;
      result.type = LISP_OBJ;
      result.data = DataHolder(new ObjectHolder(), [](void * content) {
	  delete static_cast<ObjectHolder*>(content);});
      return result;
    }

    static LispObject List() {
      LispObject result;
      result.type = LISP_LIST;
      result.data = DataHolder(new std::vector<LispObject>(), [](void * content) {
	  delete static_cast<std::vector<LispObject>*>(content);});
      return result;
    }

    void Set(const std::string name, LispObject &&value) {
      CHECK(LISP_OBJ == this->type);
      (*static_cast<ObjectHolder*>(data.get()))[name] = std::move(value);
    }

    void Push(LispObject &&value) {
      CHECK(LISP_LIST == this->type);
      static_cast<std::vector<LispObject>*>(data.get())->emplace_back(std::move(value));
    }

    LispObject &operator[](const std::string &name) {
      CHECK(LISP_OBJ == this->type);
      return (*static_cast<ObjectHolder*>(data.get()))[name];
    }

    LispObject &operator[](int id) {
      CHECK(LISP_LIST == this->type);
      return (*static_cast<std::vector<LispObject>*>(data.get()))[id];
    }

    int Size() const {
      return static_cast<std::vector<LispObject>*>(data.get())->size();
    }

    ObjectHolder &GetMap() {
      return (*static_cast<ObjectHolder*>(data.get()));
    }
  };

  std::wostream &operator<<(std::wostream &out, LispObject &lisp_object) {
    
    switch (lisp_object.type) {
    case LispObject::LISP_NUM: 
      out << *static_cast<int*>(lisp_object.data.get()); 
      break;
    case LispObject::LISP_STR: 
      out << "\"" << *static_cast<std::wstring*>(lisp_object.data.get())
	  << "\"";
      break;
    case LispObject::LISP_LIST:
      out << "(";
      for (int i = 0; i < lisp_object.Size(); ++i) {
	if (i > 0) out << " ";
	out << lisp_object[i];
      }
      out << ")";
      break;
    case LispObject::LISP_OBJ:
      out << "(";
      int i = 0;
      for (auto &item : lisp_object.GetMap()) {
	std::wstring name;
	name.assign(item.first.begin(), item.first.end());
	if (i > 0) out << " ";
	out << ":" << name
	    << " " << item.second;
	i++;
      }
      out << ")";
      break;
    }
    return out;
  }
}

#endif  // _MONSTER_AVENGERS_LISP_OBJ_
