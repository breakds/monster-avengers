#ifndef _MONSTER_AVENGERS_DATA_LOADER_
#define _MONSTER_AVENGERS_DATA_LOADER_

#include <string>
#include <memory>
#include <vector>

#include "supp/helpers.h"
#include "lisp/lisp_object.h"
#include "lisp/reader.h"

#include "language_text.h"
#include "effect.h"
#include "skill.h"
#include "jewel.h"
#include "item.h"
#include "armor.h"

namespace monster_avengers {

  class LoaderCore {
  public:
    virtual std::vector<SkillSystem> LoadSkillSystems() const = 0;
    virtual std::vector<Jewel> LoadJewels() const = 0;
    virtual std::vector<Item> LoadItems() const = 0;
    virtual std::vector<Armor> LoadArmors() const = 0;
  };


  // ---------- Standard Loader Core ----------
  class StandardLoaderCore : public LoaderCore {
  public:
    StandardLoaderCore(const std::string &data_folder) 
      : data_folder_(data_folder) {}

    virtual std::vector<SkillSystem> LoadSkillSystems() const override {
      return lisp::ReadList<SkillSystem>(data_folder_ + "/skills.lisp");
    }

    virtual std::vector<Jewel> LoadJewels() const override {
      return lisp::ReadList<Jewel>(data_folder_ + "/jewels.lisp");
    }

    virtual std::vector<Item> LoadItems() const override {
      return lisp::ReadList<Item>(data_folder_ + "/items.lisp");
    }

    virtual std::vector<Armor> LoadArmors() const override {
      return lisp::ReadList<Armor>(data_folder_ + "/armors.lisp");
    }

  private:
    const std::string data_folder_;
  };
  
  
  // ---------- Interface: DataLoader ----------
  class DataLoader {
  public:
    DataLoader() : core_(nullptr) {}
    
    void Initialize(const std::string &descriptor) {
      std::string format;
      std::string path;
      CHECK(ParseDescriptor(descriptor, &format, &path));
      if ("standard" == format) {
        core_.reset(new StandardLoaderCore(path));
      } else {
        CHECK(false);
      }
    }

    const LoaderCore *const Get() {
      return core_.get();
    }

  private:
    // Separate a path descriptor into two parts, the prefix
    // (format) and the postfix (the actual path). The delimiter is
    // ':'.
    static bool ParseDescriptor(const std::string &descriptor, 
                                std::string *prefix,
                                std::string *postfix) {
      size_t colon_index = descriptor.find_first_of(':');
      if (colon_index == std::string::npos) {
        // Fallback to standard case when we cannot find format
        // specifier in descriptor.
        *prefix = "standard";
        *postfix = descriptor;
        return true;
      }
      *prefix = descriptor.substr(0, colon_index);
      *postfix = descriptor.substr(colon_index + 1);
      return true;
    }

    std::unique_ptr<LoaderCore> core_;
  };
  
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_DATA_LOADER_
