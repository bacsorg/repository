#include <bunsan/config.hpp>

#include <bacs/system/single/test/storage.hpp>

#include <bunsan/filesystem/fstream.hpp>

#include <boost/filesystem/operations.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <bunsan/serialization/unordered_set.hpp>

namespace bacs {
namespace system {
namespace single {
namespace test {

class copy_storage : public storage {
 public:
  copy_storage() {
    bunsan::filesystem::ifstream fin("etc/tests");
    BUNSAN_FILESYSTEM_FSTREAM_WRAP_BEGIN(fin) {
      boost::archive::text_iarchive ia(fin);
      ia >> m_test_set >> m_data_set;
    } BUNSAN_FILESYSTEM_FSTREAM_WRAP_END(fin)
    fin.close();
  }

  void copy(const std::string &test_id, const std::string &data_id,
            const boost::filesystem::path &path) override {
    boost::filesystem::copy_file(location(test_id, data_id), path);
  }

  boost::filesystem::path location(const std::string &test_id,
                                   const std::string &data_id) override {
    return boost::filesystem::path("share/tests") / (test_id + "." + data_id);
  }

  std::unordered_set<std::string> data_set() override { return m_data_set; }
  std::unordered_set<std::string> test_set() override { return m_test_set; }

  static storage_uptr make_instance() {
    return std::make_unique<copy_storage>();
  }

 private:
  std::unordered_set<std::string> m_test_set, m_data_set;
};

BUNSAN_PLUGIN_AUTO_REGISTER(storage, copy_storage, copy_storage::make_instance)

}  // namespace test
}  // namespace single
}  // namespace system
}  // namespace bacs
