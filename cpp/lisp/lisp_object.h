#ifndef _MONSTER_AVENGERS_LISP_OBJ_
#define _MONSTER_AVENGERS_LISP_OBJ_

#include <string>
#include <locale>
#include <cwchar>
#include <iostream>
#include <sstream>
#include <iosfwd>
#include <fstream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <initializer_list>
#include <functional>

#include "supp/helpers.h"

namespace monster_avengers {

  namespace lisp {

    namespace {
      std::string CoerceUTF8ToAscii(const std::wstring input) {
        std::string result;
        result.reserve(input.size());
        for (const wchar_t wide_char : input) {
          result += static_cast<char>(wide_char);
        }
        return result;
      }
    }  // namespace

    struct Object;

    std::wostream &operator<<(std::wostream &out, const Object &lisp_object);

    struct Formattable {
    public:
      virtual Object Format() const = 0;
    };

    struct Object {
      typedef std::unique_ptr<void, std::function<void(void*)>> DataHolder;
      typedef std::unordered_map<std::string, Object> ObjectHolder;

      enum ObjectType {
        LISP_NUM = 0,
        LISP_STR = 1,
        LISP_OBJ = 2,
        LISP_LIST = 3,
        LISP_NIL = 4
      };
      ObjectType type;
      DataHolder data;

      Object() : data(nullptr), type(LISP_OBJ) {}
      
#if _WIN32
      Object(Object &&other) {
	type = other.type;
	data = std::move(other.data);
      }
#else
      Object(Object &&other) = default;
#endif
      
      Object(const Formattable &formattable) 
        : Object(std::move(formattable.Format())) {}
      
      Object(int x) :
        type(LISP_NUM), data(new int(x), [](void * content) {
            delete static_cast<int*>(content);}) {}

      Object(const std::wstring &x) :
        type(LISP_STR), data(new std::wstring(x), [](void * content) {
            delete static_cast<std::wstring*>(content);}) {}

      template <typename FormattableType>
      Object(const std::vector<FormattableType> &other) :
        type(LISP_LIST) {
        std::vector<Object> *list = new std::vector<Object>();
        for (const FormattableType &item : other) {
          list->emplace_back(item);
        }
        data = DataHolder(list, [](void * content) {
              delete static_cast<std::vector<Object>*>(content);
          });
      }
      
      Object(std::vector<Object> &&other) :
        type(LISP_LIST), 
        data(new std::vector<Object>(std::move(other)),
             [](void * content) {
               delete static_cast<std::vector<Object>*>(content);
             }) {}

      const Object &operator=(Object &&other) {
        type = other.type;
        data = std::move(other.data);
        return (*this);
      }

      static Object Number(int x = 0) {
        return Object(x);
      }

      static Object String(const std::wstring &x) {
        return Object(x);
      }

      static Object Struct() {
        Object result;
        result.type = LISP_OBJ;
        result.data = DataHolder(new ObjectHolder(), [](void * content) {
            delete static_cast<ObjectHolder*>(content);});
        return result;
      }

      static Object List() {
        Object result;
        result.type = LISP_LIST;
        result.data = DataHolder(new std::vector<Object>(), [](void * content) {
            delete static_cast<std::vector<Object>*>(content);});
        return result;
      }

      static Object Nil() {
        Object result;
        result.type = LISP_NIL;
        return result;
      }

      bool IsNil() {
        return LISP_NIL == type;
      }

      void Set(const std::string name, Object &&value) {
        CHECK(LISP_OBJ == this->type);
        (*static_cast<ObjectHolder*>(data.get()))[name] = std::move(value);
      }

      void Push(Object &&value) {
        CHECK(LISP_LIST == this->type);
        static_cast<std::vector<Object>*>(data.get())->emplace_back(std::move(value));
      }

      
      inline void TypeCheck(ObjectType expected) const {
        if (type != expected) {
          Log(WARNING, L"%d expected but %d is given.",
              expected,
              type);
        }
      }
      
      void AssignSlotTo(const std::string &slot,
                        int *output,
                        int default_value = 0) const {
        TypeCheck(LISP_OBJ);
        const ObjectHolder &children = GetMap();
        auto it = children.find(slot);
        if (children.end() == it) {
          *output = default_value;
        } else {
          it->second.TypeCheck(LISP_NUM);
          *output = *static_cast<int*>(it->second.data.get());
        }
      }

      void AssignSlotTo(const std::string &slot,
                        std::wstring *output,
                        std::wstring default_value = L"") const {
        TypeCheck(LISP_OBJ);
        const ObjectHolder &children = GetMap();
        auto it = children.find(slot);
        if (children.end() == it) {
          *output = default_value;
        } else {
          it->second.TypeCheck(LISP_STR);
          *output = *static_cast<std::wstring*>(it->second.data.get());
        }
      }

