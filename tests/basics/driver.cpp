#include <crails/databases.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <any>
#include <string>

#undef NODEBUG
#include <cassert>

using namespace std;
using namespace Crails;

struct TestDatabase : public Databases::Database
{
  static std::string ClassType() { return "test"; }
  static inline std::atomic<int> construction_count{0};

  std::string value;
  int         connect_count = 0;

  TestDatabase(const Databases::DatabaseSettings& settings) : Database(ClassType())
  {
    construction_count++;
    auto it = settings.find("value");
    value = it != settings.end() ? std::any_cast<std::string>(it->second) : std::string();
  }

  void connect() override { connect_count++; }
};

struct OtherTestDatabase : public Databases::Database
{
  static std::string ClassType() { return "other"; }
  OtherTestDatabase(const Databases::DatabaseSettings&) : Database(ClassType()) {}
  void connect() override {}
};

int main()
{
  // BEGIN get_database: creates on first access, reuses the same instance afterward
  {
    Databases                   databases;
    Databases::DatabaseSettings settings = { { "value", std::string("hello") } };

    TestDatabase::construction_count = 0;
    TestDatabase& first  = databases.get_database<TestDatabase>("main", settings);
    TestDatabase& second = databases.get_database<TestDatabase>("main", settings);

    assert(&first == &second);           // same instance, not recreated
    assert(first.value == "hello");
    assert(first.connect_count == 2);    // connect() runs on every access, not just creation
    assert(TestDatabase::construction_count == 1);
  }

  // BEGIN get_database_from_name: nullptr when absent, found once registered
  {
    Databases                   databases;
    Databases::DatabaseSettings settings = { { "value", std::string("x") } };

    assert(databases.get_database_from_name("missing") == nullptr);
    databases.get_database<TestDatabase>("present", settings);
    assert(databases.get_database_from_name("present") != nullptr);
  }

  // BEGIN registering a different TYPE under an already-used name throws
  {
    Databases                   databases;
    Databases::DatabaseSettings settings = { { "value", std::string("x") } };
    bool                        threw = false;

    databases.get_database<TestDatabase>("shared_key", settings);
    try
    {
      databases.get_database<OtherTestDatabase>("shared_key", settings);
    }
    catch (const Databases::Exception&) { threw = true; }
    assert(threw);
  }

  // BEGIN cleanup_database removes and deletes a specific database
  {
    Databases                   databases;
    Databases::DatabaseSettings settings = { { "value", std::string("x") } };
    Databases::Database&        db = databases.get_database<TestDatabase>("to_remove", settings);

    databases.cleanup_database(db); // db is dangling from here on, don't touch it
    assert(databases.get_database_from_name("to_remove") == nullptr);

    bool         threw = false;
    TestDatabase orphan(settings); // never registered with `databases`

    try { databases.cleanup_database(orphan); }
    catch (const boost_ext::out_of_range&) { threw = true; }
    assert(threw);
  }

  // BEGIN cleanup_databases clears everything at once
  {
    Databases                   databases;
    Databases::DatabaseSettings settings = { { "value", std::string("x") } };

    databases.get_database<TestDatabase>("one", settings);
    databases.get_database<TestDatabase>("two", settings);
    databases.cleanup_databases();
    assert(databases.get_database_from_name("one") == nullptr);
    assert(databases.get_database_from_name("two") == nullptr);
  }

  // BEGIN concurrent first-access does not create two instances
  {
    Databases              databases;
    Databases::DatabaseSettings settings = { { "value", std::string("race") } };
    const int               thread_count = 16;
    vector<TestDatabase*>   seen(thread_count, nullptr);
    vector<thread>          threads;

    TestDatabase::construction_count = 0;
    for (int i = 0 ; i < thread_count ; ++i)
    {
      threads.emplace_back([&, i]()
      {
        seen[i] = &databases.get_database<TestDatabase>("racy", settings);
      });
    }
    for (auto& t : threads)
      t.join();

    assert(TestDatabase::construction_count == 1);
    for (int i = 1 ; i < thread_count ; ++i)
      assert(seen[i] == seen[0]);
  }

  return 0;
}