      template <typename TargetType>
      void AssignSlotTo(const std::string &slot,
                        TargetType *output) const {
        TypeCheck(LISP_OBJ);
        const ObjectHolder &children = GetMap();
        auto it = children.find(slot);
        if (children.end() == it) {
          Log(FATAL, L"No slot \"%s\" found in the following object:", 
              slot.c_str());
          std::wcout << (*this) << L"\n";
          exit(-1);
        } else {
          it->second.TypeCheck(LISP_OBJ);
          *output = TargetType(it->second);
        }
      }

      template <typename ElementType>
      void AppendSlotTo(const std::string &slot,
                        std::vector<ElementType> *output) const {
        TypeCheck(LISP_OBJ);
        const ObjectHolder &children = GetMap();
        auto it = children.find(slot);
        if (children.end() != it) {
          for (int i = 0; i < it->second.Size(); ++i) {
            output->emplace_back(it->second[i]);
          }
        }
      }

      void AppendSlotTo(const std::string &slot,
                        std::vector<int> *output) const {
        TypeCheck(LISP_OBJ);
        const ObjectHolder &children = GetMap();
        auto it = children.find(slot);
        if (children.end() != it) {
          for (int i = 0; i < it->second.Size(); ++i) {
            output->emplace_back(*static_cast<int*>(it->second[i].data.get()));
          }
        }
      }
      
      Object &operator[](const std::string &name) {
        CHECK(LISP_OBJ == this->type);
        return (*static_cast<ObjectHolder*>(data.get()))[name];
      }

      const Object &operator[](const std::string &name) const {
        CHECK(LISP_OBJ == this->type);
        return (*static_cast<ObjectHolder*>(data.get()))[name];
      }

      Object &operator[](int id) {
        CHECK(LISP_LIST == this->type);
        return (*static_cast<std::vector<Object>*>(data.get()))[id];
      }

      const Object &operator[](int id) const {
        CHECK(LISP_LIST == this->type);
        return (*static_cast<std::vector<Object>*>(data.get()))[id];
      }

      int Size() const {
        return static_cast<std::vector<Object>*>(data.get())->size();
      }

      ObjectHolder &GetMap() {
        return (*static_cast<ObjectHolder*>(data.get()));
      }

      const ObjectHolder &GetMap() const {
        return (*static_cast<ObjectHolder*>(data.get()));
      }

      void OutputJson(std::wostream *out) {
        switch (type) {
        case Object::LISP_NIL:
          (*out) << "{}";
          break;
        case Object::LISP_NUM: 
          (*out) << std::to_wstring(*static_cast<int*>(data.get())); 
          break;
        case Object::LISP_STR: 
          (*out) << "\"" << *static_cast<std::wstring*>(data.get())
              << "\"";
          break;
        case Object::LISP_LIST:
          (*out) << "[";
          for (int i = 0; i < Size(); ++i) {
            if (i > 0) (*out) << ", ";
            (*this)[i].OutputJson(out);
          }
          (*out) << "]";
          break;
        case Object::LISP_OBJ:
          (*out) << "{";
          int i = 0;
          for (auto &item : GetMap()) {
            std::wstring name;
            name.assign(item.first.begin(), item.first.end());
            if (i > 0) (*out) << ", ";
            (*out) << "\"" << name << "\": ";
            item.second.OutputJson(out);
            i++;
          }
          (*out) << "}";
          break;
        }
      }
    };

    std::wostream &operator<<(std::wostream &out, const Object &lisp_object) {
    
      switch (lisp_object.type) {
      case Object::LISP_NIL:
        out << "NIL";
        break;
      case Object::LISP_NUM: 
        out << *static_cast<int*>(lisp_object.data.get()); 
        break;
      case Object::LISP_STR: 
        out << "\"" << *static_cast<std::wstring*>(lisp_object.data.get())
            << "\"";
        break;
      case Object::LISP_LIST:
        out << "(";
        for (int i = 0; i < lisp_object.Size(); ++i) {
          if (i > 0) out << " ";
          out << lisp_object[i];
        }
        out << ")";
        break;
      case Object::LISP_OBJ:
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
    
    template <typename EnumType>
    class EnumConverter {
    public:
      typedef std::unordered_map<std::wstring, EnumType> TableType;
      EnumConverter(TableType table, 
                    EnumType default_value) 
        : table_(std::move(table)), 
          default_(new EnumType(default_value)) {}

      EnumConverter(TableType table) 
        : table_(std::move(table)),
          default_(nullptr) {}

      EnumType operator()(const Object &object, const std::string slot) {
        std::wstring buffer;
        object.AssignSlotTo(slot, &buffer);
        auto it = table_.find(buffer);
        if (table_.end() == it) {
          if (default_) {
            return *default_;
          } else {
            Log(FATAL, L"\"%ls\" does not translate to a known enum value.",
                buffer.c_str());
            exit(-1);
          }
        } else {
          return it->second;
        }
      }

    private:
      TableType table_;
      std::unique_ptr<EnumType> default_;
    };

    template <typename FormattableObject>
    std::vector<Object> FormatList(const std::vector<FormattableObject> &input) {
      std::vector<Object> output;
      for (const FormattableObject &item : input) {
        output.emplace_back(item);
      }
      return output;
    }
  }  // namespace lisp

}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_LISP_OBJ_
